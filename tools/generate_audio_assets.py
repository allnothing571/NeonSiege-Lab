"""Neon Siege original audio asset generator.

Every runtime sound effect in assets/audio/ is synthesized here from scratch by
this repository, so the project owns the copyright of the generated WAV files
and there is no third-party licence to satisfy for them.

Design rules enforced by this script (matching
docs/commercial_audio_upgrade_manual.md section 4 and the acceptance prompt):

* PCM, 16-bit, 44.1 kHz, stereo for every runtime sound.
* No audible silence at the head: the first sample with content starts within a
  handful of frames of frame 0.
* No hard truncation at the tail: every sound ends with an explicit fade and
  stays below the noise floor before the file ends.
* No clipping and a consistent headroom target, so a layer stack never pins the
  converter.
* Player shot, enemy shot, projectile hit, player damaged and enemy died are
  built from deliberately different pitch ranges, spectra and envelopes so a
  listener can tell them apart with sound alone.

The script only uses the Python standard library (math, struct, wave, random,
array, cmath) and is deterministic: the same interpreter produces byte-for-byte
identical files.

Usage:
    python tools/generate_audio_assets.py             # write every file
    python tools/generate_audio_assets.py --check     # compare with disk
    python tools/generate_audio_assets.py --only select
"""

import argparse
import math
import os
import struct
import sys
import wave

SAMPLE_RATE = 44100
CHANNELS = 2
BITS_PER_SAMPLE = 16
FULL_SCALE = 32767.0

# Per-sound headroom in dBFS applied after the layer sum is normalized. Two
# decibels of reserve keeps the peak near 80% full scale, which leaves room for
# several simultaneous instances in the SDL_mixer channel mix without clipping
# at the device.
HEADROOM_DB = -2.0
TAIL_FADE_SECONDS = 0.012
# Frames of literal leading silence. Zero means content starts on frame 0.
HEAD_SILENCE_FRAMES = 0


# --------------------------------------------------------------------------
# Deterministic pseudo random generator
# --------------------------------------------------------------------------

class Rng:
    """Small deterministic PRNG so generated audio never shifts between runs."""

    def __init__(self, seed):
        self._state = (seed ^ 0x9E3779B97F4A7C15) & 0xFFFFFFFFFFFFFFFF

    def next_u64(self):
        self._state = (self._state * 6364136223846793005 +
                       1442695040888963407) & 0xFFFFFFFFFFFFFFFF
        return self._state

    def uniform(self, low=-1.0, high=1.0):
        unit = (self.next_u64() >> 11) / float(1 << 53)
        return low + (high - low) * unit

    def choice(self, items):
        return items[self.next_u64() % len(items)]


# --------------------------------------------------------------------------
# Filters
# --------------------------------------------------------------------------

def one_pole_lowpass(samples, cutoff_hz):
    """Single pole low pass; used to darken noise layers."""
    if cutoff_hz >= SAMPLE_RATE * 0.5:
        return list(samples)
    alpha = 1.0 - math.exp(-2.0 * math.pi * cutoff_hz / SAMPLE_RATE)
    out = [0.0] * len(samples)
    state = 0.0
    for index, value in enumerate(samples):
        state += alpha * (value - state)
        out[index] = state
    return out


def one_pole_highpass(samples, cutoff_hz):
    """Single pole high pass; used to remove rumble and add brightness."""
    if cutoff_hz <= 0.0:
        return list(samples)
    alpha = math.exp(-2.0 * math.pi * cutoff_hz / SAMPLE_RATE)
    out = [0.0] * len(samples)
    previous_input = 0.0
    previous_output = 0.0
    for index, value in enumerate(samples):
        current = alpha * (previous_output + value - previous_input)
        out[index] = current
        previous_input = value
        previous_output = current
    return out


def bandpass(samples, low_hz, high_hz):
    return one_pole_highpass(one_pole_lowpass(samples, high_hz), low_hz)


# --------------------------------------------------------------------------
# Envelopes
# --------------------------------------------------------------------------

def percussive(count, attack_seconds=0.001, decay_seconds=None,
               curve=3.0, total_seconds=None):
    """Fast attack then exponential decay, normalized to a peak of 1."""
    attack_frames = max(1, int(attack_seconds * SAMPLE_RATE))
    if decay_seconds is None:
        decay_seconds = (total_seconds if total_seconds is not None
                         else count / float(SAMPLE_RATE))
    decay_frames = max(1, int(decay_seconds * SAMPLE_RATE))
    out = [0.0] * count
    for index in range(count):
        if index < attack_frames:
            gain = index / float(attack_frames)
        else:
            position = (index - attack_frames) / float(decay_frames)
            gain = math.exp(-curve * position)
        out[index] = gain
    return out


def adsr(count, attack, decay, sustain_level, release):
    """Classic four stage envelope in absolute seconds."""
    attack_frames = max(1, int(attack * SAMPLE_RATE))
    decay_frames = max(0, int(decay * SAMPLE_RATE))
    release_frames = max(1, int(release * SAMPLE_RATE))
    sustain_frames = max(0, count - attack_frames - decay_frames -
                         release_frames)
    out = []
    for index in range(attack_frames):
        out.append(index / float(attack_frames))
    for index in range(decay_frames):
        position = index / float(max(1, decay_frames))
        out.append(1.0 + (sustain_level - 1.0) * position)
    out.extend([sustain_level] * sustain_frames)
    for index in range(release_frames):
        position = index / float(release_frames)
        out.append(sustain_level * (1.0 - position))
    if len(out) < count:
        out.extend([0.0] * (count - len(out)))
    return out[:count]


def smoothstep_ramp(count, start, end, curve=1.0):
    """Smooth start/end ramp between two gains."""
    out = [0.0] * count
    for index in range(count):
        position = index / float(max(1, count - 1))
        shaped = position ** curve
        eased = shaped * shaped * (3.0 - 2.0 * shaped)
        out[index] = start + (end - start) * eased
    return out


def apply_fades(samples, fade_in_seconds=0.0, fade_out_seconds=TAIL_FADE_SECONDS):
    out = list(samples)
    fade_in = int(fade_in_seconds * SAMPLE_RATE)
    if fade_in > 1:
        for index in range(min(fade_in, len(out))):
            out[index] *= index / float(fade_in)
    fade_out = int(fade_out_seconds * SAMPLE_RATE)
    if fade_out > 1:
        for offset in range(min(fade_out, len(out))):
            index = len(out) - 1 - offset
            out[index] *= offset / float(fade_out)
    return out


# --------------------------------------------------------------------------
# Oscillators and layers
# --------------------------------------------------------------------------

def sine_sweep(count, start_hz, end_hz, phase=0.0, exponential=False,
               phase_mod=None):
    """Frequency swept sine. `phase_mod` adds a per-sample phase offset in
    radians (used for a little vibrato or drift)."""
    out = [0.0] * count
    accumulator = phase
    for index in range(count):
        if exponential and start_hz > 0.0 and end_hz > 0.0:
            ratio = end_hz / start_hz
            frequency = start_hz * (ratio ** (index / float(max(1, count - 1))))
        else:
            position = index / float(max(1, count - 1))
            frequency = start_hz + (end_hz - start_hz) * position
        accumulator += 2.0 * math.pi * frequency / SAMPLE_RATE
        extra = phase_mod(index) if phase_mod is not None else 0.0
        out[index] = math.sin(accumulator + extra)
    return out


def saw_sweep(count, start_hz, end_hz, exponential=True):
    out = [0.0] * count
    accumulator = 0.0
    for index in range(count):
        if exponential and start_hz > 0.0 and end_hz > 0.0:
            ratio = end_hz / start_hz
            frequency = start_hz * (ratio ** (index / float(max(1, count - 1))))
        else:
            position = index / float(max(1, count - 1))
            frequency = start_hz + (end_hz - start_hz) * position
        accumulator += frequency / SAMPLE_RATE
        accumulator -= math.floor(accumulator)
        out[index] = 2.0 * accumulator - 1.0
    return out


def square_tone(count, frequency, duty=0.5):
    out = [0.0] * count
    accumulator = 0.0
    for index in range(count):
        accumulator += frequency / SAMPLE_RATE
        accumulator -= math.floor(accumulator)
        out[index] = 1.0 if accumulator < duty else -1.0
    return out


def triangle_sweep(count, start_hz, end_hz):
    out = [0.0] * count
    accumulator = 0.0
    for index in range(count):
        position = index / float(max(1, count - 1))
        frequency = start_hz + (end_hz - start_hz) * position
        accumulator += frequency / SAMPLE_RATE
        accumulator -= math.floor(accumulator)
        out[index] = 4.0 * abs(accumulator - 0.5) - 1.0
    return out


def noise(count, rng, low_hz=None, high_hz=None):
    raw = [rng.uniform(-1.0, 1.0) for _ in range(count)]
    if low_hz is not None and high_hz is not None:
        return bandpass(raw, low_hz, high_hz)
    if low_hz is not None:
        return one_pole_highpass(raw, low_hz)
    if high_hz is not None:
        return one_pole_lowpass(raw, high_hz)
    return raw


def mix(*layers):
    """Sum equal-length layers element-wise."""
    if not layers:
        return []
    count = len(layers[0])
    out = [0.0] * count
    for layer in layers:
        for index in range(min(count, len(layer))):
            out[index] += layer[index]
    return out


def gain(samples, amount):
    return [value * amount for value in samples]


def shaped(samples, envelope):
    count = min(len(samples), len(envelope))
    return [samples[index] * envelope[index] for index in range(count)]


def delayed(samples, seconds):
    """Delay inside the same buffer: content shifts right and is truncated at
    the original length, so every layer stays sample-aligned."""
    shift = int(seconds * SAMPLE_RATE)
    out = [0.0] * len(samples)
    for position in range(len(samples) - shift):
        out[position + shift] = samples[position]
    return out


def nth_partial(samples, index, offset_seconds, amount):
    """Return a copy of `samples` delayed and scaled, or zeros when it would
    exceed the requested length."""
    shift = int(offset_seconds * SAMPLE_RATE)
    out = [0.0] * len(samples)
    for position in range(len(samples) - shift):
        out[position + shift] = samples[position] * amount
    return out


def soft_clip(samples, drive=1.0):
    """tanh saturation: adds harmonic bite without ever exceeding +/-1."""
    return [math.tanh(value * drive) for value in samples]


def normalize(samples, target=1.0):
    peak = max((abs(value) for value in samples), default=0.0)
    if peak <= 1e-9:
        return list(samples)
    scale = target / peak
    return [value * scale for value in samples]


# --------------------------------------------------------------------------
# Stereo image
# --------------------------------------------------------------------------

def stereo(samples, width=0.0, pan=0.0, haas_ms=0.0):
    """Build a stereo pair. `width` decorrelates the channels with a tiny
    delay and gain difference, `pan` shifts the image, `haas_ms` adds a short
    right-channel delay for the wide arcade feel."""
    width = max(-0.5, min(0.5, width))
    left_scale = math.sqrt(max(0.0, 0.5 - pan + 0.5) / 1.0) * (1.0 + width)
    right_scale = math.sqrt(max(0.0, 0.5 + pan + 0.5) / 1.0) * (1.0 - width)
    if pan == 0.0 and width == 0.0:
        left_scale = right_scale = 1.0

    left = [value * left_scale for value in samples]
    right = list(samples)
    if haas_ms > 0.0:
        shift = int(haas_ms * 0.001 * SAMPLE_RATE)
        right = [0.0] * shift + right
        right = right[:len(samples)]
    right = [value * right_scale for value in right]
    return left, right


# --------------------------------------------------------------------------
# Encoding
# --------------------------------------------------------------------------

def remove_dc(samples):
    if not samples:
        return samples
    mean = sum(samples) / float(len(samples))
    return [value - mean for value in samples]


def encode_payload(left, right):
    """Interleave, convert to signed 16-bit little endian PCM."""
    frames = min(len(left), len(right))
    payload = bytearray()
    for index in range(frames):
        left_sample = max(-1.0, min(1.0, left[index]))
        right_sample = max(-1.0, min(1.0, right[index]))
        payload += struct.pack(
            "<hh",
            int(round(left_sample * FULL_SCALE)),
            int(round(right_sample * FULL_SCALE)),
        )
    return bytes(payload)


def encode_wave_file(left, right):
    """Serialize a complete RIFF/WAVE file: 44-byte header plus payload."""
    payload = encode_payload(left, right)
    header = bytearray()
    header += b"RIFF"
    header += struct.pack("<I", 36 + len(payload))
    header += b"WAVE"
    header += b"fmt "
    header += struct.pack("<IHHIIHH", 16, 1, CHANNELS, SAMPLE_RATE,
                          SAMPLE_RATE * CHANNELS * BITS_PER_SAMPLE // 8,
                          CHANNELS * BITS_PER_SAMPLE // 8, BITS_PER_SAMPLE)
    header += b"data"
    header += struct.pack("<I", len(payload))
    return bytes(header) + payload


def write_wave(path, left, right):
    """Write the file byte-for-byte as the deterministic encoder produces it."""
    directory = os.path.dirname(os.path.abspath(path))
    if directory and not os.path.isdir(directory):
        os.makedirs(directory)
    data = encode_wave_file(left, right)
    with open(path, "wb") as handle:
        handle.write(data)
    return len(data)


def frames_for(seconds):
    return int(round(seconds * SAMPLE_RATE))


def pad_to(samples, count):
    if len(samples) >= count:
        return samples[:count]
    return list(samples) + [0.0] * (count - len(samples))


def db_to_linear(decibels):
    return 10.0 ** (decibels / 20.0)


def decay_tail(samples, curve=3.0):
    """Force a monotonic decaying envelope onto the final part of a sound so
    the tail can never be a hard cut and never contains a late transient."""
    count = len(samples)
    tail_frames = min(count, frames_for(0.35))
    out = list(samples)
    for offset in range(tail_frames):
        index = count - tail_frames + offset
        position = offset / float(max(1, tail_frames - 1))
        out[index] *= math.exp(-curve * position)
    return out


def finish(samples, seconds=None, target_db=HEADROOM_DB,
           tail_curve=3.0, fade_out=TAIL_FADE_SECONDS):
    """Normalize to the headroom target, decay the tail, and fade out."""
    out = remove_dc(list(samples))
    if seconds is not None:
        out = pad_to(out, frames_for(seconds))
    out = normalize(out, 1.0)
    out = decay_tail(out, curve=tail_curve)
    out = apply_fades(out, 0.0, fade_out)
    return gain(out, db_to_linear(target_db))


# --------------------------------------------------------------------------
# Sound designs
# --------------------------------------------------------------------------

SELECT_SECONDS = 0.12


def render_select():
    """Very short upward navigation tick. A bright two-part tone makes menu
    movement readable without sounding like confirmation or weapon fire."""
    count = frames_for(SELECT_SECONDS)
    tone_env = percussive(count, 0.0005, None, 8.5, SELECT_SECONDS)
    tone = shaped(
        sine_sweep(count, 820.0, 1260.0, exponential=True),
        tone_env,
    )
    harmonic = shaped(
        sine_sweep(count, 1640.0, 2520.0, phase=0.7, exponential=True),
        percussive(count, 0.0004, None, 11.0, SELECT_SECONDS),
    )
    tick = shaped(
        bandpass(noise(count, Rng(0x5E1EC7)), 2400.0, 7600.0),
        percussive(count, 0.0002, None, 20.0, SELECT_SECONDS),
    )
    layered = mix(
        gain(tone, 0.82),
        gain(harmonic, 0.22),
        gain(tick, 0.18),
    )
    return finish(layered, SELECT_SECONDS, target_db=-5.0, tail_curve=4.5)


CONFIRM_SECONDS = 0.26


def render_confirm():
    """Positive two-note confirmation. The second note is a clear perfect
    fifth above the first, with a soft mechanical click at the leading edge."""
    count = frames_for(CONFIRM_SECONDS)
    first_env = percussive(count, 0.001, None, 5.8, CONFIRM_SECONDS)
    first = shaped(
        mix(
            sine_sweep(count, 620.0, 650.0),
            gain(sine_sweep(count, 1240.0, 1300.0, phase=0.5), 0.24),
        ),
        first_env,
    )

    second = shaped(
        mix(
            sine_sweep(count, 930.0, 990.0),
            gain(sine_sweep(count, 1860.0, 1980.0, phase=1.1), 0.22),
        ),
        percussive(count, 0.001, None, 6.5, CONFIRM_SECONDS),
    )
    second = delayed(second, 0.065)

    click = shaped(
        bandpass(noise(count, Rng(0xC0F1A1)), 1200.0, 6200.0),
        percussive(count, 0.0003, None, 18.0, CONFIRM_SECONDS),
    )

    layered = mix(
        gain(first, 0.66),
        gain(second, 0.78),
        gain(click, 0.16),
    )
    return finish(layered, CONFIRM_SECONDS, target_db=-4.0, tail_curve=3.8)


BACK_SECONDS = 0.28


def render_back():
    """Cancel/backward feedback: a soft descending two-tone with a muted
    wooden body. Deliberately darker and lower than the confirm sound, with no
    bright transient, so 'going back' never sounds like 'accepting'."""
    count = frames_for(BACK_SECONDS)
    body_env = percussive(count, 0.0008, None, 4.2, BACK_SECONDS)

    tone = sine_sweep(count, 520.0, 330.0, exponential=True)
    detuned = sine_sweep(count, 508.0, 322.0, phase=1.1, exponential=True)
    body = gain(mix(tone, gain(detuned, 0.55)), 0.72)

    # A short closed-mouth thump under the tone keeps it from feeling thin.
    thump_env = percussive(count, 0.0006, None, 9.0, BACK_SECONDS)
    thump = shaped(
        one_pole_lowpass(noise(count, Rng(0xBAC1)), 320.0),
        thump_env,
    )

    # Second, quieter step a fifth down: the "descending" cue.
    second = delayed(gain(body, 0.45), 0.085)
    second = shaped(second, smoothstep_ramp(count, 0.0, 1.0))

    layered = mix(body, gain(thump, 0.35), second)
    return finish(layered, BACK_SECONDS, target_db=-3.5, tail_curve=2.6)


PLAYER_SHOT_SECONDS = 0.16


def render_player_shot():
    """Bright energy laser. Silenced across roughly 2.6 kHz to 7 kHz so it
    sits above the enemy shot, with a glassy ring for sci-fi character and a
    very tight decay so rapid fire never smears."""
    count = frames_for(PLAYER_SHOT_SECONDS)
    env = percussive(count, 0.0004, None, 7.0, PLAYER_SHOT_SECONDS)

    core = sine_sweep(count, 1650.0, 240.0, exponential=True)
    harmonic = sine_sweep(count, 3300.0, 520.0, phase=0.6, exponential=True)

    sizzle_env = percussive(count, 0.0003, None, 12.0, PLAYER_SHOT_SECONDS)
    sizzle = shaped(
        bandpass(noise(count, Rng(0x51D7)), 2600.0, 7000.0),
        sizzle_env,
    )

    # Metallic ring gives the shot a "designed" arcade colour.
    ring_env = percussive(count, 0.0005, None, 16.0, PLAYER_SHOT_SECONDS)
    ring = shaped(sine_sweep(count, 5400.0, 4300.0), ring_env)

    layered = mix(
        gain(core, 1.0),
        gain(harmonic, 0.42),
        gain(sizzle, 0.5),
        gain(ring, 0.16),
    )
    layered = soft_clip(layered, 1.05)
    return finish(layered, PLAYER_SHOT_SECONDS, target_db=-2.5, tail_curve=4.0)


ENEMY_SHOT_SECONDS = 0.30


def _falloff(count, hold_seconds, start_gain, end_gain):
    """Gain that holds, then falls off exponentially. Used to give a short
    sound a clear body plus a quiet tail instead of a flat sustain."""
    hold_frames = min(count, frames_for(hold_seconds))
    out = [start_gain] * count
    remaining = max(1, count - hold_frames)
    ratio = max(1e-4, end_gain / max(1e-6, start_gain))
    for index in range(remaining):
        position = index / float(remaining)
        out[hold_frames + index] = start_gain * (ratio ** position)
    return out


def render_enemy_shot():
    """Dark, heavier plasma lob. Energy sits between 180 Hz and 1.6 kHz, the
    pitch drop is slower and the waveform is detuned saw, so it reads as
    'incoming' next to the player's bright laser. Attack is soft, then it
    falls away quickly so a shooter volley never sustains into mud."""
    count = frames_for(ENEMY_SHOT_SECONDS)
    env = percussive(count, 0.0016, None, 3.4, ENEMY_SHOT_SECONDS)
    falloff = _falloff(count, 0.06, 1.0, 0.10)
    env = [env[index] * falloff[index] for index in range(count)]

    fundamental = saw_sweep(count, 470.0, 104.0)
    sub = sine_sweep(count, 190.0, 52.0, exponential=True)
    detuned = saw_sweep(count, 484.0, 110.0)

    growl_env = percussive(count, 0.0012, None, 5.0, ENEMY_SHOT_SECONDS)
    growl = shaped(
        bandpass(noise(count, Rng(0xE33A)), 220.0, 2100.0),
        growl_env,
    )

    layered = mix(
        gain(one_pole_lowpass(fundamental, 2400.0), 0.68),
        gain(one_pole_lowpass(detuned, 2400.0), 0.38),
        gain(sub, 0.30),
        gain(growl, 0.48),
    )
    layered = shaped(soft_clip(layered, 1.15), env)
    return finish(layered, ENEMY_SHOT_SECONDS, target_db=-3.0, tail_curve=3.2)


HIT_SECONDS = 0.15


def render_projectile_hit():
    """Short spark impact. Presents as a broadband crack with a bright
    metallic ping, but only 150 ms long, and the catalog cooldown of 35 ms
    keeps a dense volley from turning into a wall of noise."""
    count = frames_for(HIT_SECONDS)
    crack_env = percussive(count, 0.0003, None, 14.0, HIT_SECONDS)
    crack = shaped(
        bandpass(noise(count, Rng(0x81F3)), 1200.0, 9000.0),
        crack_env,
    )

    ping_env = percussive(count, 0.0004, None, 8.5, HIT_SECONDS)
    ping = shaped(sine_sweep(count, 2600.0, 1500.0, exponential=True), ping_env)
    ping2 = shaped(sine_sweep(count, 3900.0, 2300.0, phase=1.9,
                              exponential=True), ping_env)

    punch_env = percussive(count, 0.0005, None, 11.0, HIT_SECONDS)
    punch = shaped(sine_sweep(count, 260.0, 90.0, exponential=True), punch_env)

    layered = mix(
        gain(crack, 0.85),
        gain(ping, 0.35),
        gain(ping2, 0.2),
        gain(punch, 0.3),
    )
    return finish(layered, HIT_SECONDS, target_db=-2.5, tail_curve=4.5)


DAMAGED_SECONDS = 0.44


def render_player_damaged():
    """Alarm-like damage cue. Occupies 260 Hz to 2.4 kHz with a slow downward
    glide and a two-beat pulse, which keeps it readable under a stream of
    bright player shots and clearly distinct from the enemy death."""
    count = frames_for(DAMAGED_SECONDS)

    glide = sine_sweep(count, 640.0, 250.0, exponential=True)
    glide2 = sine_sweep(count, 1280.0, 500.0, phase=2.2, exponential=True)
    buzz = square_tone(count, 210.0, duty=0.32)

    # Two descending alarm beats: "you are taking damage".
    beat_env = [0.0] * count
    for beat_index, beat_start in enumerate((0.0, 0.13)):
        start = frames_for(beat_start)
        length = frames_for(0.20)
        amplitude = 1.0 if beat_index == 0 else 0.78
        local_env = percussive(
            min(length, count - start), 0.004, None, 5.5, 0.22)
        for offset, value in enumerate(local_env):
            index = start + offset
            if index < count:
                beat_env[index] = max(beat_env[index], value * amplitude)

    fault_env = percussive(count, 0.002, None, 6.5, DAMAGED_SECONDS)
    fault = shaped(
        bandpass(noise(count, Rng(0x2B77)), 300.0, 2400.0),
        fault_env,
    )

    layered = mix(
        shaped(glide, beat_env),
        shaped(gain(glide2, 0.3), beat_env),
        shaped(gain(one_pole_lowpass(buzz, 900.0), 0.22), beat_env),
        gain(fault, 0.4),
    )
    layered = soft_clip(layered, 1.2)
    return finish(layered, DAMAGED_SECONDS, target_db=-2.0, tail_curve=2.8)


ENEMY_DIED_SECONDS = 0.62


def render_enemy_died():
    """Mechanical collapse: a burst, a tumbling pitch drop, and metal
    debris partials that decay at different rates. Longer and noisier than the
    hit spark, with a low thud the player damage cue never has."""
    count = frames_for(ENEMY_DIED_SECONDS)

    burst_env = percussive(count, 0.0006, None, 8.5, ENEMY_DIED_SECONDS)
    burst = shaped(
        bandpass(noise(count, Rng(0x6D21)), 400.0, 6500.0),
        burst_env,
    )

    fall_env = percussive(count, 0.002, None, 4.0, ENEMY_DIED_SECONDS)
    fall = shaped(saw_sweep(count, 330.0, 44.0), fall_env)
    fall_sub = shaped(sine_sweep(count, 165.0, 36.0, exponential=True), fall_env)

    thud_env = percussive(count, 0.0008, None, 10.0, ENEMY_DIED_SECONDS)
    thud = shaped(sine_sweep(count, 120.0, 42.0, exponential=True), thud_env)

    # Inharmonic debris: five metal partials with staggered delays and decays.
    debris = [0.0] * count
    for index, (frequency, offset_seconds, amount) in enumerate((
        (1620.0, 0.015, 0.34),
        (2380.0, 0.032, 0.26),
        (3120.0, 0.048, 0.19),
        (4410.0, 0.070, 0.13),
        (5700.0, 0.095, 0.09),
    )):
        local = percussive(count, 0.0006, None, 9.0 + index, ENEMY_DIED_SECONDS)
        partial = shaped(
            sine_sweep(count, frequency, frequency * 0.86, exponential=True),
            local,
        )
        debris = mix(debris, nth_partial(partial, index, offset_seconds,
                                         amount))

    layered = mix(
        gain(burst, 0.7),
        gain(one_pole_lowpass(fall, 2200.0), 0.5),
        gain(fall_sub, 0.45),
        gain(thud, 0.42),
        debris,
    )
    layered = soft_clip(layered, 1.1)
    return finish(layered, ENEMY_DIED_SECONDS, target_db=-2.5, tail_curve=2.6)


RELOAD_START_SECONDS = 0.22


def render_reload_start():
    """Magazine release, short energy vent, and a descending mechanism tone.
    This marks the start of reloading without pretending the reload is done."""
    count = frames_for(RELOAD_START_SECONDS)

    release_env = percussive(count, 0.0003, None, 20.0,
                             RELOAD_START_SECONDS)
    release_click = shaped(
        bandpass(noise(count, Rng(0x51A7)), 900.0, 7600.0),
        release_env,
    )
    release_body = shaped(
        sine_sweep(count, 560.0, 240.0, exponential=True),
        percussive(count, 0.0005, None, 13.0, RELOAD_START_SECONDS),
    )

    vent_env = percussive(count, 0.003, None, 5.5,
                          RELOAD_START_SECONDS)
    vent = shaped(
        bandpass(noise(count, Rng(0xAE17)), 650.0, 4300.0),
        vent_env,
    )
    descending_energy = shaped(
        sine_sweep(count, 720.0, 190.0, exponential=True),
        percussive(count, 0.001, None, 5.0, RELOAD_START_SECONDS),
    )

    layered = mix(
        gain(release_click, 0.78),
        gain(release_body, 0.72),
        gain(vent, 0.22),
        gain(descending_energy, 0.32),
    )
    layered = soft_clip(layered, 1.08)
    return finish(
        layered,
        RELOAD_START_SECONDS,
        target_db=-3.8,
        tail_curve=3.2,
    )


RELOAD_COMPLETE_SECONDS = 0.16


def render_reload_complete():
    """A magazine seating thud followed by a bright bolt-lock confirmation."""
    count = frames_for(RELOAD_COMPLETE_SECONDS)
    layered = [0.0] * count

    def place(local, offset_seconds, loudness):
        start = frames_for(offset_seconds)
        for offset, value in enumerate(local):
            index = start + offset
            if index < count:
                layered[index] += value * loudness

    seat_length = frames_for(0.085)
    seat_env = percussive(seat_length, 0.0004, None, 17.0, 0.08)
    seat = mix(
        gain(shaped(
            bandpass(noise(seat_length, Rng(0x5EA7)), 240.0, 2600.0),
            seat_env,
        ), 0.72),
        gain(shaped(
            sine_sweep(seat_length, 230.0, 82.0, exponential=True),
            seat_env,
        ), 0.78),
    )
    place(seat, 0.0, 0.95)

    lock_length = frames_for(0.075)
    lock_env = percussive(lock_length, 0.0002, None, 22.0, 0.07)
    bolt_lock = mix(
        gain(shaped(
            bandpass(noise(lock_length, Rng(0xB017)), 1700.0, 9000.0),
            lock_env,
        ), 0.82),
        gain(shaped(
            sine_sweep(lock_length, 980.0, 620.0, exponential=True),
            lock_env,
        ), 0.58),
    )
    place(bolt_lock, 0.055, 0.88)

    ready_length = frames_for(0.052)
    ready = shaped(
        sine_sweep(ready_length, 1450.0, 2250.0, exponential=True),
        percussive(ready_length, 0.0003, None, 20.0, 0.05),
    )
    place(ready, 0.098, 0.32)

    layered = soft_clip(layered, 1.08)
    return finish(
        layered,
        RELOAD_COMPLETE_SECONDS,
        target_db=-3.2,
        tail_curve=3.5,
    )


WAVE_START_SECONDS = 1.05


def render_wave_start():
    """Wave announcement: a rising alarm sweep, a sub impact on the downbeat,
    and a stacked confirmation interval. One second long, high priority, meant
    to sit above combat fire."""
    count = frames_for(WAVE_START_SECONDS)
    layered = [0.0] * count

    def place(local, offset_seconds, loudness):
        start = frames_for(offset_seconds)
        for offset, value in enumerate(local):
            index = start + offset
            if index < count:
                layered[index] += value * loudness
        return layered

    # Rising alarm: triangle sweep with a slight tremolo for urgency.
    alarm_length = frames_for(0.72)
    alarm_env = adsr(alarm_length, 0.02, 0.10, 0.72, 0.30)
    tremolo = [
        0.82 + 0.18 * math.sin(2.0 * math.pi * 15.0 * index / SAMPLE_RATE)
        for index in range(alarm_length)
    ]
    alarm = shaped(mix(
        triangle_sweep(alarm_length, 330.0, 990.0),
        gain(sine_sweep(alarm_length, 660.0, 1980.0, phase=0.9), 0.4),
        gain(sine_sweep(alarm_length, 165.0, 495.0, phase=2.1), 0.3),
    ), [value * tremolo[index] for index, value in enumerate(alarm_env)])
    place(alarm, 0.02, 0.72)

    # Sub impact on the downbeat.
    impact_length = frames_for(0.26)
    impact = shaped(sine_sweep(impact_length, 190.0, 46.0, exponential=True),
                    percussive(impact_length, 0.001, None, 6.0, 0.24))
    place(impact, 0.0, 0.7)

    # Confirmation stack: two short square blips, root then fifth.
    for offset_seconds, frequency in ((0.46, 660.0), (0.58, 990.0)):
        blip_length = frames_for(0.24)
        blip = shaped(
            gain(one_pole_lowpass(square_tone(blip_length, frequency,
                                              duty=0.42), 4500.0), 0.45),
            percussive(blip_length, 0.002, None, 5.5, 0.22),
        )
        place(blip, offset_seconds, 0.42)

    # Air whoosh under the whole announcement for arcade lift.
    whoosh_env = adsr(count, 0.10, 0.20, 0.45, 0.45)
    whoosh = shaped(
        bandpass(noise(count, Rng(0x77A1)), 500.0, 5200.0),
        whoosh_env,
    )
    layered = mix(layered, gain(whoosh, 0.22))

    layered = soft_clip(layered, 1.05)
    return finish(layered, WAVE_START_SECONDS, target_db=-2.0, tail_curve=2.4)


UPGRADE_SELECTED_SECONDS = 0.78


def render_upgrade_selected():
    """Compact ascending arpeggio with a restrained energy lift. It confirms
    an upgrade without borrowing the longer alarm character of wave_start."""
    count = frames_for(UPGRADE_SELECTED_SECONDS)
    layered = [0.0] * count

    def place(local, offset_seconds, loudness):
        start = frames_for(offset_seconds)
        for offset, value in enumerate(local):
            index = start + offset
            if index < count:
                layered[index] += value * loudness

    for index, (offset_seconds, frequency) in enumerate((
        (0.00, 523.25),
        (0.12, 659.25),
        (0.24, 880.00),
    )):
        note_length = frames_for(0.42)
        note_env = percussive(note_length, 0.002, None,
                              4.8 + index * 0.4, 0.42)
        note = shaped(mix(
            sine_sweep(note_length, frequency * 0.98, frequency),
            gain(sine_sweep(note_length, frequency * 2.0,
                            frequency * 2.01, phase=0.8), 0.22),
        ), note_env)
        place(note, offset_seconds, 0.62 + index * 0.08)

    lift = shaped(
        sine_sweep(count, 280.0, 1120.0, exponential=True),
        adsr(count, 0.02, 0.16, 0.32, 0.34),
    )
    sparkle = shaped(
        bandpass(noise(count, Rng(0xA69ADE)), 2600.0, 9000.0),
        percussive(count, 0.010, None, 4.2, UPGRADE_SELECTED_SECONDS),
    )
    layered = mix(layered, gain(lift, 0.18), gain(sparkle, 0.08))
    layered = soft_clip(layered, 1.04)
    return finish(
        layered,
        UPGRADE_SELECTED_SECONDS,
        target_db=-3.5,
        tail_curve=2.8,
    )


VICTORY_SECONDS = 2.55


def render_victory():
    """Bright major fanfare: a four-note ascent resolves into a sustained
    major chord. Its upward contour deliberately opposes game_over."""
    count = frames_for(VICTORY_SECONDS)
    layered = [0.0] * count

    def place(local, offset_seconds, loudness):
        start = frames_for(offset_seconds)
        for offset, value in enumerate(local):
            index = start + offset
            if index < count:
                layered[index] += value * loudness

    for index, (offset_seconds, frequency) in enumerate((
        (0.00, 392.00),
        (0.14, 523.25),
        (0.28, 659.25),
        (0.42, 783.99),
    )):
        note_length = frames_for(0.50)
        note_env = percussive(note_length, 0.003, None, 4.6, 0.50)
        note = shaped(mix(
            sine_sweep(note_length, frequency * 0.99, frequency),
            gain(sine_sweep(note_length, frequency * 2.0,
                            frequency * 1.995, phase=0.6), 0.20),
            gain(triangle_sweep(note_length, frequency, frequency * 1.002),
                 0.12),
        ), note_env)
        place(note, offset_seconds, 0.42 + index * 0.05)

    chord_length = frames_for(1.75)
    chord_env = adsr(chord_length, 0.025, 0.26, 0.50, 0.78)
    chord = shaped(mix(
        gain(sine_sweep(chord_length, 261.63, 262.20), 0.42),
        sine_sweep(chord_length, 523.25, 524.40, phase=0.2),
        gain(sine_sweep(chord_length, 659.25, 660.10, phase=0.9), 0.72),
        gain(sine_sweep(chord_length, 783.99, 785.20, phase=1.8), 0.60),
        gain(sine_sweep(chord_length, 1046.50, 1048.00, phase=2.4), 0.24),
    ), chord_env)
    place(chord, 0.66, 0.56)

    shimmer_length = frames_for(1.55)
    shimmer = shaped(
        bandpass(noise(shimmer_length, Rng(0x71C701)), 1800.0, 8200.0),
        adsr(shimmer_length, 0.08, 0.30, 0.22, 0.80),
    )
    place(shimmer, 0.72, 0.07)

    layered = soft_clip(layered, 1.03)
    return finish(layered, VICTORY_SECONDS, target_db=-2.5, tail_curve=2.2)


GAME_OVER_SECONDS = 2.6


def render_game_over():
    """Defeat sting: three descending minor chords with a slow widening
    detune, a low descending drone, and a noise tail that fades well before
    the file ends. Roughly 2.6 seconds. Rising wave_start and this descending
    game_over are polar opposites by design."""
    count = frames_for(GAME_OVER_SECONDS)
    layered = [0.0] * count

    def place(local, offset_seconds, loudness):
        start = frames_for(offset_seconds)
        for offset, value in enumerate(local):
            index = start + offset
            if index < count:
                layered[index] += value * loudness
        return layered

    # Descending minor progression: A3 -> F3 -> D3.
    notes = (
        (220.0, 261.63, 329.63, 0.00, 0.62, 1.00),
        (174.61, 220.00, 261.63, 0.58, 0.72, 0.85),
        (146.83, 185.00, 220.00, 1.18, 1.10, 0.70),
    )
    for root, third, fifth, offset_seconds, length, loudness in notes:
        note_length = frames_for(length)
        note_env = adsr(note_length, 0.03, 0.20, 0.55, length - 0.30)
        voices = mix(
            sine_sweep(note_length, root, root * 0.995),
            gain(sine_sweep(note_length, root * 1.006, root, phase=1.3), 0.6),
            gain(sine_sweep(note_length, third, third * 0.995, phase=0.4),
                 0.42),
            gain(sine_sweep(note_length, fifth, fifth * 0.99, phase=2.0), 0.3),
            gain(one_pole_lowpass(saw_sweep(note_length, root, root * 0.99),
                                  1400.0), 0.18),
        )
        place(shaped(voices, note_env), offset_seconds, 0.42 * loudness)

    # Low descending drone across the whole sting.
    drone_env = adsr(count, 0.25, 0.40, 0.40, 1.25)
    drone = shaped(mix(
        sine_sweep(count, 110.0, 55.0, exponential=True),
        gain(sine_sweep(count, 55.0, 33.0, phase=1.6, exponential=True), 0.5),
    ), drone_env)
    layered = mix(layered, gain(drone, 0.5))

    # Air/reverb-ish noise bed that decays into the fade.
    bed_env = adsr(count, 0.18, 0.60, 0.28, 1.40)
    bed = shaped(bandpass(noise(count, Rng(0x0DE7)), 200.0, 4200.0), bed_env)
    layered = mix(layered, gain(bed, 0.14))

    layered = soft_clip(layered, 1.05)
    return finish(layered, GAME_OVER_SECONDS, target_db=-2.0, tail_curve=2.0)


# --------------------------------------------------------------------------
# Registry
# --------------------------------------------------------------------------

SOUNDS = (
    ("select", "assets/audio/ui/select.wav", render_select, 0.08),
    ("confirm", "assets/audio/ui/confirm.wav", render_confirm, 0.06),
    ("back", "assets/audio/ui/back.wav", render_back, 0.02),
    ("player_shot", "assets/audio/combat/player_shot.wav",
     render_player_shot, 0.18),
    ("enemy_shot", "assets/audio/combat/enemy_shot.wav",
     render_enemy_shot, -0.12),
    ("projectile_hit", "assets/audio/combat/projectile_hit.wav",
     render_projectile_hit, 0.10),
    ("player_damaged", "assets/audio/combat/player_damaged.wav",
     render_player_damaged, 0.0),
    ("enemy_died", "assets/audio/combat/enemy_died.wav",
     render_enemy_died, 0.0),
    ("reload_start", "assets/audio/combat/reload_start.wav",
     render_reload_start, 0.10),
    ("reload_complete", "assets/audio/combat/reload_complete.wav",
     render_reload_complete, 0.10),
    ("wave_start", "assets/audio/system/wave_start.wav",
     render_wave_start, 0.0),
    ("upgrade_selected", "assets/audio/system/upgrade_selected.wav",
     render_upgrade_selected, 0.12),
    ("victory", "assets/audio/system/victory.wav",
     render_victory, 0.16),
    ("game_over", "assets/audio/system/game_over.wav",
     render_game_over, 0.0),
)


def build(name, render, width):
    """Render one sound and return the stereo pair as a list of floats."""
    mono = render()
    if HEAD_SILENCE_FRAMES:
        mono = [0.0] * HEAD_SILENCE_FRAMES + mono
    left, right = stereo(mono, width=width)
    return left, right


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--only", action="append", default=None,
                        help="generate only the named sound(s)")
    parser.add_argument("--check", action="store_true",
                        help="compare freshly rendered bytes with the files "
                             "on disk instead of writing them")
    parser.add_argument("--root", default=None,
                        help="repository root (defaults to the script's "
                             "parent directory)")
    options = parser.parse_args(argv)

    root = options.root or os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))

    selected = SOUNDS
    if options.only:
        wanted = set(options.only)
        selected = tuple(item for item in SOUNDS if item[0] in wanted)
        unknown = wanted - {item[0] for item in SOUNDS}
        if unknown:
            sys.stderr.write(
                "unknown sound(s): {0}\n".format(", ".join(sorted(unknown))))
            return 2

    failures = 0
    for name, relative_path, render, width in selected:
        left, right = build(name, render, width)
        data = encode_wave_file(left, right)
        destination = os.path.join(root, relative_path.replace("/", os.sep))

        if options.check:
            try:
                with open(destination, "rb") as handle:
                    existing = handle.read()
            except OSError as error:
                sys.stderr.write(
                    "{0}: cannot read {1} ({2})\n".format(
                        name, relative_path, error))
                failures += 1
                continue
            if existing != data:
                sys.stderr.write(
                    "{0}: {1} differs from the generator output\n".format(
                        name, relative_path))
                failures += 1
                continue
            print("{0}: {1} matches ({2} bytes)".format(
                name, relative_path, len(data)))
            continue

        write_wave(destination, left, right)
        peak = max((abs(value) for value in left + right), default=0.0)
        print("{0}: wrote {1} ({2} bytes, {3} frames, peak {4:.3f})".format(
            name, relative_path, len(data), len(left), peak))

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
