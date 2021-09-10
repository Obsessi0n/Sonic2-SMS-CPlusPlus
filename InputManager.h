#pragma once
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H
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
		
		void update();

	private:
		InputManager() {};
		~InputManager() {};

		static InputManager* instance;
	};

	typedef InputManager _InputManager;
}

#endif