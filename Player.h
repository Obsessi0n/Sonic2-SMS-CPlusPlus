#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"
#include <SDL.h>
#include <SDL_image.h>

namespace Sanic {
	class Player : public Entity {
	public:
		Player();
		~Player();

		void TakeDamage();
		void Move() override;
		void Jump();
		void Physics() override;
		void Update();
		void Render(int camX, int camY) override;


		void HorizontalMovementDir(signed char _direction);
		

		signed char direction;

		int getPosX() { return m_x; };
		int getPosY() { return m_y; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

		void Destroy() override;

		const float maxSpeed = 3;
		const float initialSpeed = 0.0001;
		float speed = initialSpeed;
		const float acceleration = 0.1;
		float currentAcceleration = acceleration;

	private:
		
		signed char lastDir = 1;
	};
}
#endif