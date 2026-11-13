#pragma once
#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "MapLoader.h"
#include "UserInterface.h"
#include "EntityManager.h"

#include <SDL.h>
#include <string>

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

		// The window is the picture made bigger by a whole number, scale, so that every pixel of the game is a
		// square of the screen's; 0 picks the biggest that fits on the screen, up to 4.
		bool init(const char* title, int x, int y, int flags, int scale = 0);
		// The game with no window, no pictures and no keyboard: only its rules, on a level given as text (what
		// each character is, then the grid). The tests of the physics start it this way; nothing is ever drawn.
		bool initWithoutWindow(const std::string& tilesText, const std::string& levelText);

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

		int getScreenWidth() const { return RES_WIDTH * scale; };
		int getScreenHeight() const { return RES_HEIGHT * scale; };
		// The biggest scale that fits on the screen the game opens on, from 1 to MAX_FIT_SCALE.
		static int fittingScale();
		static constexpr int MAX_FIT_SCALE = 4;
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

		// The game's own clock. It counts updates, not the time on the wall: the same run shows the same thing
		// however fast the machine went through it.
		int getUpdates() const { return updates; };
		// The seconds the clock of the level shows. One of them is 60 updates at either rate, so at 50 updates a
		// second the clock runs slow against a watch, as everything else does.
		double getTimeSinceStart() { return timeSinceStart; };
		// The same in milliseconds, for what is animated: an animation moves on with the updates too.
		Uint32 getTicks() const { return (Uint32)(updates * 1000.0 / PhysicsManager::CLOCK_UPDATES_PER_SECOND); };

		// The line the camera keeps the player's feet near, in pixels from the top of the picture.
		int getCameraLine() const { return cameraLine; };

		// How many updates there are to a second: 50, as a PAL console draws its picture, or 60, as an NTSC one does.
		// Everything in the game is counted in updates, so at 50 the same game simply goes by more slowly, as it did
		// on the two consoles. Anything else asked for is refused. Set before init.
		static bool isUpdateRate(int rate) { return rate == 50 || rate == 60; };
		bool setUpdatesPerSecond(int rate) {
			if (!isUpdateRate(rate))
				return false;
			updatesPerSecond = rate;
			return true;
		};
		int getUpdatesPerSecond() const { return updatesPerSecond; };

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

		// The picture is the size an 8-bit console of the time drew, 256 by 192, in a window a whole number of
		// times that (see init).
		static constexpr int RES_WIDTH = 256;
		static constexpr int RES_HEIGHT = 192;
		int scale = MAX_FIT_SCALE;
		const int TileSize = 32;
		int updatesPerSecond = 50;

		// The size of the level in pixels: read off the level when it is loaded.
		int level_width = 0;
		int level_height = 0;

		bool is_running = true;

		int playerRings = 0;
		int playerLives = 3;

		void GameTimer();
		// The camera follows the player: see Game.cpp.
		void moveCamera();
		int cameraLine = PhysicsManager::CAMERA_LINE_START;
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
