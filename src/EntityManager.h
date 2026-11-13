#pragma once
#include "Ring.h"

#include <SDL.h>
#include <vector>

namespace Sanic {
	class EntityManager
	{
	public:
		~EntityManager();

		// One thing to pick up at each place the level names, in pixels.
		void LoadPickups(const std::vector<SDL_Point>& places);
		void Update();
		void RenderEntities(int camX, int camY);

		std::vector<Ring*> getRings() { return rings; };
	private:
		std::vector<Ring*> rings;
	};
}
