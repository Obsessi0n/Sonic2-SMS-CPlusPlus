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

	private:
		InputManager() {};
		~InputManager() {};

		Command* Z_ = new JumpCommand;
		Command* arrowLeft_ = new MoveLeftCommand;
		Command* arrowRight_ = new MoveRightCommand;
		Command* arrowDown_ = new RollingCommand;
		Command* noArrow_ = new StopMovement;

		static InputManager* instance;
	};

	typedef InputManager _InputManager;
}

#endif