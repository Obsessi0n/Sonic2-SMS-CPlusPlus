#include "Player.h"
#include "Game.h"
#include "TextureManager.h"
#include "PhysicsManager.h"
#include <cstdlib>

//The numbers of the movement: see PhysicsManager.h.
typedef Sanic::PhysicsManager Rules;

Sanic::Player::Player()
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player.bmp", "player", Sanic::_Game::Instance()->getRenderer());

	playerAnim = new PlayerAnimation;

	Place(Sanic::_Game::Instance()->getResWidth() / 2 - (m_width / 2), 0);

	entityCollisionBox = GetCollisionRect();
	CollisionBox = GetCollisionRect();
}

Sanic::Player::~Player()
{
	delete playerAnim;
}

void Sanic::Player::Place(int frameX, int frameY) {
	//The middle of his feet is the middle of the frame's bottom edge
	subX = (frameX + m_width / 2) * Rules::SUB;
	subY = (frameY + m_height) * Rules::SUB;
	speedX = 0;
	speedY = 0;
	topSpeed = Rules::TOP_SPEED;
	state = PlayerState::Falling;
	onFloor = false;
	wallLeft = false;
	wallRight = false;
	slopePush = 0;
	jumpBoost = false;

	m_x = frameX;
	m_y = frameY;
}

void Sanic::Player::HorizontalMovementDir(char _direction) {
	direction = _direction;
}

//The jump key is down. A jump starts at the update it goes down, and is lifted while it stays down.
void Sanic::Player::Jump() {
	jumpHeld = true;
}

void Sanic::Player::ReleaseJump() {
	jumpHeld = false;
}

void Sanic::Player::LookUp(bool held) {
	upHeld = held;
}

void Sanic::Player::Crouch(bool held) {
	downHeld = held;
}

//What the keys add to his speed sideways in this update, by the state he is in. Positive is to the right.
int Sanic::Player::InputTerm() {
	//The way he is moving: 1 right, -1 left, 0 not at all
	int moving = (speedX > 0) - (speedX < 0);
	int term = 0;
	//What slows him down when no direction is held: always against the way he is moving
	int friction = 0;

	switch (state) {
	case PlayerState::Standing:
	case PlayerState::Crouching:
	case PlayerState::LookingUp:
		//A direction held does nothing yet: he starts walking, and gains speed from the next update on
		if (direction == 0)
			friction = Rules::FRICTION;
		break;
	case PlayerState::Walking:
		//The same push whichever way he is moving: with it he gains speed, against it he loses it
		if (direction != 0)
			term = Rules::ACCELERATION * direction;
		else
			friction = Rules::FRICTION;
		break;
	case PlayerState::Skidding:
		if (direction == 0)
			friction = Rules::FRICTION;
		else if (direction == -moving)
			term = Rules::SKID_BRAKING * direction;
		break;
	case PlayerState::Rolling:
		//A roll cannot be pushed: every key slows it, the one against it the most
		if (direction == 0)
			friction = Rules::ROLL_FRICTION;
		else if (direction == moving)
			term = -Rules::ROLL_FORWARD * moving;
		else if (direction == -moving)
			term = -Rules::ROLL_BACK * moving;
		break;
	case PlayerState::Jumping:
	case PlayerState::RampJumping:
		//In a jump he can turn back hard
		if (direction == 0)
			friction = Rules::AIR_DRAG;
		else if (direction == -moving)
			term = Rules::AIR_BRAKING * direction;
		else
			term = Rules::AIR_ACCELERATION * direction;
		break;
	case PlayerState::Falling:
		//Having walked off a ledge he cannot
		if (direction == 0)
			friction = Rules::AIR_DRAG;
		else
			term = Rules::AIR_ACCELERATION * direction;
		break;
	case PlayerState::Hurt:
		//Thrown back, he has no say
		break;
	}

	term -= friction * moving;

	//At a crawl everything counts double
	if (std::abs(speedX) < Rules::LOW_SPEED)
		term *= 2;

	//On the ground the friction may take him a little past zero: the rule that makes him stand takes care
	//of it. In the air nothing would, so there the drag stops at zero. (The reference lets it go past and
	//swing about zero by a fraction of a pixel, unseen: to have that, take these two lines out.)
	if (friction != 0 && IsInTheAir() && std::abs(term) > std::abs(speedX))
		term = -speedX;

	return term;
}

//The start of an update: the sides of the picture, and what the keys and the slope add to his speed.
void Sanic::Player::Physics() {
	jumpPressed = jumpHeld && !jumpWasHeld;
	jumpWasHeld = jumpHeld;

	//He cannot leave the picture. The camera stops at the two ends of the level, so this is also what
	//stops him there.
	int cameraX = Sanic::_Game::Instance()->camera.x;
	int screenX = Rules::ToPixel(subX) - cameraX;
	if (screenX < Rules::SCREEN_LEFT) {
		subX = Rules::AtPixel(subX, cameraX + Rules::SCREEN_LEFT);
		speedX = 0;
	}
	else if (screenX > Rules::SCREEN_RIGHT) {
		subX = Rules::AtPixel(subX, cameraX + Rules::SCREEN_RIGHT);
		speedX = 0;
	}

	accelerationX = InputTerm();

	//The slope pushes downhill while he is moving on it. Standing, crouching or looking up it does not, so
	//he never slides off his feet; in the air there is no slope. (In the reference the push of the last
	//slope stays on while he is thrown back by a hit: a slip not copied.)
	if (state == PlayerState::Walking || state == PlayerState::Skidding || state == PlayerState::Rolling)
		accelerationX += slopePush;

	//He looks the way of the key, except in a skid, where he still looks the way he is going
	if (direction != 0 && state != PlayerState::Skidding && state != PlayerState::Hurt)
		lastDir = direction;
}

//Finaly we aply the movement: sideways, then up or down.
void Sanic::Player::Move() {
	int speed = speedX + accelerationX;
	if ((speed > 0 && wallRight) || (speed < 0 && wallLeft)) {
		//The wall he reached at the last update stops him now, and he does not move
		speedX = 0;
	}
	else {
		if (speed > topSpeed)
			speed = topSpeed;
		else if (speed < -topSpeed)
			speed = -topSpeed;
		speedX = speed;
		subX += speedX;
	}

	if (IsInTheAir()) {
		//While the jump key stays down, for the first updates of a jump, he keeps the speed the jump began
		//with. Letting go ends that for good: it does not slow the rise he already has.
		if (state == PlayerState::Jumping && jumpBoost) {
			if (jumpHeld) {
				jumpUpdates++;
				if (jumpUpdates < Rules::JUMP_HOLD_UPDATES)
					speedY = -Rules::JUMP_SPEED;
				else
					jumpBoost = false;
			}
			else
				jumpBoost = false;
		}

		//Calculating the gravity.
		speedY += state == PlayerState::RampJumping ? Rules::RAMP_GRAVITY : Rules::GRAVITY;
		if (speedY > Rules::MAX_FALL_SPEED)
			speedY = Rules::MAX_FALL_SPEED;
	}
	else {
		//On the ground he is driven down into it at every update, for the floor check to lift him back to
		//the surface: on a way down that is what keeps his feet on it
		speedY = Rules::GROUND_STICK;
	}
	subY += speedY;
}

//The floor, felt at one point: the middle of his feet.
void Sanic::Player::CheckSensors() {
	int surfaceY = 0;
	int push = 0;

	switch (Sanic::_PhysicsManager::Instance()->CheckFloor(Rules::ToPixel(subX), Rules::ToPixel(subY), speedY, &surfaceY, &push)) {
	case Rules::NO_GROUND:
		//Only an empty block takes the floor away
		onFloor = false;
		break;
	case Rules::ON_GROUND:
		//Lifted to the surface: only whole pixels, the fraction he had is kept
		subY = Rules::AtPixel(subY, surfaceY);
		onFloor = true;
		slopePush = push;
		break;
	case Rules::ABOVE_GROUND:
		//In a block of ground but not down to its surface: nothing changes. On the ground he goes on being
		//driven down until he meets it.
		break;
	}
}

//The walls, left then right, at every update whichever way he moves; then the ceiling when he is not on the floor.
void Sanic::Player::PushBack()
{
	PhysicsManager* physics = Sanic::_PhysicsManager::Instance();
	int wallY = Rules::ToPixel(subY) - Rules::WALL_PROBE_Y;

	//Player inside Left Wall
	int probe = Rules::ToPixel(subX) - Rules::WALL_PROBE_X;
	wallLeft = physics->IsSolid(probe, wallY);
	if (wallLeft) {
		//First we need to know how far inside we are: on to the first pixel that is not solid
		int out = probe;
		for (int i = 0; i < 32; i++) {
			out++;
			if (!physics->IsSolid(out, wallY))
				break;
		}
		//Now we need to place the player that far to the right
		subX += (out - probe) * Rules::SUB;
	}

	//Player inside Right Wall
	probe = Rules::ToPixel(subX) + Rules::WALL_PROBE_X;
	wallRight = physics->IsSolid(probe, wallY);
	if (wallRight) {
		int out = probe;
		for (int i = 0; i < 32; i++) {
			out--;
			if (!physics->IsSolid(out, wallY))
				break;
		}
		subX -= (probe - out) * Rules::SUB;
	}

	if (onFloor) // We don't need to check head if we are on the floor
		return;

	//Player inside ceiling
	int ceilingY = Rules::ToPixel(subY) - Rules::CEILING_PROBE_Y;
	if (physics->IsSolid(Rules::ToPixel(subX), ceilingY)) {
		//So first we need to know what block we are inside!
		int blockY = Rules::ToBlock(ceilingY);
		//Now we need to place the player under it, and he comes away from it downwards
		subY = Rules::AtPixel(subY, (blockY + 1) * 32 + Rules::CEILING_PROBE_Y);
		speedY = Rules::CEILING_BUMP_SPEED;
	}
}

void Sanic::Player::Update()
{
	DamageCooldownTimer();
	Physics();
	Move();
	CheckSensors();
	PushBack();

	//A jump starts when the key goes down while he is on the floor, from whatever he was doing there. It
	//is a speed upwards and nothing else: his speed sideways stays, and a slope makes no difference.
	if (jumpPressed && !IsInTheAir() && onFloor) {
		state = PlayerState::Jumping;
		speedY = -Rules::JUMP_SPEED;
		onFloor = false;
		jumpUpdates = 0;
		jumpBoost = true;
	}
	else
		ChangeState();

	//Where his frame is drawn, and what the rest of the game asks of an entity
	m_x = Rules::ToPixel(subX) - m_width / 2;
	m_y = Rules::ToPixel(subY) - m_height;
	isGrounded = onFloor;
	isJumping = state == PlayerState::Jumping || state == PlayerState::RampJumping;

	playerAnim->UpdateState();

}

//What he does next. The new state counts from the next update on.
void Sanic::Player::ChangeState() {
	if (IsInTheAir()) {
		//Landing: he walks on with the speed sideways he had. His top speed is the walking one again, so
		//what a roll had added is lost at the next update.
		if (onFloor) {
			state = PlayerState::Walking;
			topSpeed = Rules::TOP_SPEED;
		}
		return;
	}

	if (!onFloor) {
		//The floor is gone from under his middle. He has already been driven 7 pixels down by this update.
		if (state == PlayerState::Rolling) {
			if (slopePush != 0 && ((speedX > 0 && slopePush < 0) || (speedX < 0 && slopePush > 0))) {
				//Rolled off the end of a slope going up it: thrown up at one and a half times his speed sideways
				state = PlayerState::RampJumping;
				speedY = -(std::abs(speedX) + std::abs(speedX) / 2);
			}
			else {
				//Rolled off a ledge: a ball in the air with no speed downwards yet
				state = PlayerState::Jumping;
				speedY = 0;
			}
			//Holding the jump key lifts only a jump that the key began. (The reference does not clear its
			//count here, so a key held while rolling off can lift him as if he had jumped: a slip not copied.)
			jumpBoost = false;
		}
		else {
			//Walked off a ledge
			state = PlayerState::Falling;
			speedY = Rules::WALK_OFF_SPEED;
		}
		slopePush = 0;
		return;
	}

	int speed = std::abs(speedX);
	int moving = (speedX > 0) - (speedX < 0);

	switch (state) {
	case PlayerState::Standing:
		if (direction != 0)
			state = PlayerState::Walking;
		else if (downHeld)
			state = PlayerState::Crouching;
		else if (upHeld)
			state = PlayerState::LookingUp;
		break;
	case PlayerState::Crouching:
		if (!downHeld)
			state = PlayerState::Standing;
		break;
	case PlayerState::LookingUp:
		if (direction != 0)
			state = PlayerState::Walking;
		else if (!upHeld)
			state = PlayerState::Standing;
		break;
	case PlayerState::Walking:
		if (direction != 0 && direction == -moving && speed >= Rules::MIN_SKID_SPEED) {
			//Player changed direction, fast enough to skid: he looks the way he is going until he has turned
			state = PlayerState::Skidding;
			skidDirection = moving;
			lastDir = moving;
		}
		else if (downHeld && speed >= Rules::MIN_ROLL_SPEED) {
			state = PlayerState::Rolling;
			topSpeed = Rules::ROLL_TOP_SPEED;
		}
		else if (direction == 0 && speed < Rules::STOP_SPEED) {
			//If player is not pressing left or right and has all but stopped, he stops
			state = PlayerState::Standing;
			speedX = 0;
		}
		break;
	case PlayerState::Skidding:
		if (moving == -skidDirection) {
			//He has turned round
			state = PlayerState::Walking;
			topSpeed = Rules::TOP_SPEED;
		}
		else if (direction == 0 && speed < Rules::STOP_SPEED) {
			state = PlayerState::Standing;
			speedX = 0;
			topSpeed = Rules::TOP_SPEED;
		}
		else if (direction != -skidDirection) {
			//He let go of the key that was braking: he walks on. (The reference tests the same key for both
			//sides, so there only a skid to the left ends this way: a slip not copied. To have it, make this
			//"skidDirection == -1 && direction != 1".)
			state = PlayerState::Walking;
			topSpeed = Rules::TOP_SPEED;
		}
		break;
	case PlayerState::Rolling:
		if (direction != 0 && direction == -moving) {
			//Holding against a roll turns it into a skid
			state = PlayerState::Skidding;
			skidDirection = moving;
			lastDir = moving;
		}
		else if (slopePush == 0 && speed < Rules::ROLL_STOP_SPEED) {
			//On the flat a roll that has all but stopped is over; on a slope it never ends by itself
			state = downHeld ? PlayerState::Crouching : PlayerState::Standing;
			speedX = 0;
			topSpeed = Rules::TOP_SPEED;
		}
		break;
	default:
		break;
	}
}

void Sanic::Player::Render(int camX, int camY) {
	if (Immortal && (immortalUpdates & 2) != 0) //Blinks while out of harm: 2 updates seen, 2 not
		return;

	playerAnim->Render(camX, camY);

	CollisionBox = GetCollisionRect();
	CollisionBox.x -= camX;
	CollisionBox.y -= camY;
	SDL_SetRenderDrawColor(Sanic::_Game::Instance()->getRenderer(), 255, 192, 203, 150);
	SDL_RenderDrawRect(Sanic::_Game::Instance()->getRenderer(), &CollisionBox);
}

//His box against what is not the level: narrower than his picture, and lower when he is curled up.
SDL_Rect Sanic::Player::GetCollisionRect() {
	int height = IsBall() ? Rules::BALL_HEIGHT : Rules::BOX_HEIGHT;
	return SDL_Rect{ getFeetX() - Rules::BOX_HALF_WIDTH, getFeetY() - height, 2 * Rules::BOX_HALF_WIDTH, height };
}

void Sanic::Player::Destroy()
{
	//Destroy player
}

void Sanic::Player::TakeDamage(bool dir) {

	if (!Immortal && state != PlayerState::Hurt) {
		Immortal = true;
		immortalUpdates = Rules::HURT_UPDATES;
		state = PlayerState::Hurt;

		//Apply force! Thrown away from what hit him, at a speed he cannot change until he lands
		if (dir)
			speedX = Rules::HURT_SPEED_X;
		else
			speedX = -Rules::HURT_SPEED_X;

		//And up, unless his head is against a ceiling
		bool headAgainstCeiling = Sanic::_PhysicsManager::Instance()->IsSolid(getFeetX(), getFeetY() - Rules::CEILING_PROBE_Y - 1);
		speedY = headAgainstCeiling ? 0 : -Rules::HURT_SPEED_Y;

		onFloor = false;
		slopePush = 0;
		jumpBoost = false;

		//Lose rings
		int all = -100;
		Sanic::_Game::Instance()->changePlayerRings(&all);
	}

}

void Sanic::Player::DamageCooldownTimer() {

	if (immortalUpdates > 0)
		immortalUpdates--;

	Immortal = immortalUpdates > 0;

}
