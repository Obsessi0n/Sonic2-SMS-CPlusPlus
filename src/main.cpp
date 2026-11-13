#include "Game.h"
#include "InputManager.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// The game runs 50 updates a second, as on a PAL console, unless it is started with "--hz 60", as on an NTSC one.
// "--scale N" makes the window N times the picture's 256 by 192; without it, the biggest that fits, up to 4.
//
// A run with nobody at the keys, for checking the game from a script or on a machine with no screen:
//
//   --frames 300              runs that many updates as fast as they go, draws once, says where the player is, leaves
//   --hold right+jump         holds those keys during the run: left, right, up, down, jump, joined by '+', or none
//   --hold right:120          the same for 120 updates only; several --hold follow one another in the order given
//   --screenshot shot.bmp     saves the frame drawn at the end as a BMP picture
//
// Set SDL_VIDEODRIVER=dummy to run it with no screen.
struct Hold
{
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool jump = false;
	// How many updates the keys stay down; 0 is until the run ends.
	int frames = 0;
};

struct Script
{
	int frames = 0;
	std::vector<Hold> holds;
	const char* screenshot = nullptr;
	// Updates a second: 50 or 60.
	int hz = 50;
	// The window, in times the picture; 0 is the biggest that fits.
	int scale = 0;
	bool understood = true;
};

// "right+jump:20" to the keys and the count it stands for.
static bool readHold(const std::string& text, Hold* hold)
{
	std::string keys = text;
	size_t colon = text.find(':');
	if (colon != std::string::npos)
	{
		keys = text.substr(0, colon);
		hold->frames = std::atoi(text.substr(colon + 1).c_str());
		if (hold->frames <= 0)
			return false;
	}

	size_t start = 0;
	while (start <= keys.size())
	{
		size_t plus = keys.find('+', start);
		if (plus == std::string::npos)
			plus = keys.size();
		std::string key = keys.substr(start, plus - start);
		if (key == "left")
			hold->left = true;
		else if (key == "right")
			hold->right = true;
		else if (key == "up")
			hold->up = true;
		else if (key == "down")
			hold->down = true;
		else if (key == "jump")
			hold->jump = true;
		else if (key != "none")
			return false;
		start = plus + 1;
	}
	return true;
}

static Script readScript(int argc, char* args[])
{
	Script script;
	for (int i = 1; i < argc; i++)
	{
		bool hasValue = i + 1 < argc;
		if (std::strcmp(args[i], "--frames") == 0 && hasValue)
		{
			script.frames = std::atoi(args[++i]);
		}
		else if (std::strcmp(args[i], "--screenshot") == 0 && hasValue)
		{
			script.screenshot = args[++i];
		}
		else if (std::strcmp(args[i], "--hz") == 0 && hasValue)
		{
			script.hz = std::atoi(args[++i]);
			if (!Sanic::Game::isUpdateRate(script.hz))
			{
				std::printf("--hz %s: expected 50 (PAL) or 60 (NTSC)\n", args[i]);
				script.understood = false;
			}
		}
		else if (std::strcmp(args[i], "--scale") == 0 && hasValue)
		{
			script.scale = std::atoi(args[++i]);
			if (script.scale < 1 || script.scale > 16)
			{
				std::printf("--scale %s: expected a whole number from 1 to 16\n", args[i]);
				script.understood = false;
			}
		}
		else if (std::strcmp(args[i], "--hold") == 0 && hasValue)
		{
			Hold hold;
			if (!readHold(args[++i], &hold))
			{
				std::printf("--hold %s: expected keys joined by '+' (left, right, up, down, jump, none) and maybe ':' and a number of updates\n", args[i]);
				script.understood = false;
			}
			script.holds.push_back(hold);
		}
		else
		{
			std::printf("Unknown option %s. Options: --hz <50|60>, --scale <1-16>, --frames <updates>, --hold <keys>[:<updates>], --screenshot <file.bmp>\n", args[i]);
			script.understood = false;
		}
	}
	return script;
}

// The keys the script holds at this update of the run.
static Hold holdAt(const Script& script, int frame)
{
	int end = 0;
	for (const Hold& hold : script.holds)
	{
		if (hold.frames == 0)
			return hold;
		end += hold.frames;
		if (frame < end)
			return hold;
	}
	return Hold();
}

int main(int argc, char* args[])
{
	Script script = readScript(argc, args);
	if (!script.understood)
		return 2;

	Sanic::_Game::Instance()->setUpdatesPerSecond(script.hz);

	if (Sanic::_Game::Instance()->init("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_SHOWN, script.scale))
	{
		std::printf("Game initialized\n");
	}
	else {
		std::printf("Game failed to initialize.%s\n", SDL_GetError());
		return -1;
	}

	if (script.frames > 0 || script.screenshot != nullptr)
	{
		for (int i = 0; i < script.frames; i++)
		{
			Hold hold = holdAt(script, i);
			Sanic::_InputManager::Instance()->HoldKeys(hold.left, hold.right, hold.up, hold.down, hold.jump);
			Sanic::_Game::Instance()->handleEvents();
			Sanic::_Game::Instance()->update();
		}

		bool drawn = true;
		if (script.screenshot != nullptr)
			drawn = Sanic::_Game::Instance()->screenshot(script.screenshot);
		else
			Sanic::_Game::Instance()->render();

		Sanic::Player* player = Sanic::_Game::Instance()->getPlayer();
		// The clock is the level's own, which counts 60 updates to its second at either rate.
		int clock = (int)Sanic::_Game::Instance()->getTimeSinceStart();
		std::printf("After %d frames at %d Hz (clock %d:%02d): player at %d,%d level %dx%d camera %d,%d picked up %d\n",
			script.frames, script.hz, clock / 60, clock % 60, player->getPosX(), player->getPosY(),
			Sanic::_Game::Instance()->getLevelWidth(), Sanic::_Game::Instance()->getLevelHeight(),
			Sanic::_Game::Instance()->camera.x, Sanic::_Game::Instance()->camera.y,
			Sanic::_Game::Instance()->getPlayerRings());
		Sanic::_Game::Instance()->clean();
		return drawn ? 0 : 1;
	}

	// The updates are kept to the wall clock: the n-th is made when n / hz seconds have gone by since the start.
	// A frame is drawn after the updates, and then the program sleeps until the next one is due. Nothing changes
	// between two updates, so drawing more often would only keep a processor core busy for no new picture.
	const Uint64 frequency = SDL_GetPerformanceFrequency();
	const Uint64 begin = SDL_GetPerformanceCounter();
	const Uint64 hz = (Uint64)script.hz;
	// After the program was held up (the window dragged, the machine busy) the updates missed are not all made
	// at once, which would make the game jump ahead: a few are, and the rest of that time is let go.
	const Uint64 MAX_CATCH_UP = 5;
	Uint64 done = 0;

	while (Sanic::_Game::Instance()->isRunning())
	{
		Uint64 due = (SDL_GetPerformanceCounter() - begin) * hz / frequency;
		if (due > done + MAX_CATCH_UP)
			done = due - MAX_CATCH_UP;

		bool updated = false;
		while (done < due && Sanic::_Game::Instance()->isRunning())
		{
			Sanic::_Game::Instance()->handleEvents();
			Sanic::_Game::Instance()->update();
			done++;
			updated = true;
		}
		if (updated)
			Sanic::_Game::Instance()->render();

		// Asleep until the next update is due; at least a millisecond, so that the wait is never spent spinning.
		Uint64 next = begin + ((done + 1) * frequency + hz - 1) / hz;
		Uint64 now = SDL_GetPerformanceCounter();
		if (next > now)
		{
			Uint32 ms = (Uint32)((next - now) * 1000 / frequency);
			SDL_Delay(ms > 0 ? ms : 1);
		}
	}

	Sanic::_Game::Instance()->clean();

	return 0;
}
