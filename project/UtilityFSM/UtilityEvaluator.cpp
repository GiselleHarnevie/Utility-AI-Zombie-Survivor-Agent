#include "../stdafx.h"
#include "UtilityEvaluator.h"
#include "FiniteStateMachine.h"
#include "../Blackboard.h"


void UtilityEvaluator::AddState(FSMState* pState)
{
	m_pStates.emplace_back(pState);
}

FSMState* UtilityEvaluator::GetBestStateScore(Blackboard* pBlackboard)
{
    m_StoredScores.clear();

    //Each state calculates its own score
    for (auto pState : m_pStates)
    {
        float score = pState->CalculateScore(pBlackboard);
        m_StoredScores.push_back({ pState, score });
    }

    //find the best state
    auto bestIt = std::max_element(m_StoredScores.begin(), m_StoredScores.end(),
        [](const StateScores& a, const StateScores& b)
        {
            return a.score < b.score;
        });

    return bestIt != m_StoredScores.end() ? bestIt->pState : nullptr;
}

const std::vector<StateScores>& UtilityEvaluator::GetAllStateScores() const
{
    return m_StoredScores;
}
