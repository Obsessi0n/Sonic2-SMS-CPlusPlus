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

	camera = { 0, 0, RES_WIDTH, RES_HEIGHT };

	player = new Player;
	mapLoader = new MapLoader();
	interface = new Interface();

	mapLoader->LoadMapDatabase("assets/sprites/Zone1", "assets/mapLayout.txt");

	SDL_RenderSetLogicalSize(renderer, RES_WIDTH, RES_HEIGHT);
	return true;
}

void Sanic::Game::handleEvents()
{
	Sanic::_InputManager::Instance()->HandleInput();
}

void Sanic::Game::update()
{
	player->Update();
	camera.x = (player->getPosX() + player->getWidth() / 2) - RES_WIDTH / 2;
	camera.y = (player->getPosY() + player->getHeight() / 2) - RES_HEIGHT / 2;

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
	interface->Render();
	player->Render("player", camera.x, camera.y);
	SDL_RenderPresent(renderer);
}

void Sanic::Game::clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	delete(mapLoader);
	delete(interface);
	delete(player);
	SDL_Quit();
}

void Sanic::Game::quit()
{
	is_running = false;
}