#include "../stdafx.h"
#include "PlunderHouseState.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "../Steering/SteeringBehaviors.h"
#include "../Steering/CombinedSteeringBehaviors.h"
#include "IExamInterface.h"

PlunderHouseState::PlunderHouseState(IExamInterface* pInterface)
	: m_pInterface(pInterface)
{
	m_pSeek = std::make_unique<Seek>();
}

PlunderHouseState::~PlunderHouseState()
{
}

void PlunderHouseState::OnEnter(Blackboard* pBlackboard)
{
	std::cout << "OnEnter Plunder state\n";
	pBlackboard->ChangeData("IsRunning", false);
	
}

void PlunderHouseState::OnExit(Blackboard* pBlackboard)
{
	std::cout << "OnExit Plunder state\n";

}

void PlunderHouseState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgentInfo agentInfo{};
	std::vector<Elite::Vector2> houseWaypoints;
	unsigned int currentIndex = 0;
	 unsigned int waypointCycles = 0;

	pBlackboard->GetData("AgentInfo", agentInfo);
	pBlackboard->GetData("HouseWaypoints", houseWaypoints);
	pBlackboard->GetData("CurrentHouseWaypointIndex", currentIndex);

	std::vector<HouseInfo> houses;
	std::vector<Elite::Vector2> plundered;
	pBlackboard->GetData("HousesInFOV", houses);
	pBlackboard->GetData("PlunderedHouses", plundered);

	for (const auto& house : houses)
	{
		if (std::find(plundered.begin(), plundered.end(), house.Center) == plundered.end())
		{
			float now = static_cast<float>(clock()) / CLOCKS_PER_SEC;
			pBlackboard->ChangeData("LastUnplunderedHouseSeenTime", now);
			break;
		}
	}

	/*if (houseWaypoints.empty())
		return;*/

	Elite::Vector2 target = houseWaypoints[currentIndex];
	bool isThereItemTarget{};
	bool isInGrabbingDistance{};
	TargetData itemTarget{};
	pBlackboard->GetData("IsThereItemTarget", isThereItemTarget);
	pBlackboard->GetData("IsInGrabbingDistance", isInGrabbingDistance);
	pBlackboard->GetData("ItemTarget", itemTarget);
	pBlackboard->GetData("HouseWaypointCycles", waypointCycles);

	if (isThereItemTarget)
	{
		//std::cout << "Seeking item in plunder" << std::endl;
		m_pSeek->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(itemTarget.Position));
	}
	m_pSeek->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(target));


	// mark as plundered if all waypoints has been visited twice
	if (Distance(agentInfo.Position, target) < 0.3f)
	{
		std::cout << "Plunder: Waypoint " << currentIndex << " reached" << std::endl;
		currentIndex = (currentIndex + 1) % static_cast<unsigned int>(houseWaypoints.size());
		pBlackboard->ChangeData("CurrentHouseWaypointIndex", currentIndex);

		// Increment cycle count if we complete a full cycle
		if (currentIndex == 0)
		{
			waypointCycles++;
			pBlackboard->ChangeData("HouseWaypointCycles", waypointCycles);

			// Mark house as visited after two cycles
			if (waypointCycles >= 2)
			{
				std::vector<Elite::Vector2> plunderedHouses;
				HouseInfo currentHouse;
				if (pBlackboard->GetData("CurrentHouse", currentHouse) &&
					pBlackboard->GetData("PlunderedHouses", plunderedHouses))
				{
					if (std::find(plunderedHouses.begin(), plunderedHouses.end(), currentHouse.Center) == plunderedHouses.end())
					{
						plunderedHouses.push_back(currentHouse.Center);
						pBlackboard->ChangeData("PlunderedHouses", plunderedHouses);
						std::cout << "House marked as plundered: " << currentHouse.Center.x << ", " << currentHouse.Center.y << std::endl;
					}
				}
			}
		}
	}

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

	SteeringOutput steering = m_pSeek->CalculateSteering(deltaTime, &agentInfo);
	pBlackboard->ChangeData("SteeringOutput", steering);
}

float PlunderHouseState::CalculateScore(Blackboard* pBlackboard) const
{
	static float lastScore = -1.0f;

	std::vector<HouseInfo> houses;
	std::vector<Elite::Vector2> plundered;
	pBlackboard->GetData("HousesInFOV", houses);
	pBlackboard->GetData("PlunderedHouses", plundered);

	if (houses.empty())
		return 0.f;

	float lastSeen = -1000.f;
	pBlackboard->GetData("LastUnplunderedHouseSeenTime", lastSeen);
	float now = static_cast<float>(clock()) / CLOCKS_PER_SEC;

	// If we saw an unplundered house recently, stay in plunder state
	if (now - lastSeen < 2.0f) // 2 seconds sticky time
		return 1.0f;
	
	for (const HouseInfo& house : houses)
	{
		// If the house is not plundered, give it high priority
		if (std::find(plundered.begin(), plundered.end(), house.Center) == plundered.end())
		{
			return 1.0f; // High priority for unvisited houses
		}
	}


	return 0.0f;
}
