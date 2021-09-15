#pragma once
#include <iostream>
namespace Sanic {
	class Entity
	{
	public:
		virtual void Move(bool dir);
		virtual void Physics();
		virtual void Render(std::string id, int camX, int camY);
		virtual void Destroy();
	protected:
		float m_x = 0;
		float m_y = 0;
		const int m_width = 32;
		const int m_height = 32;

		bool isJumping = false;
		bool isGrounded = false;

		float fallingTimer = 0;

		float jumpVelocity = 0;
		float jumpForce = 3;
	};
}