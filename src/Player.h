#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"
#include "PhysicsManager.h"
#include "PlayerAnimation.h"
#include <SDL.h>
#include <SDL_image.h>
#include <cstdint>
#include <vector>

namespace Sanic {
	class Player : public Entity {
	public:
		Player();
		~Player();

		void TakeDamage(bool dir);
		void Move() override;
		void Jump();
		void ReleaseJump();
		void Physics() override;
		void Update();
		void Render(int camX, int camY) override;

		void HorizontalMovementDir(char _direction);



		int getPosX() { return m_x; };
		int getPosY() { return m_y; };
		//The same before it is rounded to a pixel: a test wants to know exactly.
		float getExactX() { return m_x; };
		float getExactY() { return m_y; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

		//The speed the player moves at, in pixels per update: along the ground when he is on it, sideways when
		//he is in the air. Positive is to the right.
		float GetSpeed() { return isGrounded ? groundSpeed : xSpeed; };
		//His speed downwards: negative while he rises.
		float GetFallSpeed() { return isGrounded ? 0 : ySpeed; };
		//The side he looks to: 1 is right, -1 is left.
		char GetFacing() { return lastDir; };
		bool IsChangingDirection() { return isChangingDirection; };

		void Destroy() override;
		SDL_Rect GetCollisionRect();

		void DamageCooldownTimer();

	private:
		//Movement. The numbers that shape it are all in PhysicsManager.h.
		//The speed along the ground, whatever way the ground leans: positive is to the right.
		float groundSpeed = 0;
		//How the ground under the feet leans, in radians: positive when it rises to the right.
		float groundAngle = 0;
		//The direction held: -1 left, 1 right, 0 none.
		signed char direction = 0;
		char lastDir = 1;
		bool isChangingDirection = false;
		//The jump key: down now, and down at the update before. A jump starts when it goes down, not while it stays down.
		bool jumpHeld = false;
		bool jumpWasHeld = false;

		//The sensors, from the top left corner of the 32 by 32 frame. The body is the 16 pixels in the middle of it.
		//A and B are under the feet, a pixel in from each side of the body: they read the floor.
		//C and D are at the top of the head: they read the ceiling.
		//E and F are at the two sides of the body, as high above the feet as a step can be: they read the walls.
		const std::vector<int> sensorA = { 9,32 }, sensorB = { 22,32 };
		const std::vector<int> sensorC = { 9,0 }, sensorD = { 22,0 };
		const std::vector<int> sensorE = { 8,32 - PhysicsManager::STEP_HEIGHT }, sensorF = { 24,32 - PhysicsManager::STEP_HEIGHT };

		PlayerAnimation* playerAnim;
		void CheckSensors();
		void PushBack();
		bool Immortal = false;
		//Updates left of being out of harm after a hit.
		int immortalUpdates = 0;
	};
}
#endif
