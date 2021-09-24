#include "EntityManager.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

void Sanic::EntityManager::LoadEntities(const std::string& path)
{
	// Find .txt file
	std::string line;
	std::ifstream entitytxt(path);
	// Open .txt file
	if (entitytxt)
	{
		uint32_t i = 0;
		for (uint32_t i = 0u; getline(entitytxt, line); ++i)
		{
			std::stringstream x(line);
			std::string position;
			bool firstNumber = true;

			// Instantiate and set Ring locations
			rings.push_back(new Ring());

			while (getline(x, position, ','))
			{
				if (firstNumber)
				{
					rings[i]->setX(std::stoi(position));
					firstNumber = false;
				}
				else
				{
					rings[i]->setY(std::stoi(position));
				}
			}
		}
	}
	else std::puts("Unable to open file");
}

void Sanic::EntityManager::Update()
{
	for (uint32_t i = 0; i < rings.size(); i++)
	{
		rings[i]->CheckCollision();
		if (rings[i]->GetCollected())
		{
			rings.erase(std::next(std::begin(rings), i));
		}
	}
}

void Sanic::EntityManager::RenderEntities(int camX, int camY)
{
	for (uint32_t i = 0; i < rings.size(); ++i)
	{
		rings[i]->Render(camX, camY);
	};
}