#include "EntityManager.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

void Sanic::EntityManager::LoadEntities(std::string path)
{
	// Find .txt file
	std::string line;
	std::ifstream entitytxt(path);
	// Open .txt file
	if (entitytxt.is_open())
	{
		unsigned i = 0;
		while (getline(entitytxt, line))
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
			++i;
		}
		entitytxt.close();
	}
	else std::cout << "Unable to open file" << '\n';
}

void Sanic::EntityManager::Update()
{
	for (int i = 0; i < rings.size(); i++) 
	{
		rings[i]->CheckCollision();
	}
}

void Sanic::EntityManager::RenderEntities(int camX, int camY)
{
	for (unsigned i = 0; i < rings.size(); ++i)
	{
		rings[i]->Render(camX, camY);
	}
}