#include "PhysicsManager.h"
#include <iostream>
#include "Game.h"


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

//Later this needs to be adapted for collisionboxes

bool Sanic::PhysicsManager::IsColliding(int x, int y) {
	//later to be replaced with collisionboxes
	int playerSizeTile = 1;

	//We convert the literal position into tile number
	int xBlock = x / 32;
	int yBlock = y / 32;

	//If player is outside bounds on the Y
	if (yBlock + playerSizeTile > Sanic::_Game::Instance()->getMapLoader()->GetMapSizeY())
		return true;
	else if (y <= 5) //if y < 0 we are in big shit. Player will be out of bounds and SDL will crash.
		return true;

	//Now we check the map	
	if (Sanic::_Game::Instance()->getMapLoader()->IsTileSolid(xBlock, yBlock)) {
		return true;
	}

	//If we get to this point there is no collision
	return false;

}