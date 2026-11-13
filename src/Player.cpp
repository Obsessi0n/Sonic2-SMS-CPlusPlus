#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"
#include <cmath>

Sanic::Player::Player()
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player.png", "player", Sanic::_Game::Instance()->getRenderer());

	m_x = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2);

	m_y = Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2) - 60;

	playerAnim = new PlayerAnimation;

	entityCollisionBox = { (int)m_x+16, (int)m_y+16, 8, 32 };
	CollisionBox = { (int)m_x, (int)m_y, 16, 32 };
}

Sanic::Player::~Player()
{
	delete playerAnim;
}

void Sanic::Player::HorizontalMovementDir(char _direction) {
	direction = _direction;
}

//The jump key is down. A jump starts at the update it goes down, and lasts while it stays down.
void Sanic::Player::Jump() {
	jumpHeld = true;
}

void Sanic::Player::ReleaseJump() {
	jumpHeld = false;
}

//The speeds of this update: what the keys, the slope, the ground and gravity make of them.
void Sanic::Player::Physics() {
	const float acceleration = Sanic::_PhysicsManager::ACCELERATION;
	const float topSpeed = Sanic::_PhysicsManager::TOP_SPEED;
	const float maxSpeed = Sanic::_PhysicsManager::MAX_SPEED;

	bool jumpPressed = jumpHeld && !jumpWasHeld;
	jumpWasHeld = jumpHeld;

	if (isGrounded) {
		isChangingDirection = false;

		if (direction != 0) {
			if (groundSpeed * direction < 0) {
				//Player changed direction: he brakes, and still looks the way he is moving
				groundSpeed += direction * Sanic::_PhysicsManager::DECELERATION;
				isChangingDirection = true;
			}
			else {
				//Calculate acceleration: running adds speed up to the top speed. Speed over that, gained on a
				//way down, is not taken away while the direction is held.
				if (groundSpeed * direction < topSpeed) {
					groundSpeed += direction * acceleration;
					if (groundSpeed * direction > topSpeed)
						groundSpeed = direction * topSpeed;
				}
				lastDir = direction;
			}
		}

		//The slope pulls downhill: it slows the way up and speeds the way down
		groundSpeed -= Sanic::_PhysicsManager::SLOPE_FACTOR * std::sin(groundAngle);

		//If player is not pressing left or right the ground slows him down, to a stop and no further
		if (direction == 0) {
			const float friction = Sanic::_PhysicsManager::FRICTION;
			if (groundSpeed > friction)
				groundSpeed -= friction;
			else if (groundSpeed < -friction)
				groundSpeed += friction;
			else
				groundSpeed = 0;
		}

		if (groundSpeed > maxSpeed)
			groundSpeed = maxSpeed;
		else if (groundSpeed < -maxSpeed)
			groundSpeed = -maxSpeed;

		//The same speed as sideways and downwards: what moves him now, and what he keeps if he leaves the ground
		xSpeed = groundSpeed * std::cos(groundAngle);
		ySpeed = -groundSpeed * std::sin(groundAngle);

		if (jumpPressed) {
			//A jump goes straight up, on top of the speed he already has
			ySpeed -= Sanic::_PhysicsManager::JUMP_SPEED;
			isGrounded = false;
			isJumping = true;
			isChangingDirection = false;
		}
	}
	else {
		isChangingDirection = false;

		//In the air a direction held still pushes sideways, up to the top speed
		if (direction != 0) {
			if (xSpeed * direction < topSpeed) {
				xSpeed += direction * Sanic::_PhysicsManager::AIR_ACCELERATION;
				if (xSpeed * direction > topSpeed)
					xSpeed = direction * topSpeed;
			}
			lastDir = direction;
		}

		//Calculating the gravity.
		ySpeed += Sanic::_PhysicsManager::GRAVITY;
		if (ySpeed > Sanic::_PhysicsManager::MAX_FALL_SPEED)
			ySpeed = Sanic::_PhysicsManager::MAX_FALL_SPEED;

		//Letting go of the jump key cuts the rise short: that is what makes a short press a small jump
		if (isJumping && !jumpHeld && ySpeed < -Sanic::_PhysicsManager::JUMP_RELEASE_SPEED)
			ySpeed = -Sanic::_PhysicsManager::JUMP_RELEASE_SPEED;
	}
}

//Finaly we aply the movement. On the ground only sideways: how high he stands is read off the floor afterwards.
void Sanic::Player::Move() {
	m_x += xSpeed;

	if (!isGrounded)
		m_y += ySpeed;
}

void Sanic::Player::PushBack()
{
	PhysicsManager* physics = Sanic::_PhysicsManager::Instance();
	float wallY = m_y + sensorF[1];

	//Player inside Right Wall: the sensor is the first column of pixels to the right of the body
	int column = (int)std::floor(m_x + sensorF[0]);
	if (physics->IsSolid(column, wallY)) {
		//First we need to know how far inside we are: back to the first column that is solid
		for (int i = 0; i < 32 && physics->IsSolid(column - 1, wallY); i++)
			column--;
		//Now we need to place the player against it
		m_x = column - sensorF[0];
		if (xSpeed > 0)
			xSpeed = 0;
		if (groundSpeed > 0)
			groundSpeed = 0;
	}
	else {
		//Player inside Left Wall: the column of pixels his left side is in, or the one beside it
		column = (int)std::ceil(m_x + sensorE[0]) - 1;
		if (physics->IsSolid(column, wallY)) {
			for (int i = 0; i < 32 && physics->IsSolid(column + 1, wallY); i++)
				column++;
			//Now we need to place the player on the column on the right
			m_x = column + 1 - sensorE[0];
			if (xSpeed < 0)
				xSpeed = 0;
			if (groundSpeed < 0)
				groundSpeed = 0;
		}
	}

	if (isGrounded || ySpeed >= 0) // We don't need to check head if we are grounded, or on the way down
		return;

	//Player inside ceiling
	if (physics->IsSolid(m_x + sensorC[0], m_y + sensorC[1]) ||
			physics->IsSolid(m_x + sensorD[0], m_y + sensorD[1])) {
		//So first we need to know what block we are inside!
		int blockY = (int)std::floor((m_y + sensorC[1]) / 32);
		//Now we need to place the player on the block below
		m_y = (blockY + 1) * 32 - sensorC[1];
		ySpeed = 0;
	}
}

void Sanic::Player::Update()
{
	DamageCooldownTimer();
	Physics();
	Move();
	PushBack();
	CheckSensors();
	playerAnim->UpdateState();

}

void Sanic::Player::Render(int camX, int camY) {
	if (Immortal && (Sanic::_Game::Instance()->getUpdates() % 6) < 3 ) //Blinks while out of harm: 3 updates seen, 3 not
		return;

	playerAnim->Render(camX, camY);

	CollisionBox.x = (int)m_x + 8 - camX;
	CollisionBox.y = (int)m_y - camY;
	SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 255, 192, 203, 150);
	SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &CollisionBox);
}

SDL_Rect Sanic::Player::GetCollisionRect() {
	return SDL_Rect{ (int)m_x + 8, (int)m_y, 16, 32 };
}


//The floor. The two sensors under the feet each read how high the ground stands under them, inside its tile,
//and the higher of the two is where the player stands: so he follows any shape of ground, up and down.
void Sanic::Player::CheckSensors() {
	PhysicsManager* physics = Sanic::_PhysicsManager::Instance();
	const int step = Sanic::_PhysicsManager::STEP_HEIGHT;
	float feet = m_y + m_height;

	bool found = false;
	int floorY = 0;
	float floorAngle = 0;

	const std::vector<int>* sensors[2] = { &sensorA, &sensorB };
	for (const std::vector<int>* sensor : sensors) {
		int surfaceY = 0;
		float angle = 0;
		if (!physics->FindFloor(m_x + (*sensor)[0], feet, &surfaceY, &angle))
			continue;

		//Ground too high to step on to is a wall, and the wall sensors' business
		if (surfaceY <= feet - step)
			continue;

		if (isGrounded) {
			//On the ground he keeps to it while it is no more than a step down: that is what holds him to a slope
			if (surfaceY > feet + step)
				continue;
		}
		else {
			//In the air the ground only counts once the feet have reached it
			if (surfaceY > feet || (surfaceY == feet && ySpeed < 0))
				continue;
		}

		if (!found || surfaceY < floorY) {
			found = true;
			floorY = surfaceY;
			floorAngle = angle;
		}
	}

	if (found) {
		m_y = floorY - m_height;
		groundAngle = floorAngle;

		if (!isGrounded) {
			//Landing: the part of his speed that runs along the ground is the speed he lands with
			groundSpeed = xSpeed * std::cos(groundAngle) - ySpeed * std::sin(groundAngle);
			isGrounded = true;
			isJumping = false;
		}
	}
	else {
		//No ground under the feet: off a ledge, into the air with the speed he had
		isGrounded = false;
	}

}

void Sanic::Player::Destroy()
{
	//Destroy player
}

void Sanic::Player::TakeDamage(bool dir) {

	if (!Immortal) {
		Immortal = true;
		immortalUpdates = Sanic::_PhysicsManager::HURT_UPDATES;
		//Apply force! Thrown off the ground, to the right or to the left
		groundSpeed = 0;
		if (dir)
			xSpeed = Sanic::_PhysicsManager::HURT_SPEED_X;
		else
			xSpeed = -Sanic::_PhysicsManager::HURT_SPEED_X;

		ySpeed = -Sanic::_PhysicsManager::HURT_SPEED_Y;
		isGrounded = false;
		isJumping = false;

		//Lose rings
		int all = -100;
		Sanic::_Game::Instance()->changePlayerRings(&all);
	}

}

void Sanic::Player::DamageCooldownTimer() {

	if (immortalUpdates > 0) {
		Immortal = true;
		immortalUpdates--;
	}
	else {
		Immortal = false;
	}

}
