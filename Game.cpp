#pragma once
#include <SDL.h>
#include "Game.h"
#include "InputSystem.h"


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

}

Sanic::Game::~Game()
{
	delete player;
	delete input;
	delete background;

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
	player->Render(renderer);
	SDL_RenderPresent(renderer);
}