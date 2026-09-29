"""Render a contact sheet of waveform + energy envelopes for human review.

Usage:
    python tools/audio_contact_sheet.py [--root .] [--out out/audio_contact_sheet.txt]

The listening pass must be done by a human; this sheet gives that person the
objective shape of every effect (level envelope in dBFS, spectral balance per
octave band, duration, peak) so the pass can be judged against real numbers
rather than impressions alone.
"""

import argparse
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import audio_verify as verify  # noqa: E402  (path set up above)


BLOCKS = " .:-=+*#%@"
ENVELOPE_ROWS = 8
ENVELOPE_COLUMNS = 68
DB_FLOOR = -54.0


def envelope_rows(samples, sample_rate, rows=ENVELOPE_ROWS,
                  columns=ENVELOPE_COLUMNS):
    """ASCII level envelope. Each row covers one slice of the dB range and the
    character position inside the row shows where the level sits, so the
    picture actually slopes as the sound decays."""
    if not samples:
        return ["(silent)"] * rows
    block = max(1, len(samples) // columns)
    rms = []
    for index in range(columns):
        start = index * block
        chunk = samples[start:start + block]
        if not chunk:
            rms.append(0.0)
            continue
        total = 0.0
        for value in chunk:
            total += float(value) * float(value)
        rms.append(math.sqrt(total / len(chunk)))

    peak = max(rms) if rms else 0.0
    if peak <= 0.0:
        return ["(silent)"] * rows

    decibels = []
    for value in rms:
        ratio = value / peak
        decibels.append(DB_FLOOR if ratio <= 1e-6
                        else 20.0 * math.log10(ratio))

    span = -DB_FLOOR / float(rows)
    lines = []
    for row in range(rows):
        top = -span * row
        bottom = -span * (row + 1)
        characters = []
        for decibel in decibels:
            if decibel < bottom:
                characters.append(" ")
                continue
            if decibel >= top:
                characters.append(BLOCKS[-1])
                continue
            position = (top - decibel) / span
            characters.append(
                BLOCKS[min(len(BLOCKS) - 1,
                           int(position * (len(BLOCKS) - 1)))])
        lines.append("".join(characters))
    return lines


def band_bars(spectrum):
    lines = []
    for name, _, _ in verify.BANDS:
        share = spectrum.get(name, 0.0)
        filled = int(round(share * 40.0))
        lines.append("    {0:>7} |{1}{2} {3:5.1f}%".format(
            name, "#" * filled, "." * (40 - filled), share * 100.0))
    return lines


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=None)
    parser.add_argument("--dir", default="assets/audio")
    parser.add_argument("--out", default=None)
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

    lines = []
    lines.append("Neon Siege audio contact sheet (objective measurements)")
    lines.append("=" * 72)
    lines.append("Envelope rows span 0 dB to -54 dB of each file's own peak,")
    lines.append("with the character position inside a row showing the level.")
    lines.append("")

    for path in files:
        relative = os.path.relpath(path, root).replace(os.sep, "/")
        info, mono = verify.read_mono(path)
        if not mono:
            lines.append("{0}: no audio data".format(relative))
            continue
        peak = max(abs(value) for value in mono)
        duration = info["frames"] / float(info["sample_rate"])
        lines.append("{0}".format(relative))
        lines.append("  {0} Hz / {1}-bit / {2}ch / {3:.3f}s / peak {4:.1%} FS "
                     "/ peak sample {5}".format(
                         info["sample_rate"],
                         info["sample_width"] * 8,
                         info["channels"],
                         duration,
                         peak / float(verify.CLIP_LEVEL),
                         peak))
        for row in envelope_rows(mono, info["sample_rate"]):
            lines.append("  |" + row + "|")
        for band_line in band_bars(verify.spectrum(mono,
                                                   info["sample_rate"])):
            lines.append(band_line)
        lines.append("")

    text = "\n".join(lines)
    if options.out:
        destination = os.path.join(root, options.out.replace("/", os.sep))
        directory = os.path.dirname(destination)
        if directory and not os.path.isdir(directory):
            os.makedirs(directory)
        with open(destination, "w", encoding="utf-8") as handle:
            handle.write(text + "\n")
        print("wrote {0}".format(destination))
    else:
        try:
            sys.stdout.write(text + "\n")
        except UnicodeEncodeError:
            sys.stdout.write(text.encode("utf-8", "replace").decode("ascii",
                                                                    "replace"))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
