// Tests of the movement. They need no window, no pictures and no keyboard: each one starts the game's rules on a
// small level written here as text, holds keys for a number of updates as a player would, and checks where the
// player is and how fast he goes. Everything in the movement is a whole number, so the same keys on the same
// level give exactly the same numbers every time, on every machine.
//
// The numbers expected are those the rules give: speeds in 1/256 of a pixel per update ("sub"), positions in
// pixels or in sub. Where the movement reference states a figure (60 updates and 129 pixels from rest to top
// speed, a jump of 46 or 95 pixels, a roll of 294 updates), the test says so. Changing a constant in
// src/PhysicsManager.h changes the game on purpose, and the numbers here with it.
//
//   cmake --build build
//   ctest --test-dir build --output-on-failure

#include "Game.h"
#include "InputManager.h"
#include "PhysicsManager.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

typedef Sanic::PhysicsManager Rules;
typedef PlayerState State;

static int checks = 0;
static int failures = 0;
static std::string current;

// The tiles the levels below are made of: solid blocks, the slopes of the three classes in both directions,
// and a one-way platform.
static const char* TILES =
	"G ground     solid\n"
	"S soil       solid\n"
	"a rise_low   slope 31 16 1:2\n"
	"b rise_high  slope 15 0  1:2\n"
	"c fall_high  slope 0 15  1:2\n"
	"d fall_low   slope 16 31 1:2\n"
	"r rise_steep slope 31 0  1:1\n"
	"f fall_steep slope 0 31  1:1\n"
	"1 rise_q1    slope 31 24 1:4\n"
	"2 rise_q2    slope 23 16 1:4\n"
	"3 rise_q3    slope 15 8  1:4\n"
	"4 rise_q4    slope 7 0   1:4\n"
	"5 fall_q1    slope 0 7   1:4\n"
	"6 fall_q2    slope 8 15  1:4\n"
	"7 fall_q3    slope 16 23 1:4\n"
	"8 fall_q4    slope 24 31 1:4\n"
	"= plank      oneway\n";

static Sanic::Game* game = nullptr;
static Sanic::Player* player = nullptr;

static void check(bool ok, const std::string& what)
{
	checks++;
	if (!ok)
	{
		failures++;
		std::cout << "FAILED  " << current << ": " << what << '\n';
	}
}

static void equal(long value, long expected, const std::string& what)
{
	std::ostringstream text;
	text << what << " (is " << value << ", expected " << expected << ")";
	check(value == expected, text.str());
}

static void state(State expected, const std::string& what)
{
	check(player->getState() == expected, what);
}

typedef std::vector<std::string> Rows;

static std::string dots(int count) { return std::string(count, '.'); }

static void start(const std::string& name, const Rows& rows)
{
	current = name;
	std::string text;
	for (const std::string& row : rows)
		text += row + "\n";
	game = Sanic::_Game::Instance();
	if (!game->initWithoutWindow(TILES, text))
	{
		std::cout << "FAILED  " << name << ": the level could not be loaded" << '\n';
		std::exit(1);
	}
	player = game->getPlayer();
}

// One update of the game with these keys down: "", or some of left, right, up, down, jump and hurt joined by '+'.
static void update(const std::string& keys = "")
{
	auto has = [&keys](const char* key) { return keys.find(key) != std::string::npos; };
	Sanic::_InputManager::Instance()->Press(has("left"), has("right"), has("up"), has("down"), has("jump"), has("hurt"));
	game->update();
}

static void hold(const std::string& keys, int updates) { for (int i = 0; i < updates; i++) update(keys); }
static void wait(int updates) { hold("", updates); }

// Flat ground with its top at y = 192, and the player at tile (start, 5): standing with the middle of his feet
// at x = start * 32 + 16.
static Rows flat(int width, int startTile = 3)
{
	Rows rows(5, dots(width));
	rows.push_back(dots(startTile) + "P" + dots(width - startTile - 1));
	rows.push_back(std::string(width, 'G'));
	rows.push_back(std::string(width, 'G'));
	return rows;
}

static void standsWhereTheLevelPutsHim()
{
	start("stands where the level puts him", flat(32));
	wait(10);
	state(State::Standing, "he stands");
	equal(player->getFeetX(), 112, "the middle of his feet is the middle of his tile");
	equal(player->getFeetY(), 192, "and on the top row of the ground");
	equal(player->getPosX(), 96, "his frame is 16 to the left of that");
	equal(player->getPosY(), 160, "and 32 above");
	equal(player->getSpeedX(), 0, "he has no speed");
	equal(game->camera.x, 0, "the camera is at the left of the level");
	equal(game->camera.y, 64, "and at its bottom");
	equal(game->getCameraLine(), 128, "its line has come down from 120 to 128");
}

static void walksUpToTopSpeed()
{
	start("walks up to top speed", flat(64));
	wait(10);
	// Standing, a direction does nothing for one update: he starts walking. Then 32 an update (16, doubled
	// under half a pixel an update), then 16.
	const int expected[7] = { 0, 32, 64, 96, 128, 144, 160 };
	for (int i = 0; i < 7; i++)
	{
		update("right");
		equal(player->getSpeedX(), expected[i], "his speed in the first updates of walking");
	}
	state(State::Walking, "he is walking");

	start("walks up to top speed", flat(64));
	wait(10);
	update("right");
	int from = player->getSubX();
	int updates = 0;
	while (player->getSpeedX() < Rules::TOP_SPEED && updates < 1000)
	{
		update("right");
		updates++;
	}
	// The reference: 60 updates and 129 pixels from rest to top speed.
	equal(updates, 60, "updates from rest to top speed");
	equal(player->getSubX() - from, 33024, "and the way gone in them, in sub (129 pixels)");
	hold("right", 20);
	equal(player->getSpeedX(), 1024, "he stays at top speed");

	// The reference: 48 updates and 100 pixels to stop when the key is let go.
	from = player->getSubX();
	update();
	equal(player->getSpeedX(), 1004, "letting go takes the friction off");
	updates = 1;
	while (player->getState() != State::Standing && updates < 1000)
	{
		update();
		updates++;
	}
	equal(updates, 48, "updates to come to a stop");
	equal(player->getSubX() - from, 25512, "and the way gone in them, in sub (99.7 pixels)");
	equal(player->getSpeedX(), 0, "he has stopped dead");
	from = player->getSubX();
	wait(30);
	equal(player->getSubX() - from, 0, "and stays where he stopped");
}

static void skids()
{
	start("skids", flat(96));
	wait(10);
	hold("right", 100);
	int from = player->getSubX();
	update("left");
	equal(player->getSpeedX(), 1008, "the first update against the movement is still a walking one");
	state(State::Skidding, "then he skids");
	equal(player->GetFacing(), 1, "looking the way he is going");
	update("left");
	equal(player->getSpeedX(), 976, "braking twice as hard");
	int updates = 2;
	while (player->getSpeedX() >= 0 && updates < 1000)
	{
		update("left");
		updates++;
	}
	// The reference: 31 updates from top speed to turning round.
	equal(updates, 31, "updates from top speed to turning round");
	equal(player->getSubX() - from, 16272, "and the way gone in them, in sub (63.6 pixels)");
	equal(player->getSpeedX(), -16, "he is going the other way");
	state(State::Walking, "and walking again");
	update("left");
	equal(player->GetFacing(), -1, "looking the new way");
	equal(player->getSpeedX(), -48, "and gaining speed");

	start("skids: letting go of the brake", flat(96));
	wait(10);
	hold("right", 100);
	hold("left", 5);
	equal(player->getSpeedX(), 880, "five updates into a skid");
	state(State::Skidding, "skidding");
	update();
	equal(player->getSpeedX(), 860, "with the key let go only the friction slows him");
	state(State::Walking, "and he walks on");

	start("skids: not when slow", flat(96));
	wait(10);
	hold("right", 12);
	equal(player->getSpeedX(), 240, "under one pixel an update");
	update("left");
	equal(player->getSpeedX(), 224, "turning back is the plain walking push");
	state(State::Walking, "and no skid");
}

// A jump with the key held for a number of updates, followed until it is over.
struct Jump
{
	int rise = 0;      // how high the feet went, in sub
	int top = 0;       // the update at which they were highest
	int landed = 0;    // the update at which he was on the floor again
	int takeOffs = 0;
};

static Jump jump(int updatesHeld, const std::string& otherKeys = "")
{
	Jump result;
	int from = player->getSubY();
	int highest = from;
	bool wasOnFloor = player->IsGrounded();
	for (int i = 0; i < 200; i++)
	{
		update(i < updatesHeld ? otherKeys + "+jump" : otherKeys);
		if (wasOnFloor && !player->IsGrounded() && player->getState() == State::Jumping)
			result.takeOffs++;
		wasOnFloor = player->IsGrounded();
		if (player->getSubY() < highest)
		{
			highest = player->getSubY();
			result.top = i;
		}
		if (player->IsGrounded() && i > 0 && result.landed == 0)
			result.landed = i;
	}
	result.rise = from - highest;
	return result;
}

static void jumps()
{
	start("jumps: the key going down", flat(32));
	wait(10);
	update("jump");
	state(State::Jumping, "he is in a jump");
	equal(player->getSpeedY(), -1088, "going up at the jump speed");
	equal(player->getFeetY(), 192, "from the next update on");

	// The reference: a tap rises 46.1 pixels with its top at update 22 and lands after 45; a held key rises
	// 94.8 with its top at 34 and lands after 66. Holding lifts him for 13 updates and no longer.
	const int held[5] = { 1, 2, 3, 14, 200 };
	const int rise[5] = { 11792, 11792, 12832, 24272, 24272 };
	const int top[5] = { 22, 22, 23, 34, 34 };
	const int landed[5] = { 45, 45, 47, 66, 66 };
	for (int i = 0; i < 5; i++)
	{
		std::ostringstream name;
		name << "jumps: key held for " << held[i] << " updates";
		start(name.str(), flat(32));
		wait(10);
		Jump result = jump(held[i]);
		equal(result.rise, rise[i], "how high, in sub");
		equal(result.top, top[i], "the update of the top");
		equal(result.landed, landed[i], "the update of landing");
		equal(result.takeOffs, 1, "one jump, also when the key stays down after landing");
		state(State::Standing, "he ends standing");
		equal(player->getFeetY(), 192, "on the ground");
	}

	start("jumps: while running", flat(64));
	wait(10);
	hold("right", 100);
	update("right+jump");
	equal(player->getSpeedX(), 1024, "a jump leaves his speed sideways alone");
	int updates = 0;
	while (!player->IsGrounded() && updates < 1000)
	{
		update("right+jump");
		updates++;
	}
	equal(updates, 66, "updates in the air");
	equal(player->getSpeedX(), 1024, "the direction held keeps his speed");
	state(State::Walking, "he lands walking");

	start("jumps: with no direction held", flat(64));
	wait(10);
	hold("right", 100);
	update("jump");
	std::vector<int> speeds;
	while (!player->IsGrounded() && speeds.size() < 1000)
	{
		update("jump");
		speeds.push_back(player->getSpeedX());
	}
	equal(speeds[0], 988, "the air takes 16 off every update");
	equal(speeds[1], 972, "and again");
	equal(speeds.back(), 0, "until none is left: it stops at nothing, it does not turn him back");
	equal((long)speeds.size(), 66, "updates in the air");
	update();
	state(State::Standing, "he lands and stands");

	start("jumps: turning back in the air", flat(64));
	wait(10);
	hold("right", 100);
	update("jump");
	const int braking[20] = { 940, 876, 812, 748, 684, 620, 556, 492, 428, 364, 300, 236, 172, 108, -20, -52, -84, -116, -148, -164 };
	for (int i = 0; i < 20; i++)
	{
		update("left+jump");
		equal(player->getSpeedX(), braking[i], "64 off an update against the movement, twice that at a crawl, then 16 the new way");
	}
}

// A ledge 8 tiles long with its top at y = 96, three tiles above the ground.
static Rows ledge()
{
	Rows rows(2, dots(32));
	rows.push_back("...P" + dots(28));
	for (int i = 0; i < 3; i++)
		rows.push_back("GGGGGGGG" + dots(24));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	return rows;
}

static void walksOffALedge()
{
	start("walks off a ledge", ledge());
	wait(10);
	equal(player->getFeetY(), 96, "he starts on the ledge");
	int left = -1;
	int landed = -1;
	for (int i = 0; i < 120; i++)
	{
		update("right");
		if (player->getState() == State::Falling && left < 0)
		{
			left = i;
			// Only the middle of his feet holds him up: the ledge ends at x = 256.
			equal(player->getFeetX(), 257, "he falls as soon as his middle is past the edge");
			equal(player->getFeetY(), 103, "already 7 pixels down, driven there by the update that found no floor");
			equal(player->getSpeedY(), 256, "and starts down at a pixel an update");
		}
		else if (left >= 0 && i == left + 1)
			equal(player->getSpeedY(), 304, "then gravity");
		if (left >= 0 && landed < 0 && player->IsGrounded())
			landed = i;
	}
	equal(left, 64, "the update he left the ledge");
	equal(landed, 90, "the update he landed");
	equal(player->getFeetY(), 192, "on the ground below");
	equal(player->getSpeedX(), 1024, "still at the speed he walked at");
	state(State::Walking, "walking");
}

// Level ground, then a way down of four tiles at 45 degrees, then level ground one tile... lower each tile.
static Rows wayDown()
{
	Rows rows(12, dots(96));
	for (int c = 0; c < 96; c++)
	{
		int top = c < 12 ? 4 : (c >= 16 ? 8 : 4 + (c - 12) + 1);
		if (c >= 12 && c < 16)
			rows[4 + (c - 12)][c] = 'f';
		for (int r = top; r < 12; r++)
			rows[r][c] = 'G';
	}
	rows[3][3] = 'P';
	return rows;
}

// Flat ground with one tile of 45 degrees going up on it, and nothing after its top.
static Rows ramp()
{
	Rows rows(4, dots(64));
	rows.push_back("...P" + dots(16) + "r" + dots(43));
	for (int i = 0; i < 3; i++)
		rows.push_back(std::string(64, 'G'));
	return rows;
}

static void rolls()
{
	start("rolls", flat(96));
	wait(10);
	hold("right", 12);
	update("down");
	state(State::Walking, "under one pixel an update, down does nothing");
	hold("right", 5);
	equal(player->getSpeedX(), 300, "fast enough now");
	update("right+down");
	state(State::Rolling, "down starts a roll");
	equal(player->getSpeedX(), 316, "at the speed he had");
	equal(player->getTopSpeed(), 1536, "a roll may go faster than a walk");
	update("down");
	equal(player->getSpeedX(), 311, "with no direction a roll loses 5 an update");
	update("right");
	equal(player->getSpeedX(), 307, "the direction of the roll does not push it: it loses 4");
	update("left");
	equal(player->getSpeedX(), 291, "the other direction takes 16 off");
	state(State::Skidding, "and turns the roll into a skid");
	update("left");
	equal(player->getSpeedX(), 259, "which brakes as a skid does");
	equal(player->GetFacing(), 1, "looking the way he is going");

	start("rolls: to a stop", flat(96));
	wait(10);
	hold("right", 100);
	hold("right+down", 2);
	int from = player->getSubX();
	int updates = 0;
	while (player->getState() == State::Rolling && updates < 1000)
	{
		update("down");
		updates++;
	}
	equal(updates, 190, "updates a roll from walking speed lasts on the flat");
	equal(player->getSubX() - from, 102745, "and the way gone in them, in sub (401 pixels)");
	state(State::Crouching, "with down still held he ends crouching");
	update();
	state(State::Standing, "and stands when it is let go");

	start("rolls: down a slope", wayDown());
	wait(10);
	while (player->getFeetX() < 300)
		update("right");
	while (player->getState() != State::Rolling)
		update("right+down");
	int fastest = 0;
	int inTheAir = 0;
	std::set<int> gains;
	while (player->getFeetX() < 16 * 32)
	{
		int push = player->getSlopePush();
		int before = player->getSpeedX();
		update();
		if (push == 31 && player->getSpeedX() < 1536)
			gains.insert(player->getSpeedX() - before);
		if (player->getSpeedX() > fastest)
			fastest = player->getSpeedX();
		if (player->IsInTheAir())
			inTheAir++;
	}
	check(gains.size() == 1 && *gains.begin() == 26, "on 45 degrees a roll gains 26 an update: the slope's 31 less its own 5");
	equal(fastest, 1536, "up to its top speed of 6 pixels an update");
	equal(inTheAir, 0, "with his feet on the slope all the way down");
	// The reference: from 6 pixels an update a roll lasts 294 updates and 915 pixels on the flat.
	equal(player->getSpeedX(), 1536, "he reaches the flat at top speed");
	from = player->getSubX();
	updates = 0;
	while (player->getState() == State::Rolling && updates < 1000)
	{
		update();
		updates++;
	}
	equal(updates, 294, "updates the roll lasts from there");
	equal(player->getSubX() - from, 234369, "and the way gone in them, in sub (915.5 pixels)");
	state(State::Standing, "he ends standing");
	equal(player->getTopSpeed(), 1024, "with the top speed of a walk again");

	start("rolls: off a ledge", ledge());
	wait(10);
	while (player->getSpeedX() < 600)
		update("right");
	bool left = false;
	for (int i = 0; i < 120 && !(left && player->IsGrounded()); i++)
	{
		update("down");
		if (player->getState() == State::Jumping && !left)
		{
			left = true;
			equal(player->getFeetX(), 257, "he leaves as soon as his middle is past the edge");
			equal(player->getFeetY(), 103, "7 pixels down already");
			equal(player->getSpeedY(), 0, "as a ball, with no speed downwards yet");
			equal(player->getSpeedX(), 308, "and the speed he rolled at");
		}
	}
	check(left, "he rolls off");
	state(State::Walking, "and lands walking");
	equal(player->getTopSpeed(), 1024, "with the top speed of a walk");

	start("rolls: off the end of a ramp", ramp());
	wait(10);
	while (player->getFeetX() < 580)
		update("right");
	while (player->getState() != State::Rolling)
		update("right+down");
	int before = 0;
	for (int i = 0; i < 200 && player->getState() != State::RampJumping; i++)
	{
		before = player->getSpeedX();
		update();
	}
	state(State::RampJumping, "rolling up a ramp and off its end throws him up");
	equal(before, 589, "his speed on the ramp's last pixels");
	equal(player->getSpeedX(), 553, "his speed sideways as he leaves");
	equal(player->getSpeedY(), -829, "and one and a half times that, upwards");
	equal(player->getFeetX(), 674, "past the ramp's end at x = 672");
	update();
	equal(player->getSpeedY(), -793, "a lighter gravity brings him down: 36 an update");
	int highest = player->getSubY();
	while (!player->IsGrounded())
	{
		update();
		if (player->getSubY() < highest)
			highest = player->getSubY();
	}
	equal(Rules::ToPixel(highest), 99, "the height of his feet at the top: 61 pixels above the ground he left");
	equal(player->getFeetX(), 709, "where he comes down");
	equal(player->getFeetY(), 160, "on the ground");

	start("walks over the same ramp", ramp());
	wait(10);
	std::vector<State> states;
	for (int i = 0; i < 200; i++)
	{
		update("right");
		if (states.empty() || states.back() != player->getState())
			states.push_back(player->getState());
	}
	check(states.size() == 3 && states[0] == State::Walking && states[1] == State::Falling && states[2] == State::Walking,
		"walking off the end of it is only a fall");
	equal(player->getFeetX(), 792, "where he is after 200 updates");
	equal(player->getFeetY(), 160, "on the ground");
}

// A hill with all three classes of slope: four tiles of 1:4, a step, two of 1:2, a step, one of 1:1, the top,
// and the same down the other side.
static Rows hill()
{
	Rows rows(4, dots(64));
	rows.push_back(dots(28) + "rGGGGf" + dots(30));
	rows.push_back(dots(22) + "abGGGG" + "SSSSSS" + "cd" + dots(28));
	rows.push_back("...P" + dots(12) + "1234" + "GG" + std::string(16, 'S') + "5678" + dots(22));
	rows.push_back(std::string(64, 'G'));
	rows.push_back(std::string(64, 'G'));
	return rows;
}

static void slopes()
{
	start("slopes: over the hill", hill());
	wait(10);
	int inTheAir = 0;
	int updates = 0;
	int slowest14 = 9999, slowest12 = 9999, slowest11 = 9999;
	bool wrongHeight = false;
	while (player->getFeetX() < 1500 && updates < 2000)
	{
		update("right");
		updates++;
		if (player->IsInTheAir())
			inTheAir++;
		int x = player->getFeetX();
		int y = player->getFeetY();
		// The five level stretches: the two steps going up, the top, the step going down, the ground after.
		if ((x >= 640 && x <= 703 && y != 192) || (x >= 768 && x <= 895 && y != 160) || (x >= 928 && x <= 1055 && y != 128) ||
			(x >= 1152 && x <= 1215 && y != 192) || (x >= 1344 && y != 224))
			wrongHeight = true;
		int push = player->getSlopePush();
		if (push == -16 && player->getSpeedX() < slowest14) slowest14 = player->getSpeedX();
		if (push == -24 && player->getSpeedX() < slowest12) slowest12 = player->getSpeedX();
		if (push == -31 && player->getSpeedX() < slowest11) slowest11 = player->getSpeedX();
	}
	equal(updates, 379, "updates to walk over it");
	equal(inTheAir, 0, "his feet never leave the ground: up, over the top and down");
	check(!wrongHeight, "on every level stretch his feet are on its top row");
	equal(slowest14, 1024, "a slope of 1:4 pushes as hard as he walks: it does not slow him at top speed");
	equal(slowest12, 896, "a slope of 1:2 does");
	equal(slowest11, 919, "and so does one of 1:1, shorter and steeper");
	equal(player->getSpeedX(), 1024, "on the flat he is back at top speed");

	start("slopes: down with no keys", hill());
	wait(10);
	while (player->getFeetX() < 30 * 32)
		update("right");
	std::set<int> on11, on12;
	updates = 0;
	while (player->getState() != State::Standing && updates < 600)
	{
		int push = player->getSlopePush();
		int before = player->getSpeedX();
		update();
		updates++;
		if (push == 31) on11.insert(player->getSpeedX() - before);
		if (push == 24) on12.insert(player->getSpeedX() - before);
	}
	check(on11.size() == 1 && *on11.begin() == 11, "down 1:1 he gains 11 an update: the slope's 31 less the friction's 20");
	check(on12.size() == 1 && *on12.begin() == 4, "down 1:2 he gains 4: 24 less 20");
	equal(updates, 117, "updates until he has stopped");
	equal(player->getFeetX(), 1188, "on the level step after the 1:2");
	equal(player->getFeetY(), 192, "which is at y = 192");

	start("slopes: standing on one", hill());
	wait(10);
	while (player->getFeetX() < 23 * 32)
		update("right");
	wait(100);
	state(State::Standing, "left alone going up 1:2 he comes to a stop");
	equal(player->getFeetX(), 789, "having got as far as the step above it");
	wait(100);
	equal(player->getFeetX(), 789, "and stays: a slope does not push a player who stands");
	equal(player->getSpeedX(), 0, "with no speed");

	// Four tiles of 45 degrees, one above the other.
	Rows steep(2, dots(32));
	steep.push_back(dots(12) + "rG" + dots(18));
	steep.push_back(dots(11) + "rSG" + dots(18));
	steep.push_back(dots(10) + "rSSG" + dots(18));
	steep.push_back("...P" + dots(5) + "rSSSG" + dots(18));
	steep.push_back(std::string(32, 'G'));
	steep.push_back(std::string(32, 'G'));
	start("slopes: up 45 degrees from rest", steep);
	wait(10);
	while (player->getFeetX() < 9 * 32 - 2)
		update("right");
	wait(60);
	hold("right", 2);
	int fastest = 0;
	updates = 0;
	while (player->getFeetY() > 192 - 4 * 32 && updates < 2000)
	{
		update("right");
		updates++;
		if (player->getSlopePush() != 0 && player->getSpeedX() > fastest)
			fastest = player->getSpeedX();
	}
	// The reference: walking up 45 degrees from rest cannot exceed about half a pixel an update.
	equal(fastest, 128, "he cannot get above half a pixel an update");
	equal(updates, 247, "updates to climb the four tiles");
	equal(player->getFeetY(), 64, "but he gets to the top");
}

static void walls()
{
	Rows rows(1, dots(32));
	for (int i = 0; i < 4; i++)
		rows.push_back(dots(14) + "G" + dots(17));
	rows.push_back("...P" + dots(10) + "G" + dots(17));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	start("walls: on his right", rows);
	wait(10);
	// The wall's face is at x = 448. His wall point is 10 to the right of his middle and must stay outside it.
	int updates = 0;
	int before = 0;
	do
	{
		before = player->getSpeedX();
		update("right");
		updates++;
	} while (!(before == 1024 && player->getSpeedX() == 0) && updates < 1000);
	equal(updates, 112, "updates until the wall has stopped him");
	equal(player->getFeetX(), 437, "his middle is 11 pixels from the wall");
	bool moved = false;
	for (int i = 0; i < 300; i++)
	{
		update("right");
		if (player->getFeetX() != 437)
			moved = true;
	}
	check(!moved, "pushing on against it does not move him a pixel");
	bool inside = false;
	for (int i = 0; i < 100; i++)
	{
		update(i < 30 ? "right+jump" : "right");
		if (player->getFeetX() + Rules::WALL_PROBE_X >= 448)
			inside = true;
	}
	check(!inside, "nor does jumping against it");

	rows = Rows(1, dots(32));
	for (int i = 0; i < 4; i++)
		rows.push_back(dots(4) + "G" + dots(27));
	rows.push_back(dots(4) + "G" + dots(9) + "P" + dots(17));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	start("walls: on his left", rows);
	wait(10);
	hold("left", 300);
	// The wall's face is at x = 160. On this side he rests a pixel nearer, as the reference has it.
	equal(player->getFeetX(), 170, "his middle is 10 pixels from the wall");
}

static void theSidesOfThePicture()
{
	start("the sides of the picture", flat(32));
	wait(10);
	hold("left", 200);
	equal(player->getFeetX(), 16, "at the left end of the level his middle stops 16 pixels in");
	equal(player->getPosX(), 0, "so that his frame is just inside");
	equal(game->camera.x, 0, "with the camera at the level's end");
	bool outside = false;
	for (int i = 0; i < 50; i++)
	{
		update("left");
		if (player->getFeetX() < 15 || player->getFeetX() > 16)
			outside = true;
	}
	check(!outside, "pushing on, he is put back whenever he is a pixel past it");
	hold("right", 600);
	equal(game->getLevelWidth(), 1024, "the level is 1024 wide");
	equal(game->camera.x, 765, "the camera stops short of the right end when a whole step would take it past");
	equal(player->getFeetX(), 1012, "and he stops 247 pixels into the picture");
}

static void ceilings()
{
	Rows rows(3, dots(32));
	rows.push_back("..GGGG" + dots(26));
	rows.push_back(dots(32));
	rows.push_back("...P" + dots(28));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	// The block over him has its underside at y = 128. His head point is 24 above his feet.
	const char* keys[2] = { "jump", "" };
	for (int k = 0; k < 2; k++)
	{
		start(k == 0 ? "ceilings: a held jump" : "ceilings: a tap", rows);
		wait(10);
		update("jump");
		int highest = player->getSubY();
		for (int i = 0; i < 120; i++)
		{
			update(keys[k]);
			if (player->getSubY() < highest)
				highest = player->getSubY();
		}
		equal(Rules::ToPixel(highest), 152, "the jump stops with his head point at the underside");
		state(State::Standing, "and he comes down again");
		equal(player->getFeetY(), 192, "to the ground");
	}

	start("ceilings: the bump", rows);
	wait(10);
	update("jump");
	bool bumped = false;
	for (int i = 0; i < 30 && !bumped; i++)
	{
		int before = player->getSpeedY();
		update();
		if (before < 0 && player->getSpeedY() == 256)
			bumped = true;
	}
	check(bumped, "meeting the ceiling he comes away from it at a pixel an update downwards");
	equal(player->getFeetY(), 152, "from right under it");
}

static void oneWayPlatforms()
{
	Rows rows(3, dots(32));
	rows.push_back(dots(2) + "====" + dots(26));
	rows.push_back(dots(32));
	rows.push_back("...P" + dots(28));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	// The platform's top is at y = 96, three tiles above the ground: a little more than a jump rises.
	start("one-way platforms: from below", rows);
	wait(10);
	hold("right", 60);
	equal(player->getFeetX(), 237, "he walks under it");
	equal(player->getFeetY(), 192, "without being lifted on to it");

	start("one-way platforms: jumping up through", rows);
	wait(10);
	int highest = player->getSubY();
	for (int i = 0; i < 200; i++)
	{
		update(i < 30 ? "jump" : "");
		if (player->getSubY() < highest)
			highest = player->getSubY();
	}
	state(State::Standing, "a full jump from under it ends standing");
	equal(player->getFeetY(), 96, "on the platform");
	equal(Rules::ToPixel(highest), 96, "which caught him at the top of the jump, a pixel into it");
	std::vector<State> states;
	for (int i = 0; i < 200; i++)
	{
		update("right");
		if (states.empty() || states.back() != player->getState())
			states.push_back(player->getState());
	}
	check(states.size() == 3 && states[0] == State::Walking && states[1] == State::Falling && states[2] == State::Walking,
		"walking off its end he falls to the ground");
	equal(player->getFeetX(), 797, "where he is after 200 updates");
	equal(player->getFeetY(), 192, "on the ground");
}

static void hurt()
{
	Rows rows(5, dots(32));
	rows.push_back(dots(6) + "ooo" + dots(3) + "P" + dots(2) + "ooo" + dots(14));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	start("hurt", rows);
	wait(10);
	hold("right", 40);
	wait(60);
	equal(game->getPlayerRings(), 1, "he has picked one thing up");
	int fromX = player->getSubX();
	int fromY = player->getSubY();
	update("hurt");
	state(State::Hurt, "a hit throws him");
	equal(player->getSpeedX(), -256, "back from the way he looks, at a pixel an update");
	equal(player->getSpeedY(), -976, "and up at four, less the first update of gravity");
	equal(game->getPlayerRings(), 0, "he loses what he had picked up");
	check(player->IsImmortal(), "and is out of harm");
	int updates = 1;
	int highest = player->getSubY();
	while (!player->IsGrounded() && updates < 1000)
	{
		update("right");
		updates++;
		if (player->getSubY() < highest)
			highest = player->getSubY();
	}
	// The reference: about 42 updates, 42 pixels back and 41 high.
	equal(updates, 42, "updates until he lands");
	equal(player->getSubX() - fromX, -10752, "the way back, in sub (42 pixels): the key held made no difference");
	equal(fromY - highest, 10416, "how high, in sub (40.7 pixels)");
	state(State::Walking, "he lands walking");
	equal(player->getSpeedX(), -256, "still at the speed he was thrown at");
	update("hurt");
	check(player->getState() != State::Hurt, "a second hit does nothing yet");
	updates = 0;
	while (player->IsImmortal() && updates < 1000)
	{
		update();
		updates++;
	}
	equal(updates, 77, "updates left of the 120 he is out of harm for");
	update("hurt");
	state(State::Hurt, "after them a hit throws him again");

	start("hurt: looking left", rows);
	wait(10);
	hold("left", 2);
	wait(30);
	update("hurt");
	equal(player->GetFacing(), -1, "he looks left");
	equal(player->getSpeedX(), 256, "and is thrown to the right");
	while (!player->IsGrounded())
		update();
	equal(game->getPlayerRings(), 0, "thrown through three things, he picks none of them up");
	equal(player->getFeetX(), 441, "where he lands");
}

static void picksUp()
{
	Rows rows(4, dots(32));
	rows.push_back(dots(6) + "o" + dots(25));
	rows.push_back("...P" + dots(2) + "o.o" + dots(23));
	rows.push_back(std::string(32, 'G'));
	rows.push_back(std::string(32, 'G'));
	start("picks up", rows);
	wait(10);
	while (game->getPlayerRings() == 0 && player->getFeetX() < 400)
		update("right");
	// What is picked up is 16 pixels wide in the middle of its tile: this one starts at x = 200. His box is 4
	// pixels to each side of his middle.
	equal(player->getFeetX(), 197, "he picks it up when his box, not his picture, reaches it");
	hold("right", 60);
	equal(game->getPlayerRings(), 2, "he has the two at the height of his body, and not the one over his head");
}

static void camera()
{
	Rows rows(13, dots(64));
	rows.push_back("...P" + dots(60));
	for (int i = 0; i < 6; i++)
		rows.push_back(std::string(64, 'G'));
	start("camera", rows);
	equal(game->camera.x, 0, "it starts at the left end of the level");
	equal(game->camera.y, 328, "with his feet 120 pixels down the picture");
	equal(game->getCameraLine(), 120, "which is where its line starts");
	wait(10);
	equal(game->getCameraLine(), 128, "the line comes down to 128, a pixel an update");
	equal(game->camera.y, 328, "without moving the picture: his feet are still near enough to it");

	std::set<int> steps;
	int nearest = 9999, furthest = -9999;
	int last = game->camera.x;
	for (int i = 0; i < 200; i++)
	{
		update("right");
		steps.insert(game->camera.x - last);
		last = game->camera.x;
		int screenX = player->getFeetX() - game->camera.x;
		if (screenX < nearest) nearest = screenX;
		if (screenX > furthest) furthest = screenX;
	}
	check(*steps.begin() == 0 && *steps.rbegin() == 4, "it moves as fast as he walks and no faster");
	equal(nearest, 112, "the furthest left he is in the picture");
	equal(furthest, 140, "the furthest right: 8 past the middle, and the 4 he walks before it follows");
	equal(game->camera.x, 657, "where it is after 200 updates");
	wait(100);
	equal(game->camera.x, 760, "it settles with him 8 pixels right of the middle");
	equal(game->camera.y, 328, "and has not moved up or down");

	int firstMove = -1;
	for (int i = 0; i < 120; i++)
	{
		update("up");
		if (firstMove < 0 && game->camera.y != 328)
			firstMove = i;
	}
	state(State::LookingUp, "up makes him look up");
	equal(game->getCameraLine(), 184, "the line goes down the picture to 184");
	equal(firstMove, 9, "the picture starts to move once the line has gone far enough");
	equal(game->camera.y, 280, "and shows 48 pixels more of what is above");
	wait(120);
	equal(game->getCameraLine(), 128, "letting go brings the line back");
	equal(game->camera.y, 304, "and the picture only as far as it has to");
	hold("down", 200);
	state(State::Crouching, "down makes him crouch");
	equal(game->getCameraLine(), 48, "the line goes up the picture to 48");
	equal(game->camera.y, 384, "and the picture shows what is below");

	start("camera: a jump", rows);
	wait(10);
	int highestView = game->camera.y;
	bool tooFast = false;
	last = game->camera.y;
	for (int i = 0; i < 100; i++)
	{
		update(i < 30 ? "jump" : "");
		int step = game->camera.y - last;
		last = game->camera.y;
		if (step > 8 || step < -8)
			tooFast = true;
		if (game->camera.y < highestView)
			highestView = game->camera.y;
	}
	equal(highestView, 241, "it follows a jump up");
	check(!tooFast, "never by more than 8 pixels an update");
	equal(game->camera.y, 304, "and back down as far as it has to");
}

static void theClock()
{
	// One second on the level's clock is 60 updates, at 50 updates a second as at 60.
	const int rates[2] = { 50, 60 };
	for (int i = 0; i < 2; i++)
	{
		Sanic::_Game::Instance()->setUpdatesPerSecond(rates[i]);
		start(i == 0 ? "clock at 50 updates a second" : "clock at 60 updates a second", flat(32));
		wait(599);
		equal((long)game->getTimeSinceStart(), 9, "after 599 updates it shows 9 seconds");
		wait(1);
		equal((long)game->getTimeSinceStart(), 10, "after 600 it shows 10");
	}
	Sanic::_Game::Instance()->setUpdatesPerSecond(50);
}

static void anEntityFalls()
{
	start("an entity falls", flat(32));
	// What every entity does when it has no rules of its own: here a 32 by 32 one, let go in the air.
	Sanic::Entity thing;
	thing.setX(200);
	thing.setY(40);
	for (int i = 0; i < 200; i++)
		thing.Update();
	check(thing.IsGrounded(), "it lands");
	equal(thing.getY(), 160, "on the top of the ground");
}

int main()
{
	standsWhereTheLevelPutsHim();
	walksUpToTopSpeed();
	skids();
	jumps();
	walksOffALedge();
	rolls();
	slopes();
	walls();
	theSidesOfThePicture();
	ceilings();
	oneWayPlatforms();
	hurt();
	picksUp();
	camera();
	theClock();
	anEntityFalls();

	std::cout << checks << " checks, " << failures << " failed" << '\n';
	return failures == 0 ? 0 : 1;
}
