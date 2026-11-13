#ifndef MAPLOADER_H
#define MAPLOADER_H

#include <SDL.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace Sanic {

	/* What a tile is to the physics:
	* Air or decoration = 0
	* Solid = 1
	* Slope = 2  (solid under its surface)
	* One-way platform = 3  (a floor from above, nothing else)
	*/
	struct TileKind {
		std::string name;
		char type = 0;
		// For a slope: how far down from the tile's top its surface is at the left and at the right edge.
		int yLeft = 0;
		int yRight = 0;
		// What the physics reads, worked out once from the lines above when the tile is loaded: how high the ground
		// stands in each of the tile's 32 columns of pixels, counted up from the tile's bottom (0 is no ground in
		// that column, 32 is ground to the top). There are no angles anywhere: a new shape of ground only has to
		// fill these in.
		int heights[32] = {};
		// How hard a slope pushes what stands on it, downhill, in 1/256 pixel per update every update: negative
		// pushes to the left, positive to the right, 0 on the flat. It goes by the slope's class (1:1, 1:2, 1:4).
		int push = 0;
	};

	// A level, read once into memory: a grid of tiles, one character each, and the tiles those characters stand for.
	class MapLoader {
	public:
		MapLoader();
		~MapLoader();

		// tilesFile says what each character is, tilesFolder holds a picture named after each tile, levelFile is the grid.
		bool Load(const std::string& tilesFile, const std::string& tilesFolder, const std::string& levelFile);
		// The same from the text of the two files: a test keeps its small levels in its own code.
		bool LoadText(const std::string& tilesText, const std::string& tilesFolder, const std::string& levelText);

		void DrawBackground();
		void DrawMap(int camX, int camY);

		char GetBlockType(int const& x, int const& y);
		char GetBlockType(float const& xLiteralPos, float const& yLiteralpos);
		// Ground that is a floor, a wall and a ceiling: a solid tile, or a slope under its surface.
		bool IsSolidBlock(int const& xBlock, int const& yBlock);
		// A platform that is only a floor, and only from above.
		bool IsOneWayBlock(int const& xBlock, int const& yBlock);

		// How high the ground stands in one column of pixels (0 to 31) of a block, from the block's bottom: 0 to 32.
		int GetGroundHeight(int const& xBlock, int const& yBlock, int const& column);
		// How hard the ground of a block pushes downhill: see TileKind::push.
		int GetSlopePush(int const& xBlock, int const& yBlock);

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
		// The bottom row of the level, and what is beyond its two sides: solid, whatever is drawn there.
		bool IsEdge(int xBlock, int yBlock) const;
		void ShapeGround(TileKind* kind, const std::string& slopeClass);

		std::unordered_map<char, TileKind> kinds;
		std::vector<std::string> rows;
		int width = 0;
		SDL_Point playerStart{ 64, 64 };
		std::vector<SDL_Point> pickups;
	};
}
#endif
