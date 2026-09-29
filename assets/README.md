# Neon Siege Assets

All runtime paths are relative to the executable's `assets/` directory.

## Texture layout

- `textures/entities/player.png`
- `textures/entities/enemy_chaser.png`
- `textures/entities/enemy_shooter.png`
- `textures/projectiles/player.png`
- `textures/projectiles/enemy.png`
- `textures/maps/crossfire/background.png`
- `textures/maps/crossfire/obstacle.png`
- `textures/maps/split_corridors/background.png`
- `textures/maps/split_corridors/obstacle.png`
- `textures/maps/broken_district/background.png`
- `textures/maps/broken_district/obstacle.png`

Entity and projectile textures are direction-neutral in this first visual pass.
Transparent padding is trimmed when a texture is loaded. Horizontal obstacle
strips are rotated for vertical collision rectangles; collision geometry is
still defined exclusively by the core map data.

If a texture cannot be loaded, the renderer must continue with its existing
colored-rectangle fallback. Missing visual assets must never prevent the core
game from starting.

See `ASSET_SOURCES.md` for provenance.

## Audio layout

All fourteen catalog slots are populated:

- `audio/ui/select.wav`
- `audio/ui/confirm.wav`
- `audio/ui/back.wav`
- `audio/combat/player_shot.wav`
- `audio/combat/enemy_shot.wav`
- `audio/combat/projectile_hit.wav`
- `audio/combat/player_damaged.wav`
- `audio/combat/enemy_died.wav`
- `audio/combat/reload_start.wav`
- `audio/combat/reload_complete.wav`
- `audio/system/wave_start.wav`
- `audio/system/upgrade_selected.wav`
- `audio/system/victory.wav`
- `audio/system/game_over.wav`

Audio assets use PCM WAV at 44.1 kHz, 16-bit, stereo. All fourteen original
synthesized files are produced by
`tools/generate_audio_assets.py`; do not hand-edit them, re-run the generator
instead, and check the result with `python tools/audio_verify.py`. Missing or
invalid audio is non-fatal: the game logs the problem and continues without
that sound. Every production audio file must have its source and commercial-use
terms recorded in `ASSET_SOURCES.md` before distribution.
