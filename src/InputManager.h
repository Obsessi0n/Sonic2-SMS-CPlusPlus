#pragma once
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
#include "Command.h"
#include "SDL.h"

namespace Sanic {
	class InputManager {
	public:
		static InputManager* Instance() {
			if (instance == 0)
			{
				instance = new InputManager();
			}
			return instance;
		}

		void HandleInput();

		// Keys held by a script instead of a hand ("--hold" on the command line): they count as pressed, together
		// with whatever is pressed on the keyboard, until this is called again.
		void HoldKeys(bool left, bool right, bool up, bool down, bool jump);

		// What the keys do to the game, whoever holds them. HandleInput calls it with the keyboard and the script;
		// a test calls it itself, once before every update, and so needs neither a keyboard nor a window.
		void Press(bool left, bool right, bool up, bool down, bool jump, bool hurt);

	private:
		InputManager() {};
		~InputManager() {};

		Command* Z_ = new JumpCommand;
		Command* noZ_ = new ReleaseJumpCommand;
		Command* arrowLeft_ = new MoveLeftCommand;
		Command* arrowRight_ = new MoveRightCommand;
		Command* arrowUp_ = new LookUpCommand;
		Command* noUp_ = new ReleaseLookUpCommand;
		Command* arrowDown_ = new RollingCommand;
		Command* noDown_ = new ReleaseRollingCommand;
		Command* noArrow_ = new StopMovement;
		Command* J_ = new TakeDamage;

		bool heldLeft = false;
		bool heldRight = false;
		bool heldUp = false;
		bool heldDown = false;
		bool heldJump = false;

		static InputManager* instance;
	};

	typedef InputManager _InputManager;
}

#endif
