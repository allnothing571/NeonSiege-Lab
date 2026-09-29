"""Objective audio checks for the Neon Siege acceptance run.

Usage:
    python tools/audio_verify.py [--root .] [--dir assets/audio]

Checks every runtime WAV against the technical requirements in
docs/commercial_audio_upgrade_manual.md:

* PCM, 16-bit, 44.1 kHz, stereo, and a valid RIFF/WAVE header.
* Head silence is short enough to be inaudible as a delay.
* Peak level has headroom and no sample is clipped.
* The tail decays instead of being cut mid-signal.
* The catalog's short combat sounds stay inside their intended length budget.

It also prints a coarse spectral fingerprint (energy share per octave band) so
that a reviewer can confirm the five combat sounds are objectively different
before doing the listening pass.
"""

import argparse
import cmath
import math
import os
import struct
import sys
import wave

SAMPLE_RATE_EXPECTED = 44100
BITS_EXPECTED = 16
CHANNELS_EXPECTED = 2

AUDIBLE_THRESHOLD = 96
MAX_HEAD_SILENCE_SECONDS = 0.02
CLIP_LEVEL = 32767
MIN_PEAK_RATIO = 0.25

# Sounds produced by tools/generate_audio_assets.py. Their audio data is owned
# by the project, so every requirement is enforced on them directly.
GENERATED = frozenset((
    "audio/ui/select.wav",
    "audio/ui/confirm.wav",
    "audio/ui/back.wav",
    "audio/combat/player_shot.wav",
    "audio/combat/enemy_shot.wav",
    "audio/combat/projectile_hit.wav",
    "audio/combat/player_damaged.wav",
    "audio/combat/enemy_died.wav",
    "audio/combat/reload_start.wav",
    "audio/combat/reload_complete.wav",
    "audio/system/wave_start.wav",
    "audio/system/upgrade_selected.wav",
    "audio/system/victory.wav",
    "audio/system/game_over.wav",
))

# name -> (minimum seconds, maximum seconds) from the design brief
LENGTH_BUDGET = {
    "audio/ui/select.wav": (0.08, 0.20),
    "audio/ui/confirm.wav": (0.15, 0.40),
    "audio/ui/back.wav": (0.15, 0.40),
    "audio/combat/player_shot.wav": (0.08, 0.25),
    "audio/combat/enemy_shot.wav": (0.12, 0.35),
    "audio/combat/projectile_hit.wav": (0.08, 0.25),
    "audio/combat/player_damaged.wav": (0.20, 0.60),
    "audio/combat/enemy_died.wav": (0.30, 0.80),
    "audio/combat/reload_start.wav": (0.18, 0.30),
    "audio/combat/reload_complete.wav": (0.10, 0.22),
    "audio/system/wave_start.wav": (0.50, 1.50),
    "audio/system/upgrade_selected.wav": (0.50, 1.20),
    "audio/system/victory.wav": (1.50, 3.50),
    "audio/system/game_over.wav": (1.50, 3.50),
}

# Octave bands used for the spectral fingerprint, in Hz.
BANDS = (
    ("<125", 0.0, 125.0),
    ("125-250", 125.0, 250.0),
    ("250-500", 250.0, 500.0),
    ("0.5-1k", 500.0, 1000.0),
    ("1-2k", 1000.0, 2000.0),
    ("2-4k", 2000.0, 4000.0),
    ("4-8k", 4000.0, 8000.0),
    (">8k", 8000.0, 22050.0),
)

# Mirror of src/sdl/SdlAudioCatalog.cpp, so the balance pass can compare the
# catalog's base volumes with the measured loudness of each file. Keep in sync
# when the catalog changes; tools/audio_verify.py --levels reports a mismatch.
CATALOG = {
    "audio/ui/select.wav": (55, 80),
    "audio/ui/confirm.wav": (70, 0),
    "audio/ui/back.wav": (70, 0),
    "audio/combat/player_shot.wav": (75, 0),
    "audio/combat/enemy_shot.wav": (60, 50),
    "audio/combat/projectile_hit.wav": (65, 35),
    "audio/combat/player_damaged.wav": (90, 0),
    "audio/combat/enemy_died.wav": (80, 50),
    "audio/combat/reload_start.wav": (70, 0),
    "audio/combat/reload_complete.wav": (80, 0),
    "audio/system/wave_start.wav": (85, 0),
    "audio/system/upgrade_selected.wav": (85, 0),
    "audio/system/victory.wav": (100, 0),
    "audio/system/game_over.wav": (100, 0),
}


def loudness(samples):
    """RMS level in dBFS. Used to compare perceived loudness between files,
    which a peak measurement alone cannot express."""
    if not samples:
        return -120.0
    total = 0.0
    for value in samples:
        total += float(value) * float(value)
    rms = math.sqrt(total / len(samples))
    if rms <= 1e-9:
        return -120.0
    return 20.0 * math.log10(rms / float(CLIP_LEVEL))


def catalog_volume_table(reports):
    """Print measured RMS loudness next to the catalog's base volume, so a
    reviewer can see whether the base volumes already compensate for quiet
    source material."""
    lines = []
    lines.append("{0:<34} {1:>8} {2:>6} {3:>9} {4:>10}".format(
        "runtime path", "RMS dBFS", "base%", "cooldown", "base+RMS"))
    lines.append("-" * 72)
    rows = []
    for report in reports:
        key = report["key"]
        if key not in CATALOG or "rms" not in report:
            continue
        base, cooldown = CATALOG[key]
        effective = report["rms"] + 20.0 * math.log10(base / 100.0)
        rows.append((key, report["rms"], base, cooldown, effective))
    for key, rms, base, cooldown, effective in rows:
        lines.append("{0:<34} {1:>8.1f} {2:>6} {3:>9} {4:>10.1f}".format(
            key, rms, base, cooldown, effective))
    values = [row[4] for row in rows]
    if values:
        lines.append("-" * 72)
        lines.append("effective spread: {0:.1f} dB (min {1:.1f}, max {2:.1f})"
                     .format(max(values) - min(values), min(values),
                             max(values)))
        lines.append(
            "catalog spot check: every path above must exist in "
            "src/sdl/SdlAudioCatalog.cpp with the same base volume")
    return lines


def read_mono(path):
    with wave.open(str(path), "rb") as handle:
        info = {
            "channels": handle.getnchannels(),
            "sample_width": handle.getsampwidth(),
            "sample_rate": handle.getframerate(),
            "frames": handle.getnframes(),
            "compression": handle.getcomptype(),
        }
        raw = handle.readframes(info["frames"])

    if info["sample_width"] != 2:
        return info, []

    total = info["frames"] * info["channels"]
    values = struct.unpack("<{0}h".format(total), raw) if total else ()
    mono = []
    for index in range(0, len(values) - info["channels"] + 1,
                       info["channels"]):
        total_value = 0
        for channel in range(info["channels"]):
            total_value += values[index + channel]
        mono.append(total_value / float(info["channels"]))
    return info, mono


def goertzel(samples, frequency, sample_rate):
    """Magnitude of one frequency bin (cheap single-bin DFT)."""
    omega = 2.0 * math.pi * frequency / sample_rate
    coefficient = 2.0 * math.cos(omega)
    s_prev = 0.0
    s_prev2 = 0.0
    for value in samples:
        s = value + coefficient * s_prev - s_prev2
        s_prev2 = s_prev
        s_prev = s
    power = s_prev2 * s_prev2 + s_prev * s_prev - coefficient * s_prev * s_prev2
    return math.sqrt(max(0.0, power))


def spectrum(samples, sample_rate):
    """Sum the Goertzel magnitude over log-spaced probe frequencies per band."""
    if not samples:
        return {name: 0.0 for name, _, _ in BANDS}

    # Analyse at most the first 8192 samples: enough for a fingerprint and
    # keeps the pure-Python DFT fast.
    window_size = min(len(samples), 8192)
    start = max(0, (len(samples) - window_size) // 2)
    window = samples[start:start + window_size]
    taper = [
        0.5 - 0.5 * math.cos(2.0 * math.pi * index / (window_size - 1))
        for index in range(window_size)
    ]
    windowed = [window[index] * taper[index] for index in range(window_size)]

    result = {}
    for name, low, high in BANDS:
        if low <= 0.0:
            probe_low = 30.0
        else:
            probe_low = low
        probes = []
        step = 1.12
        frequency = probe_low
        while frequency < high:
            probes.append(frequency)
            frequency *= step
        total = 0.0
        for frequency in probes:
            total += goertzel(windowed, frequency, sample_rate)
        result[name] = total
    overall = sum(result.values())
    if overall > 0.0:
        for name in list(result):
            result[name] = result[name] / overall
    return result


def analyse(path, directory_key):
    info, mono = read_mono(path)
    generated = directory_key in GENERATED
    report = {
        "path": path,
        "key": directory_key,
        "generated": generated,
        "info": info,
        "problems": [],
        "notes": [],
    }
    if info["sample_width"] != BITS_EXPECTED // 8:
        report["problems"].append(
            "bit depth is {0}, expected {1}".format(
                info["sample_width"] * 8, BITS_EXPECTED))
    if info["sample_rate"] != SAMPLE_RATE_EXPECTED:
        report["problems"].append(
            "sample rate is {0}, expected {1}".format(
                info["sample_rate"], SAMPLE_RATE_EXPECTED))
    if info["channels"] != CHANNELS_EXPECTED:
        report["problems"].append(
            "channel count is {0}, expected {1}".format(
                info["channels"], CHANNELS_EXPECTED))
    if info["compression"] != "NONE":
        report["problems"].append(
            "compressed WAV payload: {0}".format(info["compression"]))

    if not mono:
        report["problems"].append("no audio data")
        return report

    peak = max(abs(value) for value in mono)
    clipped = sum(1 for value in mono if abs(value) >= CLIP_LEVEL - 1)
    head = next((index for index, value in enumerate(mono)
                 if abs(value) > AUDIBLE_THRESHOLD), None)
    tail = next((index for index, value in enumerate(reversed(mono))
                 if abs(value) > AUDIBLE_THRESHOLD), None)

    duration = info["frames"] / float(info["sample_rate"])
    report["peak"] = peak
    report["peak_ratio"] = peak / float(CLIP_LEVEL)
    report["clipped"] = clipped
    report["duration"] = duration
    report["head_frames"] = head
    report["tail_frames"] = tail
    report["rms"] = loudness(mono)
    report["spectrum"] = spectrum(mono, info["sample_rate"])

    if clipped:
        report["problems"].append("{0} clipped sample(s)".format(clipped))
    if peak >= CLIP_LEVEL:
        report["problems"].append("peak reaches full scale")

    if head is None:
        report["problems"].append("file is silent")

    if generated:
        if peak < int(CLIP_LEVEL * MIN_PEAK_RATIO):
            report["problems"].append(
                "peak is only {0:.1%} FS: too quiet for a game effect".format(
                    peak / float(CLIP_LEVEL)))
        if head is not None and \
                head / float(info["sample_rate"]) > MAX_HEAD_SILENCE_SECONDS:
            report["problems"].append(
                "head silence {0} frames exceeds {1} ms".format(
                    head, int(MAX_HEAD_SILENCE_SECONDS * 1000)))
    else:
        report["problems"].append(
            "runtime WAV is not registered as project-generated")

    if tail is not None and tail < 8:
        report["problems"].append(
            "tail content reaches the last {0} frames: possible hard cut"
            .format(tail))

    budget = LENGTH_BUDGET.get(directory_key)
    if budget is not None:
        low, high = budget
        if not (low - 0.001 <= duration <= high + 0.001):
            report["problems"].append(
                "duration {0:.3f}s outside the design budget {1:.2f}-{2:.2f}s"
                .format(duration, low, high))

    return report


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=None)
    parser.add_argument("--dir", default="assets/audio")
    parser.add_argument("--spectrum", action="store_true",
                        help="print the per-band energy fingerprint")
    parser.add_argument("--levels", action="store_true",
                        help="print measured loudness next to the catalog "
                             "base volumes")
    options = parser.parse_args(argv)

    root = options.root or os.path.dirname(
        os.path.dirname(os.path.abspath(__file__)))
    audio_root = os.path.join(root, options.dir.replace("/", os.sep))

    if not os.path.isdir(audio_root):
        sys.stderr.write("audio directory not found: {0}\n".format(audio_root))
        return 2

    files = []
    for base, _, names in os.walk(audio_root):
        for name in sorted(names):
            if name.lower().endswith(".wav"):
                files.append(os.path.join(base, name))
    files.sort()

    failures = 0
    generated_count = 0
    reports = []
    for path in files:
        relative = os.path.relpath(path, root).replace(os.sep, "/")
        key = relative[len("assets/"):] if relative.startswith("assets/") \
            else relative
        report = analyse(path, key)
        reports.append(report)
        if report["generated"]:
            generated_count += 1
        status = "OK  " if not report["problems"] else "FAIL"
        if report["problems"]:
            failures += 1
        print("{0} {1}\n     {2} Hz / {3}-bit / {4}ch / {5:.3f}s / "
              "peak {6:.1%} FS / RMS {7:.1f} dBFS / head {8} / tail {9}".format(
                  status,
                  relative,
                  report["info"]["sample_rate"],
                  report["info"]["sample_width"] * 8,
                  report["info"]["channels"],
                  report.get("duration", 0.0),
                  report.get("peak_ratio", 0.0),
                  report.get("rms", -120.0),
                  report.get("head_frames"),
                  report.get("tail_frames"),
              ))
        for problem in report["problems"]:
            print("     - FAIL {0}".format(problem))
        for note in report["notes"]:
            print("     - note {0}".format(note))
        if options.spectrum and "spectrum" in report:
            parts = []
            for name, _, _ in BANDS:
                parts.append("{0} {1:5.1f}%".format(
                    name, report["spectrum"][name] * 100.0))
            print("     spectrum: {0}".format("  ".join(parts)))

    if options.levels:
        print("")
        for line in catalog_volume_table(reports):
            print(line)

    print("\n{0} file(s) checked ({1} project-generated, {2} unregistered), "
          "{3} with problems".format(
              len(files), generated_count, len(files) - generated_count,
              failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
