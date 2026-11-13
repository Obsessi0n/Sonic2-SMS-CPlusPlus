#pragma once
#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "MapLoader.h"
#include "UserInterface.h"
#include "EntityManager.h"

#include <SDL.h>
#include <iostream>
#include <istream>

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
		// The game with no window, no pictures and no keyboard: only its rules, on a level given as text (what
		// each character is, then the grid). The tests of the physics start it this way; nothing is ever drawn.
		bool initWithoutWindow(std::istream& tilestxt, std::istream& leveltxt);

		void render();
		// Draws one frame and saves it as a BMP picture: what "--screenshot" on the command line asks for.
		bool screenshot(const char* fileName);
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

		int getPlayerRings() const { return playerRings; };
		int getPlayerLives() const { return playerLives; };
		void changePlayerRings(int *amount);
		void changePlayerLives(int* amount);

		// The game's own clock. It counts updates, 60 to the second, not the time on the wall: the same run
		// shows the same thing however fast the machine went through it.
		int getUpdates() const { return updates; };
		double getTimeSinceStart() { return timeSinceStart; };
		// The same in milliseconds, for what is animated.
		Uint32 getTicks() const { return (Uint32)(updates * 1000.0 / UPDATES_PER_SECOND); };

	private:
		static Game* instance;

		Game() {}
		~Game(){}

		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		Player* player = nullptr;
		MapLoader* mapLoader = nullptr;
		UserInterface* userInterface = nullptr;
		EntityManager* entityManager = nullptr;

		const int SCREEN_WIDTH = 256 * 4;
		const int SCREEN_HEIGHT = 196 * 4;
		const int RES_WIDTH = 256;
		const int RES_HEIGHT = 196;
		const int TileSize = 32;
		const int UPDATES_PER_SECOND = 60;

		// The size of the level in pixels: read off the level when it is loaded.
		int level_width = 0;
		int level_height = 0;

		bool is_running = true;

		int playerRings = 0;
		int playerLives = 3;

		void GameTimer();
		// Everything a frame shows, drawn but not yet put on the screen.
		void draw();
		// What is in a level once its map is loaded, put at its start; and all of it taken away again.
		void startLevel();
		void endLevel();

		int updates = 0;
		double timeSinceStart=0;
	};

	typedef Game _Game;
}
#endif
