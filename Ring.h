#pragma once
#include "Entity.h"
namespace Sanic {
	class Ring : public Entity
	{
	public:
		Ring();
		~Ring();

		void CheckCollision();

		void Physics() override;
		void Render(int camX, int camY) override;

		bool GetCollected() { return collected; };
		void SetCollected(bool c) { collected = c; };
		bool isStatic() { return staticRing; };
	private:
		bool collected = false;
		bool staticRing = false;
		SDL_Rect ringCol;
	};
}