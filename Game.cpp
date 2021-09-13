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

	camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

	player = new Player;
	mapLoader = new MapLoader();

	mapLoader->LoadMapDatabase("assets/sprites/Zone1", "assets/mapLayout.txt");

	return true;
}

void Sanic::Game::handleEvents()
{
	Sanic::_InputManager::Instance()->update();
}

void Sanic::Game::update()
{
	player->Physics();
	camera.x = (player->getPosX() + player->getWidth() / 2) - SCREEN_WIDTH / 2;
	camera.y = (player->getPosY() + player->getHeight() / 2) - SCREEN_HEIGHT / 2;

	if (camera.x < 0)
	{
		camera.x = 0;
	}

	if (camera.y < 0)
	{
		camera.y = 0;
	}

	if (camera.x > level_width - camera.w)
	{
		camera.x = level_width - camera.w;
	}

	if (camera.y > level_height - camera.h)
	{
		camera.y = level_height - camera.h;
	}
}

void Sanic::Game::render()
{
	SDL_RenderClear(renderer);
	mapLoader->DrawBackground();
	mapLoader->DrawMap(camera.x, camera.y);
	player->Render(camera.x, camera.y);
	SDL_RenderPresent(renderer);
}

void Sanic::Game::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	delete(mapLoader);
	delete(player);
	SDL_Quit();
}

void Sanic::Game::quit()
{
	is_running = false;
}