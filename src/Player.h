#pragma once
#ifndef PLAYER_H
#define PLAYER_H
#include "Entity.h"
#include "PhysicsManager.h"
#include "PlayerAnimation.h"
#include "PlayerState.h"
#include <SDL.h>
#include <cstdint>
#include <vector>

namespace Sanic {
	//The player moves by the rules of an 8-bit console game, as the movement reference describes them: whole
	//numbers of 1/256 of a pixel, a speed sideways and a speed downwards and nothing else (no angle, no speed
	//"along the ground"), and the level felt at a few single points. The numbers are in PhysicsManager.h.
	//
	//Where he is, is the middle of his feet: the pixel row just under his picture, which is the top row of
	//the floor when he stands. His picture is a 32 by 32 frame drawn with its bottom edge on that row and its
	//middle on that column, so the frame's top left corner (what getPosX and getPosY give) is 16 to the left
	//and 32 above. From the middle of the feet, the points that feel the level are:
	//
	//    the floor      ( 0,   0)   one point, under the middle of the feet
	//    the walls      (-10, -12)  and (+10, -12)
	//    the ceiling    ( 0, -24)
	//
	//so against the level he is 21 pixels wide and 24 high inside the 32 by 32 frame, and stands on one point:
	//he falls as soon as his middle is past an edge. Against things that are not the level (what he picks
	//up) he is a box 8 wide, and 28 high from the feet, or 18 as a ball.
	class Player : public Entity {
	public:
		Player();
		~Player();

		//Thrown back and up, and out of harm for a while. dir is true when what hit him was on his left: he
		//is thrown to the right.
		void TakeDamage(bool dir);
		void Move() override;
		void Jump();
		void ReleaseJump();
		void LookUp(bool held);
		void Crouch(bool held);
		void Physics() override;
		void Update();
		void Render(int camX, int camY) override;

		void HorizontalMovementDir(char _direction);

		//Puts him in the level with the top left corner of his frame at this pixel, in the air and at rest.
		void Place(int frameX, int frameY);

		//The top left corner of his frame, in whole pixels.
		int getPosX() { return m_x; };
		int getPosY() { return m_y; };
		//The middle of his feet, in whole pixels and in 1/256 of a pixel.
		int getFeetX() { return PhysicsManager::ToPixel(subX); };
		int getFeetY() { return PhysicsManager::ToPixel(subY); };
		int getSubX() { return subX; };
		int getSubY() { return subY; };

		int getWidth() { return m_width; };
		int getHeight() { return m_height; };

		//His speed sideways and downwards, in 1/256 of a pixel per update: to the right and down are positive.
		int getSpeedX() { return speedX; };
		int getSpeedY() { return speedY; };
		//The most his speed sideways can be just now: it goes up with a roll and back on landing.
		int getTopSpeed() { return topSpeed; };
		//The push of the slope he stands on, downhill: negative is to the left, 0 on the flat.
		int getSlopePush() { return slopePush; };
		PlayerState getState() { return state; };
		bool IsInTheAir() { return state == PlayerState::Jumping || state == PlayerState::Falling || state == PlayerState::RampJumping || state == PlayerState::Hurt; };
		//Curled up: rolling, or in the air from a jump or a roll.
		bool IsBall() { return state == PlayerState::Rolling || state == PlayerState::Jumping || state == PlayerState::RampJumping; };
		//Out of harm after a hit.
		bool IsImmortal() { return Immortal; };

		//The side he looks to: 1 is right, -1 is left.
		char GetFacing() { return lastDir; };

		void Destroy() override;
		SDL_Rect GetCollisionRect();

		void DamageCooldownTimer();

	private:
		//Where he is and how fast he moves, in 1/256 of a pixel: see the top of the class.
		int subX = 0;
		int subY = 0;
		int speedX = 0;
		int speedY = 0;
		int topSpeed = PhysicsManager::TOP_SPEED;
		PlayerState state = PlayerState::Falling;

		//What the last check of the level found. onFloor and the two walls are used by the update after the
		//one that set them, as the reference has it: a wall stops him one update after he reaches it.
		bool onFloor = false;
		bool wallLeft = false;
		bool wallRight = false;
		//The push of the slope he stands on, downhill: see TileKind::push.
		int slopePush = 0;

		//The keys. The direction held: -1 left, 1 right, 0 none.
		signed char direction = 0;
		bool upHeld = false;
		bool downHeld = false;
		//The jump key: down now, and down at the update before. A jump starts when it goes down, not while it stays down.
		bool jumpHeld = false;
		bool jumpWasHeld = false;
		//It went down at this update.
		bool jumpPressed = false;
		//What this update adds to his speed sideways: the keys and the slope.
		int accelerationX = 0;

		char lastDir = 1;
		//The way he was moving when a skid began: it ends when he moves the other way.
		signed char skidDirection = 0;
		//How many updates of this jump the key has been held, and whether holding it still lifts him.
		int jumpUpdates = 0;
		bool jumpBoost = false;

		PlayerAnimation* playerAnim;
		void CheckSensors();
		void PushBack();
		//What the keys add to his speed sideways in the state he is in.
		int InputTerm();
		//After the update, what he does next: land, fall, skid, roll, stop.
		void ChangeState();
		bool Immortal = false;
		//Updates left of being out of harm after a hit.
		int immortalUpdates = 0;
	};
}
#endif
