#include "../stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "Exam_HelperStructs.h"


BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput blendedSteering = {};

	//TODO: Calculate the weighted average steeringbehavior
	float totalWeight = 0.f;

	for (auto weightedBehaviour : m_WeightedBehaviors)
	{
		auto steering = weightedBehaviour.pBehavior->CalculateSteering(deltaT, pAgent);
		blendedSteering.LinearVelocity += weightedBehaviour.weight * steering.LinearVelocity;
		blendedSteering.AngularVelocity += weightedBehaviour.weight * steering.AngularVelocity;

		totalWeight += weightedBehaviour.weight;
	}

	if (totalWeight > 0.f)
	{
		float scale = 1.f / totalWeight;
		blendedSteering *= scale;
	}

	/*if (pAgent->GetDebugRenderingEnabled())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), blendedSteering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);*/

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float deltaT, AgentInfo* const pAgent)
{
	SteeringOutput steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}