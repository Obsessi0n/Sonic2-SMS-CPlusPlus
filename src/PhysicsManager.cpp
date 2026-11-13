#include "PhysicsManager.h"
#include <cmath>
#include <iostream>
#include "Game.h"


Sanic::PhysicsManager* Sanic::PhysicsManager::instance = 0;

Sanic::PhysicsManager::PhysicsManager() {

}

Sanic::PhysicsManager::~PhysicsManager() {

}

bool Sanic::PhysicsManager::IsSolid(float const& xLiteralPosition, float const& yLiteralPosition) {
	int tileSize = Sanic::_Game::Instance()->getTileSize();

	//We convert the literal position into the pixel it is in, and the pixel into tile number
	int xPixel = (int)std::floor(xLiteralPosition);
	int yPixel = (int)std::floor(yLiteralPosition);
	int xBlock = (int)std::floor(xLiteralPosition / tileSize);
	int yBlock = (int)std::floor(yLiteralPosition / tileSize);

	//How high the ground stands in this column of the tile: the point is in it when it is no higher than that
	int height = Sanic::_Game::Instance()->getMapLoader()->GetGroundHeight(xBlock, yBlock, xPixel - xBlock * tileSize);

	return yPixel - yBlock * tileSize >= tileSize - height;
}

bool Sanic::PhysicsManager::FindFloor(float const& xLiteralPosition, float const& yLiteralPosition, int* surfaceY, float* angle) {
	int tileSize = Sanic::_Game::Instance()->getTileSize();
	MapLoader* map = Sanic::_Game::Instance()->getMapLoader();

	int xPixel = (int)std::floor(xLiteralPosition);
	int xBlock = (int)std::floor(xLiteralPosition / tileSize);
	int yBlock = (int)std::floor(yLiteralPosition / tileSize);
	int column = xPixel - xBlock * tileSize;

	int height = map->GetGroundHeight(xBlock, yBlock, column);

	if (height == tileSize) {
		//This column is ground to the top of its tile: the surface may be further up, in the tile above
		int heightAbove = map->GetGroundHeight(xBlock, yBlock - 1, column);
		if (heightAbove > 0) {
			yBlock = yBlock - 1;
			height = heightAbove;
		}
	}
	else if (height == 0) {
		//Nothing in this column of the tile: the ground may be just under it, in the tile below
		height = map->GetGroundHeight(xBlock, yBlock + 1, column);
		if (height == 0)
			return false;
		yBlock = yBlock + 1;
	}

	*surfaceY = yBlock * tileSize + tileSize - height;
	*angle = map->GetGroundAngle(xBlock, yBlock);
	return true;
}
