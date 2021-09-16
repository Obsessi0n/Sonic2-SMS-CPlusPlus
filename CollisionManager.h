#pragma once
#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H
#include "SDL.h"


namespace Sanic {
	class CollisionManager
	{
	public:
		static CollisionManager* Instance() {
			if (instance == 0)
			{
				instance = new CollisionManager();
			}
			return instance;
		}

		CollisionManager();
		~CollisionManager();
		bool CheckCollision(SDL_Rect A, SDL_Rect B);

	private:
		static CollisionManager* instance;

	};
	typedef CollisionManager _CollisionManager;
}

#endif