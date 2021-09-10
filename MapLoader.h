#ifndef MAPLOADER_H
#define MAPLOADER_H
#include <list>
#include <string>
#include <SDL.h>


namespace Sanic {

	class MapLoader {

	public:

		MapLoader(std::string databasePath, std::string actPath);

		~MapLoader();

		void LoadMapDatabase(std::string act);

		void DrawBackground(SDL_Renderer* renderer);
		void DrawMap();

	private:

		std::list<SDL_Surface*> mapDatabase;
		std::string mapLayoutPath;
		std::list<char unsigned> mapLayout;

		SDL_Rect* empty;
	};
}
#endif