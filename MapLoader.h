#pragma once
#include <list>
#include <string>
#include <SDL.h>


namespace Sanic {

	class MapLoader {

	public:

		MapLoader(std::string databasePath, std::string actPath);

		~MapLoader();

		void LoadMapDatabase(std::string act);
		
		void DrawMap();

	private:

		std::list<SDL_Surface*> mapDatabase;
		std::string mapLayoutPath;
		std::list<char unsigned> mapLayout;


	};



}
