#pragma once
#include "FiniteStateMachine.h"
#include <memory>
#include <vector>

class IExamInterface;
class Seek;

class PlunderHouseState final : public FSMState
{
public:
	PlunderHouseState(IExamInterface* pInterface);
	~PlunderHouseState();


	// Inherited via FSMState
	void OnEnter(Blackboard* pBlackboard) override;

	void OnExit(Blackboard* pBlackboard) override;

	void Update(Blackboard* pBlackboard, float deltaTime) override;

	float CalculateScore(Blackboard* pBlackboard) const override;
private:
	std::unique_ptr<Seek> m_pSeek = nullptr;
	IExamInterface* m_pInterface = nullptr;
	float m_RunTimer = 0.f;
};
