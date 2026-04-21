#include "../stdafx.h"
#include "FiniteStateMachine.h"
#include "Exam_HelperStructs.h"

FiniteStateMachine::FiniteStateMachine(FSMState* const startState, Blackboard* const pBlackboard)
	:m_pCurrentState(startState),
	m_pBlackboard(pBlackboard)
{
	if (m_pCurrentState)
		m_pCurrentState->OnEnter(m_pBlackboard);
}

FiniteStateMachine::~FiniteStateMachine()
{

}

void FiniteStateMachine::Update(float deltaTime)
{
   //TODO 8: Update the current state (if one exists)
	if (m_pCurrentState)
		m_pCurrentState->Update(m_pBlackboard, deltaTime);
}


FSMState* FiniteStateMachine::GetCurrentState() const
{
	return m_pCurrentState;
}

void FiniteStateMachine::ChangeState(FSMState* const newState)
{
	
	// FiniteStateMachine TODO:
   //TODO 1. If currently in a state => make sure the OnExit of that state gets called
	if (m_pCurrentState)
		m_pCurrentState->OnExit(m_pBlackboard);

	//TODO 2. Change the current state to the new state
	m_pCurrentState = newState;

	//TODO 3. Call the OnEnter of the new state  
	if (m_pCurrentState)
		newState->OnEnter(m_pBlackboard);
}
