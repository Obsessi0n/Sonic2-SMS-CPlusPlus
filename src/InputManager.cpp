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

	//A key counts as pressed when it is down on the keyboard or held by a script.
	Press(currentKeyStates[SDL_SCANCODE_LEFT] || heldLeft,
		currentKeyStates[SDL_SCANCODE_RIGHT] || heldRight,
		currentKeyStates[SDL_SCANCODE_Z] || heldJump,
		currentKeyStates[SDL_SCANCODE_J]);
}

void Sanic::InputManager::Press(bool left, bool right, bool jump, bool hurt)
{
	if (jump) {
		Z_->execute();
	}
	else {
		noZ_->execute();
	}

	if (left) {
		arrowLeft_->execute();
	}

	if (right) {
		arrowRight_->execute();
	}
	if (hurt) {
		J_->execute();
	}



	if (!left && !right) {
		noArrow_->execute();
	}

}

void Sanic::InputManager::HoldKeys(bool left, bool right, bool jump)
{
	heldLeft = left;
	heldRight = right;
	heldJump = jump;
}
