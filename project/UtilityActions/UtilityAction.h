#pragma once

class Blackboard;

class UtilityAction
{
public:
	UtilityAction();
	virtual ~UtilityAction() = default;

	virtual float CalculateUtility(Blackboard* pBlackboard) const = 0;
	virtual void Execute(Blackboard* pBlackboard, float deltaTime) = 0;
private:

};

	