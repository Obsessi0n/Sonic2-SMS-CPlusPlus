#include "Game.h"

//1 sec = 1000ms so 60fps is 1000/60 = 16.6666666667
const double MS_PER_UPDATE = 16.6666666667;

int main(int argc, char* args[])
{
	if (Sanic::_Game::Instance()->init("Sanic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_SHOWN))
	{
		std::cout << "Game initialized" << '\n';
	}

	else {
		std::cout << "Game failed to initialize." << SDL_GetError() << '\n';
		return -1;
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