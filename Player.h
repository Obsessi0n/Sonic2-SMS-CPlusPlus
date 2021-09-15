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
		void Move(bool dir) override;
		void Jump();
		void Physics() override;
		void Update();
		void Render(std::string id, int camX, int camY) override;

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
	};
}
#endif