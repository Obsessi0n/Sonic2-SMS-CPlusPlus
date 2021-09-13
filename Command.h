#pragma once
#include "Game.h"
namespace Sanic {
	class Command
	{
	public:
		virtual ~Command() {}
		virtual void execute() = 0;
	};

	class JumpCommand : public Command 
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->Jump(); };
	};

	class MoveLeftCommand : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->Move(false); };
	};

	class MoveRightCommand : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->Move(true); };
	};
}