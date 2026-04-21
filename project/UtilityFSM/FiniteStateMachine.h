#pragma once
#include <map>
//#include <vector>
class Blackboard;


class FSMState
{
public:
	FSMState() {};
	virtual ~FSMState() = default;

	virtual void OnEnter(Blackboard* pBlackboard) = 0; 
	virtual void OnExit(Blackboard* pBlackboard) = 0; 
	virtual void Update(Blackboard* pBlackboard, float deltaTime) = 0;

	virtual float CalculateScore(Blackboard* pBlackboard) const = 0;
private:

};

class FiniteStateMachine final
{
public:
	FiniteStateMachine(FSMState* const startState, Blackboard* const pBlackboard);
	~FiniteStateMachine();

	void Update(float deltaTime);

	FSMState* GetCurrentState() const;
	void ChangeState(FSMState* const newState);

private:

	FSMState* m_pCurrentState = nullptr;
	Blackboard* m_pBlackboard = nullptr;
};