#pragma once
#include "Player.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include "Game.h"

Sanic::Player::Player() {

	lives = 3;
	rings = 0;
	playerSprite = IMG_Load("assets/sprites/player/player.png");
	
	playerSpritePos = new SDL_Rect();

	playerSpritePos->x = (256/2)-(32/2);
	playerSpritePos->y = (196/2)-(32/2);
	
	playerSpritePos->w = spriteSize;
	playerSpritePos->h = spriteSize;


	if (!playerSprite) {
		std::cout << "Error loading Player sprite!" << SDL_GetError();
		// handle error
	}
}
Sanic::Player::~Player() {

}

void Sanic::Player::Move(bool dir) {

	//Accelerating
	speed += acceleration;
	acceleration += acceleration;

	if (acceleration > maxSpeed)
		acceleration = maxSpeed;

	int roundedSpeed = std::roundf(speed);

	if (dir && roundedSpeed >= 1) {		
		playerSpritePos->x += roundedSpeed;
		if (playerSpritePos->x > 256 - spriteSize)
			playerSpritePos->x = 256 - spriteSize;
		speed = initialSpeed;
	}
	else if(!dir && roundedSpeed >= 1)
	{

		playerSpritePos->x -= roundedSpeed;
		if (playerSpritePos->x < 0)
			playerSpritePos->x = 0;
		speed = initialSpeed;
	}

	//Decelarating, still to be implemented.

}

void Sanic::Player::Render(SDL_Window* _window) {
	//Apply the image
	gScreenSurface = SDL_GetWindowSurface(_window);
	//Clear previous frame
	SDL_FillRect(gScreenSurface, NULL, SDL_MapRGB(playerSprite->format, 0, 0, 0));
	//Draw new frame
	SDL_BlitSurface(playerSprite, NULL, gScreenSurface, playerSpritePos);
}