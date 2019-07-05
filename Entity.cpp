#include "Entity.h"
#include <iostream>



Entity::Entity(b2World* world, int id, HistoryMark* hm)
{

	MyWorld = world;
	mind.historyMark = hm;

	CreateBody();
	
	initPositions = body1->GetPosition().x;
	CreateMind();

}

Entity::Entity(b2World * world, int id, HistoryMark * hm, Neuron * bNeurons[], Connection* bConnections[])
{

	MyWorld = world;
	mind.historyMark = hm;
	CreateBody();
	initPositions = body1->GetPosition().x;
	CreateMind(bNeurons, bConnections);
}

Entity::Entity(int id, Entity * cloner)
{
	MyWorld = cloner->MyWorld;
	mind.historyMark = cloner->mind.historyMark;

	CreateBody();
	initPositions = body1->GetPosition().x;

	Brain* ClonerBrain = cloner->GetBrain();

	mind.connectionAmount = ClonerBrain->connectionAmount;
	mind.neuronAmount = ClonerBrain->neuronAmount;

	for (int i = 0; i < ClonerBrain->connectionAmount; i++) {
	
		mind.connections[i]->disabled = ClonerBrain->connections[i]->disabled;

	
	}

}

Entity::~Entity()
{

	DestroyBody();

	mind.Erase();

}

void Entity::CreateMind()
{

	//input
	mind.CreateNeuron(0, (randfloat()*randposneg()));
	mind.CreateNeuron(0, (randfloat()*randposneg()));
	mind.CreateNeuron(0, (randfloat()*randposneg()));

	//output
	mind.CreateNeuron(-1, (randfloat()*randposneg()));

	

	mind.CreateConnection(mind.neurons[0], mind.neurons[3], (randfloat()*randposneg()));
	mind.CreateConnection(mind.neurons[1], mind.neurons[3], (randfloat()*randposneg()));
	mind.CreateConnection(mind.neurons[2], mind.neurons[3], (randfloat()*randposneg()));

	

}

void Entity::CreateMind(Neuron * bNeurons[], Connection* bConnections[])
{

/*	for (int k = 0; bNeurons[k] != nullptr; k++) {
	
		mind.CreateNeuron(bNeurons[k]->layer, bNeurons[k]->bias);
	
	}

	for (int k = 0; bConnections[k] != nullptr; k++) {
	
		mind.CreateConnection(bConnections[k]->from, bConnections[k]->to, bConnections[k]->weight);
	
	}*/
	int k;
	for ( k = 0; bConnections[k] != nullptr; k++) {
	
		mind.connections[k] = bConnections[k];
	
	}
	mind.connectionAmount = k;

	for ( k = 0; bNeurons[k] != nullptr; k++) {

		mind.neurons[k] = bNeurons[k];

	}
	mind.neuronAmount = k;
}

void Entity::AdjustPointers()
{



}

void Entity::Engage(float value)
{
	if(value > 0)

	bodyjoint->SetMotorSpeed(-45);
	else 
		bodyjoint->SetMotorSpeed(45);
		
		
	//bodyjoint->SetMotorSpeed(45*value);
}

void Entity::SetFitness(float fitt)
{
	fitness = fitt;

}

void Entity::CreateBody()
{
	//first piece

	b2BodyDef bd1;
	bd1.position.Set(10.0f, 3.0f);
	bd1.angle = 90 * DEGTORAD;
	bd1.type = b2_dynamicBody;
	bd1.allowSleep = false;

	body1 = MyWorld->CreateBody(&bd1);

	b2PolygonShape shape1;
	shape1.SetAsBox(2.0f, 1.0f);

	b2FixtureDef fd1;
	fd1.shape = &shape1;
	fd1.density = 1.0f;
	fd1.friction = 5.0f;
	fd1.filter.groupIndex = -1;
	body1->CreateFixture(&fd1);

	//second piece

	b2BodyDef bodyDef;
	bodyDef.angle = 90 * DEGTORAD;
	bodyDef.position.Set(12.0f, 3.0f);
	bodyDef.type = b2_dynamicBody;
	bodyDef.allowSleep = false;

	body2 = MyWorld->CreateBody(&bodyDef);

	b2PolygonShape dynamicBox;
	dynamicBox.SetAsBox(2.0f, 1.0f);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &dynamicBox;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 5.0f;
	fixtureDef.filter.groupIndex = -1;
	body2->CreateFixture(&fixtureDef);

	//body joint

	b2RevoluteJointDef rjd;

	rjd.Initialize(body1, body2, b2Vec2(0, 0));
	rjd.localAnchorA.Set(2.0f, -0.6f);
	rjd.localAnchorB.Set(-2.0f, 0.6f);
	rjd.motorSpeed = 124.0f * DEGTORAD;
	rjd.lowerAngle = -90 * DEGTORAD;
	rjd.upperAngle = 90 * DEGTORAD;
	rjd.enableLimit = true;
	rjd.maxMotorTorque = 800.0f;
	rjd.collideConnected = false;

	bodyjoint = (b2RevoluteJoint*)MyWorld->CreateJoint(&rjd);

	//body1->SetAngularDamping(5);
	//body2->SetAngularDamping(5);
	bodyjoint->EnableMotor(true);
	bodyjoint->SetMotorSpeed(45.0f);
	

/*	b2BodyDef bod;
	bod.type = b2_dynamicBody;
	b2FixtureDef fix;
	fix.density = 1;

	//two shapes
	b2PolygonShape boxShape;
	boxShape.SetAsBox(2, 2);
	b2CircleShape circleShape;
	circleShape.m_radius = 2;

	//make box a little to the left
	bod.position.Set(-3, 10);
	fix.shape = &boxShape;
	b2Body * m_bodyA = MyWorld->CreateBody(&bod);
	m_bodyA->CreateFixture(&fix);

	//and circle a little to the right
	bod.position.Set(3, 10);
	fix.shape = &circleShape;
	b2Body *m_bodyB = MyWorld->CreateBody(&bod);
	m_bodyB->CreateFixture(&fix);

	b2RevoluteJointDef revoluteJointDef;
	revoluteJointDef.bodyA = m_bodyA;
	revoluteJointDef.bodyB = m_bodyB;
	revoluteJointDef.collideConnected = false;
	revoluteJointDef.localAnchorA.Set(2, 2);//the top right corner of the box
	revoluteJointDef.localAnchorB.Set(0, 0);//center of the circle
	revoluteJointDef.enableMotor = true;
	revoluteJointDef.maxMotorTorque = 10000.0f;
	revoluteJointDef.motorSpeed = 2.0f;
	MyWorld->CreateJoint(&revoluteJointDef);*/ 

}

void Entity::DestroyBody()
{
	MyWorld->DestroyJoint(bodyjoint);
	MyWorld->DestroyBody(body1);
	MyWorld->DestroyBody(body2);

}

void Entity::SetAngularSpeed(int index, float32 speed)
{

	if (index == 1) {

		speed1 = speed;

	}
	if (index == 2) {

		speed2 = speed;

	}

}

void Entity::EvaluateFitness()
{

	fitness = body1->GetPosition().x ; 
	
	if (fitness < 0)
		fitness = 0.0;

	
	
}

float Entity::GetFitness()
{
	return this->fitness;
}

float Entity::GestAdjFitness()
{
	return adjustedFitness;
}

void Entity::SetAdjFitness(float Fi)
{

	adjustedFitness = Fi;
	return;
}

void Entity::UpdateForces()
{
	Output output;
	int analyzed = 0;
	//std::cout << body1->GetInertia()<< std::endl;
	//std::cout << body2->GetAngle() * RADTODEG << std::endl;
	//std::cout << (bodyjoint->GetJointAngle() / (90.0f * DEGTORAD)) << "   " << bodyjoint->GetMotorSpeed() / (180.0f * DEGTORAD) << std::endl;
	for (int i = 0; i < mind.neuronAmount; i++) { // setting sensor values
	
		if (mind.neurons[i]->layer == 0) {
		
			if (analyzed == 0) { 
				mind.neurons[i]->value = (body2->GetAngle()) / (360.0f*DEGTORAD); analyzed++; }	
			else if (analyzed == 1) { mind.neurons[i]->value = (bodyjoint->GetJointAngle() / (90.0f * DEGTORAD)); analyzed++; }//bodyjoint->GetJointAngle() / (90.0f * DEGTORAD)
			else mind.neurons[i]->value = (body1->GetAngle()) / (360.0f*DEGTORAD); 
		
		}
	
	
	}

	output = mind.RunNetwork();
	//std::cout << body1->GetPosition().x << std::endl;
	//std::cout << bodyjoint->GetJointSpeed() << "    " << output.out1 << std::endl;
	
	Engage(output.out1);

	//std::cout << 15 * output.out1 << "   " << -15 * output.out2 << "    " << output.out1 << "    " << output.out2 << std::endl;

	//body1->ApplyAngularImpulse(15*output.out1, true);
	//body2->ApplyAngularImpulse(-15*output.out2, true);

	//if(output.out1 > 0.5f)body1->ApplyAngularImpulse(10,true); else body1->ApplyAngularImpulse(-10,true);
	//if(output.out2 > 0.5f)body2->ApplyAngularImpulse(-10,true); else body2->ApplyAngularImpulse(10,true);
	
	

}

Brain * Entity::GetBrain()
{
	return &mind;
}

Neuron* Brain::CreateNeuron(int Layer, float Bias)
{

#define A this->neuronAmount

	this->neurons[A] = new Neuron();

	this->neurons[A]->layer = Layer;
	this->neurons[A]->bias = Bias;
	this->neurons[A]->index = this->neuronAmount;
	this->neuronAmount++;

	if (Layer > this->layers) this->layers++;

	return this->neurons[this->neuronAmount-1];
}

void Brain::CreateConnection(Neuron* From, Neuron* To, float Weight)
{

#define B this->connectionAmount

	if (To->layer <= From->layer && To->layer != -1) {
	
		std::cout << "Cannot create a connection From layer " << From->layer << " to " << To->layer << std::endl;
		return;

	}


	
	this->connections[B] = new Connection();

	this->connections[B]->from = From;
	this->connections[B]->to = To;
	this->connections[B]->weight = Weight;
	

	To->Connections[To->conAmount] = this->connections[B];
	
	this->connections[B]->innovation = this->GetHistoryMark(this->connections[B]);

	To->conAmount++;
	this->connectionAmount++;
}

int Brain::GetHistoryMark(Connection* connection) {
	//look for existing innovations
	for (int i = 0; i < historyMark->innovation; i++) {
	
		if (*historyMark->track[i][0] == connection->from->index) {
		
			if (*historyMark->track[i][1] == connection->to->index) {
			
				return i;
			
			}
		
		}
	
	}
	//no match found
	//historyMark->connections[historyMark->innovation] = connection;

	historyMark->track[historyMark->innovation][0] = new int(connection->from->index);
	historyMark->track[historyMark->innovation][1] = new int(connection->to->index);

	historyMark->innovation += 1;
	return historyMark->innovation-1;

}

void Brain::CreateRandomConnection()
{

#define A this->neuronAmount

	Neuron* From;
	Neuron* To;
	float weight;
again:
	From = this->neurons[rand() % (A)];
	To   = this->neurons[rand() % (A)];

	if (From->layer >= To->layer && To->layer != -1) goto again;
	if (From->layer == -1) goto again;
	

	for (int i = 0; i < this->connectionAmount; i++) {
	
		if (this->connections[i]->from == From) {
		
			if (this->connections[i]->to == To) return;
		
		}
	
	}

	weight = randfloat()*randposneg();

	CreateConnection(From, To, weight);
}

void Brain::CreateRandomNeuron() {//PROBLEM


	Connection* target = this->connections[rand() % this->connectionAmount];

	target->disabled = true;
	
	int oldLayer = target->from->layer;

	for (int i = 0; i < this->neuronAmount; i++) {
	
		if (this->neurons[i]->layer > oldLayer)
			this->neurons[i]->layer++;
	
	}

	Neuron* offspring;

	offspring = CreateNeuron(target->from->layer + 1, 0);

//	std::cout << "tip" << std::endl;
	CreateConnection(target->from, offspring, 1.0f);
	//std::cout << "top" << std::endl; PROBLEM
	CreateConnection(offspring, target->to, target->weight);
}

Output Brain::RunNetwork()
{

	Output output;

	for (int k = 1; k <= this->layers; k++) {

		for (int i = 0; i < this->neuronAmount; i++) {

			if (this->neurons[i]->layer == k) {
			
				float weightSum = 0;

				for (int b = 0; b < this->neurons[i]->conAmount; b++) {
				
					if(!this->neurons[i]->Connections[b]->disabled)
						weightSum += this->neurons[i]->Connections[b]->from->value * this->neurons[i]->Connections[b]->weight;
				
				}
				
				weightSum += this->neurons[i]->bias;
				this->neurons[i]->value = tanh(weightSum);
			
			}

		}

	}
	int analyzedOut = 0;

	for (int i = 0; i < this->neuronAmount; i++) {
	
		if (this->neurons[i]->layer == -1) {
		
			float weightSum = 0;

			for (int b = 0; b < this->neurons[i]->conAmount; b++) {

				weightSum += this->neurons[i]->Connections[b]->from->value * this->neurons[i]->Connections[b]->weight;

			}

			weightSum += this->neurons[i]->bias;

			this->neurons[i]->value = TanH(weightSum);

			if (analyzedOut == 0) {
			
				output.out1 = this->neurons[i]->value;
				analyzedOut++;
			}
			else if (analyzedOut == 1) {
				output.out2 = this->neurons[i]->value;
				analyzedOut++;
			}
		}
	
	}

	return output;

}

void Brain::Erase()
{

	for (int i = 0; i < connectionAmount; i++) {
	
		delete connections[i];
	
	}
	for (int i = 0; i < neuronAmount; i++) {
	
		delete neurons[i];
	
	}

}

