#include "MapLoader.h"
#include <SDL_image.h>

#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
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

		while (getline(maptxt, line)) {
			//cout << line << endl;
			//Draw the tiles
		}
		maptxt.close();
	}

	else cout << "Unable to open file";
}