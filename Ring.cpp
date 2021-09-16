#include "Ring.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"
#include "CollisionManager.h"


Sanic::Ring::Ring() 
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/entities/ring.png", "ring", Sanic::_Game::Instance()->getRenderer());
	m_width = 16;
	m_height = 16;

	collisionBox = { (int)m_x, (int)m_y, m_width, m_height };
}
Sanic::Ring::~Ring() {
	
}
void Sanic::Ring::Physics()
{

}

void Sanic::Ring::Render(int camX, int camY)
{
	ringCol = { (int)m_x - camX, (int)m_y - camY, m_width, m_height };
	if (!isCollected())
	{
		Sanic::_TextureManager::Instance()->DrawFrame("ring", (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
	}
	else
	{
		return;
	}
}

void Sanic::Ring::CheckCollision() {	
	if (Sanic::_CollisionManager::Instance()->CheckCollision(ringCol, Sanic::_Game::Instance()->getPlayer()->GetCollisionRect()))
		GetPicked();
}

void Sanic::Ring::GetPicked() {
	collected = true;
}