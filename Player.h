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
		void Jump();
		void Physics();
		void Render(int camX, int camY);

		int getPosX() { return m_x; };
		int getPosY() { return m_y; };

		int getSpeed() { return speed; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

	private:
		void Die();
		const float maxSpeed = 3;
		const float initialSpeed = 0.0001;
		float speed = initialSpeed;
		const float acceleration = 0.1;
		float currentAcceleration = acceleration;

		float m_x = 0;
		float m_y = 0;

		int rings = 0;
		int lives = 0;

		bool isJumping = false;
		bool isGrounded = false;

		float fallingTimer = 0;
		const int m_width = 32;
		const int m_height = 32;

		float jumpVelocity = 0;
		float jumpForce = 3;
	};
}
#endif