//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include "Game.h"

Sanic::Game* game = 0;
//1 sec = 1000ms so 60fps is 1000/60 = 16.6666666667
const double MS_PER_UPDATE = 16.6666666667;

int main(int argc, char* args[])
{
	game = new Sanic::Game();

	double previous = SDL_GetTicks();
	double lag = 0.0;

	while (game->isRunning())
	{
		double current = SDL_GetTicks();
		double elapsed = current - previous;
		previous = current;
		lag += elapsed;

		while (lag >= MS_PER_UPDATE) {
			game->handleEvents();
			game->update();
			lag -= MS_PER_UPDATE;
	
		}

		game->render();
		
	}

	delete game;

	return 0;
}