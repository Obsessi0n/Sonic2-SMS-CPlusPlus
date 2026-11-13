#include "CollisionManager.h"
#include "SDL.h"

Sanic::CollisionManager* Sanic::CollisionManager::instance = 0;


Sanic::CollisionManager::CollisionManager() {
}

Sanic::CollisionManager::~CollisionManager() {
}

bool Sanic::CollisionManager::CheckCollision(SDL_Rect A, SDL_Rect B) {



	SDL_bool col = SDL_HasIntersection(&A, &B);
	if (col)
		return true;
	else
		return false;
}