#include "../stdafx.h"
#include "PickBestItemAction.h"
#include "../Blackboard.h"
#include "Exam_HelperStructs.h"
#include "IExamInterface.h"
#include "../Steering/SteeringBehaviors.h"


PickBestItemAction::PickBestItemAction(IExamInterface* pInterface)
	:m_pInterface(pInterface)
{
}

float PickBestItemAction::CalculateUtility(Blackboard* pBlackboard) const
{
	std::vector<ItemInfo> itemsInFOV;
	pBlackboard->GetData("ItemsInFOV", itemsInFOV);
	if (itemsInFOV.empty())
		return 0.f;

	unsigned int MedkitCount{}, WeaponCount{}, FoodCount{};
	bool HasGarbage{};
	float health{};
	pBlackboard->GetData("MedkitCount", MedkitCount);
	pBlackboard->GetData("FoodCount", FoodCount);
	pBlackboard->GetData("HasGarbage", HasGarbage);
	pBlackboard->GetData("WeaponCount", WeaponCount);
	pBlackboard->GetData("Health", health);

	float bestScore = 0.f;
	for (const auto& item : itemsInFOV)
	{
		float score = 0.f;
		switch (item.Type)
		{
		case eItemType::FOOD:
			score = 1.0f;
			break;
		case eItemType::PISTOL:
		case eItemType::SHOTGUN:
			if (WeaponCount < 2)
				score = 1.0f;
			else
				score = 0.5f;
			break;
		case eItemType::MEDKIT:
			if (MedkitCount < 2)
				score = 1.0f;
			else
				score = 0.6;
			break;
		case eItemType::GARBAGE:
			score = 1.0f;
			break;
		default:
			break;
		}
		bestScore = max(bestScore, score);
	}
	
	return bestScore;
}

void PickBestItemAction::Execute(Blackboard* pBlackboard, float /*deltaTime*/)
{
	//std::cout << "Executing PickBestItem" << std::endl;
	std::vector<ItemInfo> itemsInFOV;
	pBlackboard->GetData("ItemsInFOV", itemsInFOV);
	unsigned int MedkitCount{}, WeaponCount{}, FoodCount{};
	pBlackboard->GetData("MedkitCount", MedkitCount);
	pBlackboard->GetData("FoodCount", FoodCount);
	pBlackboard->GetData("WeaponCount", WeaponCount);
	AgentInfo agent = m_pInterface->Agent_GetInfo();

	// Destroy 
	for (const auto& item : itemsInFOV)
	{
		if (item.Type == eItemType::GARBAGE)
		{
			m_pInterface->DestroyItem(item);
			std::cout << "destroyed a garbage";
		}
		if ((item.Type == eItemType::PISTOL || item.Type == eItemType::SHOTGUN) && WeaponCount >= 2)
		{
			m_pInterface->DestroyItem(item);
			std::cout << "destroyed a weapon";
		}
		if (item.Type == eItemType::MEDKIT && MedkitCount >= 2)
		{
			m_pInterface->DestroyItem(item);
			std::cout << "destroyed a medkit";
		}

	}

	if (itemsInFOV.empty())
	{
		// No items in FOV, reset the target
		pBlackboard->ChangeData("IsThereItemTarget", false);
		pBlackboard->ChangeData("ItemTarget", TargetData{});
		return;
	}


	// Find the best item
	float bestScore = 0.f;
	const ItemInfo* bestItem = nullptr;
	for (const auto& item : itemsInFOV)
	{
		float score = 0.f;
		switch (item.Type)
		{
		case eItemType::FOOD:
			score = (FoodCount < 2) ? 1.0f : 0.2f;
			break;
		case eItemType::PISTOL:
		case eItemType::SHOTGUN:
			score = (WeaponCount < 2) ? 1.0f : 0.0f;
			break;
		case eItemType::MEDKIT:
			score = (MedkitCount < 2) ? 1.0f : 0.0f;
			break;
		case eItemType::GARBAGE:
			score = 1.0f;
			break;
		default:
			break;
		}
		if (score > bestScore)
		{
			bestScore = score;
			bestItem = &item;
		}
	}

	if (!bestItem)
	{
		// No valid item found, reset the target
		pBlackboard->ChangeData("IsThereItemTarget", false);
		pBlackboard->ChangeData("ItemTarget", TargetData{});
		return;
	}

	float dist = (agent.Position - bestItem->Location).Magnitude();

	//if agent is within grabbing distance
	if ((dist <= agent.GrabRange) && !itemsInFOV.empty())
	{
		// In grabbing range, grab the item
		UINT capacity = m_pInterface->Inventory_GetCapacity();
		int emptySlot = -1;

		// Find the first empty inventory slot
		for (UINT i = 0; i < capacity; ++i)
		{
			ItemInfo invItem;
			if (!m_pInterface->Inventory_GetItem(i, invItem))
			{
				emptySlot = static_cast<int>(i);
				break;
			}
		}


		if (emptySlot != -1 && m_pInterface->GrabItem(*bestItem))
		{
			m_pInterface->Inventory_AddItem(emptySlot, *bestItem);

			// Update blackboard counts
			if (bestItem->Type == eItemType::FOOD)
			{
				unsigned int foodCount = 0;
				pBlackboard->GetData("FoodCount", foodCount);
				pBlackboard->ChangeData("FoodCount", foodCount + 1);
				pBlackboard->GetData("FoodCount", foodCount);
				std::cout << "food " << foodCount << std::endl;
			}
			else if (bestItem->Type == eItemType::MEDKIT)
			{1520
				unsigned int medkitCount = 0;
				pBlackboard->GetData("MedkitCount", medkitCount);
				pBlackboard->ChangeData("MedkitCount", medkitCount + 1);
				pBlackboard->GetData("MedkitCount", medkitCount);
				std::cout << "medkit " << medkitCount << std::endl;
			}
			else if (bestItem->Type == eItemType::PISTOL || bestItem->Type == eItemType::SHOTGUN)
			{
				unsigned int weaponCount = 0;
				pBlackboard->GetData("WeaponCount", weaponCount);
				pBlackboard->ChangeData("WeaponCount", weaponCount + 1);
				pBlackboard->GetData("WeaponCount", weaponCount);
				std::cout << "weapon " << weaponCount << std::endl;
			}

		}

		// Clear the target after grabbing
		pBlackboard->ChangeData("IsThereItemTarget", false);
		pBlackboard->ChangeData("ItemTarget", TargetData{});
	}
	else
	{
		//Not in range, set the item as the target
		TargetData target;
		target.Position = bestItem->Location;
		pBlackboard->ChangeData("IsThereItemTarget", true);
		pBlackboard->ChangeData("ItemTarget", target);
	}
}

