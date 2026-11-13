#ifndef PHYSICMANAGER_H
#define PHYSICMANAGER_H


namespace Sanic {
	class PhysicsManager {

	public:

		static PhysicsManager* Instance() {
			if (instance == 0)
			{
				instance = new PhysicsManager();
			}
			return instance;
		}


		PhysicsManager();
		~PhysicsManager();

		//The numbers that decide how the game feels, all of them here. A speed is in pixels per update, and what
		//changes a speed is in pixels per update, every update. They are the same at 50 updates a second and at
		//60: at 50 the same movement takes a fifth longer on the clock, as it did on a PAL console.

		//Running: what holding a direction adds to the speed along the ground, up to TOP_SPEED.
		static constexpr float ACCELERATION = 0.1f;
		//Braking: what holding the direction against the movement takes off.
		static constexpr float DECELERATION = 0.25f;
		//What the ground takes off when no direction is held.
		static constexpr float FRICTION = 0.05f;
		//The most that running alone reaches. A way down adds to it, and what it adds is kept.
		static constexpr float TOP_SPEED = 2.2f;
		//How hard a slope pulls downhill: this, times the sine of the slope's angle, every update. At 0.1 the
		//gentle slopes (1 up for 2 along) pull less than FRICTION, so the player can stand on them; a slope of
		//45 degrees pulls more, and he slides down it.
		static constexpr float SLOPE_FACTOR = 0.1f;
		//Nothing goes faster than this, along the ground or sideways in the air. The sensors count on it: keep
		//it well under STEP_HEIGHT.
		static constexpr float MAX_SPEED = 6.0f;

		//In the air: what holding a direction adds sideways, up to TOP_SPEED.
		static constexpr float AIR_ACCELERATION = 0.15f;
		//What every update adds to the speed downwards, and where that stops.
		static constexpr float GRAVITY = 0.2f;
		static constexpr float MAX_FALL_SPEED = 7.0f;
		//The speed upwards a jump starts with: with the GRAVITY above it rises some 105 pixels, over 3 tiles.
		static constexpr float JUMP_SPEED = 6.5f;
		//Letting go of the jump key while rising faster than this cuts the rise down to it: a short press is a
		//hop of half a tile, a held key the whole jump.
		static constexpr float JUMP_RELEASE_SPEED = 2.5f;

		//Being hit: thrown back and up at these speeds, then out of harm for this many updates.
		static constexpr float HURT_SPEED_X = 2.0f;
		static constexpr float HURT_SPEED_Y = 4.0f;
		static constexpr int HURT_UPDATES = 180;

		//Ground that is no more than this far above or below the feet is still ground: the player steps on
		//to it or follows it down. Anything higher is a wall. In pixels.
		static constexpr int STEP_HEIGHT = 16;

		//Is this point of the level inside something solid? Slopes count from their surface down.
		bool IsSolid(float const& xLiteralPosition, float const& yLiteralPosition);
		//The ground at a point, read from the height of the ground inside the tiles: where its surface is
		//(surfaceY, in pixels down the level) and how it leans there (angle, in radians, positive when it rises
		//to the right). It looks in the tile the point is in, in the one above when that one is ground to the
		//top, in the one below when it is empty there. False when there is no ground that near.
		bool FindFloor(float const& xLiteralPosition, float const& yLiteralPosition, int* surfaceY, float* angle);
	private:
		static PhysicsManager* instance;

	};


	typedef PhysicsManager _PhysicsManager;

}



#endif
