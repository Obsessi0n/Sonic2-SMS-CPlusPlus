// Tests of the physics. They need no window, no pictures and no keyboard: each one starts the game's rules on a
// small level written here as text, holds keys for a number of updates as a player would, and checks where the
// player is and how fast he goes. The same keys on the same level give the same numbers every time.
//
// What is expected is worked out from the constants in src/PhysicsManager.h, not written down as numbers:
// tuning a constant should not break a test, unless it breaks what the test is about (a jump too low to count,
// a slope the player can no longer stand on).
//
//   cmake --build build
//   ctest --test-dir build --output-on-failure

#include "Game.h"
#include "InputManager.h"
#include "PhysicsManager.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

typedef Sanic::PhysicsManager Physics;

static int checks = 0;
static int failures = 0;
static std::string current;

// The tiles the levels below are made of: a solid block, the four gentle slopes of the game (1 up for 2 along)
// and a slope of 45 degrees.
static const char* TILES =
	"G ground     solid\n"
	"a rise_low   slope 31 16\n"
	"b rise_high  slope 15 0\n"
	"c fall_high  slope 0 15\n"
	"d fall_low   slope 16 31\n"
	"u rise_steep slope 31 0\n";

// The ground is the top of row 6: y = 192. A player standing on it is at y = 160.
static const float GROUND = 192;
static const float STANDING = GROUND - 32;

static void check(bool ok, const std::string& what)
{
	checks++;
	if (!ok)
	{
		failures++;
		std::cout << "FAILED  " << current << ": " << what << '\n';
	}
}

static void checkNear(float value, float expected, float tolerance, const std::string& what)
{
	std::ostringstream text;
	text << what << " (is " << value << ", expected " << expected << " give or take " << tolerance << ")";
	check(std::fabs(value - expected) <= tolerance, text.str());
}

static Sanic::Player* start(const std::string& name, const char* level)
{
	current = name;
	std::istringstream tilestxt(TILES);
	std::istringstream leveltxt(level);
	if (!Sanic::_Game::Instance()->initWithoutWindow(tilestxt, leveltxt))
	{
		std::cout << "FAILED  " << name << ": the level could not be loaded" << '\n';
		std::exit(1);
	}
	return Sanic::_Game::Instance()->getPlayer();
}

// One update of the game with these keys down.
static void update(bool left, bool right, bool jump)
{
	Sanic::_InputManager::Instance()->Press(left, right, jump, false);
	Sanic::_Game::Instance()->update();
}

static void wait(int updates) { for (int i = 0; i < updates; i++) update(false, false, false); }
static void holdRight(int updates) { for (int i = 0; i < updates; i++) update(false, true, false); }
static void holdLeft(int updates) { for (int i = 0; i < updates; i++) update(true, false, false); }

// The middle of the player's body, sideways: his frame is 32 wide.
static float middle(Sanic::Player* player) { return player->getExactX() + 16; }

static const char* FLAT =
	"........................\n"
	"........................\n"
	"........................\n"
	"........................\n"
	"........................\n"
	"...P....................\n"
	"GGGGGGGGGGGGGGGGGGGGGGGG\n"
	"GGGGGGGGGGGGGGGGGGGGGGGG\n";

static void standsOnTheGround()
{
	Sanic::Player* player = start("stands on the ground", FLAT);
	wait(60);
	check(player->IsGrounded(), "he is on the ground");
	checkNear(player->getExactY(), STANDING, 0, "his feet are on the top of the ground");
	checkNear(player->getExactX(), 96, 0, "he has not moved sideways");
	checkNear(player->GetSpeed(), 0, 0, "he has no speed");
}

static void fallsAndLands()
{
	Sanic::Player* player = start("falls and lands",
		"........................\n"
		"...P....................\n"
		"........................\n"
		"........................\n"
		"........................\n"
		"........................\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n");
	float drop = STANDING - 32;
	int updates = 0;
	float fastest = 0;
	while (!player->IsGrounded() && updates < 600)
	{
		wait(1);
		updates++;
		if (player->GetFallSpeed() > fastest)
			fastest = player->GetFallSpeed();
	}
	check(player->IsGrounded(), "he lands");
	checkNear(player->getExactY(), STANDING, 0, "he lands on the top of the ground, not in it");
	// Falling from rest under a constant gravity: the drop is half the gravity times the square of the time.
	checkNear((float)updates, std::sqrt(2 * drop / Physics::GRAVITY), 3, "the fall takes the time gravity gives it");
	check(fastest <= Physics::MAX_FALL_SPEED, "he never falls faster than the limit");
	check(fastest > Physics::GRAVITY * (updates - 3), "his fall gains speed at every update");
}

static void runsUpToTopSpeed()
{
	Sanic::Player* player = start("runs up to top speed", FLAT);
	wait(5);
	holdRight(1);
	checkNear(player->GetSpeed(), Physics::ACCELERATION, 0.0001f, "one update of running adds the acceleration");
	holdRight(4);
	checkNear(player->GetSpeed(), 5 * Physics::ACCELERATION, 0.0001f, "five updates add five times as much");
	float before = player->getExactX();
	holdRight(1);
	checkNear(player->getExactX() - before, player->GetSpeed(), 0.0001f, "he moves by his speed at every update");
	holdRight(200);
	checkNear(player->GetSpeed(), Physics::TOP_SPEED, 0.0001f, "running stops gaining speed at the top speed");
	check(player->GetFacing() == 1, "he looks to the right");
}

static void slowsDownWhenNoDirectionIsHeld()
{
	Sanic::Player* player = start("slows down when no direction is held", FLAT);
	holdRight(100);
	wait(1);
	checkNear(player->GetSpeed(), Physics::TOP_SPEED - Physics::FRICTION, 0.0001f, "one update takes the friction off");
	wait((int)(Physics::TOP_SPEED / Physics::FRICTION) + 2);
	checkNear(player->GetSpeed(), 0, 0, "he comes to a stop");
	float stopped = player->getExactX();
	wait(30);
	checkNear(player->getExactX(), stopped, 0, "and stays where he stopped");
}

static void brakesAndTurns()
{
	Sanic::Player* player = start("brakes and turns", FLAT);
	holdRight(100);
	holdLeft(1);
	checkNear(player->GetSpeed(), Physics::TOP_SPEED - Physics::DECELERATION, 0.0001f, "holding the other way takes the deceleration off");
	check(player->IsChangingDirection(), "he is skidding");
	check(player->GetFacing() == 1, "and still looks the way he is going");
	holdLeft((int)(Physics::TOP_SPEED / Physics::DECELERATION) + 3);
	check(player->GetSpeed() < 0, "then he goes the other way");
	check(!player->IsChangingDirection(), "no longer skidding");
	check(player->GetFacing() == -1, "and looks to the left");
}

// Holds the jump key for a number of updates, lets go, and follows the jump until it is over.
// Returns how high the feet rose.
static float jump(Sanic::Player* player, int updatesHeld, int* takeOffs)
{
	float highest = player->getExactY();
	bool wasGrounded = player->IsGrounded();
	*takeOffs = 0;
	for (int i = 0; i < 400; i++)
	{
		update(false, false, i < updatesHeld);
		if (wasGrounded && !player->IsGrounded())
			(*takeOffs)++;
		wasGrounded = player->IsGrounded();
		if (player->getExactY() < highest)
			highest = player->getExactY();
	}
	return STANDING - highest;
}

static void jumpsAsHighAsTheKeyIsHeld()
{
	Sanic::Player* player = start("jumps as high as the key is held", FLAT);
	wait(5);
	int takeOffs = 0;

	float whole = jump(player, 400, &takeOffs);
	// Thrown up at JUMP_SPEED against GRAVITY: the square of the speed over twice the gravity, and up to one
	// update's worth more, as the first update of a jump has no gravity in it.
	float expected = Physics::JUMP_SPEED * Physics::JUMP_SPEED / (2 * Physics::GRAVITY);
	check(whole >= expected && whole <= expected + Physics::JUMP_SPEED, "a held key gives the whole jump");
	check(player->IsGrounded() && !player->IsJumping(), "he comes down again");
	checkNear(player->getExactY(), STANDING, 0, "on to the top of the ground");
	check(takeOffs == 1, "a key that stays down jumps once, not again on landing");

	wait(2);
	float hop = jump(player, 2, &takeOffs);
	check(takeOffs == 1, "a short press jumps too");
	check(hop > 4 && hop < whole / 2, "but less than half as high");

	float middling = jump(player, 12, &takeOffs);
	check(middling > hop && middling < whole, "and a longer press goes higher than a short one");
}

static void wallsStopHim()
{
	// The wall is 5 tiles high: more than a jump clears. (A wall he can get the upper half of his body over is
	// not a wall but a step: he lands on top of it.)
	Sanic::Player* player = start("walls stop him",
		"........................\n"
		"..............G.........\n"
		"..............G.........\n"
		"..............G.........\n"
		"..............G.........\n"
		"...P..........G.........\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n");
	holdRight(300);
	// The body is the 16 pixels in the middle of the frame: its right side is 24 pixels from the frame's left.
	checkNear(player->getExactX() + 24, 14 * 32, 0, "his right side is against the wall, not in it and not short of it");
	checkNear(player->GetSpeed(), 0, 0, "and he has no speed left");
	checkNear(player->getExactY(), STANDING, 0, "still standing on the ground");

	holdLeft(400);
	checkNear(player->getExactX() + 8, 0, 0, "the left side of the level stops him the same way");
	checkNear(player->GetSpeed(), 0, 0, "with no speed left");

	// In the air too: a jump against the wall does not go into it.
	holdRight(300);
	bool inside = false;
	for (int i = 0; i < 80; i++)
	{
		update(false, true, i < 40);
		if (player->getExactX() + 24 > 14 * 32)
			inside = true;
	}
	check(!inside, "jumping against the wall does not take him into it");
}

static void ceilingsStopAJump()
{
	Sanic::Player* player = start("ceilings stop a jump",
		"........................\n"
		"........................\n"
		"........................\n"
		"..GGGG..................\n"
		"........................\n"
		"...P....................\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n");
	wait(5);
	int takeOffs = 0;
	float rise = jump(player, 400, &takeOffs);
	// The block over his head has its underside at y = 128: one tile above the head of a player standing.
	checkNear(rise, 32, 0, "the jump stops with his head against the block above");
	check(player->IsGrounded(), "and he falls back to the ground");
	checkNear(player->getExactY(), STANDING, 0, "on to the top of it");
}

// Two gentle slopes up to a platform one tile higher, and two down again: the shape of the game's own level.
static const char* HILL =
	"................................\n"
	"................................\n"
	"................................\n"
	"................................\n"
	"................................\n"
	"...P........abGGGGcd............\n"
	"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n"
	"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n";

static void followsTheSlopes()
{
	Sanic::Player* player = start("follows the slopes", HILL);
	wait(5);

	int inTheAir = 0;
	float slowestGoingUp = 100;
	float onTop = 0;
	float fastestAfter = 0;
	bool wrongHeightOnTop = false;
	for (int i = 0; i < 400 && middle(player) < 28 * 32; i++)
	{
		holdRight(1);
		if (!player->IsGrounded())
			inTheAir++;
		float x = middle(player);
		// With the whole body over the way up (tiles 12 and 13), over the top (14 to 17), past the way down (20 on).
		if (x > 12 * 32 + 8 && x < 14 * 32 - 8 && player->GetSpeed() < slowestGoingUp)
			slowestGoingUp = player->GetSpeed();
		if (x > 14 * 32 + 8 && x < 18 * 32 - 8)
		{
			onTop = player->GetSpeed();
			if (player->getExactY() != STANDING - 32)
				wrongHeightOnTop = true;
		}
		if (x > 20 * 32 + 8 && player->GetSpeed() > fastestAfter)
			fastestAfter = player->GetSpeed();
	}

	check(middle(player) >= 28 * 32, "he gets over the hill");
	check(inTheAir == 0, "without his feet ever leaving the ground: up, over the top and down");
	check(!wrongHeightOnTop, "on the top he stands one tile higher");
	checkNear(player->getExactY(), STANDING, 0, "and past it he is back on the low ground");
	check(slowestGoingUp < Physics::TOP_SPEED - 0.01f, "the way up slows him below his top speed");
	checkNear(onTop, Physics::TOP_SPEED, 0.0001f, "on the flat top he is back at top speed");
	check(fastestAfter > Physics::TOP_SPEED + 0.5f, "the way down takes him well over his top speed");
}

static void standsOnAGentleSlope()
{
	Sanic::Player* player = start("stands on a gentle slope", HILL);
	wait(5);
	// On to the first slope, then hands off the keys.
	for (int i = 0; i < 400 && middle(player) < 12 * 32 + 16; i++)
		holdRight(1);
	wait(200);
	checkNear(player->GetSpeed(), 0, 0, "he comes to a stop");
	check(player->getExactY() < STANDING && player->getExactY() > STANDING - 32, "part of the way up the slope");
	float x = player->getExactX();
	float y = player->getExactY();
	wait(120);
	check(player->getExactX() == x && player->getExactY() == y, "and stays there: a gentle slope does not pull him down");
}

static void steepSlopes()
{
	const char* ramp =
		"................................\n"
		"................................\n"
		"................................\n"
		"................................\n"
		"................................\n"
		"...P........uGGGG...............\n"
		"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG\n";

	Sanic::Player* player = start("climbs a slope of 45 degrees", ramp);
	wait(5);
	int inTheAir = 0;
	for (int i = 0; i < 400 && middle(player) < 14 * 32 + 16; i++)
	{
		holdRight(1);
		if (!player->IsGrounded())
			inTheAir++;
	}
	check(middle(player) >= 14 * 32 + 16, "with a run he gets to the top of it");
	check(inTheAir == 0, "with his feet on it all the way");
	checkNear(player->getExactY(), STANDING - 32, 0, "and stands one tile higher");

	player = start("slides down a slope of 45 degrees", ramp);
	wait(5);
	// On to the foot of it, then hands off the keys: his run takes him part of the way up.
	for (int i = 0; i < 400 && middle(player) < 12 * 32 + 2; i++)
		holdRight(1);
	check(player->getExactY() < STANDING, "he is on the slope");
	wait(300);
	check(middle(player) < 12 * 32, "left alone he slides back down it: it pulls harder than the ground holds");
	checkNear(player->getExactY(), STANDING, 0, "to the low ground");
	checkNear(player->GetSpeed(), 0, 0, "where he stops");
}

static void runsOffALedge()
{
	Sanic::Player* player = start("runs off a ledge",
		"................\n"
		"................\n"
		"...P............\n"
		"GGGGGG..........\n"
		"GGGGGG..........\n"
		"GGGGGG..........\n"
		"GGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGG\n");
	wait(5);
	checkNear(player->getExactY(), 3 * 32 - 32, 0, "he starts on the ledge");
	int inTheAir = 0;
	bool jumping = false;
	float speedInTheAir = 0;
	for (int i = 0; i < 200; i++)
	{
		holdRight(1);
		if (!player->IsGrounded())
		{
			inTheAir++;
			speedInTheAir = player->GetSpeed();
		}
		if (player->IsJumping())
			jumping = true;
	}
	check(inTheAir > 10, "past the edge he falls");
	check(!jumping, "which is not a jump");
	checkNear(speedInTheAir, Physics::TOP_SPEED, 0.0001f, "keeping the speed he ran at");
	check(player->IsGrounded(), "and lands");
	checkNear(player->getExactY(), STANDING, 0, "on the ground below");
}

static void picksUpAndTheCameraStaysInTheLevel()
{
	Sanic::Player* player = start("picks up, and the camera stays in the level",
		"........................\n"
		"........................\n"
		"........................\n"
		"........................\n"
		"......o.................\n"
		"...P..ooo...............\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n"
		"GGGGGGGGGGGGGGGGGGGGGGGG\n");
	Sanic::Game* game = Sanic::_Game::Instance();
	int levelWidth = game->getLevelWidth();
	int levelHeight = game->getLevelHeight();
	check(levelWidth == 24 * 32 && levelHeight == 8 * 32, "the level is as big as its text");

	bool outside = false;
	for (int i = 0; i < 500; i++)
	{
		holdRight(1);
		if (game->camera.x < 0 || game->camera.x > levelWidth - game->getResWidth() ||
			game->camera.y < 0 || game->camera.y > levelHeight - game->getResHeight())
			outside = true;
	}
	check(game->getPlayerRings() == 3, "running through three things picks up three, and not the one above his head");
	check(!outside, "the camera never looks outside the level");
	checkNear(player->getExactX() + 24, (float)levelWidth, 0, "he ends against the right side of the level");
	check(game->camera.x == levelWidth - game->getResWidth(), "with the camera against it too");

	Sanic::_InputManager::Instance()->Press(false, false, false, true);
	game->update();
	check(game->getPlayerRings() == 0, "a hit takes what he picked up");
	check(!player->IsGrounded(), "and throws him off the ground");
	wait(200);
	check(player->IsGrounded(), "he lands again");
}

static void anEntityFalls()
{
	start("an entity falls", FLAT);
	// What every entity does when it has no rules of its own: here a 32 by 32 one, let go in the air.
	Sanic::Entity thing;
	thing.setX(200);
	thing.setY(40);
	for (int i = 0; i < 120; i++)
		thing.Update();
	check(thing.IsGrounded(), "it lands");
	check(thing.getY() == (int)STANDING, "on the top of the ground");
}

int main()
{
	standsOnTheGround();
	fallsAndLands();
	runsUpToTopSpeed();
	slowsDownWhenNoDirectionIsHeld();
	brakesAndTurns();
	jumpsAsHighAsTheKeyIsHeld();
	wallsStopHim();
	ceilingsStopAJump();
	followsTheSlopes();
	standsOnAGentleSlope();
	steepSlopes();
	runsOffALedge();
	picksUpAndTheCameraStaysInTheLevel();
	anEntityFalls();

	std::cout << checks << " checks, " << failures << " failed" << '\n';
	return failures == 0 ? 0 : 1;
}
