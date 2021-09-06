#pragma once
#include "Player.h"
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include "Game.h"

Sanic::Player::Player(SDL_Renderer * renderer) {

	lives = 3;
	rings = 0;
	playerSurf = IMG_Load("assets/sprites/player/player.png");
	playerTex = SDL_CreateTextureFromSurface(renderer, playerSurf);
	SDL_FreeSurface(playerSurf);
	
	playerDestRect = new SDL_Rect();

	playerDestRect->x = (Game::SCREEN_WIDTH / 2) - (32 / 2);
	playerDestRect->y = (Game::SCREEN_HEIGHT / 2) - (32 / 2);
	
	playerDestRect->w = spriteSize;
	playerDestRect->h = spriteSize;


	if (!playerTex) {
		std::cerr << "Error loading Player sprite!" << SDL_GetError();
		// handle error
	}
}
Sanic::Player::~Player() {
	delete playerDestRect;
}

void Sanic::Player::Move(bool dir) {

	//Accelerating
	speed += acceleration;
	acceleration += acceleration;

	if (acceleration > maxSpeed)
		acceleration = maxSpeed;

	int roundedSpeed = std::roundf(speed);

	if (dir && roundedSpeed >= 1) {		
		playerDestRect->x += roundedSpeed;
		if (playerDestRect->x > Game::SCREEN_WIDTH - spriteSize)
			playerDestRect->x = Game::SCREEN_WIDTH - spriteSize;
		speed = initialSpeed;
	}
	else if(!dir && roundedSpeed >= 1)
	{

		playerDestRect->x -= roundedSpeed;
		if (playerDestRect->x < 0)
			playerDestRect->x = 0;
		speed = initialSpeed;
	}

	//Decelarating, still to be implemented.

}

void Sanic::Player::Render(SDL_Renderer * renderer) {
	SDL_RenderCopyEx(renderer, playerTex, NULL, playerDestRect, NULL, NULL, SDL_FLIP_NONE);
}