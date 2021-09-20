#pragma once
#include "Ring.h"
#include <vector>
namespace Sanic {
	class EntityManager
	{
	public:
		void LoadEntities(const std::string& path);
		void Update();
		void RenderEntities(int camX, int camY);
		std::vector<Ring*> getRings() { return rings; };
	private:
		std::vector<Ring*> rings;
	};
}


