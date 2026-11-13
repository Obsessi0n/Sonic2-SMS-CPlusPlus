#include "Game.h"
#include "TextureManager.h"
#include "InputManager.h"

#include <vector>

Sanic::Game* Sanic::Game::instance = 0;

bool Sanic::Game::init(const char* title, int x, int y, int flags) {
	// Only what the game uses: a window and its events. Asking for everything makes the start fail on a machine
	// that has no sound card or cannot open a controller, such as the ones that build and test the game.
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "SDL could not start! SDL_Error: " << SDL_GetError() << '\n';
		return false;
	}

	window = SDL_CreateWindow(title, x, y, SCREEN_WIDTH, SCREEN_HEIGHT, flags);
	if (window == nullptr)
	{
		std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << '\n';
		return false;
	}

	// A graphics card when there is one, software when there is not (a machine with no screen, a test run).
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == nullptr)
	{
		std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << '\n';
		return false;
	}

	endLevel();
	mapLoader = new MapLoader();
	if (!mapLoader->Load("assets/tiles.txt", "assets/tiles", "assets/levels/test.txt"))
		return false;
	startLevel();

	userInterface = new UserInterface();

	SDL_RenderSetLogicalSize(renderer, RES_WIDTH, RES_HEIGHT);

	return true;
}

bool Sanic::Game::initWithoutWindow(std::istream& tilestxt, std::istream& leveltxt) {
	endLevel();
	mapLoader = new MapLoader();
	// No renderer: the pictures the map, the player and the things to pick up ask for are quietly not loaded.
	if (!mapLoader->Load(tilestxt, "", leveltxt))
		return false;
	startLevel();
	return true;
}

void Sanic::Game::startLevel()
{
	is_running = true;

	camera = { 0, 0, RES_WIDTH, RES_HEIGHT };

	level_width = mapLoader->GetWidthPixels();
	level_height = mapLoader->GetHeightPixels();

	player = new Player;
	player->setX(mapLoader->GetPlayerStart().x);
	player->setY(mapLoader->GetPlayerStart().y);

	entityManager = new EntityManager();
	entityManager->LoadPickups(mapLoader->GetPickups());

	playerRings = 0;
	playerLives = 3;

	//Start clock  timer
	updates = 0;
	timeSinceStart = 0;
}

void Sanic::Game::endLevel()
{
	delete(mapLoader);
	delete(userInterface);
	delete(player);
	delete(entityManager);

	mapLoader = nullptr;
	userInterface = nullptr;
	player = nullptr;
	entityManager = nullptr;
}

void Sanic::Game::handleEvents()
{
	Sanic::_InputManager::Instance()->HandleInput();
}

void Sanic::Game::update()
{
	GameTimer();

	player->Update();

	camera.x = (player->getPosX() + player->getWidth() / 2) - RES_WIDTH / 2;
	camera.y = (player->getPosY() + player->getHeight() / 2) - RES_HEIGHT / 2;

	entityManager->Update();

	if (camera.x > level_width - camera.w)
	{
		camera.x = level_width - camera.w;
	}
	if (camera.y > level_height - camera.h)
	{
		camera.y = level_height - camera.h;
	}
	if (camera.x < 0)
	{
		camera.x = 0;
	}
	if (camera.y < 0)
	{
		camera.y = 0;
	}
}

void Sanic::Game::draw()
{
	SDL_RenderClear(renderer);

	mapLoader->DrawBackground();
	mapLoader->DrawMap(camera.x, camera.y);

	player->Render(camera.x, camera.y);
	entityManager->RenderEntities(camera.x, camera.y);

	// The display last, so that nothing in the level is drawn over it.
	userInterface->Render();
}

void Sanic::Game::render()
{
	draw();
	SDL_RenderPresent(renderer);
}

bool Sanic::Game::screenshot(const char* fileName)
{
	draw();

	// The frame is read back before it is shown: once shown, what the renderer still holds is not defined.
	// It comes out at the size of the window, not at the game's own 256 by 192.
	int width = 0, height = 0;
	SDL_GetRendererOutputSize(renderer, &width, &height);
	SDL_Surface* shot = SDL_CreateRGBSurfaceWithFormat(0, width, height, 24, SDL_PIXELFORMAT_BGR24);
	if (shot == nullptr)
	{
		std::cout << "Screenshot could not be made! SDL_Error: " << SDL_GetError() << '\n';
		return false;
	}

	bool saved = SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_BGR24, shot->pixels, shot->pitch) == 0
		&& SDL_SaveBMP(shot, fileName) == 0;
	if (!saved)
		std::cout << "Screenshot could not be saved to " << fileName << "! SDL_Error: " << SDL_GetError() << '\n';

	SDL_FreeSurface(shot);
	SDL_RenderPresent(renderer);
	return saved;
}

void Sanic::Game::clean()
{
	Sanic::_TextureManager::Instance()->ClearFromSpritesheet("player");

	endLevel();

	if (renderer != nullptr)
		SDL_DestroyRenderer(renderer);
	if (window != nullptr)
		SDL_DestroyWindow(window);
	renderer = nullptr;
	window = nullptr;
	SDL_Quit();
}

void Sanic::Game::quit()
{
	is_running = false;
}

void Sanic::Game::changePlayerRings(int *amount) {
	playerRings += *amount;

	if (playerRings > 99) {
		playerRings = 0;
		//Add extra life!
		int i = 1;
		changePlayerLives(&i);
	}

	if (playerRings < 0)
		playerRings = 0;
}

void Sanic::Game::changePlayerLives(int* amount) {
	playerLives += *amount;
	if (playerLives < 0) {
		playerLives = 0;
		//GameOver
	}
}

void Sanic::Game::GameTimer() {
	updates++;

	timeSinceStart = (double)updates / updatesPerSecond;
}
