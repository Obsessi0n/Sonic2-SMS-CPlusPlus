#pragma once
#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <string>
#include <map>
#include "SDL.h"

namespace Sanic {
	class TextureManager
	{
	public:
		static TextureManager* Instance() {
			if (instance == 0)
			{
				instance = new TextureManager();
			}
			return instance;
		}

		// A picture is a BMP of 16 colours or fewer, as tools/make_art.py writes them: colour 0 of its palette is
		// see-through, the way the colour 0 of a sprite was on the consoles of the time. SDL reads BMP by itself, so
		// the game needs no library for pictures.
		bool Load(const std::string& fileName, const std::string& id, SDL_Renderer* renderer);

		void Draw(const std::string& id, int x, int y, int w, int h, double scale, SDL_Renderer* renderer, SDL_RendererFlip flip = SDL_FLIP_NONE);

		void DrawFrame(const std::string& id, int x, int y, int w, int h, double scale, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip = SDL_FLIP_NONE);

		void DrawAnimation(const std::string& id, int x, int y, int w, int h, double scale, int currentRow, int frameAmount, int speed, SDL_Renderer* renderer, SDL_RendererFlip flip = SDL_FLIP_NONE);

		void ClearFromSpritesheet(const std::string& id);

		std::map<std::string, SDL_Texture*> spritesheet;

	private:
		static TextureManager* instance;

		TextureManager(){}
		~TextureManager(){}
	};

	typedef TextureManager _TextureManager;
}

#endif
