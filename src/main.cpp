#include "Game.h"
#include "InputManager.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// The game runs 50 updates a second, as on a PAL console, unless it is started with "--hz 60", as on an NTSC one.
//
// A run with nobody at the keys, for checking the game from a script or on a machine with no screen:
//
//   --frames 300              runs that many updates as fast as they go, draws once, says where the player is, leaves
//   --hold right+jump         holds those keys during the run: left, right, jump, joined by '+', or none
//   --hold right:120          the same for 120 updates only; several --hold follow one another in the order given
//   --screenshot shot.bmp     saves the frame drawn at the end as a BMP picture
//
// Set SDL_VIDEODRIVER=dummy to run it with no screen.
struct Hold
{
	bool left = false;
	bool right = false;
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
				std::cout << "--hz " << args[i] << ": expected 50 (PAL) or 60 (NTSC)" << '\n';
				script.understood = false;
			}
		}
		else if (std::strcmp(args[i], "--hold") == 0 && hasValue)
		{
			Hold hold;
			if (!readHold(args[++i], &hold))
			{
				std::cout << "--hold " << args[i] << ": expected keys joined by '+' (left, right, jump, none) and maybe ':' and a number of updates" << '\n';
				script.understood = false;
			}
			script.holds.push_back(hold);
		}
		else
		{
			std::cout << "Unknown option " << args[i] << ". Options: --hz <50|60>, --frames <updates>, --hold <keys>[:<updates>], --screenshot <file.bmp>" << '\n';
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
	const double MS_PER_UPDATE = 1000.0 / script.hz;

	if (Sanic::_Game::Instance()->init("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_SHOWN))
	{
		std::cout << "Game initialized" << '\n';
	}
	else {
		std::cout << "Game failed to initialize." << SDL_GetError() << '\n';
		return -1;
	}

	if (script.frames > 0 || script.screenshot != nullptr)
	{
		for (int i = 0; i < script.frames; i++)
		{
			Hold hold = holdAt(script, i);
			Sanic::_InputManager::Instance()->HoldKeys(hold.left, hold.right, hold.jump);
			Sanic::_Game::Instance()->handleEvents();
			Sanic::_Game::Instance()->update();
		}

		bool drawn = true;
		if (script.screenshot != nullptr)
			drawn = Sanic::_Game::Instance()->screenshot(script.screenshot);
		else
			Sanic::_Game::Instance()->render();

		Sanic::Player* player = Sanic::_Game::Instance()->getPlayer();
		std::cout << "After " << script.frames << " frames at " << script.hz << " Hz (" << (int)Sanic::_Game::Instance()->getTimeSinceStart() << " s): player at " << player->getPosX() << "," << player->getPosY()
			<< " level " << Sanic::_Game::Instance()->getLevelWidth() << "x" << Sanic::_Game::Instance()->getLevelHeight()
			<< " camera " << Sanic::_Game::Instance()->camera.x << "," << Sanic::_Game::Instance()->camera.y
			<< " picked up " << Sanic::_Game::Instance()->getPlayerRings() << '\n';
		Sanic::_Game::Instance()->clean();
		return drawn ? 0 : 1;
	}

	double previous = SDL_GetTicks();
	double lag = 0.0;

	while (Sanic::_Game::Instance()->isRunning())
	{
		double current = SDL_GetTicks();
		double elapsed = current - previous;
		previous = current;
		lag += elapsed;

		while (lag >= MS_PER_UPDATE) {
			Sanic::_Game::Instance()->handleEvents();
			Sanic::_Game::Instance()->update();
			lag -= MS_PER_UPDATE;
		}

		Sanic::_Game::Instance()->render();
	}

	Sanic::_Game::Instance()->clean();

	return 0;
}
