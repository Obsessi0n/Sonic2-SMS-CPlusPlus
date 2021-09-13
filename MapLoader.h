#ifndef MAPLOADER_H
#define MAPLOADER_H
#include <list>
#include <string>
#include <SDL.h>
#include <string>

#include <vector>
using namespace std;

namespace Sanic {

	class MapLoader {

	public:

		MapLoader();

		~MapLoader();

		void LoadMapDatabase(std::string databasePath, std::string actPath);

		void DrawBackground();
		void DrawMap(int camX, int camY);

		bool GetBlockCollision(int x, int y);

	private:

		vector<vector<string>> mapLayoutData;

		//std::list<SDL_Surface*> mapDatabase;
		std::string mapLayoutPath;
		//std::list<char unsigned> mapLayout;

		SDL_Rect* empty;

		const std::string collisionsPath = "assets/blocksCollisions.txt";
	};
}
#endif