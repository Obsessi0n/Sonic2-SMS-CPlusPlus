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
	int ringXPostion = m_width;
	int timerXPosition = m_width;
	int timerYPosition = m_height + 1;

	DrawRings(ringXPostion,1);
	DrawTimer(timerXPosition, timerYPosition);

}

void Sanic::UserInterface::DrawRings(const int &xPosition, const int &yPosition) {
	//Rings Icon
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition, yPosition, m_width, m_height, 1, 1, 0, Sanic::_Game::Instance()->getRenderer());
	//Rings value	
	int firstRingIconCol = floor(Sanic::Game::Instance()->getPlayerRings()/10)+2;
	int secondRingIconCol = (Sanic::Game::Instance()->getPlayerRings() % 10) +2;
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition + m_width, yPosition, m_width, m_height, 1, 0, firstRingIconCol, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition + (m_width * 1.5), yPosition, m_width, m_height, 1, 0, secondRingIconCol, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::UserInterface::DrawTimer(const int& xPosition, const int& yPosition){

	double timeInSeconds = Sanic::Game::Instance()->getTimeSinceStart();
	int seconds = fmod(timeInSeconds, 60);
	int minutes = floor(Sanic::Game::Instance()->getTimeSinceStart() / 60);
	if (minutes > 9)// need to check how original game handles values above 9
		minutes = 9;

	int minutesIconCol = minutes + 2;
	int firstSecondsIconCol = floor(seconds / 10) + 2;
	int secondSecondsIconCol = (seconds % 10) + 2;
	//Timer
	//Minutes
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition-2, yPosition, m_width, m_height, 1, 0, minutesIconCol, Sanic::_Game::Instance()->getRenderer());
	//Dots
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition + (m_width-7), yPosition-2, m_width, m_height, 1, 1, 1, Sanic::_Game::Instance()->getRenderer());
	//Seconds
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition + (m_width*2 - 16), yPosition, m_width, m_height, 1, 0, firstSecondsIconCol, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", xPosition + (m_width*2 - 8), yPosition, m_width, m_height, 1, 0, secondSecondsIconCol, Sanic::_Game::Instance()->getRenderer());
}


void Sanic::UserInterface::BottomUI()
{
	int bX = m_width / 2;
	int bY = Sanic::_Game::Instance()->getResHeight() - m_height;
	int offset = 8;

	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX, bY, m_width, m_height, 1, 0, 0, Sanic::_Game::Instance()->getRenderer());
	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + m_width, bY, m_width, m_height, 1, 0, 1, Sanic::_Game::Instance()->getRenderer());

	int lives = Sanic::Game::Instance()->getPlayerLives();
	if (lives > 9) // need to check how original game handles values above 9
		lives = 9;

	int livesIconCol = lives + 2;
	Sanic::_TextureManager::Instance()->DrawFrame("interface", bX + (m_width * 2) - offset, bY, m_width, m_height, 1, 0, livesIconCol, Sanic::_Game::Instance()->getRenderer());
}

void Sanic::UserInterface::Render()
{
	TopUI();
	BottomUI();
}