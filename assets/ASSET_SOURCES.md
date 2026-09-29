# Asset provenance

## Generated visual set

- Created: 2026-09-24
- Generator: OpenAI built-in image generation
- Purpose: original production artwork for the Neon Siege project
- Style: high-contrast pixel art, neon outer-space setting
- Post-processing: no external stock-art library; transparent padding is
  cropped at runtime by the game loader

The set contains one player, two enemy archetypes, two projectile sprites,
three map backgrounds, and one obstacle strip for each map palette.

The earlier multi-character and multi-map concept boards were used only for
visual review and are not included in the runtime asset directory.

Before any commercial distribution, retain this provenance record and verify
the distribution terms that apply to the image-generation service and the
storefront where the game will be published.

## Neon Siege original synthesized sound set

All fourteen runtime sound effects are original works authored by this
repository. They are not downloaded material: every sample is computed by
`tools/generate_audio_assets.py`, which lives in this repository and uses only
the Python standard library. The generator is deterministic, so the committed
WAV files can be reproduced byte for byte:

```powershell
python tools/generate_audio_assets.py --check   # byte-compare with the tree
```

- Created: 2026-09-27
- UI/system replacement completed: 2026-09-29
- Author: Neon Siege project (original work produced for this project)
- License: same terms as the project source; the project owns the copyright
- Commercial use allowed: yes (the generating tool is part of the project)
- Redistribution in a public source repository: yes
  (`assets/audio/` is committed; there is no third-party material in these
  fourteen files)
- Attribution required: no
- Third-party samples, stock libraries, loops or presets used: none
- Service terms to satisfy: none, because no generative audio service and no
  sample pack was used
- Processing: synthesized from scratch; the only operations applied are the
  ones performed inside the generator - additive and subtractive synthesis,
  one-pole low/high-pass and band-pass filtering, `tanh` soft clipping,
  deterministic pseudo-random noise, amplitude envelopes, equal-power stereo
  placement, per-sound peak normalization with deliberate headroom, a tail decay
  curve and a 12 ms fade-out. No resampling, no external editing software, no
  third-party plug-in.

| Runtime path | Generator function | Intended event | Duration | Peak | Format |
| --- | --- | --- | --- | --- | --- |
| `audio/ui/select.wav` | `render_select` | move between menu choices | 0.120 s | 58.7% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/ui/confirm.wav` | `render_confirm` | accept the highlighted choice | 0.260 s | 51.9% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/ui/back.wav` | `render_back` | return from tutorial / settings / results | 0.280 s | 57.4% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/player_shot.wav` | `render_player_shot` | player actually fires a bullet | 0.160 s | 74.1% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/enemy_shot.wav` | `render_enemy_shot` | shooter enemy actually fires | 0.300 s | 69.3% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/projectile_hit.wav` | `render_projectile_hit` | projectile hits enemy or obstacle | 0.150 s | 65.9% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/player_damaged.wav` | `render_player_damaged` | player actually loses health | 0.440 s | 79.4% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/enemy_died.wav` | `render_enemy_died` | enemy health reaches zero | 0.620 s | 75.0% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/reload_start.wav` | `render_reload_start` | manual or automatic reload actually starts | 0.220 s | 70.6% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/combat/reload_complete.wav` | `render_reload_complete` | reload has truly completed | 0.160 s | 74.5% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/system/wave_start.wav` | `render_wave_start` | a new wave formally begins | 1.050 s | 79.4% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/system/upgrade_selected.wav` | `render_upgrade_selected` | upgrade choice is applied | 0.780 s | 74.9% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/system/victory.wav` | `render_victory` | final victory state is reached | 2.550 s | 87.0% FS | 44.1 kHz, 16-bit, stereo PCM |
| `audio/system/game_over.wav` | `render_game_over` | player defeated, run ends | 2.600 s | 79.4% FS | 44.1 kHz, 16-bit, stereo PCM |

Sound design separation, measured by `tools/audio_verify.py --spectrum`
(share of spectral energy per octave band, so the seven combat sounds are
objectively different before the listening pass):

| Sound | Dominant band | Character |
| --- | --- | --- |
| `player_shot` | 0.5-2 kHz (70%) | bright, glassy, very fast decay |
| `projectile_hit` | 125-250 Hz + 2-4 kHz | broadband spark crack with a metallic ping |
| `enemy_shot` | 125-500 Hz (54%) | dark detuned saw with a soft attack |
| `player_damaged` | 250-500 Hz (69%) | two-beat alarm glide, slow decay |
| `enemy_died` | <125 Hz (54%) | burst, tumbling fall and metal debris |
| `reload_start` | 250 Hz-1 kHz (84%) | descending release mechanism and energy vent |
| `reload_complete` | 0.5-2 kHz (51%) | magazine seat, bright bolt lock and ready tick |

Specifications were checked with:

```powershell
python tools/audio_verify.py            # format, head, tail, peak, clipping
python tools/audio_contact_sheet.py     # level envelope and band balance
```

## Third-party UI replacement record

Earlier working-tree versions of `select.wav`, `confirm.wav`,
`upgrade_selected.wav` and `victory.wav` came from a third-party UI pack whose
terms did not permit redistribution of the original files in a public source
repository. On 2026-09-29, all four files were replaced completely by the
project-owned generator functions listed above.

The replacement functions synthesize every sample from oscillators,
deterministic noise, filters and envelopes. They do not open, sample, transform
or otherwise derive audio data from the former files. The third-party audio
bytes are no longer present under `assets/audio/` and are not required to build,
test or package the game. This resolves the previously recorded public-source
redistribution blocker for the runtime audio set.

## Registration rules for future audio

Future audio files must record their runtime path, original filename, author or
generating service, source page, license, commercial-use permission, whether
they may be redistributed in a public source repository, attribution
requirement, download or generation date, and every edit, mix, resample, fade
or rename that was applied. Do not treat "free download" as proof of
commercial-use permission, and do not treat "commercial use allowed" as proof
of public-source-repository redistribution permission.
