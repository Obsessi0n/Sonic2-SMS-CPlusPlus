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

		Command* arrowUp_ = new JumpCommand;
		Command* arrowLeft_ = new MoveLeftCommand;
		Command* arrowRight_ = new MoveRightCommand;

		static InputManager* instance;
	};

	typedef InputManager _InputManager;
}

#endif