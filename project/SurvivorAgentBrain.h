#pragma once
#include "Exam_HelperStructs.h"
#include "Steering/SteeringHelpers.h"

class IExamInterface;

class Blackboard;

class FiniteStateMachine;
class SearchState;
class AttackState;
class PurgeState;
class PlunderHouseState;

class UtilityEvaluator;
class UtilityAction;

class Wander;
class Seek;

class SurvivorAgentBrain
{
public:
	SurvivorAgentBrain(IExamInterface* pExamInterface);
	~SurvivorAgentBrain();

	void Initialize();
	SteeringOutput Update(float deltaTime, AgentInfo& agentInfo);
	void Render(float dt) const;

	Blackboard* const GetBlackboard() const;
private:
	//instances
	std::unique_ptr<FiniteStateMachine> m_pFSM = nullptr;
	std::unique_ptr<Blackboard> m_pBlackboard = nullptr;
	std::unique_ptr< UtilityEvaluator> m_pUtilityEvaluator = nullptr;
	std::vector<std::unique_ptr<UtilityAction>> m_UtilityActions;

	//AgentInfo* m_pAgentInfo = nullptr;
	//std::unique_ptr<AgentInfo> m_pAgentInfo = nullptr;

	std::unique_ptr<SearchState> m_pSearchState = nullptr;
	std::unique_ptr<AttackState> m_pAttackState = nullptr;
	std::unique_ptr<PurgeState> m_pPurgeState = nullptr;
	std::unique_ptr<PlunderHouseState> m_pPlunderHouseState = nullptr;

	void FillBlackboard();
	void UpdateBlackboard(float deltaTime, AgentInfo& agentInfo);
	

	std::vector<Elite::Vector2> MakeHouseWaypoints(const HouseInfo& house);

	EnemyInfo GetClosestEnemy(const AgentInfo& agentInfo, float& outDistance);


	IExamInterface* m_pInterface;

	float m_WaypointSkipTimer = 0.f;
	const float m_WaypointSkipInterval = 120.f;

};
