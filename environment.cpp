#include "environment.h"

using namespace std;


HistoryMark historyMark;

bool koo = true;

Environment::Environment()
{

	Gravity.Set(0.0f, -10.0f);
	MyWorld = new b2World(Gravity);

	g_camera.m_width = WIDTH;
	g_camera.m_height = HEIGHT;
	MyWorld->SetGravity(Gravity);
	MyWorld->SetDebugDraw(&g_debugDraw);

	CreateFloor();
	CreatePopulation(MAX_ENTITIES);

	cout << "NeuroEvolution simulator \n\n";
	cout << "Entities amount: " << MAX_ENTITIES << endl;
	cout << "Mutation Probability: 10%\n";


};

Environment::~Environment()
{

}

b2World* Environment::GetWorld()
{
	return MyWorld;
}

void Environment::Step()
{
	Output output;
	for (int i = 0; i < MAX_ENTITIES; i++) {

		entities[i]->UpdateForces();
	}

	g_debugDraw.DrawString(10, 0, "%d cycles", cycles);
	g_debugDraw.DrawString(10, 20, "generation: %d", generation);

	if (koo)
		MyWorld->Step(timeStep, velocityIterations, positionIterations);

	if (!koo) {
		for (int i = 0; i < 600; i++) {
			MyWorld->Step(1.0f/60.0f, velocityIterations, positionIterations);
			for (int i = 0; i < MAX_ENTITIES; i++) {

				entities[i]->UpdateForces();
			}
		}
		koo = true;
	}
	

	MyWorld->DrawDebugData();
	g_debugDraw.Flush();

	cycles++;

	if (cycles == LIFETIME) {
		for (int i = 0; i < MAX_ENTITIES; i++) {

			entities[i]->EvaluateFitness();

		}
		generation++;
		MateEntities();
		cycles = 0;
	}
}

void Environment::MateEntities()
{
	float totalFitness = 0.0f;
	float specTotalFitness = 0.0f;
	float averageFitness = 0.0f;
	Entity* babies[MAX_ENTITIES] = { nullptr };
	//float normalFitness[MAX_ENTITIES];
	int babyIndex = 0;
	Species* bestSp = nullptr;

	int totalOffsprings = 0;

	Entity* champion = nullptr;
	bool champDone = false;

	for (int i = 0; i < speciesAmount; i++) {

		species[i]->update();

	}

	cout << "\n----------\n\n";
	cout << "End of generation: " << (generation - 1) << endl;
	cout << "Species amount: " << speciesAmount << endl;

	//adjust fitness + totalFitness + best entity
	for (int i = 0; i < MAX_ENTITIES; i++) {

		AdjustFitness(entities[i]);
		totalFitness += entities[i]->GestAdjFitness();
		//averageFitness += entities[i]->GetFitness();

		if (champion == nullptr) { champion = entities[i]; }
		else {

			if (entities[i]->GetFitness() > champion->GetFitness())
				champion = entities[i];

		}
	}

	champion->champion = true;
	averageFitness = totalFitness / MAX_ENTITIES;

	cout << "Average fitness: " << averageFitness << endl;
	cout << "Champion fitness: " << champion->GetFitness() << "(" << champion->GestAdjFitness() << ")" << endl;
	cout << "	neurons: " << champion->GetBrain()->neuronAmount << endl;
	cout << "	connections: " << champion->GetBrain()->connectionAmount << endl;

	for (int i = 0; i < MAX_ENTITIES; i++) {

		entities[i]->offsprings = entities[i]->GestAdjFitness() / averageFitness;

	}

	for (int i = 0; i < speciesAmount; i++) {

		species[i]->setParents(); // mark bad entities "worst"

	}

	//fitness normalization
/*	for (int i = 0; i < MAX_ENTITIES; i++) {

		normalFitness[i] = entities[i]->GetFitness() / totalFitness;
	}*/

	//species fitness(adj) evaluation
	for (int i = 0; i < speciesAmount; i++) {

		for (int b = 0; b < species[i]->partAmount; b++) {

			species[i]->fitness += species[i]->partecipants[b]->GestAdjFitness();

		}
		specTotalFitness += species[i]->fitness;
	}

	float babyExcess = 0.0f;

	//calculate offsprings per species + best species
	for (int i = 0; i < speciesAmount; i++) {

		for (int k = 0; k < species[i]->partAmount; k++) {

			species[i]->offsprings += floor(species[i]->partecipants[k]->offsprings);
			babyExcess += fmod(species[i]->partecipants[k]->offsprings, 1.0f);

		}

		if (babyExcess > 1) {

			species[i]->offsprings += floor(babyExcess);

			babyExcess -= floor(babyExcess);

		}

		totalOffsprings += species[i]->offsprings;




		if (bestSp == nullptr) bestSp = species[i];
		else {

			if (species[i]->fitness > bestSp->fitness)
				bestSp = species[i];

		}
	}

	std::cout << "expected species offsprings: " << totalOffsprings << std::endl;

	//clonate champion baby

	//create babies
	for (int i = 0; i < speciesAmount; i++) {

		if (species[i]->partAmount == 0)
			continue;

		if (!champDone) {//clone the entities champion
			for (int a = 0; a < species[i]->partAmount; a++) {

				if (species[i]->partecipants[a]->champion) {

					Neuron* babyN[1000] = { nullptr };
					Connection* babyC[1000] = { nullptr };

					Clonate(babyN, babyC, species[i]->partecipants[a]);

					babies[babyIndex] = new Entity(MyWorld, babyIndex, &historyMark, babyN, babyC);
					species[i]->offsprings -= 1;
					babyIndex += 1;
					champDone = true;

					if (species[i]->offsprings > 5) {
						babies[babyIndex] = CreateChildEnitity(species[i]->partecipants[a], species[i]->getMateEntity(), babies, babyIndex, 1);
						species[i]->offsprings -= 1;
						babyIndex += 1;
					}
				}

			}
		}

		for (int a = 0; a < species[i]->offsprings; a++) {

			Entity* parent1 = species[i]->getMateEntity();
			Entity* parent2 = species[i]->getMateEntity();

			if (parent1 == nullptr) {
				std::cout << " species " << i << " dead" << std::endl;
				break;
			}
			if (parent2 == nullptr) {
				std::cout << " species " << i << " dead" << std::endl;
				break;
			}

			Entity* offspring;
			if (parent1->GetFitness() >= parent2->GetFitness())
				offspring = CreateChildEnitity(parent1, parent2, babies, babyIndex, 1);
			else
				offspring = CreateChildEnitity(parent1, parent2, babies, babyIndex, 2);

			babies[babyIndex] = offspring;
			babyIndex += 1;
		}

	}

	//create excess babies
	for (int i = 0; i < (MAX_ENTITIES - totalOffsprings); i++) {

		Entity* parent1 = bestSp->getMateEntity();
		Entity* parent2 = bestSp->getMateEntity();

		if (parent1 == nullptr) {
			std::cout << " species " << i << " dead" << std::endl;
			break;
		}
		if (parent2 == nullptr) {
			std::cout << " species " << i << " dead" << std::endl;
			break;
		}

		Entity* offspring;
		if (parent1->GetFitness() >= parent2->GetFitness())
			offspring = CreateChildEnitity(parent1, parent2, babies, babyIndex, 1);
		else
			offspring = CreateChildEnitity(parent1, parent2, babies, babyIndex, 2);

		babies[babyIndex] = offspring;
		babyIndex += 1;

	}

	/*for (int i = 0; i < MAX_ENTITIES; i++) {
		std::cout << "entity " << i << std::endl;
		for (int a = 0; a < entities[i]->GetBrain()->connectionAmount; a++) {

			std::cout << "from " << entities[i]->GetBrain()->connections[a]->from->index << " to " << entities[i]->GetBrain()->connections[a]->to->index<<std::endl;

		}

	}
	*/


	for (int i = 0; i < speciesAmount; i++) {

		species[i]->pickRep();
		species[i]->clear();

	}

	for (int i = 0; i < MAX_ENTITIES; i++) {

		delete entities[i];
		entities[i] = babies[i];
		ChooseSpecies(babies[i]);
	}

	cout << "End Mating species: " << speciesAmount << endl;
}

Entity* Environment::GetMateEntity(float normalFitness[]) {

	float summer = 0;
	float random = randfloat();

	for (int b = 0; b < MAX_ENTITIES; b++) {

		summer += normalFitness[b];

		if (random < summer) {

			return entities[b];

		}


	}

	std::cout << "nothing found!" << std::endl;

	return nullptr;

}

Entity * Environment::CreateChildEnitity(Entity * parent1, Entity * parent2, Entity* babies[], int id, int fittestParent)
{

	Neuron* babyNeurons[1000]{ nullptr };
	Connection* babyConnections[1000]{ nullptr };
	int neuronsAm = 0;
	int connAm = 0;

	//choose randomly matching neurons
	for (int i = 0; i < parent1->GetBrain()->neuronAmount; i++) {

		for (int j = 0; j < parent2->GetBrain()->neuronAmount; j++) {

			if (parent1->GetBrain()->neurons[i]->index == parent2->GetBrain()->neurons[j]->index) {

				if ((rand() % 2) == 0) {// parent 1

					babyNeurons[neuronsAm] = new Neuron();
					babyNeurons[neuronsAm]->bias = parent1->GetBrain()->neurons[i]->bias;
					babyNeurons[neuronsAm]->index = parent1->GetBrain()->neurons[i]->index;
					babyNeurons[neuronsAm]->layer = parent1->GetBrain()->neurons[i]->layer;
					neuronsAm += 1;
				}
				else {// parent 2

					babyNeurons[neuronsAm] = new Neuron();
					babyNeurons[neuronsAm]->bias = parent2->GetBrain()->neurons[j]->bias;
					babyNeurons[neuronsAm]->index = parent2->GetBrain()->neurons[j]->index;
					babyNeurons[neuronsAm]->layer = parent2->GetBrain()->neurons[j]->layer;
					neuronsAm += 1;

				}

			}

		}


	}

	//choose excess neurons from fittest
	if (parent1->GetFitness() > parent2->GetFitness()) {

		for (int i = 0; i < parent1->GetBrain()->neuronAmount; i++) {
			bool match = false;
			for (int j = 0; j < parent2->GetBrain()->neuronAmount; j++) {

				if (parent1->GetBrain()->neurons[i]->index == parent2->GetBrain()->neurons[j]->index)
					match = true;

			}

			if (!match) {

				babyNeurons[neuronsAm] = new Neuron();
				babyNeurons[neuronsAm]->bias = parent1->GetBrain()->neurons[i]->bias;
				babyNeurons[neuronsAm]->index = parent1->GetBrain()->neurons[i]->index;
				babyNeurons[neuronsAm]->layer = parent1->GetBrain()->neurons[i]->layer;
				neuronsAm += 1;

			}

		}

	}

	else {

		for (int i = 0; i < parent2->GetBrain()->neuronAmount; i++) {
			bool match = false;
			for (int j = 0; j < parent1->GetBrain()->neuronAmount; j++) {

				if (parent2->GetBrain()->neurons[i]->index == parent1->GetBrain()->neurons[j]->index)
					match = true;

			}

			if (!match) {

				babyNeurons[neuronsAm] = new Neuron();
				babyNeurons[neuronsAm]->bias = parent2->GetBrain()->neurons[i]->bias;
				babyNeurons[neuronsAm]->index = parent2->GetBrain()->neurons[i]->index;
				babyNeurons[neuronsAm]->layer = parent2->GetBrain()->neurons[i]->layer;
				neuronsAm += 1;

			}

		}

	}

	//choose matching connections randomly 
	for (int k = 0; k < parent1->GetBrain()->connectionAmount; k++) {

		for (int j = 0; j < parent2->GetBrain()->connectionAmount; j++) {

			if (parent1->GetBrain()->connections[k]->innovation == parent2->GetBrain()->connections[j]->innovation) {

				Entity* chosen;

				if ((rand() % 2) == 0) {


					babyConnections[connAm] = new Connection();
					babyConnections[connAm]->from = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->from, babyNeurons);
					babyConnections[connAm]->to = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->to, babyNeurons);
					babyConnections[connAm]->disabled = parent1->GetBrain()->connections[k]->disabled;
					babyConnections[connAm]->innovation = parent1->GetBrain()->connections[k]->innovation;
					babyConnections[connAm]->weight = (parent1->GetBrain()->connections[k]->weight + parent2->GetBrain()->connections[j]->weight) / 2;

					babyConnections[connAm]->to->Connections[babyConnections[connAm]->to->conAmount] = babyConnections[connAm];
					babyConnections[connAm]->to->conAmount += 1;

					connAm += 1;
				}
				else {

					babyConnections[connAm] = new Connection();
					babyConnections[connAm]->from = SeekRelativeNeuron(parent2->GetBrain()->connections[j]->from, babyNeurons);
					babyConnections[connAm]->to = SeekRelativeNeuron(parent2->GetBrain()->connections[j]->to, babyNeurons);
					babyConnections[connAm]->disabled = parent2->GetBrain()->connections[j]->disabled;
					babyConnections[connAm]->innovation = parent2->GetBrain()->connections[j]->innovation;
					babyConnections[connAm]->weight = (parent1->GetBrain()->connections[k]->weight + parent2->GetBrain()->connections[j]->weight) / 2;

					babyConnections[connAm]->to->Connections[babyConnections[connAm]->to->conAmount] = babyConnections[connAm];
					babyConnections[connAm]->to->conAmount += 1;

					connAm += 1;

				}

			}

		}

	}

	//choose excess connections from fittest
	if (parent1->GetFitness() > parent2->GetFitness()) {

		for (int k = 0; k < parent1->GetBrain()->connectionAmount; k++) {
			bool match = false;
			for (int j = 0; j < parent2->GetBrain()->connectionAmount; j++) {

				if (parent1->GetBrain()->connections[k]->innovation == parent2->GetBrain()->connections[j]->innovation) {

					match = true;

				}

			}
			if (!match) {

				babyConnections[connAm] = new Connection();
				babyConnections[connAm]->from = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->from, babyNeurons);
				babyConnections[connAm]->to = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->to, babyNeurons);
				babyConnections[connAm]->disabled = parent1->GetBrain()->connections[k]->disabled;
				babyConnections[connAm]->innovation = parent1->GetBrain()->connections[k]->innovation;
				babyConnections[connAm]->weight = parent1->GetBrain()->connections[k]->weight;

				babyConnections[connAm]->to->Connections[babyConnections[connAm]->to->conAmount] = babyConnections[connAm];
				babyConnections[connAm]->to->conAmount += 1;

				connAm += 1;

			}

		}
	}

	else {

		for (int k = 0; k < parent2->GetBrain()->connectionAmount; k++) {
			bool match = false;
			for (int j = 0; j < parent1->GetBrain()->connectionAmount; j++) {

				if (parent2->GetBrain()->connections[k]->innovation == parent1->GetBrain()->connections[j]->innovation) {

					match = true;

				}

			}
			if (!match) {

				babyConnections[connAm] = new Connection();
				babyConnections[connAm]->from = SeekRelativeNeuron(parent2->GetBrain()->connections[k]->from, babyNeurons);
				babyConnections[connAm]->to = SeekRelativeNeuron(parent2->GetBrain()->connections[k]->to, babyNeurons);
				babyConnections[connAm]->disabled = parent2->GetBrain()->connections[k]->disabled;
				babyConnections[connAm]->innovation = parent2->GetBrain()->connections[k]->innovation;
				babyConnections[connAm]->weight = parent2->GetBrain()->connections[k]->weight;

				babyConnections[connAm]->to->Connections[babyConnections[connAm]->to->conAmount] = babyConnections[connAm];
				babyConnections[connAm]->to->conAmount += 1;

				connAm += 1;

			}

		}

	}

	Entity* baby = new Entity(MyWorld, id, &historyMark, babyNeurons, babyConnections);

	Mutate(baby);

	return baby;

	/*
		//copy all neurons from parent1 to baby
		for (int k = 0; k < parent1->GetBrain()->neuronAmount; k++) {

			babyNeurons[k] = new Neuron();
			babyNeurons[k]->bias = parent1->GetBrain()->neurons[k]->bias;
			babyNeurons[k]->index = parent1->GetBrain()->neurons[k]->index;
			babyNeurons[k]->layer = parent1->GetBrain()->neurons[k]->layer;

		}
		//copy excesses and disjonts from parent2
		for (int j = 0; j < parent2->GetBrain()->neuronAmount; j++) {
			bool match = false;
			int offset;
			for (offset = 0; babyNeurons[offset] != nullptr; offset++) {

				//change bias if parent2 fittest
				if (babyNeurons[offset]->index == parent2->GetBrain()->neurons[j]->index) {
					if (fittestParent == 2)
						babyNeurons[offset]->bias = parent2->GetBrain()->neurons[j]->bias;
					match = true;
				}

			}

			if (match) { match = false; }
			else {

				babyNeurons[offset] = new Neuron();
				babyNeurons[offset]->bias = parent2->GetBrain()->neurons[j]->bias;
				babyNeurons[offset]->index = parent2->GetBrain()->neurons[j]->index;
				babyNeurons[offset]->layer = parent2->GetBrain()->neurons[j]->layer;

			}

		}

		for (int k = 0; k < parent1->GetBrain()->connectionAmount; k++) {
			bool match = false;
			for (int j = 0; j < parent2->GetBrain()->connectionAmount; j++) {

				if (parent1->GetBrain()->connections[k]->innovation == parent2->GetBrain()->connections[j]->innovation) {

					match = true;
					int offset = 0;
					for (offset; babyConnections[offset] != nullptr; offset++);
					babyConnections[offset] = new Connection();
					babyConnections[offset]->from = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->from, babyNeurons);
					babyConnections[offset]->to = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->to, babyNeurons);
					babyConnections[offset]->disabled = parent1->GetBrain()->connections[k]->disabled;
					babyConnections[offset]->innovation = parent1->GetBrain()->connections[k]->innovation;
					babyConnections[offset]->weight = (parent1->GetBrain()->connections[k]->weight + parent2->GetBrain()->connections[j]->weight) / 2;

					babyConnections[offset]->to->Connections[babyConnections[offset]->to->conAmount] = babyConnections[offset];
					babyConnections[offset]->to->conAmount += 1;
				}

			}
			if (match) { match = false; }

			else {

				int offset = 0;
				for (offset; babyConnections[offset] != nullptr; offset++);
				babyConnections[offset] = new Connection();
				babyConnections[offset]->from = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->from, babyNeurons);
				babyConnections[offset]->to = SeekRelativeNeuron(parent1->GetBrain()->connections[k]->to, babyNeurons);
				babyConnections[offset]->disabled = parent1->GetBrain()->connections[k]->disabled;
				babyConnections[offset]->innovation = parent1->GetBrain()->connections[k]->innovation;
				babyConnections[offset]->weight = parent1->GetBrain()->connections[k]->weight;

				babyConnections[offset]->to->Connections[babyConnections[offset]->to->conAmount] = babyConnections[offset];
				babyConnections[offset]->to->conAmount += 1;
			}

		}

		for (int k = 0; k < parent2->GetBrain()->connectionAmount; k++) {
			bool match = false;
			for (int j = 0; j < parent1->GetBrain()->connectionAmount; j++) {

				if (parent1->GetBrain()->connections[j]->innovation == parent2->GetBrain()->connections[k]->innovation) match = true;

			}

			if (match) match = false;
			else {

				int offset = 0;
				for (offset; babyConnections[offset] != nullptr; offset++);
				babyConnections[offset] = new Connection();
				babyConnections[offset]->from = SeekRelativeNeuron(parent2->GetBrain()->connections[k]->from, babyNeurons);
				babyConnections[offset]->to = SeekRelativeNeuron(parent2->GetBrain()->connections[k]->to, babyNeurons);
				babyConnections[offset]->disabled = parent2->GetBrain()->connections[k]->disabled;
				babyConnections[offset]->innovation = parent2->GetBrain()->connections[k]->innovation;
				babyConnections[offset]->weight = parent2->GetBrain()->connections[k]->weight;

				babyConnections[offset]->to->Connections[babyConnections[offset]->to->conAmount] = babyConnections[offset];
				babyConnections[offset]->to->conAmount += 1;
			}

		}

		Entity* baby = new Entity(MyWorld, id, &historyMark, babyNeurons, babyConnections);


		Mutate(baby);
		/*int offset = 0;
		for (offset; babies[offset] != nullptr; offset++);
		offset += 1;
		babies[offset] = baby;*/

}

void Environment::Mutate(Entity* baby) {

	int mutation = rand() % 3;

	int doMutate = 0 /*rand() % 2*/;

	if (doMutate == 0) {

		if (mutation == 0) {

			//Weight Mutation
			for (int count = 0; count < WEIGHT_MUT_AMOUNT; count++) {

				if (randfloat() > WEIGHT_MUT_PROB) {

					int chosen = rand() % baby->GetBrain()->connectionAmount;

					baby->GetBrain()->connections[chosen]->weight = randfloat() * randposneg();

				}
			}

		}

		else if (mutation == 1) {

			//bias mutation
			for (int count = 0; count < BIAS_MUT_AMOUNT; count++) {

				if (randfloat() > BIAS_MUT_PROB) {

					int chosen = rand() % baby->GetBrain()->neuronAmount;

					if (baby->GetBrain()->neurons[chosen]->layer != -1)
						baby->GetBrain()->neurons[chosen]->bias = randfloat() * randposneg();

				}
			}
		}

		else {
			//structural mutation
			for (int count = 0; count < STRUCT_MUT_AMOUNT; count++) {

				if (randfloat() > STRUCT_MUT_PROB) {

					if (rand() % 2 == 0)

						baby->GetBrain()->CreateRandomConnection();

					else
						baby->GetBrain()->CreateRandomNeuron();

				}

			}
		}

	}
}

Neuron * Environment::SeekRelativeNeuron(Neuron * bull, Neuron * Array[])
{

	for (int k = 0; Array[k] != nullptr; k++) {

		if (Array[k]->index == bull->index)
			return Array[k];

	}

	std::cout << "No match found from parent to baby neurons" << std::endl;

	return nullptr;
}

float Environment::AdjustFitness(Entity* organism)
{

	/*if (organism->GetFitness() < 0) {
		organism->SetAdjFitness(0.001f);
		return 0.001f;
	}*/

	float sum = 0.0f;

	for (int i = 0; i < MAX_ENTITIES; i++) {

		if (GetCompDistance(organism, entities[i]) < COMPATIBILITY_TRESHOLD)
			sum += 1.0f;
	}

	organism->SetAdjFitness(organism->GetFitness() / sum);


	return (organism->GetFitness() / sum);
}

void Environment::Clonate(Neuron * babiesN[], Connection * babiesC[], Entity * cloner)
{
	Brain* cBrain = cloner->GetBrain();

	for (int i = 0; i < cBrain->neuronAmount; i++) {

		babiesN[i] = new Neuron();
		babiesN[i]->bias = cBrain->neurons[i]->bias;
		babiesN[i]->index = cBrain->neurons[i]->index;
		babiesN[i]->layer = cBrain->neurons[i]->layer;
		babiesN[i]->value = cBrain->neurons[i]->value;

	}

	for (int i = 0; i < cBrain->connectionAmount; i++) {

		babiesC[i] = new Connection();
		babiesC[i]->disabled = cBrain->connections[i]->disabled;
		babiesC[i]->from = SeekRelativeNeuron(cBrain->connections[i]->from, babiesN);
		babiesC[i]->to = SeekRelativeNeuron(cBrain->connections[i]->to, babiesN);
		babiesC[i]->innovation = cBrain->connections[i]->innovation;
		babiesC[i]->weight = cBrain->connections[i]->weight;

		babiesC[i]->to->Connections[babiesC[i]->to->conAmount] = babiesC[i];
		babiesC[i]->to->conAmount += 1;


	}


}

float Environment::GetCompDistance(Entity * org1, Entity * org2)
{

	Brain* brain1 = org1->GetBrain();
	Brain* brain2 = org2->GetBrain();

	int excess = 0;
	float avgWeightDiff = 0;
	int matchConn = 0;
	float diffSum = 0.0f;
	int maxGenes = 0;
	float compatDistance = 0.0f;


	if (brain1->connectionAmount > brain2->connectionAmount)
		maxGenes = brain1->connectionAmount;
	else
		maxGenes = brain2->connectionAmount;

	if (maxGenes < 20)
		maxGenes = 1;

	bool match = false;
	for (int i = 0; i < brain2->connectionAmount; i++) {

		for (int k = 0; k < brain1->connectionAmount; k++) {

			if (brain1->connections[k]->innovation == brain2->connections[i]->innovation) {

				diffSum += brain1->connections[k]->weight - brain2->connections[i]->weight;
				matchConn += 1;
				match = true;
			}
		}

		if (!match) {

			excess++;

		}
		match = false;
	}

	avgWeightDiff = diffSum / (float)matchConn;

	compatDistance = ((EXCESS_IMPORTANCE*excess) / maxGenes) + (WEIGHT_IMPORTANCE*avgWeightDiff);

	return compatDistance;
}

DebugDraw * Environment::GetDebugDraw()
{
	return mDebugDraw;
}

void Environment::CreateFloor()
{


	b2BodyDef bd;
	b2Body* ground = MyWorld->CreateBody(&bd);

	b2EdgeShape shape;
	shape.Set(b2Vec2(-100.0f, 0.0f), b2Vec2(100.0f, 0.0f));
	ground->CreateFixture(&shape, 0.0f);


}

void Environment::CreatePopulation(int amount)
{

	for (int i = 0; i < amount; i++) {

		Entity* baby = new Entity(MyWorld, i, &historyMark);

		entities[i] = baby;
		ChooseSpecies(baby);
	}

}

void Environment::ChooseSpecies(Entity * baby)
{

	if (speciesAmount == 0) {

		species[0] = new Species(baby);
		speciesAmount += 1;
		return;

	}
	//look for match
	for (int i = 0; i < speciesAmount; i++) {

		if (species[i]->isCompatible(baby)) {

			species[i]->addPartecipant(baby);
			return;

		}

	}

	//no match, create new species

	species[speciesAmount] = new Species(baby);
	speciesAmount += 1;
	return;

}

void Environment::Doo() {

	//koo = false;
	//entities[0]->bodyjoint->SetMotorSpeed(entities[0]->bodyjoint->GetMotorSpeed()*-1);

}

Species::Species(Entity* rep) {

	addPartecipant(rep);
	pickRep();

}

bool Species::isCompatible(Entity* test)
{
	int excess = 0;
	//int disjoint = 0;
	float avgWeightDiff = 0;

	//Entity* tester = partecipants[rand() % partAmount];

	int matchConn = 0;
	float diffSum = 0.0f;

	int maxGenes = 0;

	float compatDistance = 0.0f;

	//highest innovations
	/*for (int i = 0; i < tester->GetBrain()->connectionAmount; i++) {

		if (tester->GetBrain()->connections[i]->innovation > testerInn)
			testerInn = tester->GetBrain()->connections[i]->innovation;

	}
	for (int k = 0; k < test->GetBrain()->connectionAmount; k++) {

		if (test->GetBrain()->connections[k]->innovation > testInn)
			testInn = test->GetBrain()->connections[k]->innovation;
	}*/

	if (repAmount > test->GetBrain()->connectionAmount)
		maxGenes = repAmount;
	else
		maxGenes = test->GetBrain()->connectionAmount;

	if (maxGenes < 20)
		maxGenes = 1;

	bool match = false;
	for (int i = 0; i < test->GetBrain()->connectionAmount; i++) {

		for (int k = 0; k < repAmount; k++) {

			if (representative[k]->innovation == test->GetBrain()->connections[i]->innovation) {

				diffSum += representative[k]->weight - test->GetBrain()->connections[i]->weight;
				matchConn += 1;
				match = true;
			}
		}

		if (!match) {

			excess++;

		}
		match = false;
	}

	avgWeightDiff = diffSum / (float)matchConn;

	compatDistance = ((EXCESS_IMPORTANCE*excess) / maxGenes) + (WEIGHT_IMPORTANCE*avgWeightDiff);

	if (compatDistance < COMPATIBILITY_TRESHOLD)
		return true;

	return false;
}

void Species::addPartecipant(Entity * member)
{

	partecipants[partAmount] = member;
	partAmount += 1;

}

Entity * Species::getMateEntity()
{
	Entity* hero;

	bool dead = true;

	for (int i = 0; i < partAmount; i++) {

		if (!partecipants[i]->worst)
			dead = false;

	}

	if (dead) {

		partecipants[rand() % partAmount]->worst = false;

	}

doagain:
	hero = partecipants[rand() % partAmount];
	if (hero->worst)
		goto doagain;

	return hero;
}

void Species::pickRep()
{
	if (partAmount == 0)
		return;

	Brain* repEntity = partecipants[rand() % partAmount]->GetBrain();
	//delete[] representative;
	representative[500] = { nullptr };
	repAmount = repEntity->connectionAmount;
	for (int i = 0; i < repEntity->connectionAmount; i++) {

		representative[i] = new Connection();
		representative[i]->innovation = repEntity->connections[i]->innovation;
		representative[i]->weight = repEntity->connections[i]->weight;

	}

}

void Species::clear()
{
	partAmount = 0;
	offsprings = 0;
}

void Species::update()
{

	age += 1;

	float nowBestFitness = 0.0f;

	for (int i = 0; i < partAmount; i++) {

		if (partecipants[i]->GetFitness() > nowBestFitness)
			nowBestFitness = partecipants[i]->GetFitness();

	}

	if (nowBestFitness > bestFitness) {

		bestFitness = nowBestFitness;
		bestFitnessAge = age;

	}

	bool dying = false;

	if ((age - bestFitnessAge) - DROPOFF_AGE == 0)
		dying = true;

	if (dying) {

		for (int i = 0; i < partAmount; i++) {

			partecipants[i]->SetFitness(partecipants[i]->GetFitness() / 100);

		}

	}


}

void Species::setParents()
{

	int parents = (SURVIVAL_TRESHOLD * partAmount) + 1;


	float* fitnesses[MAX_ENTITIES]{ nullptr };

	for (int i = 0; i < partAmount; i++) {

		fitnesses[i] = new float(partecipants[i]->GestAdjFitness());

	}

	std::vector<float> fitnessSort(*fitnesses, *fitnesses + partAmount);

	std::sort(fitnessSort.begin(), fitnessSort.end());

	std::vector<float>::iterator curr;
	int count = 0;
	for (curr = fitnessSort.begin(); curr != fitnessSort.end(); ++curr) {

		if (count > parents) {
			for (int i = 0; i < partAmount; i++) {

				if (partecipants[i]->GestAdjFitness() == *curr)
					partecipants[i]->worst = true;
			}

		}
		count++;
	}

}
