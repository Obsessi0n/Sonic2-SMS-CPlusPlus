#pragma once
#include <iostream>
#include <string>
#include <SDL.h>



namespace Sanic {
	class Entity
	{
	public:
		// Entities are deleted through pointers to what they really are (a player, something to pick up).
		virtual ~Entity() {}

		virtual void Move();
		virtual void Physics();
		void Update();
		virtual void Render(int camX, int camY);
		virtual void Destroy();

		int getX() { return m_x; };
		int getY() { return m_y; };

		void setX(int x) { m_x = x; };
		void setY(int y) { m_y = y; };

		bool IsJumping() { return isJumping; };
		bool IsGrounded() { return isGrounded; };
	protected:
		float m_x = 0;
		float m_y = 0;
		int m_width = 32;
		int m_height = 32;

		//Speed, in pixels per update: to the right and downwards are positive.
		float xSpeed = 0;
		float ySpeed = 0;

		SDL_Rect entityCollisionBox{ (int)m_x, (int)m_y, 0, 0 };
		SDL_Rect CollisionBox{ (int)m_x, (int)m_y, 0, 0 };

		bool isJumping = false;
		bool isGrounded = false;
	};
}
