#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"

Sanic::Player::Player() {
	lives = 3;
	rings = 0;

	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = _Game::Instance()->getScreenWidth() / 2 - (32 / 2);
	m_y = _Game::Instance()->getScreenWidth() / 2 - (32 / 2) -60 ;
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
		m_x += roundedSpeed;
		if (m_x > Sanic::_Game::Instance()->getScreenWidth() - spriteSizeX)
			m_x = Sanic::_Game::Instance()->getScreenWidth() - spriteSizeX;
		speed = initialSpeed;
	}
	else if (!dir && roundedSpeed >= 1)
	{
		m_x -= roundedSpeed;
		if (m_x < 0)
			m_x = 0;
		speed = initialSpeed;
	}

	//Decelarating, still to be implemented.

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
	}
	else {
		m_y = newYPOS;
		isGrounded = false;
	}

}

void Sanic::Player::Render() {
	Sanic::_TextureManager::Instance()->Draw("player", (int)m_x, (int)m_y, spriteSizeX, spriteSizeY, Sanic::_Game::Instance()->getRenderer());
}