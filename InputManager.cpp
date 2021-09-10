#include <SDL.h>
#include "InputManager.h"
#include "Game.h"

Sanic::InputManager* Sanic::InputManager::instance = 0;

bool Sanic::InputManager::update()
{
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			_Game::Instance()->quit();
		}

		if (event.key.keysym.sym == SDLK_LEFT) {
			Sanic::_Game::Instance()->getPlayer()->Move(false);
			return 0;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			Sanic::_Game::Instance()->getPlayer()->Move(true);
			return 0;
		}
	}
}