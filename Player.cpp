#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"

Sanic::Player::Player() {
	lives = 3;
	rings = 0;

	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = _Game::Instance()->getScreenWidth() / 2 - (32 / 2);
	m_y = _Game::Instance()->getScreenWidth() / 2 - (32 / 2);
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
		if (m_x > Sanic::_Game::Instance()->getScreenWidth())
			m_x = Sanic::_Game::Instance()->getScreenWidth() - spriteSize;
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

void Sanic::Player::Render() {
	Sanic::_TextureManager::Instance()->Draw("player", m_x, m_y, spriteSize, spriteSize, Sanic::_Game::Instance()->getRenderer());
}