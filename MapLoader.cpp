#include "MapLoader.h"
#include "TextureManager.h"
#include "Game.h"

#include <SDL_image.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>



Sanic::MapLoader::MapLoader() {

    empty = new SDL_Rect();
    empty->x = 0;
    empty->y = 0;
    empty->w = 32;
    empty->h = 32;
}

Sanic::MapLoader::~MapLoader() {
    mapLayoutData.clear();
}

void Sanic::MapLoader::LoadMapDatabase(std::string databasePath, std::string actPath) {

    //Send sprites to textureManager
    for (const auto& entry : std::filesystem::directory_iterator(databasePath)) {



        std::string path_string{ entry.path().u8string() };

        std::string s = path_string.c_str();
        replace(s.begin(), s.end(), '/', '\\');

        // get filename
        std::string base_filename = s.substr(s.find_last_of("/\\") + 1);

        // remove extension from filename
        std::string::size_type const p(base_filename.find_last_of('.'));
        std::string file_without_extension = base_filename.substr(0, p);

        //std::cout << file_without_extension << std::endl;

        Sanic::_TextureManager::Instance()->Load(("assets/sprites/Zone1/" + base_filename), file_without_extension, Sanic::_Game::Instance()->getRenderer());

    }
    //Save maplayoutpath;
    mapLayoutPath = actPath;

    //Save mapTilesInfo into memory
    //Open maplayoutxt
    std::string line;
    std::ifstream maptxt(mapLayoutPath);
    vector<string> tempLine;

    if (maptxt.is_open())
    {
        while (getline(maptxt, line)) {
            std::stringstream X(line);
            std::string singleTile;
            while (getline(X, singleTile, ' ')) {
                tempLine.push_back(singleTile);
            }
            mapLayoutData.push_back(tempLine);
            tempLine.clear();
        }

        maptxt.close();
    }

    else std::cout << "Unable to open file" << mapLayoutPath << '\n';
    
}


void Sanic::MapLoader::DrawBackground()
{
    SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 82, 170, 173, 255);
    SDL_RenderFillRect(Sanic::_Game::Instance()->getRenderer(), empty);
}

void Sanic::MapLoader::DrawMap(int camX, int camY) {
    //Open maplayoutxt
    std::string line;
    std::ifstream maptxt(mapLayoutPath);

    if (maptxt.is_open())
    {
        int x = 0, y = 0;
        int spriteSize = 32;
        int xPos, yPos, w, h;

        while (getline(maptxt, line)) {
            yPos = y * 32;
            std::stringstream X(line);
            std::string singleTile;

            while (getline(X, singleTile, ' ')) {               

                xPos = x * 32;
                Sanic::_TextureManager::Instance()->Draw(singleTile, xPos - camX, yPos - camY, 32, 32, 1, Sanic::_Game::Instance()->getRenderer());
                x = x + 1;
            }
            y = y + 1;
            x = 0;
        }

        maptxt.close();
    }

    else std::cout << "Unable to open file" << '\n';
}

/*Air = 0
* Floor = 1
* Slope = 2
*/
char Sanic::MapLoader::GetBlockType(int const& xBlock, int const& yBlock) {
    
    //If out of bounds
    if (yBlock + 2 > mapLayoutData.size())
        return 1;

    //We swap x and y to be compatible with maplayoutdata
    std::string block = mapLayoutData[yBlock][xBlock];
    //cout << block << '\n';
    std::string line;

    //Check if slope
    std::ifstream slopesTxt(slopeBlocks);
    std::vector<std::string> wordsInLine;
    std::string singleWord;
    while (getline(slopesTxt, line)) {
        std::stringstream X(line);      

        while (getline(X, singleWord, ' ')) {
            wordsInLine.push_back(singleWord);
        }
        if (wordsInLine[0] == block) {
            //Means it is a slope!
            return 2;            

        }           

        wordsInLine.clear();
    }

    //Check if solid block    
    std::ifstream solidBlockTxt(solidBlocks);
    while (getline(solidBlockTxt, line)) {
        if (line == block) { return 1; }        
    }

    //If we get here the block is not solid
    return 0;
}


/*
I Hate this, it's super not efficient we are opening the file again and searching for the block
we just did that on GetBlockType. This needs to be reworked!
Maybe just load it all into memory when game starts.
*/
void Sanic::MapLoader::GetSlopeStartEnd(int* yStart, int* yEnd, int const& xBlock, int const& yBlock) {

    //We swap x and y to be compatible with maplayoutdata
    std::string block = mapLayoutData[yBlock][xBlock];

    std::string line;

    //Check if slope
    std::ifstream slopesTxt(slopeBlocks);
    std::vector<std::string> wordsInLine;
    std::string singleWord;
    while (getline(slopesTxt, line)) {
        std::stringstream X(line);

        while (getline(X, singleWord, ' ')) {
            wordsInLine.push_back(singleWord);
        }
        if (wordsInLine[0] == block) {
            //block found
            *yStart = std::stoi(wordsInLine[1]);
            *yEnd = std::stoi(wordsInLine[2]);

        }

        wordsInLine.clear();
    }
}


string Sanic::MapLoader::GetBlockID(int const& xBlock, int const& yBlock) {
    //We swap x and y to be compatible with maplayoutdata
    return mapLayoutData[yBlock][xBlock];
}