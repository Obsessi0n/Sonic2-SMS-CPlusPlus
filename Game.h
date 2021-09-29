#pragma once
#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "MapLoader.h"
#include "UserInterface.h"
#include "EntityManager.h"

#include <SDL.h>
#include <iostream>

namespace Sanic {

	class Game
	{
	public:
		static Game* Instance() {
			if (instance == 0)
			{
				instance = new Game();
			}
			return instance;
		}

		bool init(const char* title, int x, int y, int flags);

		void render();
		void update();
		void handleEvents();
		void clean();
		void quit();

		bool isRunning() { return is_running; };
		void stopRunning() { is_running = false; };

		SDL_Renderer* getRenderer() const { return renderer; };
		SDL_Window* getWindow() const { return window; };

		Player* getPlayer() { return player; };
		MapLoader* getMapLoader() { return mapLoader; };

		int getScreenWidth() const { return SCREEN_WIDTH; };
		int getScreenHeight() const { return SCREEN_HEIGHT; };
		
		int getTileSize() const { return TileSize; };

		int getResWidth() const { return RES_WIDTH; };
		int getResHeight() const { return RES_HEIGHT; };

		int getLevelWidth() const { return level_width; };
		int getLevelHeight() const { return level_height; };

		SDL_Rect camera;

	private:
		static Game* instance;

		Game() {}
		~Game(){}
		SDL_Window* window;
		SDL_Renderer* renderer;

		Player* player;
		MapLoader* mapLoader;
		UserInterface* userInterface;
		EntityManager* entityManager;

		const int SCREEN_WIDTH = 256 * 4;
		const int SCREEN_HEIGHT = 196 * 4;

		const int RES_WIDTH = 256;
		const int RES_HEIGHT = 196;

		const int TileSize = 32;

		int level_width = 5376;
		int level_height = 768;

		bool is_running = true;
	};

	typedef Game _Game;
}

#endif