#pragma once
#include <assert.h>
#include <string>
#include <map>
#include "SDL.h"
#include "SDL_image.h"
#include "Game.h"
namespace Sanic {
	class TextureManager
	{
	public:
		TextureManager() {
			assert(!instantiated);
			instantiated = true;
		}

		~TextureManager() { instantiated = false; }

		bool Load(std::string fileName, std::string id, SDL_Renderer* renderer);
		
		void Draw(std::string id, int x, int y, int w, int h, SDL_Renderer* renderer, SDL_RendererFlip flip = SDL_FLIP_NONE);
		void Draw(std::string id, int x, int y, int w, int h, SDL_Renderer* renderer, SDL_RendererFlip flip = SDL_FLIP_NONE);
		
		void DrawFrame(std::string id, int x, int y, int w, int h, double scale, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip = SDL_FLIP_NONE);
		
		void ClearFromSpritesheet(std::string id);

		std::map<std::string, SDL_Texture*> spritesheet;

	private:
		Game* game = nullptr;
		static bool instantiated;
	};
}


