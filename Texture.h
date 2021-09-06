#pragma once
#include <SDL.h>
#include <SDL_image.h>

namespace Sanic {
	class Texture
	{
		SDL_Surface* surface;
		SDL_Texture* texture;
	};
}


