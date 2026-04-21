#pragma once
#include "UtilityAction.h"
#include "../Blackboard.h"

class IExamInterface;
class Seek;


class PickBestItemAction : public UtilityAction
{
public:
    PickBestItemAction(IExamInterface* pInterface);

    float CalculateUtility(Blackboard* pBlackboard) const override;
    void Execute(Blackboard* pBlackboard, float deltaTime) override;


private:
    IExamInterface* m_pInterface = nullptr;
    std::unique_ptr<Seek> m_pSeek= nullptr;

 
};