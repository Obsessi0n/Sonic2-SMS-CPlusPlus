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

	if (dir && roundedSpeed >= 1) {
		if(!Sanic::_Game::Instance()->getMapLoader()->GetBlockCollision(m_x + roundedSpeed, m_y-2))
			m_x += roundedSpeed;


		if (m_x > Sanic::_Game::Instance()->getLevelWidth() - m_width)
			m_x = Sanic::_Game::Instance()->getLevelWidth() - m_width;
		speed = initialSpeed;
	}
	else if (!dir && roundedSpeed >= 1)
	{
		if (!Sanic::_Game::Instance()->getMapLoader()->GetBlockCollision(m_x -roundedSpeed, m_y-2))
			m_x -= roundedSpeed;

		if (m_x < 0)
			m_x = 0+0.001f;

		speed = initialSpeed;
	}

	//Decelarating, still to be implemented.

}


void Sanic::Player::Jump() {
	if (!isJumping) {
		jumpVelocity = jumpForce;
		isJumping = true;
	}
		
}


void Sanic::Player::Physics() {
	//Physics
	//Gravity
	if (!isGrounded) {
		fallingTimer++;	

	}
	else
		fallingTimer = 0;

	float newYPOS = m_y + Sanic::_PhysicsManager::Instance()->CalculateGravityForce(fallingTimer);

	//Check collision
	if (Sanic::_Game::Instance()->getMapLoader()->GetBlockCollision(m_x, m_y)) {
		isGrounded = true;
		m_y -= jumpVelocity;
	}
	else {
		m_y = newYPOS - jumpVelocity;
		isGrounded = false;
	}
	if (isJumping) {
		jumpVelocity -= 0.02f;
		if (jumpVelocity < 0) {
			jumpVelocity = 0;
			isJumping = false;
		}
			
	}
	
}

void Sanic::Player::Render(int camX, int camY) {
	Sanic::_TextureManager::Instance()->DrawFrame("player", (int)m_x - camX, (int)m_y - camY, m_width, m_height, 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
}