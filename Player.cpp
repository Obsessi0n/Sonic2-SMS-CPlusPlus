#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"

Sanic::Player::Player() {
	lives = 3;
	rings = 0;

	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = Sanic::_Game::Instance()->getScreenWidth() / 2 - (m_width / 2);

	m_y = Sanic::_Game::Instance()->getScreenWidth() / 2 - (m_width / 2) + -60;
}

Sanic::Player::~Player() {
}

void Sanic::Player::Move(bool dir) {
	//Accelerating
	speed += acceleration;
	acceleration += acceleration;

	if (acceleration > maxSpeed)
		acceleration = maxSpeed;

	int roundedSpeed = std::roundf(speed);

	//Added an offset of 16 to x and y because we don't have a player collission box yet (DIRTY HACK)

	if (dir && roundedSpeed >= 1) {
		
		if (!Sanic::_Game::Instance()->getMapLoader()->GetBlockCollision(m_x + roundedSpeed+16, m_y - 16))
			m_x += roundedSpeed;

		if (m_x > Sanic::_Game::Instance()->getLevelWidth() - m_width)
			m_x = Sanic::_Game::Instance()->getLevelWidth() - m_width;
		speed = initialSpeed;
	}
	else if (!dir && roundedSpeed >= 1)
	{
		if (!Sanic::_Game::Instance()->getMapLoader()->GetBlockCollision(m_x - roundedSpeed, m_y - 16))
			m_x -= roundedSpeed;

		if (m_x < 0)
			m_x = 0 + 0.001f;

		speed = initialSpeed;
	}

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

void Sanic::Player::Render(int camX, int camY) {
	Sanic::_TextureManager::Instance()->DrawFrame("player", (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
}