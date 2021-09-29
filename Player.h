#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"
#include <SDL.h>
#include <SDL_image.h>
#include <vector>
using namespace std;

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
		void OnSlope(int* yPos);

		void HorizontalMovementDir(char _direction);

		signed char direction;

		int getPosX() { return m_x; };
		int getPosY() { return m_y; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

		void Destroy() override;

		const float maxSpeed = 2.2f;
		const float initialSpeed = 0.0001f;
		float speed = initialSpeed;
		const float acceleration = 0.1f;
		const float decelaration = 0.06f;
		const float friction = 0.05f;
		float currentAcceleration = acceleration;
		SDL_Rect GetCollisionRect();


		


	private:

		void CheckSensors();

		uint32_t cameraOffset = 30;
		char lastDir = 1;

		const vector<int> sensorA = { 8,32 }, sensorB = { 24,32 };
		vector<int> sensorC = { 8,0 }, sensorD = { 24,0 }, sensorE = { 8,16 }, sensorF = { 24,16 };
	};
}
#endif