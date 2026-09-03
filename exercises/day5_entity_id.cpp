#include "core/Enemy.h"
#include "core/EntityId.h"
#include "core/Player.h"
#include "core/Projectile.h"

int main() {
    const neon::Player player{
        {100.0f, 100.0f},
        48.0f,
        240.0f,
        100,
        0.5f,
        11
    };

    const neon::Enemy enemy{
        {200.0f, 100.0f},
        32.0f,
        70.0f,
        20,
        neon::EnemyKind::Shooter,
        180.0f,
        320.0f,
        0.3f,
        1.2f,
        12
    };

    const neon::Projectile projectile{
        {300.0f, 100.0f},
        {1.0f, 0.0f},
        8.0f,
        240.0f,
        neon::ProjectileFaction::Enemy,
        10,
        1.0f,
        13
    };

    const bool idsPassed =
        player.id() == 11 &&
        enemy.id() == 12 &&
        projectile.id() == 13 &&
        player.id() != enemy.id() &&
        enemy.id() != projectile.id();

    const bool invalidValuePassed =
        neon::invalidEntityId == 0 &&
        neon::playerEntityId == 1;

    return idsPassed &&
        invalidValuePassed
        ? 0
        : 1;
}