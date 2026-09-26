#include "core/ReplayTape.h"

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <string>
#include <utility>

#include "core/MapCatalog.h"

namespace {

	constexpr std::size_t maxReplayFrames = 1'000'000;

	bool fail(
		std::string& error,
		const char* message
	) {
		error = message;
		return false;
	}

	bool finiteCommand(
		const neon::InputCommand& command
	) {
		return
			std::isfinite(command.movement.x) &&
			std::isfinite(command.movement.y) &&
			std::isfinite(command.aimPosition.x) &&
			std::isfinite(command.aimPosition.y) &&
			command.upgradeSelection >= -1 &&
			command.upgradeSelection < 3;
	}

	bool validMapId(
		neon::MapId mapId) {
		return mapId == neon::MapId::Legacy ||
			neon::findPresetMap(mapId) != nullptr;
	}

}//namespace

namespace neon {

	bool writeReplay(
		std::ostream& output,
		const ReplayTape& tape,
		std::string& error
	) {
		error.clear();

		if (!output) {
			return fail(error, "output stream is not writable");
		}

		if (tape.version != ReplayTape::currentVersion) {
			return fail(error, "unsupported replay version");
		}

		if (!std::isfinite(tape.fixedDt) ||
			tape.fixedDt <= 0.0f) {
			return fail(error, "invalid fixed dt");
		}

		if (!validMapId(tape.mapId)) {
			return fail(error, "invalid replay map");
		}

		if (tape.frames.size() > maxReplayFrames) {
			return fail(error, "too many replay frames");
		}

		std::uint64_t previousTick = 0;

		for (const ReplayFrame& frame : tape.frames) {
			if (frame.tick == 0 ||
				frame.tick <= previousTick) {
				return fail(error, "replay ticks are not increasing");
			}

			if (!finiteCommand(frame.command)) {
				return fail(error, "replay command contains non-finite data");
			}

			previousTick = frame.tick;
		}

		output
			<< "NEON_REPLAY "
			<< tape.version
			<< '\n';

		output
			<< "SEED "
			<< tape.randomSeed
			<< '\n';

		output
			<< "MAP "
			<< static_cast<std::uint32_t>(tape.mapId)
			<< '\n';

		output
			<< "FIXED_DT "
			<< std::defaultfloat
			<< std::setprecision(
				std::numeric_limits<float>::max_digits10
			)
			<< tape.fixedDt
			<< '\n';

		output
			<< "FRAMES "
			<< tape.frames.size()
			<< '\n';

		for (const ReplayFrame& frame : tape.frames) {
			output
				<< "FRAME "
				<< frame.tick
				<< ' '
				<< frame.command.movement.x
				<< ' '
				<< frame.command.movement.y
				<< ' '
				<< frame.command.aimPosition.x
				<< ' '
				<< frame.command.aimPosition.y
				<< ' '
				<< static_cast<int>(frame.command.fireHeld)
				<< ' '
				<< static_cast<int>(frame.command.reloadPressed)
				<< ' '
				<< static_cast<int>(frame.command.pausePressed)
				<< ' '
				<< static_cast<int>(frame.command.restartPressed)
				<< ' '
				<< frame.command.upgradeSelection
				<< '\n';
		}

		output << "END\n";

		if (!output) {
			return fail(error, "faild while writing replay");
		}

		return true;
	}

	bool readReplay(
		std::istream& input,
		ReplayTape& tape,
		std::string& error
	) {
		error.clear();

		ReplayTape candidate{};
		std::string label;
		std::string magic;

		if (!(input >> magic >> candidate.version) ||
			magic != "NEON_REPLAY") {
			return fail(error, "invalid replay header");
		}

		if (candidate.version <
				ReplayTape::minimumSupportedVersion ||
			candidate.version >
				ReplayTape::currentVersion) {
			return fail(error, "unsupported replay version");
		}

		if (!(input >> label) ||
			label != "SEED" ||
			!(input >> candidate.randomSeed)) {
			return fail(error, "invalid replay seed");
		}

		if (candidate.version >= 2u) {
			std::uint32_t mapValue = 0u;
			if (!(input >> label) ||
				label != "MAP" ||
				!(input >> mapValue)) {
				return fail(error, "invalid replay map");
			}

			candidate.mapId =
				static_cast<MapId>(mapValue);
			if (!validMapId(candidate.mapId)) {
				return fail(error, "unknown replay map");
			}
		}

		if (!(input >> label) ||
			label != "FIXED_DT" ||
			!(input >> candidate.fixedDt)) {
			return fail(error, "invalid replay fixed dt");
		}

		if (!std::isfinite(candidate.fixedDt) ||
			candidate.fixedDt <= 0.0f) {
			return fail(error, "invalid replay fixed dt value");
		}

		std::uint64_t frameCount = 0;

		if (!(input >> label) ||
			label != "FRAMES" ||
			!(input >> frameCount)) {
			return fail(error, "invalid replay frame count");
		}

		if (frameCount > maxReplayFrames) {
			return fail(error, "replay frame count is too large");
		}

		candidate.frames.reserve(
			static_cast<std::size_t>(frameCount)
		);

		std::uint64_t previousTick = 0;

		for (std::uint64_t index = 0;
			index < frameCount;
			++index) {

			ReplayFrame frame{};

			int fireHeld = 0;
			int reloadPressed = 0;
			int pausePressed = 0;
			int restartPressed = 0;
			int upgradeSelection = -1;

			if (!(input >> label) ||
				label != "FRAME" ||
				!(input
					>> frame.tick
					>> frame.command.movement.x
					>> frame.command.movement.y
					>> frame.command.aimPosition.x
					>> frame.command.aimPosition.y
					>> fireHeld
					>> reloadPressed
					>> pausePressed
					>> restartPressed)) {
				return fail(error, "invalid replay frame");
			}

			if (candidate.version >= 3u &&
				!(input >> upgradeSelection)) {
				return fail(
					error,
					"invalid replay upgrade selection"
				);
			}

			if (frame.tick == 0 ||
				frame.tick <= previousTick) {
				return fail(error, "replay ticks are not increasing");
			}

			if (fireHeld < 0 || fireHeld > 1 ||
				reloadPressed < 0 || reloadPressed > 1 ||
				pausePressed < 0 || pausePressed > 1 ||
				restartPressed < 0 || restartPressed > 1 ||
				upgradeSelection < -1 ||
				upgradeSelection >= 3) {
				return fail(error, "invalid replay command flag");
			}

			frame.command.fireHeld =
				fireHeld != 0;

			frame.command.reloadPressed =
				reloadPressed != 0;

			frame.command.pausePressed =
				pausePressed != 0;

			frame.command.restartPressed =
				restartPressed != 0;

			frame.command.upgradeSelection =
				upgradeSelection;

			if (!finiteCommand(frame.command)) {
				return fail(error, "replay command contains neon-finite data");
			}

			previousTick = frame.tick;
			candidate.frames.push_back(frame);
		}

		if (!(input >> label) ||
			label != "END") {
			return fail(error, "missing replay end marker");
		}

		std::string trailingData;

		if (input >> trailingData) {
			return fail(error, "unexpected replay trailing data");
		}

		tape = std::move(candidate);
		return true;
	}

}//namespace neon
