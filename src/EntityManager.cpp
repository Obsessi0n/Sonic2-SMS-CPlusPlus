#include "EntityManager.h"
#include "Game.h"
#include "TextureManager.h"

Sanic::EntityManager::~EntityManager()
{
	for (Ring* ring : rings)
		delete ring;
}

void Sanic::EntityManager::LoadPickups(const std::vector<SDL_Point>& places)
{
	// The picture is loaded once, whatever the number of things that wear it.
	Sanic::_TextureManager::Instance()->Load("assets/sprites/pickup.png", "ring", Sanic::_Game::Instance()->getRenderer());

	for (const SDL_Point& place : places)
	{
		Ring* ring = new Ring();
		ring->setX(place.x);
		ring->setY(place.y);
		rings.push_back(ring);
	}
}

void Sanic::EntityManager::Update()
{
	// Backwards, so that taking one out does not skip the one after it.
	for (int i = (int)rings.size() - 1; i >= 0; i--)
	{
		rings[i]->CheckCollision();
		if (rings[i]->IsCollected())
		{
			delete rings[i];
			rings.erase(rings.begin() + i);
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
