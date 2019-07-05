#pragma once
#include "Box2D\Box2D.h"
#include <math.h>
#include <time.h>
#include <iostream>

#define LogisticFunction(X) (1.0/(1.0+exp(-X)))

#define TanH(X) (2.0/(1.0+(exp(-2*X))))-1

#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f

 struct Neuron;

 struct Connection {

	Neuron* from;
	Neuron* to;
	float weight;
	bool disabled = false;
	int innovation;
};

struct Neuron {

	float bias;
	int layer;
	int index;
	float value;
	int conAmount = 0;
	Connection* Connections[100];
	
};

struct HistoryMark {

	int innovation = 0; //maximum innovation
	//Connection* connections[1000];
	int* track[1000][2];
};


struct Output {

	float32 out1;
	float32 out2;
};



struct Brain {

	Neuron* neurons[1000];
	Connection* connections[1000];
	int neuronAmount = 0;
	int connectionAmount = 0;
	int layers = 2;
	HistoryMark* historyMark;
	

	Neuron* CreateNeuron(int Layer, float Bias);
	void CreateConnection(Neuron* From, Neuron* To, float Weight);
	int GetHistoryMark(Connection* connection);
	void CreateRandomConnection();
	void CreateRandomNeuron();
	Output RunNetwork();
	void Erase();
};

class Entity
{
public:

	Entity(b2World* world, int id, HistoryMark* hm);
	Entity(b2World* world, int id, HistoryMark* hm, Neuron * bNeurons[], Connection* bConnections[]);
	Entity(int id, Entity* cloner);
	~Entity();
	void SetAngularSpeed(int index, float32 speed);
	void EvaluateFitness();
	float GetFitness();
	float GestAdjFitness();
	void SetAdjFitness(float Fi);
	void UpdateForces();
	Brain* GetBrain();
	void Engage(float value);
	void SetFitness(float fitt);
	bool worst = false;
	bool champion = false;
	float offsprings = 0;
	b2RevoluteJoint* bodyjoint;

private:

	float initPositions = 0.0f;

	Brain mind;

	b2World* MyWorld;
	b2Body*  body1;
	b2Body*  body2;
	

	float fitness;
	float adjustedFitness;
	
	float32 speed1 = 0;
	float32 speed2 = 0;

	void CreateBody();
	void DestroyBody();
	void CreateMind();
	void CreateMind(Neuron * bNeurons[], Connection* bConnections[]);

	void AdjustPointers();
	

};

inline double randfloat() {
	return rand() / (double)RAND_MAX;
}

inline int randposneg() {
	if (rand() % 2)
		return 1;
	else
		return -1;
}