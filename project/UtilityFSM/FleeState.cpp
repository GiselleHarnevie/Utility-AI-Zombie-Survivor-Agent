#include "../stdafx.h"
#include "FleeState.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "../Steering/SteeringBehaviors.h"
#include "../Steering/CombinedSteeringBehaviors.h"

FleeState::FleeState()
{
	m_pFlee = std::make_unique<Flee>();
	m_pSeek = std::make_unique<Seek>();
}


void FleeState::OnEnter(Blackboard* pBlackboard)
{
	std::cout << "OnEnter Flee state\n";

	//pBlackboard->ChangeData("AutoOrient", true);
}

void FleeState::OnExit(Blackboard* pBlackboard)
{
	std::cout << "OnExit Flee state\n";

	//pBlackboard->ChangeData("AutoOrient", false);
}

void FleeState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgentInfo agent;
	Elite::Vector2 houseTarget;
	std::vector<Elite::Vector2> zigzagPoints;
	unsigned int zigzagIndex = 0;

	pBlackboard->GetData("AgentInfo", agent);
	//pBlackboard->GetData("HouseNavTarget", houseTarget);
	pBlackboard->GetData("Waypoints", zigzagPoints);
	pBlackboard->GetData("CurrentWorldWaypointIndex", zigzagIndex);

	Elite::Vector2 target = Elite::Vector2{ 0.f,0.f };

	if (houseTarget != Elite::Vector2{ 0.f,0.f })
	{
		// Flee to the last known house point
		target = houseTarget;
	}
	else if (!zigzagPoints.empty())
	{
		target = zigzagPoints[zigzagIndex];

		if (Distance(agent.Position, target) < 5.f)
		{
			zigzagIndex = (zigzagIndex + 1) % static_cast<unsigned int>(zigzagPoints.size());
			target = zigzagPoints[zigzagIndex];
			pBlackboard->ChangeData("CurrentWorldWaypointIndex", zigzagIndex);
		}
	}

	m_pSeek->SetTarget(TargetData{ target });
	SteeringOutput steering = m_pSeek->CalculateSteering(deltaTime, &agent);
	pBlackboard->ChangeData("SteeringOutput", steering);
}

float FleeState::CalculateScore(Blackboard* pBlackboard) const
{
    float health = 0.f;
    pBlackboard->GetData("Health", health);

    if (health < 3.f) 
        return 1.0f;

    return 0.0f;
}
