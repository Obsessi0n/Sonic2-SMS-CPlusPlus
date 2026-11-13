#include "Game.h"
#include "TextureManager.h"
#include "InputManager.h"

#include <cstdio>
#include <vector>

Sanic::Game* Sanic::Game::instance = 0;

int Sanic::Game::fittingScale()
{
	// What of the screen a window may take, without the taskbar; less room for the window's own title bar and
	// frame, which that does not count.
	const int frame = 40;
	SDL_Rect usable;
	if (SDL_GetDisplayUsableBounds(0, &usable) != 0)
		return MAX_FIT_SCALE;
	int fit = SDL_min(usable.w / RES_WIDTH, (usable.h - frame) / RES_HEIGHT);
	return SDL_max(1, SDL_min(fit, MAX_FIT_SCALE));
}

bool Sanic::Game::init(const char* title, int x, int y, int flags, int windowScale) {
	// Only what the game uses: a window and its events. Asking for everything makes the start fail on a machine
	// that has no sound card or cannot open a controller, such as the ones that build and test the game.
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::printf("SDL could not start! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// A bigger window costs more memory and more drawing for the same picture: by default it is the biggest that
	// fits, and never more than four times the picture.
	scale = windowScale > 0 ? windowScale : fittingScale();
	window = SDL_CreateWindow(title, x, y, RES_WIDTH * scale, RES_HEIGHT * scale, flags);
	if (window == nullptr)
	{
		std::printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// A graphics card when there is one, software when there is not (a machine with no screen, a test run).
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == nullptr)
	{
		std::printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
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

bool Sanic::Game::initWithoutWindow(const std::string& tilesText, const std::string& levelText) {
	endLevel();
	mapLoader = new MapLoader();
	// No renderer: the pictures the map, the player and the things to pick up ask for are quietly not loaded.
	if (!mapLoader->LoadText(tilesText, "", levelText))
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
	player->Place(mapLoader->GetPlayerStart().x, mapLoader->GetPlayerStart().y);

	// The camera starts with the player in the middle of the picture and his feet on its line, as far as the
	// level's edges allow.
	cameraLine = PhysicsManager::CAMERA_LINE_START;
	camera.x = player->getFeetX() - PhysicsManager::CAMERA_CENTRE_X;
	camera.y = player->getFeetY() - cameraLine;
	if (camera.x > level_width - camera.w)
		camera.x = level_width - camera.w;
	if (camera.y > level_height - camera.h)
		camera.y = level_height - camera.h;
	if (camera.x < 0)
		camera.x = 0;
	if (camera.y < 0)
		camera.y = 0;

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

	// The camera first, from where the player was at the end of the last update: so the picture is always one
	// update behind him. Then the player, then everything else.
	moveCamera();

	player->Update();

	entityManager->Update();
}

// The camera does not sit on the player: it lets him move inside a small window of the picture and follows only
// when he leaves it, by as much as he is outside and never more than a few pixels in an update. Sideways the
// window is about the middle of the picture. Up and down it is about a line his feet are kept near, the same
// on the ground and in the air; looking up and crouching move that line, a pixel an update, and so pan the view.
// The numbers are in PhysicsManager.h.
void Sanic::Game::moveCamera()
{
	const int maxStep = PhysicsManager::CAMERA_MAX_STEP;
	const int quirkStep = PhysicsManager::CAMERA_QUIRK_STEP;
	int maxX = level_width - camera.w > 0 ? level_width - camera.w : 0;
	int maxY = level_height - camera.h > 0 ? level_height - camera.h : 0;

	int target = PhysicsManager::CAMERA_LINE;
	if (player->getState() == PlayerState::LookingUp)
		target = PhysicsManager::CAMERA_LINE_LOOK_UP;
	else if (player->getState() == PlayerState::Crouching)
		target = PhysicsManager::CAMERA_LINE_CROUCH;
	if (cameraLine < target)
		cameraLine++;
	else if (cameraLine > target)
		cameraLine--;

	// A move that would take the camera past an edge of the level is not made at all.
	int right = PhysicsManager::CAMERA_CENTRE_X + PhysicsManager::CAMERA_WINDOW_X;
	int left = PhysicsManager::CAMERA_CENTRE_X - PhysicsManager::CAMERA_WINDOW_X;
	int screenX = player->getFeetX() - camera.x;
	if (screenX > right)
	{
		int step = screenX - right;
		if (step > maxStep)
			step = maxStep;
		if (camera.x + step <= maxX)
			camera.x += step;
	}
	else if (screenX < left)
	{
		int step = left - screenX;
		if (step > quirkStep)
			step = maxStep;
		if (camera.x - step >= 0)
			camera.x -= step;
	}

	int below = cameraLine + PhysicsManager::CAMERA_WINDOW_Y;
	int above = cameraLine - PhysicsManager::CAMERA_WINDOW_Y;
	int screenY = player->getFeetY() - camera.y;
	if (screenY > below)
	{
		int step = screenY - below;
		if (step > maxStep)
			step = maxStep;
		if (camera.y + step <= maxY)
			camera.y += step;
	}
	else if (screenY < above)
	{
		int step = above - screenY;
		if (step > quirkStep)
			step = maxStep;
		if (camera.y - step >= 0)
			camera.y -= step;
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
	// Nobody sees a window that is minimised: the game goes on, but nothing is drawn.
	if (window != nullptr && (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) != 0)
		return;
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
		std::printf("Screenshot could not be made! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	bool saved = SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_BGR24, shot->pixels, shot->pitch) == 0
		&& SDL_SaveBMP(shot, fileName) == 0;
	if (!saved)
		std::printf("Screenshot could not be saved to %s! SDL_Error: %s\n", fileName, SDL_GetError());

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

	//One second on the level's clock is 60 updates, whether there are 50 or 60 of them to a real second
	timeSinceStart = (double)updates / PhysicsManager::CLOCK_UPDATES_PER_SECOND;
}
