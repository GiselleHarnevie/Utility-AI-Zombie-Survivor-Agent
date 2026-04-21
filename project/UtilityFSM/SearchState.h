#pragma once
#include "FiniteStateMachine.h"

class Seek;
class Wander;
class Flee;
class BlendedSteering;
class PrioritySteering;
class IExamInterface;

class SearchState : public FSMState
{
public:
	SearchState(IExamInterface* pInterface);
	~SearchState();

	// Inherited via FSMState
	void OnEnter(Blackboard* pBlackboard) override;

	void OnExit(Blackboard* pBlackboard) override;

	void Update(Blackboard* pBlackboard, float deltaTime) override;

	float CalculateScore(Blackboard* pBlackboard) const override;

	

private:
	std::unique_ptr<Seek> m_pSeek = nullptr;
	std::unique_ptr<Wander> m_pWander = nullptr;
	//std::unique_ptr<Flee> m_pFlee = nullptr;
	std::unique_ptr<BlendedSteering> m_pBlendedSteering = nullptr;
	//PrioritySteering* m_pPrioritySteering = nullptr;

	std::vector<Elite::Vector2> m_Waypoints;
	size_t m_CurrentWaypointIndex = 0;

	IExamInterface* m_pInterface = nullptr;

	float m_RunTimer = 0.f;
};