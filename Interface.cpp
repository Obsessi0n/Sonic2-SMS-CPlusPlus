#include "Interface.h"
#include "TextureManager.h"
#include "Game.h"

Sanic::Interface::Interface()
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/ui.png", "interface", Sanic::_Game::Instance()->getRenderer());
}

void Sanic::Interface::TopUI()
{
	int tX = m_width / 2;
	int tY = 1;
	int offset = 8;

	Sanic::_TextureManager::Instance()->DrawFrame("interface", tX + offset, tY, m_width, m_height, 1, 1, 0, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + m_width, tY, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + (m_width * 2) - offset, tY, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());

}

void Sanic::Interface::BottomUI()
{
	int bX = m_width / 2;
	int bY = Sanic::_Game::Instance()->getResHeight() - m_height;
	int offset = 8;

	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX, bY, m_width, m_height, 1, 0, 0, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + m_width, bY, m_width, m_height, 1, 0, 1, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + (m_width * 2) - offset, bY, m_width, m_height, 1, 0, 3, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::Interface::Render()
{
	TopUI();
	BottomUI();
}