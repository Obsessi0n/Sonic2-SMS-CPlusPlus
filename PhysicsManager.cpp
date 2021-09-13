#include "PhysicsManager.h"
#include <iostream>



Sanic::PhysicsManager* Sanic::PhysicsManager::instance = 0;

Sanic::PhysicsManager::PhysicsManager() {

}

Sanic::PhysicsManager::~PhysicsManager() {

}
float Sanic::PhysicsManager::CalculateGravityForce(float _fallingTimer) {

	float factor = 2;

	float velocity = (GRAVITY  * (ACCELERATION * (_fallingTimer/1000) / factor));

	if(velocity > TERMINALVELOCITY)
		velocity = TERMINALVELOCITY;	

	return velocity;

}