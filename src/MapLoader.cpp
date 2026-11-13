#include "MapLoader.h"
#include "TextureManager.h"
#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// The text is read with SDL and C rather than with C++ streams: the streams would bring a megabyte of library
// into the program for two small files read once.

// The whole of a file, or false when it cannot be read.
static bool readFile(const std::string& fileName, std::string* text) {
    size_t size = 0;
    void* data = SDL_LoadFile(fileName.c_str(), &size);
    if (data == nullptr) {
        std::printf("Unable to open file %s\n", fileName.c_str());
        return false;
    }
    text->assign(static_cast<const char*>(data), size);
    SDL_free(data);
    return true;
}

// The lines of a text without their ends ("\n" or "\r\n"). Nothing after the last end is no line at all.
static std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
            end = text.size();
        std::string line = text.substr(start, end - start);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
        start = end + 1;
    }
    return lines;
}

static bool isBlank(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}

// The words of a line from a place in it on, split where it is blank.
static std::vector<std::string> wordsOf(const std::string& line, size_t from) {
    std::vector<std::string> words;
    size_t i = from;
    while (i < line.size()) {
        while (i < line.size() && isBlank(line[i]))
            i++;
        size_t start = i;
        while (i < line.size() && !isBlank(line[i]))
            i++;
        if (i > start)
            words.push_back(line.substr(start, i - start));
    }
    return words;
}

// A whole number, or false when the word is not one.
static bool readNumber(const std::string& word, int* number) {
    char* end = nullptr;
    long value = std::strtol(word.c_str(), &end, 10);
    if (end == word.c_str() || *end != '\0')
        return false;
    *number = (int)value;
    return true;
}

Sanic::MapLoader::MapLoader() {
}

Sanic::MapLoader::~MapLoader() {
}

bool Sanic::MapLoader::Load(const std::string& tilesFile, const std::string& tilesFolder, const std::string& levelFile) {
    std::string tilesText;
    std::string levelText;
    if (!readFile(tilesFile, &tilesText) || !readFile(levelFile, &levelText))
        return false;
    return LoadText(tilesText, tilesFolder, levelText);
}

bool Sanic::MapLoader::LoadText(const std::string& tilesText, const std::string& tilesFolder, const std::string& levelText) {
    // What each character of the grid is: "<character> <name> <solid|slope|oneway|decor> [yLeft yRight [class]]".
    kinds.clear();
    for (const std::string& line : linesOf(tilesText)) {
        if (line.empty() || line[0] == '#')
            continue;
        // The character is the first one that is not blank; the words follow it.
        size_t at = 0;
        while (at < line.size() && isBlank(line[at]))
            at++;
        if (at == line.size())
            continue;
        char id = line[at];
        std::vector<std::string> words = wordsOf(line, at + 1);
        if (words.size() < 2)
            continue;
        TileKind kind;
        kind.name = words[0];
        const std::string& type = words[1];
        std::string slopeClass;
        if (type == "solid")
            kind.type = 1;
        else if (type == "slope") {
            kind.type = 2;
            // Read in order, up to the first that is missing or not a number.
            if (words.size() > 2 && readNumber(words[2], &kind.yLeft)
                && words.size() > 3 && readNumber(words[3], &kind.yRight)
                && words.size() > 4)
                slopeClass = words[4];
        }
        else if (type == "oneway")
            kind.type = 3;
        ShapeGround(&kind, slopeClass);
        kinds[id] = kind;
        Sanic::_TextureManager::Instance()->Load(tilesFolder + "/" + kind.name + ".bmp", kind.name, Sanic::_Game::Instance()->getRenderer());
    }

    // The grid itself, kept in memory: nothing is read from disk again while the game runs.
    rows.clear();
    pickups.clear();
    width = 0;
    for (std::string line : linesOf(levelText)) {
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
                std::printf("Level: no tile is called '%c' (row %d, column %d)\n", line[x], y, x);
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

// Fills in what the physics reads of a tile: the height of its ground column by column, and for a slope how hard
// it pushes downhill. slopeClass is "1:1", "1:2" or "1:4" as the tiles file gives it, or empty when it does not.
void Sanic::MapLoader::ShapeGround(TileKind* kind, const std::string& slopeClass) {
    for (int column = 0; column < TILE; column++) {
        if (kind->type == 1 || kind->type == 3)
            kind->heights[column] = TILE;
        else if (kind->type == 2) {
            // The same line the picture of the tile is drawn with (tools/make_art.py): from yLeft to yRight.
            int top = (int)std::lround(kind->yLeft + (kind->yRight - kind->yLeft) * column / (float)(TILE - 1));
            kind->heights[column] = std::min(TILE, std::max(0, TILE - top));
        }
        else
            kind->heights[column] = 0;
    }

    kind->push = 0;
    int rise = kind->heights[TILE - 1] - kind->heights[0];
    if (kind->type != 2 || rise == 0)
        return;

    // The class as given; without one, by how much the tile climbs from one side to the other.
    int push = Sanic::_PhysicsManager::SLOPE_PUSH_1_4;
    if (slopeClass == "1:1" || (slopeClass.empty() && std::abs(rise) >= 24))
        push = Sanic::_PhysicsManager::SLOPE_PUSH_1_1;
    else if (slopeClass == "1:2" || (slopeClass.empty() && std::abs(rise) >= 12))
        push = Sanic::_PhysicsManager::SLOPE_PUSH_1_2;
    else if (!slopeClass.empty() && slopeClass != "1:4")
        std::printf("Tile %s: a slope is 1:1, 1:2 or 1:4, not '%s'\n", kind->name.c_str(), slopeClass.c_str());

    // Downhill is to the left when the ground rises to the right.
    kind->push = rise > 0 ? -push : push;
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

bool Sanic::MapLoader::IsEdge(int xBlock, int yBlock) const {
    return yBlock + 2 > (int)rows.size() || xBlock < 0 || xBlock >= width;
}

/*Air = 0
* Floor = 1
* Slope = 2
* One-way platform = 3
*/
char Sanic::MapLoader::GetBlockType(int const& xBlock, int const& yBlock) {
    //The bottom of the level is solid, and so are its two sides: nothing falls out of it.
    if (IsEdge(xBlock, yBlock))
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

bool Sanic::MapLoader::IsSolidBlock(int const& xBlock, int const& yBlock) {
    char type = GetBlockType(xBlock, yBlock);
    return type == 1 || type == 2;
}

bool Sanic::MapLoader::IsOneWayBlock(int const& xBlock, int const& yBlock) {
    return GetBlockType(xBlock, yBlock) == 3;
}

int Sanic::MapLoader::GetGroundHeight(int const& xBlock, int const& yBlock, int const& column) {
    //The edges of the level are ground to the top, whatever is drawn there or nothing at all.
    if (IsEdge(xBlock, yBlock))
        return TILE;
    //Everything else has the shape of its tile.
    const TileKind* kind = KindAt(xBlock, yBlock);
    return kind == nullptr ? 0 : kind->heights[column];
}

int Sanic::MapLoader::GetSlopePush(int const& xBlock, int const& yBlock) {
    if (IsEdge(xBlock, yBlock))
        return 0;
    const TileKind* kind = KindAt(xBlock, yBlock);
    return kind == nullptr ? 0 : kind->push;
}
