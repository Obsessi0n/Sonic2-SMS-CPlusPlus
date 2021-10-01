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

	entityCollisionBox = { (int)m_x, (int)m_y, 8, 32 };
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

void Sanic::Player::Update()
{
	Move();
	playerAnim->UpdateState();
	Physics();
}

void Sanic::Player::Render(int camX, int camY) {

	playerAnim->Render(camX, camY);

	//animSpeed = 100 - (currentAcceleration * 50 / maxSpeed);

	// Debug EntityCol
	//entityCollisionBox.x = (int)m_x + 5 - camX;
	//entityCollisionBox.y = (int)m_y - camY;
	//SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 116, 235, 87, 150);
	//SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &entityCollisionBox);
	//Debug Col
	CollisionBox.x = (int)m_x + 8 - camX;
	CollisionBox.y = (int)m_y - camY;
	SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 255, 192, 203, 150);
	SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &CollisionBox);
}

SDL_Rect Sanic::Player::GetCollisionRect() {
	return entityCollisionBox;
}


void Sanic::Player::CheckSensors() {

	//Ground sensors
	//A B
	//if ((Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorA[0] + 1, m_y + sensorA[1]) || Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorB[0] - 1, m_y + sensorB[1]))) {
	//	isGrounded = true;
	//}
	//else if ((Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorA[0] + 1, m_y + sensorA[1])==2 || Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorB[0] - 1, m_y + sensorB[1])==2))
	//	isGrounded = true;
	//else
	//	isGrounded = false;



	if (Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorA[0] - 1, m_y + sensorA[1]) != 0) {
		isGrounded = true;

		const float xLiteralPos = m_x + sensorA[0] - 1;
		const float yLiteralPos = m_y + sensorA[1];

		if (Sanic::_Game::Instance()->getMapLoader()->GetBlockType(xLiteralPos, yLiteralPos) == 2) {
			//SLOPE
			int ySlopePos = Sanic::_PhysicsManager::Instance()->CalculateSlope(xLiteralPos, yLiteralPos);

			if(m_y < ySlopePos)
				OnSlope(&ySlopePos);

			std::cout << "Standing on a Slope" << '\n';
		}
		else {
			std::cout << "Standing on the Ground" << '\n';
			//IS player inside ground?
			if (std::floor(yLiteralPos / 32) == std::floor((m_y + sensorA[1]) / 32)) {
				m_y = (std::floor(yLiteralPos / 32) - 1) * 32;
			}
		}
	}
	else {
		isGrounded = false;
	}
		

	//Check for slopes
	//if (isGrounded && !isJumping) {
	//	if (Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorA[0] + 1, m_y + sensorA[1]) == 2) {

	//		//Convert Literal Position to block.
	//		int xBlock = std::floor(m_x + sensorA[0] + 1) / Sanic::_Game::Instance()->getTileSize();
	//		int yBlock = std::floor(m_y + sensorA[1]) / Sanic::_Game::Instance()->getTileSize();
	//		int slopeY = Sanic::_PhysicsManager::Instance()->CalculateSlope(xBlock, yBlock);
	//		OnSlope(&slopeY);

	//	}
	//
	//	
	//	else if (Sanic::_PhysicsManager::Instance()->CheckIfGrounded(m_x + sensorB[0] - 1, m_y + sensorB[1]) == 2) {
	//		//Convert Literal Position to block.
	//		int xBlock = std::floor((m_x + sensorB[0] - 1 )/ Sanic::_Game::Instance()->getTileSize());
	//		int yBlock = std::floor((m_y + sensorB[1]) / Sanic::_Game::Instance()->getTileSize());

	//		int slopeY = Sanic::_PhysicsManager::Instance()->CalculateSlope(xBlock, yBlock);
	//	
	//		OnSlope(&slopeY);
	//		

	//	}
	//}




	//Walls	
	//Right
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorF[0], m_y + sensorF[1]) && direction == 1) {
		currentAcceleration = 0;
		speed = 0;

	}
	else if (!isGrounded && Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorD[0], m_y + sensorD[1]) && direction == 1){
		currentAcceleration = 0;
		speed = 0;

	}

	//Left
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorE[0], m_y + sensorE[1]) && direction==-1) {
		currentAcceleration = 0;
		speed = 0;
	}
	else if (!isGrounded && Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorC[0], m_y + sensorC[1]) && direction == -1) {
		currentAcceleration = 0;
		speed = 0;
	}

	//Pushback
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorE[0], m_y + sensorE[1]) && direction == -1) {//Left
		
		int block = std::floor((m_x + sensorE[0]) / 32);
		block++;
		m_x = block * 32 - sensorE[0];

	}
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorF[0], m_y + sensorF[1]) && direction == 1) {//Right

		int block = std::floor((m_x + sensorF[0]) / 32);
		block--;
		m_x = block * 32 + sensorE[0];
	}





	if (isGrounded)
		return; // We don't need to check head if we are grounded!

	//Head
	if (Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorC[0], m_y + sensorC[1]) || Sanic::_PhysicsManager::Instance()->IsColliding(m_x + sensorD[0] - 1, m_y + sensorD[1])) {
		jumpVelocity = 0;
		//Pushback
		int block = std::floor((m_y + sensorC[1]) / 32);
		block++;
		m_y = block * 32 + sensorC[1];
	}
		

		

}

void Sanic::Player::Destroy()
{
	//Destroy player
}

void Sanic::Player::OnSlope(int* yPos) {

		m_y = (float)*yPos - 32;
}