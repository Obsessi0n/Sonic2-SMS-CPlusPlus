#pragma once
#include <SDL.h>
#include <SDL_image.h>
namespace Sanic {
	class Player {

	public:
		Player(SDL_Renderer* renderer);
		~Player();

		void TakeDamage();
		void Move(bool dir);
		void Render(SDL_Renderer* renderer);

	private:
		const float maxSpeed = 3;
		const float initialSpeed = 0.0001;
		float speed = initialSpeed;
		float acceleration = 0.001;
		


		int rings;
		int lives;

		const int spriteSize=32;

		SDL_Surface* playerSurf;
		SDL_Texture* playerTex;
		void Die();
		SDL_Rect* playerDestRect;
	};
}