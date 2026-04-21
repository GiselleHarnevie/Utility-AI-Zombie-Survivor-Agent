#include "../stdafx.h"
#include "SearchState.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "../Steering/SteeringBehaviors.h"
#include "../Steering/CombinedSteeringBehaviors.h"
#include "IExamInterface.h"


SearchState::SearchState(IExamInterface* pInterface)
	: m_pInterface(pInterface)
{
	m_pSeek = std::make_unique<Seek>();
	m_pWander = std::make_unique<Wander>();

	std::vector<BlendedSteering::WeightedBehavior> blendedBehaviors =
	{
		{ m_pSeek.get(), 0.5f },
		{ m_pWander.get(), 0.5f }
	};
	m_pBlendedSteering = std::make_unique<BlendedSteering>(blendedBehaviors);
	//m_pPrioritySteering = new PrioritySteering({ m_pFlee,m_pBlendedSteering });

}

SearchState::~SearchState()
{
	//SAFE_DELETE(m_pPrioritySteering);

}

void SearchState::OnEnter(Blackboard* pBlackboard)
{
	std::cout << "OnEnter Search state\n";
	pBlackboard->ChangeData("AutoOrient", true);
	pBlackboard->ChangeData("IsRunning", false);
}

void SearchState::OnExit(Blackboard* pBlackboard)
{
	std::cout << "OnExit Search state" << std::endl;
}

void SearchState::Update(Blackboard* pBlackboard, float deltaTime)
{
	//get date from blackbaord
	AgentInfo agentInfo{};
	std::vector<Elite::Vector2> waypoints{};
	unsigned int currentIndex{};

	pBlackboard->GetData("AgentInfo", agentInfo);
	pBlackboard->GetData("Waypoints", waypoints);
	pBlackboard->GetData("CurrentWorldWaypointIndex", currentIndex);
	pBlackboard->GetData("AgentInfo", agentInfo);



	m_RunTimer += deltaTime;
	if (m_RunTimer < 2.0f)
	{
		pBlackboard->ChangeData("IsRunning", true);
	}
	else if (m_RunTimer < 7.0f)
	{
		pBlackboard->ChangeData("IsRunning", false);
	}
	else
	{
		m_RunTimer = 0.f;
		pBlackboard->ChangeData("IsRunning", true);
	}

	//set and cycle waypoint targets
	Elite::Vector2 target = waypoints[currentIndex];
	m_pWander->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(target));
	m_pSeek->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(target));

	if (Distance(agentInfo.Position, target) < 40.0f)
	{
		std::cout << "way point reached" << std::endl;
		currentIndex = (currentIndex + 1) % static_cast<unsigned int>(waypoints.size());
		pBlackboard->ChangeData("CurrentWorldWaypointIndex", currentIndex);
	}



	SteeringOutput steering = m_pBlendedSteering->CalculateSteering(deltaTime, &agentInfo);
	pBlackboard->ChangeData("SteeringOutput", steering);
}

float SearchState::CalculateScore(Blackboard* pBlackboard) const
{
	std::vector<HouseInfo> houses;
	pBlackboard->GetData("HousesInFOV", houses);
	if (!houses.empty())
		return 0.f;

	float health{}, energy{};
	pBlackboard->GetData("Health", health);
	pBlackboard->GetData("Energy", energy);

	float healthNorm = health / 10.f;
	float energyNorm = energy / 10.f;

	//[0.3-0.9]
	float score = 0.3f + 0.3f * healthNorm + 0.3f * energyNorm;
	return score;
}
