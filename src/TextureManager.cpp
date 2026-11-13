#include "TextureManager.h"
#include "Game.h"
#include <iostream>

Sanic::TextureManager* Sanic::TextureManager::instance = 0;

bool Sanic::TextureManager::Load(std::string fileName, std::string id, SDL_Renderer* renderer)
{
	// With no renderer there is nothing to draw with, and so no use for pictures: the game is running without a
	// window, for a test of its rules.
	if (renderer == nullptr)
		return false;

	// A picture already loaded under this name is kept: loading it twice would only leave the first one behind.
	if (spritesheet.find(id) != spritesheet.end())
		return true;

	SDL_Surface* tempSurface = IMG_Load(fileName.c_str());

	if (tempSurface == nullptr)
	{
		std::cerr << "Image with name " << fileName.c_str() << " could not be loaded! " << IMG_GetError() << '\n';
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

	return true;
}

void Sanic::TextureManager::Draw(std::string id, int x, int y, int w, int h, double scale, SDL_Renderer* renderer, SDL_RendererFlip flip)
{
	auto found = spritesheet.find(id);
	if (found == spritesheet.end())
		return;

	SDL_Rect srcRect, destRect;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = w;
	srcRect.h = destRect.h = h;
	destRect.x = x;
	destRect.y = y;

	destRect.w = (int)(destRect.w * scale);
	destRect.h = (int)(destRect.h * scale);

	SDL_RenderCopyEx(renderer, found->second, &srcRect, &destRect, 0, nullptr, flip);
}

void Sanic::TextureManager::DrawFrame(std::string id, int x, int y, int w, int h, double scale, int currentRow, int currentFrame, SDL_Renderer* renderer, SDL_RendererFlip flip)
{
	auto found = spritesheet.find(id);
	if (found == spritesheet.end())
		return;

	SDL_Rect srcRect, destRect;

	srcRect.x = w * currentFrame;
	srcRect.y = h * currentRow;
	srcRect.w = destRect.w = w;
	srcRect.h = destRect.h = h;
	destRect.x = x;
	destRect.y = y;

	SDL_RenderCopyEx(renderer, found->second, &srcRect, &destRect, 0, nullptr, flip);
}

void Sanic::TextureManager::DrawAnimation(std::string id, int x, int y, int w, int h, double scale, int currentRow, int frameAmount, int speed, SDL_Renderer* renderer, SDL_RendererFlip flip)
{
	// The game's own clock, not the one on the wall: an animation moves on with the updates, and a frame drawn
	// after the same number of updates is always the same frame.
	uint32_t ticks = Sanic::_Game::Instance()->getTicks();
	uint32_t currentFrame = 0;

	if (speed != 0)
	{
		currentFrame = (ticks / speed) % frameAmount;
	}

	DrawFrame(id, x, y, w, h, scale, currentRow, (int)currentFrame, renderer, flip);
}

void Sanic::TextureManager::ClearFromSpritesheet(std::string id)
{
	auto found = spritesheet.find(id);
	if (found == spritesheet.end())
		return;

	SDL_DestroyTexture(found->second);
	spritesheet.erase(found);
}
