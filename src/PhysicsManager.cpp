#include "PhysicsManager.h"
#include "Game.h"


Sanic::PhysicsManager* Sanic::PhysicsManager::instance = 0;

Sanic::PhysicsManager::PhysicsManager() {

}

Sanic::PhysicsManager::~PhysicsManager() {

}

bool Sanic::PhysicsManager::IsSolid(int const& xPixel, int const& yPixel) {
	int tileSize = Sanic::_Game::Instance()->getTileSize();
	MapLoader* map = Sanic::_Game::Instance()->getMapLoader();

	//We convert the pixel into tile number
	int xBlock = ToBlock(xPixel);
	int yBlock = ToBlock(yPixel);

	//A platform that is only a floor from above is no wall and no ceiling
	if (!map->IsSolidBlock(xBlock, yBlock))
		return false;

	//How high the ground stands in this column of the tile: the pixel is in it when it is no higher than that
	int height = map->GetGroundHeight(xBlock, yBlock, xPixel - xBlock * tileSize);

	return yPixel - yBlock * tileSize >= tileSize - height;
}

Sanic::PhysicsManager::FloorCheck Sanic::PhysicsManager::CheckFloor(int const& xPixel, int const& yPixel, int const& ySpeed, int* surfaceY, int* slopePush) {
	int tileSize = Sanic::_Game::Instance()->getTileSize();
	MapLoader* map = Sanic::_Game::Instance()->getMapLoader();

	int xBlock = ToBlock(xPixel);
	int yBlock = ToBlock(yPixel);
	int column = xPixel - xBlock * tileSize;

	if (map->IsSolidBlock(xBlock, yBlock)) {
		int height = map->GetGroundHeight(xBlock, yBlock, column);
		int push = map->GetSlopePush(xBlock, yBlock);

		//This column is ground to the top of its tile: the surface may be further up, in the tile above
		if (height == tileSize && map->IsSolidBlock(xBlock, yBlock - 1)) {
			int heightAbove = map->GetGroundHeight(xBlock, yBlock - 1, column);
			if (heightAbove > 0) {
				height += heightAbove;
				push = map->GetSlopePush(xBlock, yBlock - 1);
			}
		}

		int surface = (yBlock + 1) * tileSize - height;
		//Above the surface there is nothing to stand on yet. At it or under it, however deep and whichever
		//way the point was moving, the surface is where it belongs.
		if (yPixel < surface)
			return ABOVE_GROUND;

		*surfaceY = surface;
		*slopePush = push;
		return ON_GROUND;
	}

	if (map->IsOneWayBlock(xBlock, yBlock)) {
		int surface = (yBlock + 1) * tileSize - map->GetGroundHeight(xBlock, yBlock, column);
		//A platform catches only what comes down on to it, and only if it is not already too far through
		if (ySpeed > 0 && yPixel >= surface && yPixel - surface < ToPixel(ySpeed) + ONE_WAY_REACH) {
			*surfaceY = surface;
			*slopePush = 0;
			return ON_GROUND;
		}
		return ABOVE_GROUND;
	}

	return NO_GROUND;
}
