#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include <SDL.h>
#include <SDL_image.h>

namespace Sanic {
	class Player {
	public:
		Player();
		~Player();

		void TakeDamage();
		void Move(bool dir);
		void Render();

	private:
		void Die();
		const float maxSpeed = 3;
		const float initialSpeed = 0.0001;
		float speed = initialSpeed;
		float acceleration = 0.001;

		int m_x = 0;
		int m_y = 0;

		int rings = 0;
		int lives = 0;

		const int spriteSizeX = 18;
		const int spriteSizeY = 32;
	};
}
#endif