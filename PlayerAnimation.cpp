#include "PlayerAnimation.h"
#include "TextureManager.h"
#include "Game.h"

void Sanic::PlayerAnimation::UpdateState()
{
	if (Sanic::_Game::Instance()->getPlayer()->GetAcceleration() > 0.01f)
	{
		state = PlayerState::Running;
	}
	else
	{
		state = PlayerState::Idle;
	}

	if (Sanic::_Game::Instance()->getPlayer()->IsJumping())
	{
		state = PlayerState::Jumping;
	}
}

void Sanic::PlayerAnimation::Render(int camX, int camY)

{
	if (state == PlayerState::Idle)
	{
		Sanic::_TextureManager::Instance()->DrawFrame("player", (int)Sanic::_Game::Instance()->getPlayer()->getPosX() - camX, (int)Sanic::_Game::Instance()->getPlayer()->getPosY() - camY, (int)Sanic::_Game::Instance()->getPlayer()->getWidth(), (int)Sanic::_Game::Instance()->getPlayer()->getHeight(), 0, 0, 0, Sanic::_Game::Instance()->getRenderer());
	}
	else if (state == PlayerState::Running)
	{
		Sanic::_TextureManager::Instance()->DrawAnimation("player", (int)Sanic::_Game::Instance()->getPlayer()->getPosX() - camX, (int)Sanic::_Game::Instance()->getPlayer()->getPosY() - camY, (int)Sanic::_Game::Instance()->getPlayer()->getWidth(), (int)Sanic::_Game::Instance()->getPlayer()->getHeight(), 0, 1, 5, 200, Sanic::_Game::Instance()->getRenderer());
	}
	else if (state == PlayerState::Jumping)
	{
		Sanic::_TextureManager::Instance()->DrawAnimation("player", (int)Sanic::_Game::Instance()->getPlayer()->getPosX() - camX, (int)Sanic::_Game::Instance()->getPlayer()->getPosY() - camY, (int)Sanic::_Game::Instance()->getPlayer()->getWidth(), (int)Sanic::_Game::Instance()->getPlayer()->getHeight(), 0, 3, 4, 100, Sanic::_Game::Instance()->getRenderer());
	}
}