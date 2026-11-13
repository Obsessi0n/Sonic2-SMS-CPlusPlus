#include "Ring.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"
#include "CollisionManager.h"

Sanic::Ring::Ring()
{
	m_width = 16;
	m_height = 16;
}

Sanic::Ring::~Ring() {

}

void Sanic::Ring::Physics()
{

}

void Sanic::Ring::Render(int camX, int camY)
{
	Sanic::_TextureManager::Instance()->DrawAnimation("ring", (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 4, 200, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::Ring::CheckCollision() {
	// Both boxes are in the level's own coordinates, wherever the camera is.
	entityCollisionBox = { (int)m_x, (int)m_y, m_width, m_height };
	if (Sanic::_CollisionManager::Instance()->CheckCollision(entityCollisionBox, Sanic::_Game::Instance()->getPlayer()->GetCollisionRect()))
		GetCollected();
}

void Sanic::Ring::GetCollected() {
	Sanic::_Game::Instance()->changePlayerRings(&ringValue);
	collected = true;
}
