#pragma once
#include "FiniteStateMachine.h"
#include <memory>

class Seek;
class IExamInterface;

class PurgeState : public FSMState
{
public:
	PurgeState(IExamInterface* pInterface);
	~PurgeState() = default;

	void OnEnter(Blackboard* pBlackboard) override;
	void OnExit(Blackboard* pBlackboard) override;
	void Update(Blackboard* pBlackboard, float deltaTime) override;
	float CalculateScore(Blackboard* pBlackboard) const override;

private:
	IExamInterface* m_pInterface = nullptr;
	std::unique_ptr<Seek> m_pSeek;
};

