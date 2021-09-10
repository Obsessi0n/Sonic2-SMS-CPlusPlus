#pragma once
#include "Game.h"
#include "InputManager.h"

Sanic::Game* Sanic::Game::instance = 0;

bool Sanic::Game::init(const char* title, int x, int y, int flags) {
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow(title, x, y, SCREEN_WIDTH, SCREEN_HEIGHT, flags);

		if (window == nullptr)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError();
			is_running = false;
		}
		else
		{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		}
	}

	is_running = true;

	player = new Player;
	mapLoader = new MapLoader("C:\\Projects\\Sonic2-SMS-CPlusPlus\\DatabaseZone1", "C:\\Projects\\Sonic2-SMS-CPlusPlus\\mapLayout.txt");

	return true;
}

void Sanic::Game::handleEvents()
{
	Sanic::_InputManager::Instance()->update();
}

void Sanic::Game::update()
{
	// update player position etc
}

void Sanic::Game::render()
{
	SDL_RenderClear(renderer);
	player->Render();
	mapLoader->DrawBackground(renderer);
	SDL_RenderPresent(renderer);
}

void Sanic::Game::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Sanic::Game::quit()
{
	is_running = false;
}