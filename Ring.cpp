#include "Ring.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"

Sanic::Ring::Ring() 
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/entities/ring.png", "ring", Sanic::_Game::Instance()->getRenderer());
	m_width = 16;
	m_height = 16;

	collisionBox = { (int)m_x, (int)m_y, m_width, m_height };
}

void Sanic::Ring::Physics()
{

}

void Sanic::Ring::Render(int camX, int camY)
{
	if (!isCollected())
	{
		Sanic::_TextureManager::Instance()->DrawFrame("ring", (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
	}
	else
	{
		return;
	}
}
