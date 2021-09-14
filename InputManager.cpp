#include <SDL.h>
#include "InputManager.h"
#include "Game.h"

Sanic::InputManager* Sanic::InputManager::instance = 0;

void Sanic::InputManager::HandleInput()
{
	SDL_Event event;
	const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
			_Game::Instance()->quit();
	}

	if (currentKeyStates[SDL_SCANCODE_UP]) {
		arrowUp_->execute();
	}

	if (currentKeyStates[SDL_SCANCODE_LEFT]) {
		arrowLeft_->execute();
	}

	if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
		arrowRight_->execute();
	}
}