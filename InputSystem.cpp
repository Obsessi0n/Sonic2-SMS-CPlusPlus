#include <SDL.h>
#include "Game.h"
#include "InputSystem.h"
#include "Player.h"

Sanic::InputSystem::InputSystem() {

}
Sanic::InputSystem::~InputSystem() {

}


int Sanic::InputSystem::handleInput(Player* _player)
{

	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		
		if (event.type == SDL_QUIT)
		{
			return -1;
		}

		if (event.key.keysym.sym == SDLK_LEFT) {
			
			_player->Move(false);
			return 0;
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			_player->Move(true);
			return 0;
		}
		

	}
	return 0;
}