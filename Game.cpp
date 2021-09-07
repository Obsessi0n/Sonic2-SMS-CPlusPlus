#pragma once
#include <SDL.h>
#include "Game.h"
#include "InputSystem.h"
#include "MapLoader.h"

Sanic::Game::Game()
{
	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		window = SDL_CreateWindow("Sanic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

		if (window == NULL)
		{
			std::cout << "Window could not be created! SDL_Error: " << SDL_GetError();
			is_running = false;
		}
		else
		{
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

			screenSurface = SDL_GetWindowSurface(window);

		}
	}

	input = new InputSystem();

	player = new Player(renderer);
	background = new Background();

	is_running = true;

	mapLoader = new MapLoader("W:/GitHub/Sonic-SMS-Cplusplus/DatabaseZone1", "W:/GitHub/Sonic-SMS-Cplusplus/mapLayout.txt");
}

Sanic::Game::~Game()
{
	delete player;
	delete input;
	delete background;
	delete mapLoader;

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}


void Sanic::Game::handleEvents()
{
	input->handleInput(player);

	if (input->handleInput(player) == -1) {
		is_running = false;
	}
}

void Sanic::Game::update()
{
	// update player position etc
}


void Sanic::Game::render()
{
	
	SDL_RenderClear(renderer);
	background->DrawBackground(renderer);
	mapLoader->DrawMap();
	player->Render(renderer);
	SDL_RenderPresent(renderer);
}