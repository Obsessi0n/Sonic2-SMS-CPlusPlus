#ifndef MAPLOADER_H
#define MAPLOADER_H
#include <list>
#include <string>
#include <SDL.h>
#include <string>

#include <vector>

namespace Sanic {

	class MapLoader {

	public:

		MapLoader();

		~MapLoader();

		void LoadMapDatabase(std::string databasePath, std::string actPath);

		void DrawBackground();
		void DrawMap(int camX, int camY);

		void GetSlopeStartEnd(int* yStart, int* yEnd, int const& xBlock, int const& yBlock);

		char GetBlockType(int const&  x,int const& y);

		char GetBlockType(float const& xLiteralPos, float const& yLiteralpos);

		int GetMapSizeY() { return mapLayoutData.size(); };
		std::string GetBlockID(int const& xBlock, int const& yBlock);
	private:

		std::vector<std::vector<std::string>> mapLayoutData;

		//std::list<SDL_Surface*> mapDatabase;
		std::string mapLayoutPath;
		//std::list<char unsigned> mapLayout;

		SDL_Rect* empty;

		const std::string solidBlocks = "assets/solidblocks.txt";
		const std::string slopeBlocks = "assets/slopes.txt";
	};
}
#endif