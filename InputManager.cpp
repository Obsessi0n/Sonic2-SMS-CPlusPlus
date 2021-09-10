#include <SDL.h>
#include "InputManager.h"
#include "Game.h"

Sanic::InputManager* Sanic::InputManager::instance = 0;

void Sanic::InputManager::update()
{
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			_Game::Instance()->quit();
		}

		/*if (event.key.keysym.sym == SDLK_LEFT) {
			_player->Move(false);
			return 0;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			_player->Move(true);
			return 0;
		}*/
	}
}