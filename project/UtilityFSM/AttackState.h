#pragma once
#include "FiniteStateMachine.h"

class Seek;
class Flee;
class Face;
class BlendedSteering;
class IExamInterface;

class AttackState : public FSMState
{
public:
	AttackState(IExamInterface* pInterface);
	~AttackState() = default;

	// Inherited via FSMState
	void OnEnter(Blackboard* pBlackboard) override;

	void OnExit(Blackboard* pBlackboard) override;

	void Update(Blackboard* pBlackboard, float deltaTime) override;

	float CalculateScore(Blackboard* pBlackboard) const override;

private:
	std::unique_ptr<Seek> m_pSeek;
	std::unique_ptr<Face> m_pFace;
	std::unique_ptr<BlendedSteering> m_pBlendedSteering;
	IExamInterface* m_pInterface;

	float m_ShootCooldown = 0.f; // Cooldown timer in seconds
	const float m_ShootInterval = 0.7f; // Minimum time between shots
};