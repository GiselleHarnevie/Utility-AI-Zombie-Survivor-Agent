#include "../stdafx.h"
#include "AttackState.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include "../Steering/SteeringBehaviors.h"
#include "../Steering/CombinedSteeringBehaviors.h"

AttackState::AttackState(IExamInterface* pInterface)
	:m_pInterface(pInterface)
{
	m_pSeek = std::make_unique<Seek>();
	m_pFace = std::make_unique<Face>();

	std::vector<BlendedSteering::WeightedBehavior> blendedBehaviors =
	{
		{ m_pSeek.get(), 10.0f },
		{ m_pFace.get(), 0.5f }
	};
	m_pBlendedSteering = std::make_unique<BlendedSteering>(blendedBehaviors);
}

void AttackState::OnEnter(Blackboard* pBlackboard)
{
	std::cout << "OnEnter Attack state\n";
	pBlackboard->ChangeData("AutoOrient", false);
	pBlackboard->ChangeData("IsRunning", true);

	// Store entry time to enforce minimum duration
	pBlackboard->ChangeData("AttackStateEntryTime", static_cast<float>(clock()) / CLOCKS_PER_SEC);

	// Set bite response timer if bitten
	AgentInfo agentInfo{};
	pBlackboard->GetData("AgentInfo", agentInfo);
	if (agentInfo.Bitten || agentInfo.WasBitten) {
		pBlackboard->ChangeData("LastBittenTime", static_cast<float>(clock()) / CLOCKS_PER_SEC);
	}
}

void AttackState::OnExit(Blackboard* pBlackboard)
{
	std::cout << "OnExit Attack state\n";
	pBlackboard->ChangeData("AutoOrient", true);

}

void AttackState::Update(Blackboard* pBlackboard, float deltaTime)
{
	AgentInfo agentInfo{};
	EnemyInfo closestEnemy{};
	float closestDist{};
	unsigned int weaponCount = 0;

	pBlackboard->GetData("AgentInfo", agentInfo);
	pBlackboard->GetData("ClosestEnemy", closestEnemy);
	pBlackboard->GetData("ClosestEnemyDistance", closestDist);
	pBlackboard->GetData("WeaponCount", weaponCount);

	/*if (weaponCount == 0)
		return;*/

	// --- Ensure agent stays in attack state and faces enemy after being bitten ---
	float lastBittenTime = 0.f;
	float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
	pBlackboard->GetData("LastBittenTime", lastBittenTime);
	bool recentlyBitten = (currentTime - lastBittenTime < 4.0f);

	// Always face the closest enemy if recently bitten, even if not in range
	if (recentlyBitten && closestEnemy.Health > 0.f) {
		m_pFace->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(closestEnemy.Location));
	}
	//m_pFace->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(closestEnemy.Location));


	// Get zig-zag waypoints and current index
	std::vector<Elite::Vector2> waypoints;
	unsigned int currentIndex = 0;
	pBlackboard->GetData("Waypoints", waypoints);
	pBlackboard->GetData("CurrentWorldWaypointIndex", currentIndex);

	// Set Seek target to current waypoint
	if (!waypoints.empty()) {
		Elite::Vector2 target = waypoints[currentIndex];
		m_pSeek->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(target));

		// If close to waypoint, increment index (loop)
		if (Distance(agentInfo.Position, target) < 10.0f) {
			currentIndex = (currentIndex + 1) % static_cast<unsigned int>(waypoints.size());
			pBlackboard->ChangeData("CurrentWorldWaypointIndex", currentIndex);
		}
	}

	if (closestDist < 5.f)
		pBlackboard->ChangeData("IsRunning", true);
	else
		pBlackboard->ChangeData("IsRunning", false);


	// --- SHOOTING LOGIC WITH INVENTORY AND COOLDOWN ---
	m_ShootCooldown -= deltaTime;
	if (weaponCount > 0 && closestDist > 0.f && closestEnemy.Health > 0.f)
	{
		// Calculate angle to enemy
		Elite::Vector2 toEnemy = closestEnemy.Location - agentInfo.Position;
		float angleToEnemy = Elite::AngleBetween(
			Elite::Vector2{ cosf(agentInfo.Orientation), sinf(agentInfo.Orientation) },
			toEnemy
		);

		// Only shoot if facing enemy and cooldown expired
		if (fabsf(angleToEnemy) < Elite::ToRadians(10.0f) && m_ShootCooldown <= 0.f)
		{
			// Find weapon in inventory and use it
			UINT capacity = m_pInterface->Inventory_GetCapacity();
			for (UINT i = 0; i < capacity; ++i)
			{
				ItemInfo item;
				if (m_pInterface->Inventory_GetItem(i, item) &&
					(item.Type == eItemType::PISTOL || item.Type == eItemType::SHOTGUN))
				{
					if (m_pInterface->Inventory_UseItem(i))
					{
						m_ShootCooldown = m_ShootInterval;

						// Check if weapon is depleted and remove it
						ItemInfo checkItem;
						if (m_pInterface->Inventory_GetItem(i, checkItem) && checkItem.Value == 0)
						{
							m_pInterface->Inventory_RemoveItem(i);

							// Decrement weapon count in blackboard
							unsigned int weaponCountBB = 0;
							pBlackboard->GetData("WeaponCount", weaponCountBB);
							if (weaponCountBB > 0)
								pBlackboard->ChangeData("WeaponCount", weaponCountBB - 1);
						}
					}
					break;
				}
			}
		}
	}

	// Face the closest enemy
	m_pFace->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(closestEnemy.Location));

	// Calculate blended steering (Seek + Face)
	SteeringOutput steering = m_pBlendedSteering->CalculateSteering(deltaTime, &agentInfo);
	pBlackboard->ChangeData("SteeringOutput", steering);
}

float AttackState::CalculateScore(Blackboard* pBlackboard) const
{

	unsigned int weaponCount = 0;
	float closestDist = 0.f;
	AgentInfo agentInfo{};
	pBlackboard->GetData("WeaponCount", weaponCount);
	pBlackboard->GetData("ClosestEnemyDistance", closestDist);
	pBlackboard->GetData("AgentInfo", agentInfo);

	float lastBittenTime = 0.f;
	float currentTime = static_cast<float>(clock()) / CLOCKS_PER_SEC;
	pBlackboard->GetData("LastBittenTime", lastBittenTime);

	// Stay in attack state for 2 seconds after being bitten
	bool recentlyBitten = (currentTime - lastBittenTime < 2.0f);

	if (weaponCount == 0)
		return 0.0f;

	if (agentInfo.Bitten || agentInfo.WasBitten || recentlyBitten)
		return 2.0f; // Highest priority

	if (closestDist > 0.f && closestDist < 30.0f)
		return 0.7f + (0.3f * (1.0f - (closestDist / 30.0f)));

	return 0.0f;
}
