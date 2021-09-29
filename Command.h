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

	class RollingCommand : public Command
	{
	public:
		virtual void execute() { };
	};

	class MoveLeftCommand : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->HorizontalMovementDir(-1); };
	};

	class MoveRightCommand : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->HorizontalMovementDir(1); };
	};

	class StopMovement : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->HorizontalMovementDir(0); };
	};
}