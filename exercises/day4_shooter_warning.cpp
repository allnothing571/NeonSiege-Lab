#include "core/Enemy.h"
#include "core/GameplayConfig.h"

int main() {
    const neon::GameplayConfig config{};

    neon::Enemy shooter{
        neon::Vec2{ 0.0f, 0.0f },
        config.enemySize,
        config.enemyBaseSpeed,
        config.shooterHealth,
        neon::EnemyKind::Shooter,
        config.shooterRetreatDistance,
        config.shooterApproachDistance,
        config.shooterWarningDuration,
        config.shooterFireInterval
    };

    const bool warningStarted =
        !shooter.updateShooterAttack(
            true,
            0.1f
        ) &&
        shooter.isWarning() &&
        shooter.warningProgress() > 0.0f;

    const bool sightLossReset =
        !shooter.updateShooterAttack(
            false,
            0.1f
        ) &&
        !shooter.isWarning() &&
        shooter.warningProgress() == 0.0f;

    const bool firstShotRequested =
        shooter.updateShooterAttack(
            true,
            config.shooterWarningDuration
        ) &&
        !shooter.isWarning();

    const bool cooldownBlocked =
        !shooter.updateShooterAttack(
            true,
            config.shooterFireInterval
        );

    const bool secondShotRequested =
        shooter.updateShooterAttack(
            true,
            config.shooterWarningDuration
        );

    neon::Enemy chaser{
        neon::Vec2{ 0.0f, 0.0f },
        config.enemySize,
        config.enemyBaseSpeed,
        config.enemyHealth
    };

    const bool chaserCannotShoot =
        !chaser.updateShooterAttack(
            true,
            1.0f
        );

    const bool passed =
        warningStarted &&
        sightLossReset &&
        firstShotRequested &&
        cooldownBlocked &&
        secondShotRequested &&
        chaserCannotShoot;

    return passed ? 0 : 1;
}