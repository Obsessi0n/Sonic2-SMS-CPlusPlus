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

bool Sanic::PhysicsManager::IsColliding(float const& xLiteralPosition, float const& yLiteralPosition) {
	//later to be replaced with collisionboxes
	int playerSizeTile = 1;

	//We convert the literal position into tile number
	int xBlock = xLiteralPosition / 32;
	int yBlock = yLiteralPosition / 32;

	//If player is outside bounds on the Y
	if (yBlock + playerSizeTile > Sanic::_Game::Instance()->getMapLoader()->GetMapSizeY())
		return true;
	else if (yLiteralPosition <= 5) //if y < 0 we are in big shit. Player will be out of bounds and SDL will crash.
		return true;

	//Now we check the map	
	if (Sanic::_Game::Instance()->getMapLoader()->GetBlockType(xBlock, yBlock) != 0) {
		return true;
	}

	//If we get to this point there is no collision
	return false;

}

/*NotGrounded = 0
* Floor = 1
* Slope = 2
*/
char Sanic::PhysicsManager::CheckIfGrounded(float const& xLiteralPosition, const float& yLiteralPosition) {

	//Convert Literal Position to block.
	int xBlock = xLiteralPosition / Sanic::_Game::Instance()->getTileSize();
	int yBlock = yLiteralPosition / Sanic::_Game::Instance()->getTileSize();;
	char blockType = 0;
	//We check the tile
	blockType = Sanic::_Game::Instance()->getMapLoader()->GetBlockType(xBlock, yBlock);
	if (blockType == 1) { return blockType; }
	
	else if (blockType == 2) {
		int slopeY = CalculateSlope(xBlock, yBlock);

		if (Sanic::_Game::Instance()->getPlayer()->getPosY() + 32 >= slopeY) {
			Sanic::_Game::Instance()->getPlayer()->OnSlope(&slopeY);
			return 2;
		}
			
	}
	return 0;

}

int Sanic::PhysicsManager::CalculateSlope(int const& xBlock, int const& yBlock) {

	int yStart=-1;
	int yEnd=-1;
	//First we need to get the yStart and yEnd
	Sanic::_Game::Instance()->getMapLoader()->GetSlopeStartEnd(&yStart, &yEnd, xBlock, yBlock);



	//Find b
	int b = yStart;

	//find m
	float temp = (yEnd - b);
	float m = temp / 32;

	//Now we have
	//y = mx+b

	//Slope y on the x
	int slopeY = (m * (Sanic::_Game::Instance()->getPlayer()->getPosX() - (32 * xBlock)) + b) + (yBlock * 32);

	return slopeY;

}