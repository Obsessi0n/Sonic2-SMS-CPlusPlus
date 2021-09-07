#include "MapLoader.h"
#include <SDL_image.h>

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;
using namespace std;

Sanic::MapLoader::MapLoader(std::string databasePath, std::string actPath) {
	//Load database into memory
	for (const auto& entry : fs::directory_iterator(databasePath)) {
		
		//std::cout << entry.path() << std::endl;
		
		std::string path_string{ entry.path().u8string() };
		
		const char* path = path_string.c_str();

		mapDatabase.push_back(IMG_Load(path));
	}
	//std::cout << mapDatabase.size() << std::endl;

	//Save map layout txt path
	mapLayoutPath = actPath;
}

Sanic::MapLoader::~MapLoader() {
	mapDatabase.clear();
	
}

void Sanic::MapLoader::DrawMap() {
	//Open maplayoutxt
	string line;
	ifstream maptxt(mapLayoutPath);

	if (maptxt.is_open())
	{
		int x=0, y=0;
		int spriteSize = 32;
		int xPos, yPos;

		while (getline(maptxt, line)) {
			//cout << line << endl;
			//Draw the tiles
			yPos = y * 32;

			stringstream X(line);
			string singleTile;
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

	else cout << "Unable to open file";
}