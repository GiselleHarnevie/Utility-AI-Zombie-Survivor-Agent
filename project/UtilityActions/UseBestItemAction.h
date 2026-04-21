#pragma once
#include "UtilityAction.h"

class IExamInterface;
class UseBestItemAction : public UtilityAction
{
public:
    UseBestItemAction(IExamInterface* pInterface);
    float CalculateUtility(Blackboard* pBlackboard) const override;
    void Execute(Blackboard* pBlackboard, float deltaTime) override;

private:
    IExamInterface* m_pInterface;
};