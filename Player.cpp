#pragma once
#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"

Sanic::Player::Player()
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2);

	m_y = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2) - 60;

	playerAnim = new PlayerAnimation;

	entityCollisionBox = { (int)m_x+16, (int)m_y+16, 8, 32 };
	CollisionBox = { (int)m_x, (int)m_y, 16, 32 };
}

Sanic::Player::~Player()
{
}

void Sanic::Player::HorizontalMovementDir(char _direction) {
	direction = _direction;
}

void Sanic::Player::Move() {
	float newXPOS = m_x;

	currentAcceleration += DamageForce;

	if (DamageForce > 0) {
		DamageForce -= 1;
	}
	else
		DamageForce = 0;

	//Player changed direction
	if (direction != lastDir && currentAcceleration > 0) {
		currentAcceleration -= decelaration;
		isChangingDirection = true;
		if (currentAcceleration < 0)
		{
			currentAcceleration = 0;
			isChangingDirection = false;
		}			
	}
	else if (direction == 0) { //If player is not pressing left or right
		currentAcceleration -= friction;
		if (currentAcceleration < 0)
			currentAcceleration = 0;

		speed = lastDir;
	}

	else {
		//Calculate acceleration
		currentAcceleration += acceleration;
		if (currentAcceleration > maxSpeed)
			currentAcceleration = maxSpeed;

		//Direction
		speed = direction;
		//Check collisions

		if (direction == 1) {

				lastDir = 1;
		}
		else {

				lastDir = -1;
		}
	}

	

	CheckSensors();
	
	if (speed != 0) {
		newXPOS += speed * currentAcceleration;
		m_x = newXPOS;
	}
}

void Sanic::Player::Jump() {
	if (!isJumping && isGrounded) {
		jumpVelocity = jumpForce;
		isJumping = true;
	}
}

void Sanic::Player::Physics() {
	float newYPOS = m_y;
	float gravityForce = 0;

	CheckSensors();

	//Calculating the gravity.
	if (!isGrounded) {
		fallingTimer++;
		gravityForce = Sanic::_PhysicsManager::Instance()->CalculateGravityForce(fallingTimer);
	}
	else {
		fallingTimer = 0;
		gravityForce = 0;
	}


	//We decrease the jump velocity with time
	if (isJumping) {
		jumpVelocity -= 0.04f;
		if (jumpVelocity < 0) {
			jumpVelocity = 0;
			isJumping = false;
		}
	}

	//Finaly we aply the movement
	m_y = m_y + gravityForce - jumpVelocity;
}


void Sanic::Player::PushBack() 
{
	//The right wall pushback is super incosistent and I don't know why!
	// 
	////Player inside Right Wall
	if (Sanic::_PhysicsManager::Instance()->IsColliding(sensorB[0] + m_x, sensorB[1] + m_y - 2) ||
		(Sanic::_PhysicsManager::Instance()->IsColliding(sensorF[0] + m_x, sensorF[1] + m_y))) {

		//First we need to know what block we are inside!
		int blockX = Sanic::_Game::Instance()->getMapLoader()->GetBlockXWithCoordinates((m_x + sensorB[0]));
		//Now we need to place the player on the block on the right
		m_x = (blockX-2) * 32 + 6;
		currentAcceleration = 0;
		std::cout << "Player was inside block and we pushed him out!" << '\n';
	}

	//Player inside Left Wall
	else if (Sanic::_PhysicsManager::Instance()->IsColliding(sensorA[0] + m_x, sensorA[1] + m_y - 2) ||
		(Sanic::_PhysicsManager::Instance()->IsColliding(sensorE[0] + m_x, sensorE[1] + m_y))) {
		//First we need to know what block we are inside!
		int blockX = Sanic::_Game::Instance()->getMapLoader()->GetBlockXWithCoordinates((m_x + sensorA[0]));
		//Now we need to place the player on the block on the right
		m_x = (blockX) * 32 - sensorA[0];
		currentAcceleration = 0;
		std::cout << "Player was inside block and we pushed him out!" << '\n';
	}



	//Player inside ground
	if (Sanic::_PhysicsManager::Instance()->IsColliding(sensorB[0] + m_x-1, sensorB[1] + m_y - 2) ||
			Sanic::_PhysicsManager::Instance()->IsColliding(sensorA[0] + m_x+1, sensorA[1] + m_y - 2)) {
		//So first we need to know what block we are inside!
		int blockY = Sanic::_Game::Instance()->getMapLoader()->GetBlockYWithCoordinates((m_y-2));
		//Now we need to place the player on the block above
		m_y = (blockY - 2) * 32 + sensorB[1];

		std::cout << "Player was inside block and we pushed him out!"<<'\n';
	}


	if (isGrounded) // We don't need to check head if we are grounded
		return;

	//Player inside ceiling
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorC[0] + 2, m_y + sensorC[1]) ||
			 Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorD[0]-2, m_y + sensorD[1])) {
		//So first we need to know what block we are inside!
		int blockY = Sanic::_Game::Instance()->getMapLoader()->GetBlockYWithCoordinates((m_y + sensorC[1]));
		//Now we need to place the player on the block below
		jumpVelocity = 0;
		m_y = (blockY-1) * 32 + sensorB[1];

		std::cout << "Player was inside block and we pushed him out!" << '\n';
	}
}

void Sanic::Player::Update()
{
	Physics();
	DamageCooldownTimer();
	Move();
	PushBack();
	playerAnim->UpdateState();
	
}

void Sanic::Player::Render(int camX, int camY) {
	if (Immortal && (SDL_GetTicks() % 6) < 3 ) //I Will need to think of a better way to do this
		return;

	playerAnim->Render(camX, camY);

	CollisionBox.x = (int)m_x + 8 - camX;
	CollisionBox.y = (int)m_y - camY;
	SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 255, 192, 203, 150);
	SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &CollisionBox);
}

SDL_Rect Sanic::Player::GetCollisionRect() {
	return entityCollisionBox;
}


void Sanic::Player::CheckSensors() {

	//Ground
	if (Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorA[0] + 1, m_y + sensorA[1]) != 0 ||
		Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorB[0] - 1, m_y + sensorB[1]) != 0) {

		//Are we on a slope?
		if (Sanic::_PhysicsManager::Instance()->CheckIfSlope(sensorS[0] + m_x, sensorS[1] + m_y) ||
			 Sanic::_PhysicsManager::Instance()->CheckIfSlope(sensorS[0] + m_x, sensorS[1] + m_y - 2)) {

			//Calculate y!
			int slopeY = Sanic::_PhysicsManager::Instance()->CalculateSlope(m_x + sensorS[0], m_y + sensorS[1]);	

			if (m_y + 32 >= slopeY) {
				m_y = slopeY - 32;
				isGrounded = true;				
			}
			else
				isGrounded = false;	

			//Slope pushback
			//Goes here
		}
		else {
			isGrounded = true;				
		}
	}
	else
		isGrounded = false;

}

void Sanic::Player::Destroy()
{
	//Destroy player
}

void Sanic::Player::OnSlope(int* yPos) {

		m_y = (float)*yPos - 32;
}

void Sanic::Player::TakeDamage(bool dir) {

	if (!Immortal) {
		currentDamageCooldownTime = damageCooldownTime;
		startTime = time(&Clock);
		immortalTimerSeconds = 3;
		//Apply force!
		currentAcceleration = 0;
		if (dir)
			DamageForce = 2;
		else
			DamageForce = -2;

		jumpVelocity = 2;
		isJumping = true;

		//Lose rings
		int all = -100;
		Sanic::_Game::Instance()->changePlayerRings(&all);
	}

}

void Sanic::Player::DamageCooldownTimer() {

	if (timeSinceStart < immortalTimerSeconds) {
		Immortal = true;
		time(&Clock);  /* get current time; same as: timer = time(NULL)  */
		timeSinceStart = difftime(Clock, startTime);
		std::cout << timeSinceStart << " " << immortalTimerSeconds << "IMORTAL!";
	}
	else {
		Immortal = false;
		immortalTimerSeconds = 0;
		timeSinceStart = 0;
	}

}