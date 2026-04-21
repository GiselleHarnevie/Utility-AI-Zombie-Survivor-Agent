#pragma once
#include "FiniteStateMachine.h"

class Flee;
class Seek;

class FleeState : public FSMState
{
public:
	FleeState();
	~FleeState() = default;

	// Inherited via FSMState
	void OnEnter(Blackboard* pBlackboard) override;

	void OnExit(Blackboard* pBlackboard) override;

	void Update(Blackboard* pBlackboard, float deltaTime) override;

	float CalculateScore(Blackboard* pBlackboard) const override;
private:
	std::unique_ptr<Flee> m_pFlee;
	std::unique_ptr<Seek> m_pSeek;
};