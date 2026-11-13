#include "PlayerAnimation.h"
#include "TextureManager.h"
#include "Game.h"

void Sanic::PlayerAnimation::UpdateState()
{
	//The picture follows what the player is doing
	state = Sanic::_Game::Instance()->getPlayer()->getState();
}

void Sanic::PlayerAnimation::Render(int camX, int camY)

{
	Player* player = Sanic::_Game::Instance()->getPlayer();
	SDL_Renderer* renderer = Sanic::_Game::Instance()->getRenderer();
	int x = player->getPosX() - camX;
	int y = player->getPosY() - camY;
	int w = player->getWidth();
	int h = player->getHeight();
	//The pictures look to the right: looking left they are drawn mirrored
	SDL_RendererFlip flip = player->GetFacing() == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

	if (state == PlayerState::Standing || state == PlayerState::Crouching || state == PlayerState::LookingUp)
	{
		//Crouching and looking up have no picture of their own yet: only the camera shows them
		Sanic::_TextureManager::Instance()->DrawFrame("player", x, y, w, h, 0, 0, 0, renderer, flip);
	}
	else if (state == PlayerState::Walking || state == PlayerState::Falling || state == PlayerState::Hurt)
	{
		//Walking and running are one picture for now. When running has its own, it begins when he reaches top
		//speed and ends when he is under it. (In the reference the test that ends it is a step late for speeds
		//to the left: a slip not to copy.)
		Sanic::_TextureManager::Instance()->DrawAnimation("player", x, y, w, h, 0, 1, 5, 200, renderer, flip);
	}
	else if (state == PlayerState::Jumping || state == PlayerState::RampJumping || state == PlayerState::Rolling)
	{
		//The ball: a jump and a roll are the same picture
		Sanic::_TextureManager::Instance()->DrawAnimation("player", x, y, w, h, 0, 3, 4, 50, renderer);
	}
	else if (state == PlayerState::Skidding)
	{
		Sanic::_TextureManager::Instance()->DrawAnimation("player", x, y, w, h, 0, 5, 2, 100, renderer, flip);
	}
}
