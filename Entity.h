#pragma once
#include <iostream>
#include <SDL.h>



namespace Sanic {
	class Entity
	{
	public:
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
	protected:
		float m_x = 0;
		float m_y = 0;
		int m_width = 32;
		int m_height = 32;

		SDL_Rect entityCollisionBox{ (int)m_x, (int)m_y, 0, 0 };
		SDL_Rect CollisionBox{ (int)m_x, (int)m_y, 0, 0 };

		bool isJumping = false;
		bool isGrounded = false;

		float fallingTimer = 0;
		float jumpVelocity = 0;
		const float jumpForce = 3.5f;
	};
}