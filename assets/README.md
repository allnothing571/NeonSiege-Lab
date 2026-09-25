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
