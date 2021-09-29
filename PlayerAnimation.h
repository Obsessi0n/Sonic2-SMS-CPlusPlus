#pragma once
#include "PlayerState.h"
namespace Sanic
{
	class PlayerAnimation
	{
	public:
		void UpdateState();
		void Render(int camX, int camY);
	private:
		PlayerState state;
	};
}

