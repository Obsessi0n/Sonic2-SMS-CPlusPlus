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
		currentKeyStates[SDL_SCANCODE_UP] || heldUp,
		currentKeyStates[SDL_SCANCODE_DOWN] || heldDown,
		currentKeyStates[SDL_SCANCODE_Z] || heldJump,
		currentKeyStates[SDL_SCANCODE_J]);
}

void Sanic::InputManager::Press(bool left, bool right, bool up, bool down, bool jump, bool hurt)
{
	if (jump) {
		Z_->execute();
	}
	else {
		noZ_->execute();
	}

	if (up) {
		arrowUp_->execute();
	}
	else {
		noUp_->execute();
	}

	if (down) {
		arrowDown_->execute();
	}
	else {
		noDown_->execute();
	}

	//Right before left: with both held it is left that counts
	if (right) {
		arrowRight_->execute();
	}

	if (left) {
		arrowLeft_->execute();
	}

	if (!left && !right) {
		noArrow_->execute();
	}

	if (hurt) {
		J_->execute();
	}

}

void Sanic::InputManager::HoldKeys(bool left, bool right, bool up, bool down, bool jump)
{
	heldLeft = left;
	heldRight = right;
	heldUp = up;
	heldDown = down;
	heldJump = jump;
}
