"""Read-only WAV inspector used for Neon Siege audio acceptance.

Usage: python tools/audio_probe.py <wav> [<wav> ...]

Reports the technical properties that the commercial audio manual requires to
be checked: sample rate, bit depth, channels, duration, peak level, whether the
head starts with audible content, and whether the tail looks hard-truncated.
"""

import struct
import sys
import wave
from pathlib import Path

AUDIBLE_THRESHOLD = 64


def probe(path):
    with wave.open(str(path), "rb") as handle:
        channels = handle.getnchannels()
        sample_width = handle.getsampwidth()
        sample_rate = handle.getframerate()
        frame_count = handle.getnframes()
        compression = handle.getcomptype()
        raw = handle.readframes(frame_count)

    fmt = {1: "b", 2: "h", 4: "i"}.get(sample_width)
    if fmt is None:
        return None

    total = frame_count * channels
    samples = struct.unpack("<{0}{1}".format(total, fmt), raw) if total else ()
    peak = 0
    clipped = 0
    for value in samples:
        magnitude = abs(value)
        if magnitude > peak:
            peak = magnitude
        if magnitude >= (1 << (sample_width * 8 - 1)) - 1:
            clipped += 1

    head = next(
        (index for index, value in enumerate(samples)
         if abs(value) > AUDIBLE_THRESHOLD),
        None,
    )
    tail = next(
        (index for index, value in enumerate(reversed(samples))
         if abs(value) > AUDIBLE_THRESHOLD),
        None,
    )

    return {
        "path": str(path),
        "sample_rate": sample_rate,
        "sample_width": sample_width,
        "channels": channels,
        "frames": frame_count,
        "duration": frame_count / float(sample_rate) if sample_rate else 0.0,
        "compression": compression,
        "peak": peak,
        "peak_ratio": peak / float(1 << (sample_width * 8 - 1)),
        "clipped_samples": clipped,
        "head_silence_frames": head,
        "tail_silence_frames": tail,
    }


def main(argv):
    failures = 0
    for target in argv:
        info = probe(Path(target))
        if info is None:
            print("UNSUPPORTED {0}".format(target))
            failures += 1
            continue

        print(
            "{path}\n"
            "  {rate} Hz / {bits}-bit / {channels}ch / {duration:.3f}s "
            "({frames} frames, {compression})\n"
            "  peak {peak} ({peak_ratio:.1%} FS), clipped samples {clipped}\n"
            "  head silence {head} frames, tail silence {tail} frames".format(
                path=info["path"],
                rate=info["sample_rate"],
                bits=info["sample_width"] * 8,
                channels=info["channels"],
                duration=info["duration"],
                frames=info["frames"],
                compression=info["compression"],
                peak=info["peak"],
                peak_ratio=info["peak_ratio"],
                clipped=info["clipped_samples"],
                head=info["head_silence_frames"],
                tail=info["tail_silence_frames"],
            )
        )
    return failures


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
