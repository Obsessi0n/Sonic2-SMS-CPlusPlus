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

	class ReleaseJumpCommand : public Command
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->ReleaseJump(); };
	};

	//Down: a roll when he is moving, a crouch when he stands.
	class RollingCommand : public Command
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->Crouch(true); };
	};

	class ReleaseRollingCommand : public Command
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->Crouch(false); };
	};

	class LookUpCommand : public Command
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->LookUp(true); };
	};

	class ReleaseLookUpCommand : public Command
	{
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->LookUp(false); };
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

	//A hit, for testing: as if from what he is looking at, so it throws him backwards.
	class TakeDamage : public Command {
	public:
		virtual void execute() { Sanic::_Game::Instance()->getPlayer()->TakeDamage(Sanic::_Game::Instance()->getPlayer()->GetFacing() == -1); };
	};
}
