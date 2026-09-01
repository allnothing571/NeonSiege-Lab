#include <cmath>

#include "core/Enemy.h"
#include "core/GameplayConfig.h"

bool nearlyEqual(float first, float second) {
    constexpr float epsilon = 0.001f;
    return std::fabs(first - second) <= epsilon;
}

int main() {
    const neon::GameplayConfig config{};

    constexpr neon::Vec2 startPosition{
        0.0f,
        0.0f
    };

    constexpr float size = 10.0f;
    constexpr float speed = 100.0f;
    constexpr int health = 20;
    constexpr float dt = 1.0f;

    neon::Enemy farEnemy{
        startPosition,
        size,
        speed,
        health,
        neon::EnemyKind::Shooter,
        config.shooterRetreatDistance,
        config.shooterApproachDistance
    };

    farEnemy.update(
        neon::Vec2{ 405.0f, 5.0f },
        dt
    );

    const bool approachPassed =
        nearlyEqual(farEnemy.bounds().x, 100.0f) &&
        nearlyEqual(farEnemy.bounds().y, 0.0f);

    neon::Enemy middleEnemy{
        startPosition,
        size,
        speed,
        health,
        neon::EnemyKind::Shooter,
        config.shooterRetreatDistance,
        config.shooterApproachDistance
    };

    middleEnemy.update(
        neon::Vec2{ 255.0f, 5.0f },
        dt
    );

    const bool strafePassed =
        nearlyEqual(middleEnemy.bounds().x, 0.0f) &&
        nearlyEqual(middleEnemy.bounds().y, 100.0f);

    neon::Enemy nearEnemy{
        startPosition,
        size,
        speed,
        health,
        neon::EnemyKind::Shooter,
        config.shooterRetreatDistance,
        config.shooterApproachDistance
    };

    nearEnemy.update(
        neon::Vec2{ 125.0f, 5.0f },
        dt
    );

    const bool retreatPassed =
        nearlyEqual(nearEnemy.bounds().x, -100.0f) &&
        nearlyEqual(nearEnemy.bounds().y, 0.0f);

    return approachPassed &&
        strafePassed &&
        retreatPassed
        ? 0
        : 1;
}