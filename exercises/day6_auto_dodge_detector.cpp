#include <cstdint>

#include "core/AutoDodgeDetector.h"

namespace {

	constexpr neon::EntityId enemyEntityId =
		2u;

	neon::GameEvent makeEnemyShot(
		std::uint64_t tick
	) {
		neon::GameEvent event{};

		event.tick =
			tick;

		event.type =
			neon::GameEventType::EnemyShot;

		event.sourceKind =
			neon::GameEntityKind::Enemy;

		event.sourceId =
			enemyEntityId;

		event.position =
		{ 500.0f, 100.0f };

		return event;
	}

	neon::ReplayFrame makeFrame(
		std::uint64_t tick,
		neon::Vec2 movement
	) {
		neon::ReplayFrame frame{};

		frame.tick =
			tick;

		frame.command.movement =
			movement;

		return frame;
	}

}//namespace

int main() {
	neon::AutoDodgeDetector detector(
		neon::playerEntityId,
		0.15f,
		0.35f,
		0.8f,
		3u
	);

	const neon::Vec2 projectileVelocity{
		1.0f,
		0.0f
	};

	const float fixedDt =
		0.1f;

	// û�ез������¼�ʱ�������ƶ���Ӧ�����Զ����������
	if (detector.observe(
		makeFrame(
			1,
			{ 0.0f, 1.0f }
		),
		projectileVelocity,
		0.2f,
		fixedDt,
		{}
	).has_value()) {

		return 1;
	}

	// ��һ����в�����˿���
	if (detector.observe(
		makeFrame(
			2,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		0.25f,
		fixedDt,
		{ makeEnemyShot(2) }
	).has_value()) {

		return 2;
	}

	// ��һ֡Ѹ���򵯵��෽�ƶ����ۼ�һ�ο���������
	if (detector.observe(
		makeFrame(
			3,
			{ 0.0f, 1.0f }
		),
		projectileVelocity,
		0.20f,
		fixedDt,
		{}
	).has_value()) {

		return 3;
	}

	// ֹͣ�ƶ���Ϊ��һ����в���조���¿�ʼ�ƶ����ı��ء�
	detector.observe(
		makeFrame(
			4,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		-1.0f,
		fixedDt,
		{}
	);

	// �ڶ�����в��
	detector.observe(
		makeFrame(
			5,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		0.25f,
		fixedDt,
		{ makeEnemyShot(5) }
	);

	if (detector.observe(
		makeFrame(
			6,
			{ 0.0f, 1.0f }
		),
		projectileVelocity,
		0.20f,
		fixedDt,
		{}
	).has_value()) {

		return 4;
	}

	detector.observe(
		makeFrame(
			7,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		-1.0f,
		fixedDt,
		{}
	);

	// ��������в���ﵽ��ֵ�󱨸档
	detector.observe(
		makeFrame(
			8,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		0.25f,
		fixedDt,
		{ makeEnemyShot(8) }
	);

	const auto detection =
		detector.observe(
			makeFrame(
				9,
				{ 0.0f, 1.0f }
			),
			projectileVelocity,
			0.20f,
			fixedDt,
			{}
		);

	if (!detection.has_value() ||
		detection->type !=
		neon::DetectionType::AutoDodge ||
		detection->subjectId !=
		neon::playerEntityId ||
		detection->evidence.sampleCount !=
		3u ||
		detection->confidence <= 0.0f ||
		detection->confidence >= 1.0f) {

		return 5;
	}

	// ͬһ�����쳣״ֻ̬����һ�Ρ�
	detector.observe(
		makeFrame(
			10,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		-1.0f,
		fixedDt,
		{}
	);

	detector.observe(
		makeFrame(
			11,
			{ 0.0f, 0.0f }
		),
		projectileVelocity,
		0.25f,
		fixedDt,
		{ makeEnemyShot(11) }
	);

	if (detector.observe(
		makeFrame(
			12,
			{ 0.0f, 1.0f }
		),
		projectileVelocity,
		0.20f,
		fixedDt,
		{}
	).has_value()) {

		return 6;
	}

	return 0;
}