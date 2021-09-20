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


bool Sanic::MapLoader::IsTileSolid(int x, int y) {

    
    if (y + 2 > mapLayoutData.size())
        return true;


    std::string block = mapLayoutData[y][x];

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
            //Calculate the slope y position
            int yStart = std::stoi(wordsInLine[1]);
            int yEnd = std::stoi(wordsInLine[2]);
            int blockPosY = y*32;            
            

            //Find b
            int b = yStart;

            //find m
            float temp = (yEnd - b);
            float m = temp / 32;
            //Now we have
            //y = mx+b

            //Slope y on the x
            int slopeY = (m * (Sanic::_Game::Instance()->getPlayer()->getPosX()-(32*x)) + b)+(y*32);
            
            
            if (Sanic::_Game::Instance()->getPlayer()->getPosY() + 32 >= slopeY) {
                Sanic::_Game::Instance()->getPlayer()->OnSlope(&slopeY);
                return true;
            }

            return false;

        }
            

        wordsInLine.clear();
    }

    //Check if solid block
    
    std::ifstream solidBlockTxt(solidBlocks);

    while (getline(solidBlockTxt, line)) {
        if (line == block) {
            return true;
            break;
        }
    }
    return false;
}