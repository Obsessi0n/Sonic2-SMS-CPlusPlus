#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"

Sanic::Player::Player() {
	lives = 3;
	rings = 0;

	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = Sanic::_Game::Instance()->getScreenWidth() / 2 - (m_width / 2);

	m_y = Sanic::_Game::Instance()->getScreenWidth() / 2 - (m_width / 2) + 40;
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
		if (m_x > Sanic::_Game::Instance()->getLevelWidth() - m_width)
			m_x = Sanic::_Game::Instance()->getLevelWidth() - m_width;
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

void Sanic::Player::Render(int camX, int camY) {
	Sanic::_TextureManager::Instance()->DrawFrame("player", m_x - camX, m_y - camY, m_width, m_height, 1, 0, 1, Sanic::_Game::Instance()->getRenderer());
}