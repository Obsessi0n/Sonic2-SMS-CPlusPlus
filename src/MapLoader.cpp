#include "MapLoader.h"
#include "TextureManager.h"
#include "Game.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

Sanic::MapLoader::MapLoader() {
}

Sanic::MapLoader::~MapLoader() {
}

bool Sanic::MapLoader::Load(const std::string& tilesFile, const std::string& tilesFolder, const std::string& levelFile) {
    std::ifstream tilestxt(tilesFile);
    if (!tilestxt.is_open()) {
        std::cout << "Unable to open file " << tilesFile << '\n';
        return false;
    }
    std::ifstream leveltxt(levelFile);
    if (!leveltxt.is_open()) {
        std::cout << "Unable to open file " << levelFile << '\n';
        return false;
    }
    return Load(tilestxt, tilesFolder, leveltxt);
}

bool Sanic::MapLoader::Load(std::istream& tilestxt, const std::string& tilesFolder, std::istream& leveltxt) {
    // What each character of the grid is: "<character> <name> <solid|slope|decor> [yLeft yRight]".
    kinds.clear();
    std::string line;
    while (getline(tilestxt, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::stringstream words(line);
        char id;
        std::string type;
        TileKind kind;
        if (!(words >> id >> kind.name >> type))
            continue;
        if (type == "solid")
            kind.type = 1;
        else if (type == "slope") {
            kind.type = 2;
            words >> kind.yLeft >> kind.yRight;
        }
        ShapeGround(&kind);
        kinds[id] = kind;
        Sanic::_TextureManager::Instance()->Load(tilesFolder + "/" + kind.name + ".png", kind.name, Sanic::_Game::Instance()->getRenderer());
    }

    // The grid itself, kept in memory: nothing is read from disk again while the game runs.
    rows.clear();
    pickups.clear();
    width = 0;
    while (getline(leveltxt, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        for (int x = 0; x < (int)line.size(); x++) {
            int y = (int)rows.size();
            if (line[x] == START) {
                playerStart = { x * TILE, y * TILE };
                line[x] = AIR;
            }
            else if (line[x] == PICKUP) {
                // In the middle of its square: what is picked up is half a tile wide.
                pickups.push_back({ x * TILE + TILE / 4, y * TILE + TILE / 4 });
                line[x] = AIR;
            }
            else if (line[x] != AIR && kinds.find(line[x]) == kinds.end()) {
                std::cout << "Level: no tile is called '" << line[x] << "' (row " << y << ", column " << x << ")\n";
                line[x] = AIR;
            }
        }
        width = std::max(width, (int)line.size());
        rows.push_back(line);
    }
    // Every row as long as the longest, so a short row is air to its end.
    for (std::string& row : rows)
        row.resize(width, AIR);

    return !rows.empty();
}

// Fills in what the physics reads of a tile: the height of its ground column by column, and how it leans.
void Sanic::MapLoader::ShapeGround(TileKind* kind) {
    for (int column = 0; column < TILE; column++) {
        if (kind->type == 1)
            kind->heights[column] = TILE;
        else if (kind->type == 2) {
            // The same line the picture of the tile is drawn with (tools/make_art.py): from yLeft to yRight.
            int top = (int)std::lround(kind->yLeft + (kind->yRight - kind->yLeft) * column / (float)(TILE - 1));
            kind->heights[column] = std::min(TILE, std::max(0, TILE - top));
        }
        else
            kind->heights[column] = 0;
    }
    // One angle for the whole tile, from the first to the last column: right for a straight slope, and near
    // enough for a gentle curve.
    kind->angle = std::atan2((float)(kind->heights[TILE - 1] - kind->heights[0]), (float)(TILE - 1));
}

void Sanic::MapLoader::DrawBackground()
{
    SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 120, 190, 235, 255);
    SDL_RenderFillRect(Sanic::_Game::Instance()->getRenderer(), nullptr);
}

void Sanic::MapLoader::DrawMap(int camX, int camY) {
    // Only the tiles the camera sees.
    int firstX = std::max(0, camX / TILE);
    int firstY = std::max(0, camY / TILE);
    int lastX = std::min(width - 1, (camX + Sanic::_Game::Instance()->getResWidth()) / TILE);
    int lastY = std::min((int)rows.size() - 1, (camY + Sanic::_Game::Instance()->getResHeight()) / TILE);

    for (int y = firstY; y <= lastY; y++) {
        for (int x = firstX; x <= lastX; x++) {
            const TileKind* kind = KindAt(x, y);
            if (kind != nullptr)
                Sanic::_TextureManager::Instance()->Draw(kind->name, x * TILE - camX, y * TILE - camY, TILE, TILE, 1, Sanic::_Game::Instance()->getRenderer());
        }
    }
}

const Sanic::TileKind* Sanic::MapLoader::KindAt(int xBlock, int yBlock) const {
    if (yBlock < 0 || yBlock >= (int)rows.size() || xBlock < 0 || xBlock >= width)
        return nullptr;
    auto found = kinds.find(rows[yBlock][xBlock]);
    return found == kinds.end() ? nullptr : &found->second;
}

/*Air = 0
* Floor = 1
* Slope = 2
*/
char Sanic::MapLoader::GetBlockType(int const& xBlock, int const& yBlock) {
    //The bottom of the level is solid, and so are its two sides: nothing falls out of it.
    if (yBlock + 2 > (int)rows.size())
        return 1;
    if (xBlock < 0 || xBlock >= width)
        return 1;
    //Above the level there is only air.
    if (yBlock < 0)
        return 0;

    const TileKind* kind = KindAt(xBlock, yBlock);
    return kind == nullptr ? 0 : kind->type;
}

char Sanic::MapLoader::GetBlockType(float const& xLiteralPos, float const& yLiteralPos) {
    //Convert Literal Position to block.
    int xBlock = (int)std::floor(xLiteralPos / TILE);
    int yBlock = (int)std::floor(yLiteralPos / TILE);

    return GetBlockType(xBlock, yBlock);
}

int Sanic::MapLoader::GetGroundHeight(int const& xBlock, int const& yBlock, int const& column) {
    char type = GetBlockType(xBlock, yBlock);
    //A slope has the shape of its tile. What is solid is ground to the top, whatever is drawn there: that is
    //also the answer outside the level, where there is no tile to ask.
    if (type == 2)
        return KindAt(xBlock, yBlock)->heights[column];
    return type == 1 ? TILE : 0;
}

float Sanic::MapLoader::GetGroundAngle(int const& xBlock, int const& yBlock) {
    if (GetBlockType(xBlock, yBlock) != 2)
        return 0;
    return KindAt(xBlock, yBlock)->angle;
}
