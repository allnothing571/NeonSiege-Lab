#include "core/AutoDodgeDetector.h"

#include <algorithm>
#include <cmath>

namespace {

	constexpr float epsilon =
		0.000001f;

	bool finiteVector(
		const neon::Vec2& value
	) {
		return
			std::isfinite(value.x) &&
			std::isfinite(value.y);
	}

	float vectorLength(
		const neon::Vec2& value
	) {
		return std::hypot(
			value.x,
			value.y
		);
	}

	bool normalize(
		neon::Vec2& value
	) {
		const float length =
			vectorLength(value);

		if (!std::isfinite(length) ||
			length <= epsilon) {

			return false;
		}

		value.x /= length;
		value.y /= length;

		return true;
	}

	bool findEnemyShot(
		const std::vector<neon::GameEvent>& events,
		std::uint64_t tick,
		neon::Vec2& shotPosition
	) {
		for (const neon::GameEvent& event : events) {
			if (event.tick != tick ||
				event.type !=
				neon::GameEventType::EnemyShot ||
				event.sourceKind !=
				neon::GameEntityKind::Enemy ||
				event.sourceId ==
				neon::invalidEntityId) {

				continue;
			}

			shotPosition =
				finiteVector(event.position)
				? event.position
				: neon::Vec2{};

			return true;
		}

		return false;
	}

	float validNonNegative(
		float value,
		float fallback
	) {
		if (!std::isfinite(value) ||
			value < 0.0f) {

			return fallback;
		}

		return value;
	}

	float validLateralComponent(
		float value
	) {
		if (!std::isfinite(value)) {
			return 0.8f;
		}

		return std::clamp(
			value,
			0.0f,
			1.0f
		);
	}

}//namespace

namespace neon {

	AutoDodgeDetector::AutoDodgeDetector(
		EntityId subjectId,
		float maxReactionTime,
		float maxThreatTime,
		float minimumLateralComponent,
		std::uint32_t requiredSamples)
		: subjectId_(subjectId),
		maxReactionTime_(
			validNonNegative(
				maxReactionTime,
				0.15f
			)),
		maxThreatTime_(
			validNonNegative(
				maxThreatTime,
				0.35f
			)),
		minimumLateralComponent_(
			validLateralComponent(
				minimumLateralComponent
			)),
		requiredSamples_(
			std::max(1u, requiredSamples)) {
	}

	std::optional<DetectionEvent>
		AutoDodgeDetector::observe(
			const ReplayFrame& frame,
			Vec2 projectileVelocity,
			float timeToImpact,
			float fixedDt,
			const std::vector<GameEvent>& events
		) {
		const Vec2 currentMovement =
			frame.command.movement;

		if (!finiteVector(currentMovement) ||
			!finiteVector(projectileVelocity) ||
			!std::isfinite(timeToImpact) ||
			!std::isfinite(fixedDt) ||
			fixedDt <= 0.0f) {

			reset();
			return std::nullopt;
		}

		const float currentMovementLength =
			vectorLength(currentMovement);

		const float previousMovementLength =
			hasPreviousMovement_
			? vectorLength(previousMovement_)
			: 0.0f;

		const bool movementStarted =
			currentMovementLength > epsilon &&
			previousMovementLength <= epsilon;

		Vec2 shotPosition{};
		const bool enemyShotObserved =
			findEnemyShot(
				events,
				frame.tick,
				shotPosition
			);

		if (enemyShotObserved) {
			Vec2 projectileDirection =
				projectileVelocity;

			if (!normalize(projectileDirection)) {
				reset();
				return std::nullopt;
			}

			pendingThreat_ =
				true;

			threatStartTick_ =
				frame.tick;

			threatDirection_ =
				projectileDirection;

			threatPosition_ =
				shotPosition;
		}

		std::optional<DetectionEvent> detection;

		if (pendingThreat_) {
			const std::uint64_t elapsedTicks =
				frame.tick >= threatStartTick_
				? frame.tick - threatStartTick_
				: 0u;

			const float reactionTime =
				static_cast<float>(elapsedTicks) *
				fixedDt;

			if (!std::isfinite(reactionTime)) {
				pendingThreat_ =
					false;

				resetStreak();
			}
			else if (
				movementStarted &&
				reactionTime <= maxReactionTime_ &&
				timeToImpact >= 0.0f &&
				timeToImpact <= maxThreatTime_) {

				Vec2 movementDirection =
					currentMovement;

				if (!normalize(movementDirection)) {
					pendingThreat_ =
						false;

					resetStreak();
				}
				else {
					const float alignment =
						std::fabs(
							movementDirection.x *
							threatDirection_.x +
							movementDirection.y *
							threatDirection_.y
						);

					const float clampedAlignment =
						std::clamp(
							alignment,
							0.0f,
							1.0f
						);

					const float lateralComponent =
						std::sqrt(
							std::max(
								0.0f,
								1.0f -
								clampedAlignment *
								clampedAlignment
							)
						);

					pendingThreat_ =
						false;

					if (lateralComponent <
						minimumLateralComponent_) {

						resetStreak();
					}
					else {
						if (suspiciousSamples_ <
							requiredSamples_) {

							++suspiciousSamples_;
						}

						if (!reportedForStreak_ &&
							suspiciousSamples_ >=
							requiredSamples_) {

							const float reactionScore =
								maxReactionTime_ > epsilon
								? std::clamp(
									1.0f -
									reactionTime /
									maxReactionTime_,
									0.0f,
									1.0f
								)
								: 1.0f;

							const float lateralScore =
								minimumLateralComponent_ <
								1.0f
								? std::clamp(
									(lateralComponent -
										minimumLateralComponent_) /
									(1.0f -
										minimumLateralComponent_),
									0.0f,
									1.0f
								)
								: 1.0f;

							DetectionEvent event{};
							event.tick =
								frame.tick;

							event.type =
								DetectionType::AutoDodge;

							event.subjectId =
								subjectId_;

							// ������Ϊ֤�ݣ����Ǿ���֤����
							event.confidence =
								std::clamp(
									(reactionScore +
										lateralScore) *
									0.5f,
									0.0f,
									1.0f
								);

							event.evidence.observedValue =
								reactionTime;

							event.evidence.expectedValue =
								maxReactionTime_;

							event.evidence.deviation =
								maxReactionTime_ -
								reactionTime;

							event.evidence.sampleCount =
								suspiciousSamples_;

							event.evidence.position =
								threatPosition_;

							reportedForStreak_ =
								true;

							detection =
								event;
						}
					}
				}
			}
			else if (
				reactionTime >
				maxReactionTime_) {

				pendingThreat_ =
					false;

				resetStreak();
			}
		}

		previousMovement_ =
			currentMovement;

		hasPreviousMovement_ =
			true;

		return detection;
	}

	void AutoDodgeDetector::resetStreak() {
		suspiciousSamples_ =
			0u;

		reportedForStreak_ =
			false;
	}

	void AutoDodgeDetector::reset() {
		resetStreak();

		pendingThreat_ =
			false;

		threatStartTick_ =
			0u;

		threatDirection_ =
			Vec2{};

		threatPosition_ =
			Vec2{};

		previousMovement_ =
			Vec2{};

		hasPreviousMovement_ =
			false;
	}

}//namespace neon