#pragma once
class FSMState;
class Blackboard;

struct StateScores
{
	FSMState* pState = nullptr;
	float score = 0.0f;
};

class UtilityEvaluator
{
public:
	UtilityEvaluator() = default;
	~UtilityEvaluator() = default;

	void AddState(FSMState* pState); 
	FSMState* GetBestStateScore(Blackboard* pBlackboard); //gets best from calc scores in fsm

	const std::vector<StateScores>& GetAllStateScores() const;
private:
	std::vector<FSMState*> m_pStates;
	std::vector<StateScores> m_StoredScores;
};