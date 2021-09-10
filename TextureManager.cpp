#include "TextureManager.h"
#include <iostream>

Sanic::TextureManager* Sanic::TextureManager::instance = 0;

bool Sanic::TextureManager::Load(std::string fileName, std::string id, SDL_Renderer* renderer)
{
    SDL_Surface* tempSurface = IMG_Load(fileName.c_str());
	if (tempSurface == nullptr)
	{
		std::cerr << "Image with name " << fileName.c_str() << " could not be loaded!" << '\n';
		return false;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
	SDL_FreeSurface(tempSurface);

	if (texture == nullptr)
	{
		std::cerr << "Could not create texture from surface with name " << fileName.c_str() << '\n';
		return false;
	}

	spritesheet[id] = texture;
    return false;
}

void Sanic::TextureManager::Draw(std::string id, int x, int y, int w, int h, SDL_Renderer* renderer, SDL_RendererFlip flip)
{
	SDL_Rect srcRect, destRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = w;
	srcRect.h = destRect.h = h;
	destRect.x = x;
	destRect.y = y;

	SDL_RenderCopyEx(renderer, spritesheet[id], &srcRect, &destRect, 0, nullptr, flip);
}

void Sanic::TextureManager::DrawFrame(std::string id, int x, int y, int w, int h, double scale, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip flip)
{
	SDL_Rect srcRect, destRect;

	srcRect.x = w * currentFrame;
	srcRect.y = h * currentRow;
	srcRect.w = destRect.w = w;
	srcRect.h = destRect.h = h;
	destRect.x = x;
	destRect.y = y;

	SDL_RenderCopyEx(renderer, spritesheet[id], &srcRect, &destRect, 0, nullptr, flip);
}

void Sanic::TextureManager::ClearFromSpritesheet(std::string id)
{
	spritesheet.erase(id);
}


