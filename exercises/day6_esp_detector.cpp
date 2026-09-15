#include <cstdint>

#include "core/EspDetector.h"

namespace {

	neon::GameEvent makePlayerShot(
		std::uint64_t tick
	) {
		neon::GameEvent event{};

		event.tick =
			tick;

		event.type =
			neon::GameEventType::PlayerShot;

		event.sourceKind =
			neon::GameEntityKind::Player;

		event.sourceId =
			neon::playerEntityId;

		return event;
	}

	neon::ReplayFrame makeFrame(
		std::uint64_t tick,
		neon::Vec2 aimPosition,
		bool fireHeld
	) {
		neon::ReplayFrame frame{};

		frame.tick =
			tick;

		frame.command.aimPosition =
			aimPosition;

		frame.command.fireHeld =
			fireHeld;

		return frame;
	}

}//namespace

int main() {
	neon::EspDetector detector(
		neon::playerEntityId,
		0.5f,
		3u
	);

	const neon::Vec2 playerCenter{
		100.0f,
		100.0f
	};

	const neon::Vec2 targetCenter{
		200.0f,
		100.0f
	};

	// Ŀ��ɼ�����ʹ��׼���Ҳ�����ж�Ϊ͸�ӡ�
	if (detector.observe(
		makeFrame(
			1,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		true,
		{ makePlayerShot(1) }
	).has_value()) {

		return 1;
	}

	// ��һ������Ŀ�꾫׼�����ֻ�ۼ�������
	if (detector.observe(
		makeFrame(
			2,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{ makePlayerShot(2) }
	).has_value()) {

		return 2;
	}

	// �ڶ�������Ŀ�꾫׼�������δ�ﵽ��ֵ��
	if (detector.observe(
		makeFrame(
			3,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{ makePlayerShot(3) }
	).has_value()) {

		return 3;
	}

	// ����������Ŀ�꾫׼���������͸���쳣֤�ݡ�
	const auto detection =
		detector.observe(
			makeFrame(
				4,
				targetCenter,
				true
			),
			playerCenter,
			targetCenter,
			false,
			{ makePlayerShot(4) }
		);

	if (!detection.has_value() ||
		detection->type !=
		neon::DetectionType::EspAnomaly ||
		detection->subjectId !=
		neon::playerEntityId ||
		detection->evidence.sampleCount !=
		3u ||
		detection->confidence <= 0.0f ||
		detection->confidence >= 1.0f) {

		return 4;
	}

	// ͬһ�����쳣״ֻ̬����һ�Ρ�
	if (detector.observe(
		makeFrame(
			5,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{ makePlayerShot(5) }
	).has_value()) {

		return 5;
	}

	// Ŀ�����¿ɼ�ʱ����ʹ��һ֡û�������ҲҪ��������С�
	if (detector.observe(
		makeFrame(
			6,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		true,
		{}
	).has_value()) {

		return 6;
	}

	// fireHeld Ϊ true ��û�� PlayerShot����Ӧ����������
	if (detector.observe(
		makeFrame(
			7,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{}
	).has_value()) {

		return 7;
	}

	if (detector.observe(
		makeFrame(
			8,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{ makePlayerShot(8) }
	).has_value()) {

		return 8;
	}

	if (detector.observe(
		makeFrame(
			9,
			targetCenter,
			true
		),
		playerCenter,
		targetCenter,
		false,
		{ makePlayerShot(9) }
	).has_value()) {

		return 9;
	}

	const auto secondDetection =
		detector.observe(
			makeFrame(
				10,
				targetCenter,
				true
			),
			playerCenter,
			targetCenter,
			false,
			{ makePlayerShot(10) }
		);

	if (!secondDetection.has_value() ||
		secondDetection->type !=
		neon::DetectionType::EspAnomaly ||
		secondDetection->evidence.sampleCount !=
		3u) {

		return 10;
	}

	return 0;
}