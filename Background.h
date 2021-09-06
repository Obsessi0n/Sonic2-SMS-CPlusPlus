#pragma once
#include <iostream>
#include "SDL.h"


namespace Sanic {
	class Background {

	public:
		Background();
		~Background();

		void DrawBackground(SDL_Renderer* _render);


	private:
		SDL_Rect* empty;
	};
}
