#include "stdafx.h"
#include "SurvivorAgentBrain.h"
#include "UtilityFSM/FiniteStateMachine.h"
#include "UtilityFSM/SearchState.h"
#include "UtilityFSM/AttackState.h"
#include "UtilityFSM/PurgeState.h"
#include "UtilityFSM/PlunderHouseState.h"
#include "Blackboard.h"
#include "Steering/SteeringBehaviors.h"
#include "UtilityFSM/UtilityEvaluator.h"
#include "UtilityActions/UtilityAction.h"
#include "UtilityActions/PickBestItemAction.h"
#include "UtilityActions/UseBestItemAction.h"
#include "IExamInterface.h"

SurvivorAgentBrain::SurvivorAgentBrain(IExamInterface* pExamInterface)
	: m_pInterface(pExamInterface)
{

}

SurvivorAgentBrain::~SurvivorAgentBrain()
{

}


void SurvivorAgentBrain::Initialize()
{
	//blackboard
	FillBlackboard();

	//utility state evaluator 
	m_pUtilityEvaluator = std::make_unique<UtilityEvaluator>();

	//utility action
	m_UtilityActions.emplace_back(std::make_unique<PickBestItemAction>(m_pInterface));
	m_UtilityActions.emplace_back(std::make_unique<UseBestItemAction>(m_pInterface));

	//states
	m_pSearchState = std::make_unique<SearchState>(m_pInterface);
	m_pAttackState = std::make_unique<AttackState>(m_pInterface);
	m_pPlunderHouseState = std::make_unique<PlunderHouseState>(m_pInterface);
	m_pPurgeState = std::make_unique<PurgeState>(m_pInterface);

	//fsm
	m_pFSM = std::make_unique<FiniteStateMachine>(m_pSearchState.get(), m_pBlackboard.get());

	//utility eval register states
	m_pUtilityEvaluator->AddState(m_pSearchState.get());
	m_pUtilityEvaluator->AddState(m_pAttackState.get());
	m_pUtilityEvaluator->AddState(m_pPurgeState.get());
	m_pUtilityEvaluator->AddState(m_pPlunderHouseState.get());

}


void SurvivorAgentBrain::FillBlackboard()
{
	///blackboard list

	//make blackboard and fill initialdata
	m_pBlackboard = std::make_unique<Blackboard>();

	//AGENT STATS
	m_pBlackboard->AddData("AgentInfo", AgentInfo{});
	m_pBlackboard->AddData("Health", 10.f);
	m_pBlackboard->AddData("Stamina", 10.f);
	m_pBlackboard->AddData("Energy", 10.f);

	//FOR STEERING
	m_pBlackboard->AddData("SteeringOutput", SteeringOutput{});
	m_pBlackboard->AddData("AutoOrient", true);
	m_pBlackboard->AddData("IsRunning", false);

	//WORLD N ZIGZAG POINTS 
	WorldInfo worldInfo = m_pInterface->World_GetInfo();

	//smaller square area and in this order
	float fullWidth = worldInfo.Dimensions.x / 4.0;
	float fullHeight = worldInfo.Dimensions.y / 4.0;
	Elite::Vector2 bottomleft = { worldInfo.Center.x - fullWidth, (worldInfo.Center.y) - fullHeight };
	Elite::Vector2 topLeft = { worldInfo.Center.x - fullWidth, (worldInfo.Center.y) + fullHeight };
	Elite::Vector2 topRight = { worldInfo.Center.x + fullWidth, (worldInfo.Center.y) + fullHeight };
	Elite::Vector2 bottomRight = { worldInfo.Center.x + fullWidth, (worldInfo.Center.y) - fullHeight };

	std::vector<Elite::Vector2> worldpoints{ bottomleft, topLeft, topRight, bottomRight };
	m_pBlackboard->AddData("WorldSquare", worldpoints);

	float step = 65.0f;
	std::vector<Elite::Vector2> zigZagPoints{
			bottomleft,
			Elite::Vector2{topLeft.x + step, topLeft.y},
			Elite::Vector2{bottomleft.x + step * 2, bottomleft.y},
			Elite::Vector2{topLeft.x + step * 3, topLeft.y},
			Elite::Vector2{bottomleft.x + step * 4, bottomleft.y},
			Elite::Vector2{topLeft.x + step * 5, topLeft.y},
			Elite::Vector2{bottomleft.x + step * 6, bottomleft.y},
			Elite::Vector2{topLeft.x + step * 6.1f, topLeft.y}
	};

	m_pBlackboard->AddData("Waypoints", zigZagPoints);
	m_pBlackboard->AddData("CurrentWorldWaypointIndex", 0u);

	//HOUSE
	m_pBlackboard->AddData("HousesInFOV", m_pInterface->GetHousesInFOV());
	m_pBlackboard->AddData("CurrentHouse", HouseInfo{});
	m_pBlackboard->AddData("HouseWaypoints", std::vector<Elite::Vector2>{});
	m_pBlackboard->AddData("CurrentHouseWaypointIndex", 0u);
	m_pBlackboard->AddData("DiscoveredHouses", std::vector<Elite::Vector2>{});
	m_pBlackboard->AddData("PlunderedHouses", std::vector<Elite::Vector2>{});
	m_pBlackboard->AddData("WasInHouse", false);
	m_pBlackboard->AddData("HouseWaypointCycles", 0u);
	m_pBlackboard->AddData("LastUnplunderedHouseSeenTime", -1000.f);


	//COMBAT & FLEE
	m_pBlackboard->AddData("EnemiesInFOV", std::vector<EnemyInfo>{});
	m_pBlackboard->AddData("LastEnemySeenTime", float{});
	m_pBlackboard->AddData("LastEnemyPosition", Elite::Vector2{});
	m_pBlackboard->AddData("EnemySeenRecently", false);

	m_pBlackboard->AddData("ClosestEnemy", EnemyInfo{});
	m_pBlackboard->AddData("ClosestEnemyDistance", 0.f);
	m_pBlackboard->AddData("LastBittenTime", 0.f); 


	//PURGE
	m_pBlackboard->AddData("PurgeZonesInFOV", std::vector<PurgeZoneInfo>{});

	//INVENTORY
	m_pBlackboard->AddData("ItemsInFOV", std::vector<ItemInfo>{});
	m_pBlackboard->AddData("Inventory", std::vector<ItemInfo>(5));
	m_pBlackboard->AddData("InventorySlotIndex", 0u);

	m_pBlackboard->AddData("MedkitCount", 0u);
	m_pBlackboard->AddData("FoodCount", 0u);
	m_pBlackboard->AddData("HasGarbage", false);
	m_pBlackboard->AddData("WeaponCount", 0u);

	m_pBlackboard->AddData("ItemTarget", TargetData{});
	m_pBlackboard->AddData("IsThereItemTarget", false);
	m_pBlackboard->AddData("IsInGrabbingDistance", false);


	m_pBlackboard->PrintBlackboard();
}

void SurvivorAgentBrain::UpdateBlackboard(float deltaTime, AgentInfo& agentInfo)
{
	// change Blackboard
	m_pBlackboard->ChangeData("AgentInfo", agentInfo);
	m_pBlackboard->ChangeData("Health", agentInfo.Health);
	m_pBlackboard->ChangeData("Stamina", agentInfo.Stamina);
	m_pBlackboard->ChangeData("Energy", agentInfo.Energy);

	auto housesInFOV = m_pInterface->GetHousesInFOV();
	m_pBlackboard->ChangeData("HousesInFOV", housesInFOV);

	// Track discovered houses
	std::vector<Elite::Vector2> discovered;
	m_pBlackboard->GetData("DiscoveredHouses", discovered);
	for (const auto& h : housesInFOV)
	{
		if (std::find(discovered.begin(), discovered.end(), h.Center) == discovered.end())
			discovered.push_back(h.Center);
	}
	m_pBlackboard->ChangeData("DiscoveredHouses", discovered);

	// Update items in FOV
	m_pBlackboard->ChangeData("ItemsInFOV", m_pInterface->GetItemsInFOV());

	// mitgh delete track if agent just left a house to mark it as plundered
	bool wasInHouse = false;
	m_pBlackboard->GetData("WasInHouse", wasInHouse);
	if (wasInHouse && !agentInfo.IsInHouse)
	{
		HouseInfo currentHouse;
		if (m_pBlackboard->GetData("CurrentHouse", currentHouse))
		{
			std::vector<Elite::Vector2> plundered;
			m_pBlackboard->GetData("PlunderedHouses", plundered);
			if (std::find(plundered.begin(), plundered.end(), currentHouse.Center) == plundered.end())
			{
				plundered.push_back(currentHouse.Center);
				m_pBlackboard->ChangeData("PlunderedHouses", plundered);
				std::cout << "House plundered (on exit): " << currentHouse.Center.x << ", " << currentHouse.Center.y << std::endl;
			}
		}
	}
	m_pBlackboard->ChangeData("WasInHouse", agentInfo.IsInHouse);

	std::vector<Elite::Vector2> plundered;
	m_pBlackboard->GetData("PlunderedHouses", plundered);
	if (plundered.size() >= 7)
	{
		
		//Elite::Vector2 lastPlunderedHouse = plundered[plundered.size() - 1];
		m_pBlackboard->ChangeData("PlunderedHouses", std::vector<Elite::Vector2>{});
		/*plundered.push_back(lastPlunderedHouse);
		m_pBlackboard->ChangeData("PlunderedHouses", plundered);*/
		std::cout << "Cleared Plundered Houses" << std::endl;

	}

	//enemy
	std::vector<EnemyInfo> enemies = m_pInterface->GetEnemiesInFOV();
	m_pBlackboard->ChangeData("EnemiesInFOV", enemies);

	float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
	float lastSeenTime = -FLT_MAX;
	m_pBlackboard->GetData("LastEnemySeenTime", lastSeenTime);

	Elite::Vector2 lastEnemyPos{};
	m_pBlackboard->GetData("LastEnemyPosition", lastEnemyPos);

	float closestEnemyDist = 999.f;
	EnemyInfo closestEnemy = GetClosestEnemy(agentInfo, closestEnemyDist);
	m_pBlackboard->ChangeData("ClosestEnemy", closestEnemy);
	m_pBlackboard->ChangeData("ClosestEnemyDistance", closestEnemyDist);

	// Update bite tracking - if agent was bitten, update the last bitten time
	if (agentInfo.Bitten || agentInfo.WasBitten) {
		float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
		m_pBlackboard->ChangeData("LastBittenTime", currentTime);

		// Print debug info when bitten
		//std::cout << "Agent was bitten! Setting LastBittenTime: " << currentTime << std::endl;
	}

	m_pBlackboard->ChangeData("PurgeZonesInFOV", m_pInterface->GetPurgeZonesInFOV());

}

EnemyInfo SurvivorAgentBrain::GetClosestEnemy(const AgentInfo& agentInfo, float& outDistance)
{
	std::vector<EnemyInfo> enemies = m_pInterface->GetEnemiesInFOV();
	EnemyInfo closestEnemy{};
	float minDist = FLT_MAX;

	for (const auto& enemy : enemies)
	{
		float dist = (agentInfo.Position - enemy.Location).Magnitude();
		if (dist < minDist)
		{
			minDist = dist;
			closestEnemy = enemy;
		}
	}
	outDistance = (minDist != FLT_MAX) ? minDist : FLT_MAX;
	return closestEnemy;
}

std::vector<Elite::Vector2> SurvivorAgentBrain::MakeHouseWaypoints(const HouseInfo& house)
{
	float stepX = house.Size.x * 0.35f;
	float stepY = house.Size.y * 0.2f;

	Elite::Vector2 topLeft = { house.Center.x - stepX, house.Center.y + stepY };
	Elite::Vector2 topRight = { house.Center.x + stepX, house.Center.y + stepY };
	Elite::Vector2 bottomRight = { house.Center.x + stepX, house.Center.y - stepY };
	Elite::Vector2 bottomLeft = { house.Center.x - stepX, house.Center.y - stepY };

	return { bottomLeft, topLeft, topRight, bottomRight };
}


SteeringOutput SurvivorAgentBrain::Update(float deltaTime, AgentInfo& agentInfo)
{

	UpdateBlackboard(deltaTime, agentInfo);

	std::vector<HouseInfo> housesInFOV;
	std::vector<Elite::Vector2> plunderedHouses;
	HouseInfo currentHouse;
	m_pBlackboard->GetData("HousesInFOV", housesInFOV);
	m_pBlackboard->GetData("PlunderedHouses", plunderedHouses);
	m_pBlackboard->GetData("CurrentHouse", currentHouse);

	// Only pick a new house if not already plundering one
	bool isInHouse = agentInfo.IsInHouse;
	if (!isInHouse)
	{
		float minDist = FLT_MAX;
		const HouseInfo* closestHouse = nullptr;
		for (const auto& h : housesInFOV)
		{
			if (std::find(plunderedHouses.begin(), plunderedHouses.end(), h.Center) == plunderedHouses.end())
			{
				float dist = Distance(agentInfo.Position, h.Center);
				if (dist < minDist)
				{
					minDist = dist;
					closestHouse = &h;
				}
			}
		}
		if (closestHouse)
		{
			m_pBlackboard->ChangeData("CurrentHouse", *closestHouse);
			m_pBlackboard->ChangeData("HouseWaypoints", MakeHouseWaypoints(*closestHouse));
			m_pBlackboard->ChangeData("CurrentHouseWaypointIndex", 0u);
		}
	}

	//Run the best utility action
	float bestScore = 0.f;
	UtilityAction* bestAction = nullptr;
	for (auto& action : m_UtilityActions)
	{
		float score = action->CalculateUtility(m_pBlackboard.get());
		if (score > bestScore)
		{
			bestScore = score;
			bestAction = action.get();
		}
	}
	if (bestAction)
	{
		bestAction->Execute(m_pBlackboard.get(), deltaTime);
	}


	//evaluate the best state
	FSMState* pBestState = m_pUtilityEvaluator->GetBestStateScore(m_pBlackboard.get());

	//transition if needed
	if (pBestState && m_pFSM->GetCurrentState() != pBestState)
	{
		std::cout << "State change: " << typeid(*pBestState).name() << std::endl;
		m_pFSM->ChangeState(pBestState);
	}

	m_WaypointSkipTimer += deltaTime;
	if (m_WaypointSkipTimer >= m_WaypointSkipInterval)
	{
		std::cout << "SURVIVED >120 SECONDS" << std::endl;
		unsigned int currentIndex = 0;
		std::vector<Elite::Vector2> waypoints;
		m_pBlackboard->GetData("CurrentWorldWaypointIndex", currentIndex);
		m_pBlackboard->GetData("Waypoints", waypoints);

		if (!waypoints.empty())
		{
			currentIndex = (currentIndex + 2) % static_cast<unsigned int>(waypoints.size());
			m_pBlackboard->ChangeData("CurrentWorldWaypointIndex", currentIndex);
			std::cout << "SKIPPED 2 WAYPOINT New index: " << currentIndex << std::endl;
		}
		m_WaypointSkipTimer = 0.f;
	}

	// 
	m_pFSM->Update(deltaTime);

	//get steering from blackboard
	SteeringOutput steering;
	m_pBlackboard->GetData("SteeringOutput", steering);

	return steering;
}

void SurvivorAgentBrain::Render(float dt) const
{
	//for world area and zig zag way point render
	std::vector<Elite::Vector2> worldSquare{};
	m_pBlackboard->GetData("WorldSquare", worldSquare);
	m_pInterface->Draw_Polygon(worldSquare.data(), worldSquare.size(), Elite::Vector3{ 0.f,1.f,0.f });

	std::vector<Elite::Vector2> zigZagPoints{};
	m_pBlackboard->GetData("Waypoints", zigZagPoints);
	m_pInterface->Draw_Polygon(zigZagPoints.data(), zigZagPoints.size(), Elite::Vector3{ 0.7f,0.7f,0.7f });

	std::vector<Elite::Vector2> houseWayPoints{};
	m_pBlackboard->GetData("HouseWaypoints", houseWayPoints);
	if (!houseWayPoints.empty())
	{
		m_pInterface->Draw_Polygon(houseWayPoints.data(), houseWayPoints.size(), Elite::Vector3{ 0.f,0.f,1.f });
	}


	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, m_pInterface->Agent_GetInfo().GrabRange, { 1, 0, 0 });

}

Blackboard* const SurvivorAgentBrain::GetBlackboard() const
{
	return m_pBlackboard.get();
}
