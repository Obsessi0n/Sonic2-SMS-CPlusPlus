#include "MapLoader.h"
#include <SDL_image.h>

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

Sanic::MapLoader::MapLoader(std::string databasePath, std::string actPath) {
	//Load database into memory
	for (const auto& entry : std::filesystem::directory_iterator(databasePath)) {
		
		//std::cout << entry.path() << std::endl;
		
		std::string path_string{ entry.path().u8string() };
		
		const char* path = path_string.c_str();

		mapDatabase.push_back(IMG_Load(path));
	}
	//std::cout << mapDatabase.size() << std::endl;

	//Save map layout txt path
	mapLayoutPath = actPath;

	empty = new SDL_Rect();
	empty->x = 0;
	empty->y = 0;
	empty->w = 32;
	empty->h = 32;
}

Sanic::MapLoader::~MapLoader() {
	mapDatabase.clear();	
}

void Sanic::MapLoader::DrawBackground(SDL_Renderer* renderer)
{
	SDL_SetRenderDrawColor(renderer, 82, 170, 173, 255);
	SDL_RenderFillRect(renderer, empty);
}

void Sanic::MapLoader::DrawMap() {
	//Open maplayoutxt
	std::string line;
	std::ifstream maptxt(mapLayoutPath);

	if (maptxt.is_open())
	{
		int x=0, y=0;
		int spriteSize = 32;
		int xPos, yPos;

		while (getline(maptxt, line)) {
			//cout << line << endl;
			//Draw the tiles
			yPos = y * 32;

			std::stringstream X(line);
			std::string singleTile;
			while (getline(X, singleTile, ' ')) {
				xPos = x * 32;
				//std::cout << "Tile: " + singleTile << " PosX= " << xPos << "PosY= " << yPos << endl; // print single tile
				x = x + 1;
				//Ask to draw singleTile on position (x,y)
			}
			y = y + 1;
			x = 0;
		}
		maptxt.close();
	}

	else std::cout << "Unable to open file" << '\n';
}