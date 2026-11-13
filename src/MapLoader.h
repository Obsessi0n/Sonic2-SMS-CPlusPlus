#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <SDL.h>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Sanic {

	/* What a tile is to the physics:
	* Air or decoration = 0
	* Solid = 1
	* Slope = 2
	*/
	struct TileKind {
		std::string name;
		char type = 0;
		// For a slope: how far down from the tile's top its surface is at the left and at the right edge.
		int yLeft = 0;
		int yRight = 0;
		// What the physics reads, worked out once from the lines above when the tile is loaded: how high the ground
		// stands in each of the tile's 32 columns of pixels, counted up from the tile's bottom (0 is no ground in
		// that column, 32 is ground to the top), and how the surface leans, in radians (positive when it rises to
		// the right). A new shape of ground only has to fill these in.
		int heights[32] = {};
		float angle = 0;
	};

	// A level, read once into memory: a grid of tiles, one character each, and the tiles those characters stand for.
	class MapLoader {
	public:
		MapLoader();
		~MapLoader();

		// tilesFile says what each character is, tilesFolder holds a picture named after each tile, levelFile is the grid.
		bool Load(const std::string& tilesFile, const std::string& tilesFolder, const std::string& levelFile);
		// The same from text that is already open: a test keeps its small levels in its own code.
		bool Load(std::istream& tilestxt, const std::string& tilesFolder, std::istream& leveltxt);

		void DrawBackground();
		void DrawMap(int camX, int camY);

		char GetBlockType(int const& x, int const& y);
		char GetBlockType(float const& xLiteralPos, float const& yLiteralpos);

		// How high the ground stands in one column of pixels (0 to 31) of a block, from the block's bottom: 0 to 32.
		int GetGroundHeight(int const& xBlock, int const& yBlock, int const& column);
		// How the ground of a block leans, in radians: positive when it rises to the right.
		float GetGroundAngle(int const& xBlock, int const& yBlock);

		int GetMapSizeY() { return (int)rows.size(); };
		int GetMapSizeX() { return width; };
		int GetWidthPixels() { return width * TILE; };
		int GetHeightPixels() { return (int)rows.size() * TILE; };

		// Where the level puts the player and the things to pick up, in pixels.
		SDL_Point GetPlayerStart() { return playerStart; };
		const std::vector<SDL_Point>& GetPickups() { return pickups; };

		// The side of a tile, in pixels.
		static constexpr int TILE = 32;

	private:
		// In the grid: nothing, the player's start, something to pick up. None of them is a tile.
		static constexpr char AIR = '.';
		static constexpr char START = 'P';
		static constexpr char PICKUP = 'o';

		const TileKind* KindAt(int xBlock, int yBlock) const;
		void ShapeGround(TileKind* kind);

		std::unordered_map<char, TileKind> kinds;
		std::vector<std::string> rows;
		int width = 0;
		SDL_Point playerStart{ 64, 64 };
		std::vector<SDL_Point> pickups;
	};
}
#endif
