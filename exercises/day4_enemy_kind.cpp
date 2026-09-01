#include "core/Enemy.h"

int main() {
    const neon::Enemy chaser{
        neon::Vec2{ 10.0f, 20.0f },
        32.0f,
        70.0f,
        30
    };

    const neon::Enemy shooter{
        neon::Vec2{ 30.0f, 40.0f },
        32.0f,
        70.0f,
        20,
        neon::EnemyKind::Shooter
    };

    const bool passed =
        chaser.kind() == neon::EnemyKind::Chaser &&
        shooter.kind() == neon::EnemyKind::Shooter;

    return passed ? 0 : 1;
}