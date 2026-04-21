//Precompiled Header [ALWAYS ON TOP IN CPP]
//#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "Exam_HelperStructs.h"
#include "EliteMath\EMatrix2x3.h"
#include <limits>

////1.SEEK
SteeringOutput Seek::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	//std::cout << "SEEKING";
	return steering;
}

//2.FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput steering = {};
	Elite::Vector2 directionToTarget = m_Target.Position - pAgent->Position;
	float distance = directionToTarget.Magnitude();

	steering.LinearVelocity = pAgent->Position - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;

	if (distance < 5.f)
	{
		steering.IsValid = true;

	}
	else
		steering.IsValid = false;

	/*if (pAgent->GetDebugRenderingEnabled())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity,
			5.f, Elite::Color(0, 1, 0, 0.5f), .4f);*/

	return steering;
}

////3.ARRIVE
////****
//SteeringOutput Arrive::CalculateSteering(float deltaT, AgentInfo* const pAgent)
//{
//	SteeringOutput steering = {};
//
//	/*steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
//	const float distance = steering.LinearVelocity.Normalize() - m_TargetRadius;
//
//	if (distance < m_SlowRadius)
//		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * (distance / (m_SlowRadius * m_TargetRadius));
//	else
//		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
//
//	if (pAgent->GetDebugRenderingEnabled())
//	{
//		const Elite::Vector2 pos = pAgent->GetPosition();
//		DEBUGRENDERER2D->DrawCircle(pos, m_SlowRadius, Elite::Color(0.f, 0.f, 1.f, 0.5f), 0.7f);
//		DEBUGRENDERER2D->DrawCircle(pos, m_TargetRadius, Elite::Color(0.f, .25f, 0.f, 0.5f), 0.7f);
//
//		DEBUGRENDERER2D->DrawDirection(pos, steering.LinearVelocity, steering.LinearVelocity.Magnitude(),
//			Elite::Color(0.f, 1.f, 0.f, 0.5f), 0.4f);
//	}*/
//
//	return steering;
//}

//4.Face
//****
SteeringOutput Face::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput steering{};
	//pAgent->SetAutoOrient(false);
	
	Elite::Vector2 directionToTarget = m_Target.Position - pAgent->Position;
	float agentRotation = pAgent->Orientation;
	Elite::Vector2 agentDirection = Elite::Vector2{ cosf(agentRotation),sinf(agentRotation) };

	float angleBetween{ Elite::AngleBetween(agentDirection,directionToTarget) };

	if (fabsf(angleBetween) <= 0.1f)
	{
		steering.AngularVelocity = 0;
	}
	else
	{
		steering.AngularVelocity = pAgent->MaxAngularSpeed*5 / deltaT;
	}
	//std::cout << "FACING";
	/*if (pAgent->GetDebugRenderingEnabled())
	{
		const Elite::Vector2 pos = pAgent->GetPosition();
		DEBUGRENDERER2D->DrawDirection(pos, directionToTarget.GetNormalized(), 5.f, Elite::Color(0, 1, 0, 0.5f), .4f);
	}*/

	return steering;
}

////5.Pursuit
////****
//SteeringOutput Pursuit::CalculateSteering(float deltaT, AgentInfo* const pAgent)
//{
//	return SteeringOutput();
//}

////6.Evade
////****
//SteeringOutput Evade::CalculateSteering(float deltaT, AgentInfo* const pAgent)
//{
//	//Elite::Vector2 directionToTarget = m_Target.Position - pAgent->GetPosition();
//
//	//float predictionTime = 2.f;
//	//Elite::Vector2 predictedPos = m_Target.Position + (m_Target.LinearVelocity * predictionTime);
//
//	//m_Target.Position = predictedPos;
//	//return Flee::CalculateSteering(deltaT, pAgent);
//
//	SteeringOutput steering = {};
//	/*Elite::Vector2 directionToTarget = m_Target.Position - pAgent->GetPosition();
//	float distance = directionToTarget.Magnitude();
//
//	if (distance > 40.f)
//	{
//		steering.IsValid = false;
//		return steering;
//	}
//
//	float predictionTime = distance / (pAgent->GetMaxLinearSpeed());
//	Elite::Vector2 predictedPos = m_Target.Position + m_Target.LinearVelocity * predictionTime;
//
//	steering.LinearVelocity = pAgent->GetPosition() - predictedPos;
//	steering.LinearVelocity.Normalize();
//	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
//	steering.IsValid = true;
//
//	if (pAgent->GetDebugRenderingEnabled())
//	{
//		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color(1, 0, 0, 0.5f), 0.4f);
//		DEBUGRENDERER2D->DrawCircle(predictedPos, 1.f, Elite::Color(1, 0, 0, 0.5f), 0.4f);
//	}*/
//
//	return steering;
//}


////7.Wander
////****
SteeringOutput Wander::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput steering{};

	m_WanderOrientation += Elite::randomFloat(-1, 1) * m_WanderRate;
	float agentOrientation = pAgent->Orientation;
	float targetOrientation = m_WanderOrientation + agentOrientation;

	Elite::Vector2 targetPosition = pAgent->Position + (m_WanderOffset * Elite::OrientationToVector(agentOrientation));
	targetPosition += m_WanderRadius * Elite::OrientationToVector(targetOrientation);

	steering.LinearVelocity = targetPosition - pAgent->Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->MaxLinearSpeed;
	return steering;
}
