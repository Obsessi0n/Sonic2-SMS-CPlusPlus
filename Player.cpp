#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"

Sanic::Player::Player() 
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2);

	m_y = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2) - 60;

	collisionBox = { (int)m_x, (int)m_y, 10, 10 };
}

Sanic::Player::~Player() 
{
	
}

void Sanic::Player::Move(bool dir) {

	float newXPOS = m_x;

	//Calculate acceleration
	currentAcceleration += acceleration;
	if (currentAcceleration > maxSpeed)
		currentAcceleration = maxSpeed;

	speed = 1;

	//Added an offset of 16 to x and y because we don't have a player collission box yet (DIRTY HACK)

	if (dir) {
		
		if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + speed + 16, m_y+16)) {
			currentAcceleration = 0;
			speed = 0;
		}		

		if (m_x > Sanic::_Game::Instance()->getLevelWidth() - m_width)
			m_x = Sanic::_Game::Instance()->getLevelWidth() - m_width;

		//speed = initialSpeed;
		
	}
	else if (!dir)
	{
		speed = -speed;
		if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x - speed-5, m_y + 16)) {
			speed = 0;
			currentAcceleration = 0;
		}
			

		if (m_x <= 0.1f) // If m_x <0 we have a catastrophic failure!
			m_x = 0.1f;

		//speed = initialSpeed;
	}

	newXPOS += speed * currentAcceleration;

	m_x = newXPOS;

	//Decelarating, still to be implemented.
}

void Sanic::Player::Jump() {
	if (!isJumping && isGrounded) {
		jumpVelocity = jumpForce;
		isJumping = true;
	}
}

void Sanic::Player::Physics() {


	float newYPOS= m_y;
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

void Sanic::Player::Render(std::string id, int camX, int camY) {
	Sanic::_TextureManager::Instance()->DrawFrame(id, (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());

	// Debug Col
	collisionBox.x = (int)m_x - camX;
	collisionBox.y = (int)m_y - camY;
	SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 116, 235, 87, 200);
	SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &collisionBox);
}

void Sanic::Player::Destroy()
{
	//Destroy player
}
