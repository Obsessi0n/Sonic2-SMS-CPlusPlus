#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"
#include "PlayerAnimation.h"
#include <SDL.h>
#include <SDL_image.h>
#include <vector>

namespace Sanic {
	class Player : public Entity {
	public:
		Player();
		~Player();

		//Movement
		const float maxSpeed = 2.2f;
		const float initialSpeed = 0.0001f;
		float speed = initialSpeed;
		const float acceleration = 0.1f;
		const float decelaration = 0.06f;
		const float friction = 0.05f;
		float currentAcceleration = acceleration;
		signed char direction;

		void TakeDamage();
		void Move() override;
		void Jump();
		void Physics() override;
		void Update();
		void Render(int camX, int camY) override;
		void OnSlope(int* yPos);

		void HorizontalMovementDir(char _direction);

		

		int getPosX() { return m_x; };
		int getPosY() { return m_y; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

		float GetAcceleration() { return currentAcceleration; };
		float GetSpeed() { return speed; };
		bool IsChangingDirection() { return isChangingDirection; };

		void Destroy() override;
		SDL_Rect GetCollisionRect();

	private:
		uint32_t cameraOffset = 30;
		char lastDir = 1;
		bool isChangingDirection = false;
		const std::vector<int> sensorA = { 8,32 }, sensorB = { 24,32 }, sensorS = { 16,32 };
		std::vector<int> sensorC = { 8,0 }, sensorD = { 24,0 }, sensorE = { 8,16 }, sensorF = { 24,16 };

		PlayerAnimation* playerAnim;
		void CheckSensors();


	};
}
#endif