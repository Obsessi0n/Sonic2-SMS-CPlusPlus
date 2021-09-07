#pragma once
#include <iostream>
#include "InputSystem.h"
#include "Player.h"
#include "Background.h"
#include "MapLoader.h"

namespace Sanic {

	class Game
	{
	public:
		Game();
		~Game();

		void render();
		void update();
		void handleEvents();

		bool isRunning() { return is_running; };
		void stopRunning() { is_running = false; };

		const static int SCREEN_WIDTH = 256;
		const static int SCREEN_HEIGHT = 196;
	private:
		SDL_Window* window = nullptr;
		SDL_Surface* screenSurface = nullptr;
		SDL_Renderer* renderer = nullptr;

		InputSystem* input;

		Background* background = nullptr;
		Player* player = nullptr;

		MapLoader* mapLoader;

		bool is_running = true;
	};
}

