#pragma once
#include <SDL.h>
#include <SDL_image.h>
namespace Sanic {
	class Player {

	public:
		Player();
		~Player();

		void TakeDamage();
		void Move(bool dir);
		void Render(SDL_Window* _window);

	private:
		const float maxSpeed = 3;
		const float initialSpeed = 0.0001;
		float speed = initialSpeed;
		float acceleration = 0.001;
		


		int rings;
		int lives;

		const int spriteSize=32;

		SDL_Surface* playerSprite;
		void Die();
		//The surface contained by the window
		SDL_Surface* gScreenSurface = NULL;
		SDL_Rect* playerSpritePos;
	};
}