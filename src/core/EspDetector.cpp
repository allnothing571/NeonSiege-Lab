#include "core/EspDetector.h"

#include <algorithm>
#include <cmath>

namespace {

	constexpr float pi =
		3.14159265358979323846f;

	bool hasPlayerShot(
		const std::vector<neon::GameEvent>& events,
		std::uint64_t tick,
		neon::EntityId subjectId
	) {
		for (const neon::GameEvent& event : events) {
			if (event.tick == tick &&
				event.type ==
				neon::GameEventType::PlayerShot &&
				event.sourceKind ==
				neon::GameEntityKind::Player &&
				event.sourceId == subjectId) {

				return true;
			}
		}

		return false;
	}

	float angleBetweenDegrees(
		neon::Vec2 first,
		neon::Vec2 second
	) {
		const float firstLength =
			std::hypot(first.x, first.y);

		const float secondLength =
			std::hypot(second.x, second.y);

		constexpr float epsilon =
			0.000001f;

		if (!std::isfinite(firstLength) ||
			!std::isfinite(secondLength) ||
			firstLength <= epsilon ||
			secondLength <= epsilon) {

			return -1.0f;
		}

		const float dot =
			first.x * second.x +
			first.y * second.y;

		const float denominator =
			firstLength * secondLength;

		if (!std::isfinite(dot) ||
			!std::isfinite(denominator) ||
			denominator <= epsilon) {

			return -1.0f;
		}

		const float cosine =
			std::clamp(
				dot / denominator,
				-1.0f,
				1.0f
			);

		if (!std::isfinite(cosine)) {
			return -1.0f;
		}

		return std::acos(cosine) *
			180.0f / pi;
	}

	float validTolerance(
		float value
	) {
		if (!std::isfinite(value)) {
			return 1.0f;
		}

		return std::clamp(
			value,
			0.0f,
			180.0f
		);
	}

}//namespace

namespace neon {

	EspDetector::EspDetector(
		EntityId subjectId,
		float angleToleranceDegrees,
		std::uint32_t requiredSamples)
		: subjectId_(subjectId),
		angleToleranceDegrees_(
			validTolerance(angleToleranceDegrees)),
		requiredSamples_(
			std::max(1u, requiredSamples)) {
	}

	std::optional<DetectionEvent>
		EspDetector::observe(
			const ReplayFrame& frame,
			Vec2 playerCenter,
			Vec2 targetCenter,
			bool hasLineOfSight,
			const std::vector<GameEvent>& events
		) {
		const bool shotObserved =
			hasPlayerShot(
				events,
				frame.tick,
				subjectId_
			);

		if (!shotObserved) {
			// ��ס����������ȴʱ��������������
			// ���Ŀ�����¿ɼ����������ǰ����Ŀ��������С�
			if (!frame.command.fireHeld ||
				hasLineOfSight) {

				reset();
			}

			return std::nullopt;
		}

		// ʵ�����ʱ������Ȼ���ڳ�������״̬��
		// ���򽻸� AutoFireDetector �����
		if (!frame.command.fireHeld ||
			hasLineOfSight) {

			reset();
			return std::nullopt;
		}

		const Vec2 aimDirection{
			frame.command.aimPosition.x -
				playerCenter.x,
			frame.command.aimPosition.y -
				playerCenter.y
		};

		const Vec2 targetDirection{
			targetCenter.x -
				playerCenter.x,
			targetCenter.y -
				playerCenter.y
		};

		const float angleError =
			angleBetweenDegrees(
				aimDirection,
				targetDirection
			);

		if (angleError < 0.0f ||
			angleError > angleToleranceDegrees_) {

			reset();
			return std::nullopt;
		}

		if (suspiciousSamples_ <
			requiredSamples_) {

			++suspiciousSamples_;
		}

		if (reportedForStreak_ ||
			suspiciousSamples_ <
			requiredSamples_) {

			return std::nullopt;
		}

		DetectionEvent detection{};
		detection.tick =
			frame.tick;

		detection.type =
			DetectionType::EspAnomaly;

		detection.subjectId =
			subjectId_;

		// ������Ϊ֤�ݣ����Ǿ���֤������˲�ʹ�� 1.0��
		detection.confidence =
			0.85f;

		detection.evidence.observedValue =
			angleError;

		detection.evidence.expectedValue =
			angleToleranceDegrees_;

		detection.evidence.deviation =
			angleError -
			angleToleranceDegrees_;

		detection.evidence.sampleCount =
			suspiciousSamples_;

		detection.evidence.position =
			targetCenter;

		reportedForStreak_ =
			true;

		return detection;
	}

	void EspDetector::reset() {
		suspiciousSamples_ =
			0u;

		reportedForStreak_ =
			false;
	}

}//namespace neon