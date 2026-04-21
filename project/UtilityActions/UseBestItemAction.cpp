#include "../stdafx.h"
#include "UseBestItemAction.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"

UseBestItemAction::UseBestItemAction(IExamInterface* pInterface)
    : m_pInterface(pInterface)
{
}

float UseBestItemAction::CalculateUtility(Blackboard* pBlackboard) const
{
    AgentInfo agentInfo{};
    unsigned int medkitCount{}, foodCount{};
    bool hasGarbage{};
    pBlackboard->GetData("AgentInfo", agentInfo);
    pBlackboard->GetData("MedkitCount", medkitCount);
    pBlackboard->GetData("FoodCount", foodCount);
    pBlackboard->GetData("HasGarbage", hasGarbage);

    bool needFood = agentInfo.Energy < 4.f;
    bool needMedkit = agentInfo.Health < 5.f;

    if (needFood && (foodCount > 0))
        return 1.0f;
    if (needMedkit && (medkitCount > 0))
        return 1.0f;
    if (hasGarbage)
        return 1.0f;

    return 0.f;
}

void UseBestItemAction::Execute(Blackboard* pBlackboard, float /*deltaTime*/)
{
    AgentInfo agentInfo{};
    unsigned int medkitCount{}, foodCount{}; 
    bool hasGarbage{};
    pBlackboard->GetData("AgentInfo", agentInfo);
    pBlackboard->GetData("MedkitCount", medkitCount);
    pBlackboard->GetData("FoodCount", foodCount);
    pBlackboard->GetData("HasGarbage", hasGarbage);

    bool needFood = agentInfo.Energy < 7.5f;
    bool needMedkit = agentInfo.Health < 7.5f;

    UINT capacity = m_pInterface->Inventory_GetCapacity();

    // Try to use food first if needed and available
    if (needFood && foodCount > 0)
    {
        for (UINT i = 0; i < capacity; ++i)
        {
            ItemInfo item;
            if (m_pInterface->Inventory_GetItem(i, item) && item.Type == eItemType::FOOD)
            {
                if (m_pInterface->Inventory_UseItem(i))
                {
                    // Decrement food count in blackboard
                    pBlackboard->ChangeData("FoodCount", foodCount > 0 ? foodCount - 1 : 0);

                    ItemInfo checkValue;
                    if (m_pInterface->Inventory_GetItem(i, checkValue) && checkValue.Value == 0)
                    {
                        m_pInterface->Inventory_RemoveItem(i);
                    }
                }
                return;
            }
        }
    }

    // If no food used, try medkit if needed and available
    if (needMedkit && medkitCount > 0)
    {
        for (UINT i = 0; i < capacity; ++i)
        {
            ItemInfo item;
            if (m_pInterface->Inventory_GetItem(i, item) && item.Type == eItemType::MEDKIT)
            {
                if (m_pInterface->Inventory_UseItem(i))
                {
                    // Decrement medkit count in blackboard
                    pBlackboard->ChangeData("MedkitCount", medkitCount > 0 ? medkitCount - 1 : 0);

                    // Remove item if depleted
                    ItemInfo checkItem;
                    if (m_pInterface->Inventory_GetItem(i, checkItem) && checkItem.Value == 0)
                    {
                        m_pInterface->Inventory_RemoveItem(i);
                    }
                }
                return;
            }
        }
    }

    // Destroy garbage if present
    if (hasGarbage)
    {
        bool foundGarbage = false;
        for (UINT i = 0; i < capacity; ++i)
        {
            ItemInfo item;
            if (m_pInterface->Inventory_GetItem(i, item) && item.Type == eItemType::GARBAGE)
            {
                m_pInterface->DestroyItem(item);
                m_pInterface->Inventory_RemoveItem(i);
                foundGarbage = true;
            }
        }
        if (foundGarbage)
        {
            pBlackboard->ChangeData("HasGarbage", false);
        }
    }

}