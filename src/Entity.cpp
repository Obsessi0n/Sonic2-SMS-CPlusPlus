#include "Entity.h"
#include "PhysicsManager.h"
#include "Game.h"
#include "TextureManager.h"

void Sanic::Entity::Move()
{

}

void Sanic::Entity::Physics()
{
	//What an entity does when it has nothing of its own to do: it falls until there is ground under it.

	//Calculating the gravity: it adds to the speed downwards every update, up to a limit.
	if (!isGrounded) {
		ySpeed += Sanic::_PhysicsManager::GRAVITY;
		if (ySpeed > Sanic::_PhysicsManager::MAX_FALL_SPEED)
			ySpeed = Sanic::_PhysicsManager::MAX_FALL_SPEED;
	}

	//Finaly we aply the movement
	m_y = m_y + ySpeed;

	//Then we check the ground under the middle of the feet: when the feet are on it, or a little way into it,
	//the entity stands on it
	float feet = m_y + m_height;
	int surfaceY = 0;
	float angle = 0;
	if (Sanic::_PhysicsManager::Instance()->FindFloor(m_x + m_width / 2, feet, &surfaceY, &angle) &&
		surfaceY <= feet && surfaceY > feet - Sanic::_PhysicsManager::STEP_HEIGHT) {
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
