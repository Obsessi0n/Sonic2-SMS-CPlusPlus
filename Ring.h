#pragma once
#include "Entity.h"
namespace Sanic {
	class Ring : public Entity
	{
	public:
		Ring();
		~Ring();
		void Physics() override;
		void Render(int camX, int camY) override;

		bool isCollected() { return collected; };
		bool isStatic() { return staticRing; };
	private:
		bool collected = false;
		bool staticRing = false;
	};
}