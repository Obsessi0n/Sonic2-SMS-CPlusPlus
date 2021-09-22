#include "UserInterface.h"
#include "TextureManager.h"
#include "Game.h"
#include <chrono>

Sanic::UserInterface::UserInterface()
{
	Sanic::_TextureManager::Instance()->Load("assets/sprites/player/ui.png", "interface", Sanic::_Game::Instance()->getRenderer());
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
}

void Sanic::UserInterface::TopUI()
{
	int tX = m_width / 2;
	int tY = 1;
	int offset = 8;

	Sanic::_TextureManager::Instance()->DrawFrame("interface", tX + offset, tY, m_width, m_height, 1, 1, 0, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + m_width, tY, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + (m_width * 2) - offset, tY, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());

	//Timer
	Sanic::_TextureManager::Instance()->DrawFrame("interface", tX + offset, tY + m_height, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + m_width) + 2, (tY + m_height) - 1, m_width, m_height, 1, 1, 1, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + m_width, tY + m_height, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", (tX + offset) + (m_width + offset), tY + m_height, m_width, m_height, 1, 0, 2, Sanic::_Game::Instance()->getRenderer());

}

void Sanic::UserInterface::BottomUI()
{
	int bX = m_width / 2;
	int bY = Sanic::_Game::Instance()->getResHeight() - m_height;
	int offset = 8;

	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX, bY, m_width, m_height, 1, 0, 0, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + m_width, bY, m_width, m_height, 1, 0, 1, Sanic::_Game::Instance()->getRenderer());

	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + (m_width * 2) - offset, bY, m_width, m_height, 1, 0, 3, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::UserInterface::Render()
{
	TopUI();
	BottomUI();
}