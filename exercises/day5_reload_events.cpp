#include <algorithm>

#include "core/Simulation.h"

int main() {
    neon::GameplayConfig config{};
    config.reloadDuration = 0.05f;

    constexpr float fixedDt =
        1.0f / 60.0f;

    neon::Simulation simulation(config);
    simulation.consumeEvents();

    neon::InputCommand fireCommand{};
    fireCommand.aimPosition = {
        0.0f,
        0.0f
    };
    fireCommand.fireHeld = true;

    simulation.step(
        fireCommand,
        fixedDt
    );

    const auto shotEvents =
        simulation.consumeEvents();

    const bool shotRecorded =
        std::any_of(
            shotEvents.begin(),
            shotEvents.end(),
            [](const neon::GameEvent& event) {
                return
                    event.type ==
                    neon::GameEventType::PlayerShot;
            }
        );

    neon::InputCommand reloadCommand{};
    reloadCommand.reloadPressed = true;

    simulation.step(
        reloadCommand,
        fixedDt
    );

    const auto startedEvents =
        simulation.consumeEvents();

    const bool reloadStarted =
        std::any_of(
            startedEvents.begin(),
            startedEvents.end(),
            [](const neon::GameEvent& event) {
                return
                    event.type ==
                    neon::GameEventType::ReloadStarted &&
                    event.sourceKind ==
                    neon::GameEntityKind::Player &&
                    event.sourceId ==
                    neon::playerEntityId &&
                    event.value == 11;
            }
        );

    for (int index = 0;
        index < 10 &&
        simulation.snapshot().player.reloading;
        ++index) {
        neon::InputCommand idleCommand{};

        simulation.step(
            idleCommand,
            fixedDt
        );
    }

    const auto completedEvents =
        simulation.consumeEvents();

    const bool reloadCompleted =
        std::any_of(
            completedEvents.begin(),
            completedEvents.end(),
            [](const neon::GameEvent& event) {
                return
                    event.type ==
                    neon::GameEventType::ReloadCompleted &&
                    event.sourceKind ==
                    neon::GameEntityKind::Player &&
                    event.sourceId ==
                    neon::playerEntityId &&
                    event.value == 12;
            }
        );

    return shotRecorded &&
        reloadStarted &&
        reloadCompleted &&
        !simulation.snapshot().player.reloading
        ? 0
        : 1;
}