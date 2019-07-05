#pragma once
#include "Box2D\Box2D.h"
#include "DebugDraw.h"
#include "Entity.h"
#include <vector>
#include <algorithm>
#include <list>


#define TITLE "Walker"
#define WIDTH 720*1.5f
#define HEIGHT 440*1.5f
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define MAX_ENTITIES 100
#define LIFETIME 600


#define WEIGHT_MUT_AMOUNT 1
#define WEIGHT_MUT_PROB	  0.1f
#define BIAS_MUT_AMOUNT   1
#define BIAS_MUT_PROB	  0.1f
#define STRUCT_MUT_AMOUNT 1
#define STRUCT_MUT_PROB	  0.1f
#define EXCESS_IMPORTANCE 1.0f
#define WEIGHT_IMPORTANCE 0.5f
#define COMPATIBILITY_TRESHOLD 3.0f
#define DROPOFF_AGE 15
#define SURVIVAL_TRESHOLD 0.4f




class Species {
public:
	Species(Entity* rep);

	Entity* partecipants[1000];
	Connection* representative[500]{ nullptr };
	int repAmount = 0;
	int partAmount = 0;
	float fitness = 0.0f;
	int offsprings = 0;
	float bestFitness = 0.0f;
	int bestFitnessAge = 0;
	int age = 0;

	bool isCompatible(Entity* test);
	void addPartecipant(Entity* member);
	Entity* getMateEntity();
	void pickRep();
	void clear();
	void update();
	void setParents();

};

class Environment
{
public:
	Environment();
	~Environment();
	b2World*  GetWorld();
	void Step();
	DebugDraw* GetDebugDraw();
	void Doo();

private:
	
	Entity* entities[MAX_ENTITIES];

	b2World* MyWorld;
	b2Vec2 Gravity;
	float32 timeStep = 1.0f / 60.0f;
	int32 velocityIterations = 6;
	int32 positionIterations = 2;
	int cycles = 0;
	int generation = 1;
	DebugDraw* mDebugDraw;
	int bestFitness = 0;


	Species* species[500]{ nullptr };
	int speciesAmount = 0;

	void MateEntities();
	Entity * GetMateEntity(float normalFitness[]);
	Entity * CreateChildEnitity(Entity* parent1, Entity* parent2, Entity* babies[], int id, int fittestParent);
	Neuron* SeekRelativeNeuron(Neuron* bull, Neuron* Array[]);
	float AdjustFitness(Entity* organism);
	void Clonate(Neuron* babiesN[], Connection* babiesC[], Entity* cloner);
	float GetCompDistance(Entity* org1, Entity* org2);

	void Mutate(Entity* baby);

	void CreateFloor();
	void CreatePopulation(int amount);

	void ChooseSpecies(Entity* baby);
};