#pragma once
#include <iostream>
#include "Game.h"
#include "Player.h"
namespace Sanic {
	class InputSystem {

	public:
		InputSystem();
		~InputSystem();

		int handleInput(Player* _player);


	private:
		
	};
}