#include "../stdafx.h"
#include "PurgeState.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "../Steering/SteeringBehaviors.h"
#include "IExamInterface.h"

PurgeState::PurgeState(IExamInterface* pInterface)
    : m_pInterface(pInterface)
{
    m_pSeek = std::make_unique<Seek>();
}

void PurgeState::OnEnter(Blackboard* pBlackboard)
{
    std::cout << "OnEnter Purge state\n";
    pBlackboard->ChangeData("IsRunning", true);
}

void PurgeState::OnExit(Blackboard* pBlackboard)
{
    std::cout << "OnExit Purge state\n";
    pBlackboard->ChangeData("IsRunning", false);


}

void PurgeState::Update(Blackboard* pBlackboard, float deltaTime)
{
    AgentInfo agentInfo{};
    std::vector<PurgeZoneInfo> purgeZones;
    pBlackboard->GetData("AgentInfo", agentInfo);
    pBlackboard->GetData("PurgeZonesInFOV", purgeZones);
    const float purgeZoneRadius = purgeZones[0].Radius;

    Elite::Vector2 purgeZonePos = purgeZones[0].Center;
    Elite::Vector2 currentPosition = agentInfo.Position;

    const float margin = 5;
    Elite::Vector2 direction = (purgeZonePos - currentPosition).GetNormalized();
    Elite::Vector2 positionOutsideZone = purgeZonePos + (direction * -(purgeZoneRadius + margin));

    m_pSeek->SetTarget(m_pInterface->NavMesh_GetClosestPathPoint(positionOutsideZone));

    SteeringOutput steering = m_pSeek->CalculateSteering(deltaTime, &agentInfo);
    pBlackboard->ChangeData("SteeringOutput", steering);
}


float PurgeState::CalculateScore(Blackboard* pBlackboard) const
{
    // If any purge zone is in FOV, return high score
    std::vector<PurgeZoneInfo> purgeZones;
    pBlackboard->GetData("PurgeZonesInFOV", purgeZones);
    if (!purgeZones.empty())
        return 1.0f; // Highest priority

    return 0.0f;
}