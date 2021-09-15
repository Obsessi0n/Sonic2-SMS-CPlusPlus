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
		virtual void Render(std::string id, int camX, int camY);
		virtual void Destroy();
	protected:
		float m_x = 0;
		float m_y = 0;
		int m_width = 32;
		int m_height = 32;

		SDL_Rect collisionBox{ (int)m_x, (int)m_y, 0, 0 };

		bool isJumping = false;
		bool isGrounded = false;

		float fallingTimer = 0;

		float jumpVelocity = 0;
		float jumpForce = 3;
	};
}