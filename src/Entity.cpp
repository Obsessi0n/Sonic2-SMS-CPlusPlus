#include "Entity.h"
#include "PhysicsManager.h"
#include "Game.h"
#include "TextureManager.h"
#include <cmath>

void Sanic::Entity::Move()
{

}

void Sanic::Entity::Physics()
{
	//What an entity does when it has nothing of its own to do: it falls until there is ground under it.

	//Calculating the gravity: it adds to the speed downwards every update, up to a limit. The numbers are the
	//player's, which are in 1/256 of a pixel: here they are turned into pixels.
	const float sub = Sanic::_PhysicsManager::SUB;
	if (!isGrounded) {
		ySpeed += Sanic::_PhysicsManager::GRAVITY / sub;
		if (ySpeed > Sanic::_PhysicsManager::MAX_FALL_SPEED / sub)
			ySpeed = Sanic::_PhysicsManager::MAX_FALL_SPEED / sub;
	}

	//Finaly we aply the movement
	m_y = m_y + ySpeed;

	//Then we check the ground at the middle of the feet, as the player does: at its surface or under it, the
	//entity stands on it
	int surfaceY = 0;
	int slopePush = 0;
	if (Sanic::_PhysicsManager::Instance()->CheckFloor((int)std::floor(m_x) + m_width / 2, (int)std::floor(m_y) + m_height,
		(int)(ySpeed * sub), &surfaceY, &slopePush) == Sanic::_PhysicsManager::ON_GROUND) {
		m_y = surfaceY - m_height;
		ySpeed = 0;
		isGrounded = true;
	}
	else
		isGrounded = false;
}

void Sanic::Entity::Update()
{
	Physics();
}

void Sanic::Entity::Render(int camX, int camY)
{
	std::string id = "null";
	Sanic::_TextureManager::Instance()->DrawFrame(id, (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::Entity::Destroy()
{
	// Destroy entity
}
