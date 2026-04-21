/*=============================================================================*/
// Copyright 2023-2024 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#pragma once

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"

struct AgentInfo;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	TargetData m_Target;
};
#pragma endregion

///////////////////////////////////////
//1.SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;


	// Inherited via ISteeringBehavior
	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;
};

///////////////////////////////////////
//2.FLEE
//****
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Arrive Behaviour
	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;

protected:
};

/////////////////////////////////////////
////3.ARRIVE
////****
//class Arrive : public ISteeringBehavior
//{
//public:
//	Arrive() = default;
//	virtual ~Arrive() = default;
//
//	//Arrive Behaviour
//	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;
//
//	//Arrive Functions
//	void SetSlowRadius(float radius) { m_SlowRadius = radius; };
//	void SetTargetRadius(float radius) { m_TargetRadius = radius; };
//
//protected:
//	float m_SlowRadius = 15.f;
//	float m_TargetRadius = 3.f;
//
//};


///////////////////////////////////////
//4.FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;

protected:
};

/////////////////////////////////////////
////5.Pursuit
////****
//class Pursuit : public ISteeringBehavior
//{
//public:
//	Pursuit() = default;
//	virtual ~Pursuit() = default;
//
//	//Pursuit Behaviour
//	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;
//};

/////////////////////////////////////////
////6.Evade
////****
//class Evade : public Flee
//{
//public:
//	Evade() = default;
//	virtual ~Evade() = default;
//
//	//Evade Behaviour
//	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;
//};

///////////////////////////////////////
//7.Wander
//****
class Wander : public ISteeringBehavior
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	// Inherited via ISteeringBehavior


	// Inherited via ISteeringBehavior
	SteeringOutput CalculateSteering(float deltaT, AgentInfo* const pAgent) override;

protected:
	float m_WanderRate = 0.6f; //how much displacement
	float m_WanderOffset = 6.f; //offset of the circle
	float m_WanderRadius = 4.f;
	float m_WanderOrientation = 0.f;
};