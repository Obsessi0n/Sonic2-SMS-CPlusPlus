#include "Entity.h"
#include "PhysicsManager.h"
#include "Game.h"
#include "TextureManager.h"

void Sanic::Entity::Move(bool dir)
{
	if (dir)
	{
		m_x++;
	}
	else
	{
		m_x--;
	}
}

void Sanic::Entity::Physics()
{
	float newYPOS = m_y;
	float gravityForce = 0;


	//First we check if the player is grounded!
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x, m_y + 32))
		isGrounded = true;
	else
		isGrounded = false;


	//Calculating the gravity.
	if (!isGrounded) {
		fallingTimer++;
		gravityForce = Sanic::_PhysicsManager::Instance()->CalculateGravityForce(fallingTimer);
	}
	else {
		fallingTimer = 0;
		gravityForce = 0;
	}

	//We check if our head is banging on a tile
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x, m_y - 5))
		jumpVelocity = 0;

	//We decrease the jump velocity with time
	if (isJumping) {
		jumpVelocity -= 0.04f;
		if (jumpVelocity < 0) {
			jumpVelocity = 0;
			isJumping = false;
		}
	}

	//Finaly we aply the movement
	m_y = m_y + gravityForce - jumpVelocity;
}

void Sanic::Entity::Update()
{
	Physics();
}

void Sanic::Entity::Render(std::string id, int camX, int camY)
{
	Sanic::_TextureManager::Instance()->DrawFrame(id, (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::Entity::Destroy()
{
	// Destroy entity
}
