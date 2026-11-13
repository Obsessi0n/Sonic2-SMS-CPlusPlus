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

		//The numbers that decide how the game plays, all of them here.
		//
		//They are whole numbers in the units of an 8-bit console: a position is counted in 1/256 of a pixel
		//("sub"), a speed in sub per update, and what changes a speed in sub per update, every update. Beside
		//each is the raw value as the movement reference gives it, and the same in pixels. Every update the
		//game adds what changes a speed, holds the speed to its limit, then moves by the speed.
		//
		//The numbers are the same at 50 updates a second and at 60: at 50 the same movement simply takes a
		//fifth longer on the clock, as it did on a PAL console.
		static constexpr int SUB = 256;                    //sub to the pixel

		//On the ground                                      raw     in pixels
		static constexpr int TOP_SPEED = 1024;             //$0400   4.0 an update: the most that walking and running reach
		static constexpr int ROLL_TOP_SPEED = 1536;        //$0600   6.0: the most while rolling, and in a jump from a roll
		static constexpr int ACCELERATION = 16;            //16      0.0625: holding a direction, either way
		static constexpr int SKID_BRAKING = 32;            //32      0.125: holding against the movement while skidding
		static constexpr int FRICTION = 20;                //20      0.078: no direction held, walking or standing
		static constexpr int ROLL_FRICTION = 5;            //5       0.0195: no direction held, rolling
		static constexpr int ROLL_FORWARD = 4;             //-4      0.0156 lost: holding the way he rolls (a roll cannot be pushed)
		static constexpr int ROLL_BACK = 16;               //-16     0.0625 lost: holding against a roll, for the one update before he skids
		//Slower than this, every one of the terms above counts double: getting going, braking and stopping
		//are all quicker at a crawl. In the air too.
		static constexpr int LOW_SPEED = 128;              //$0080   0.5
		//Slower than this with no direction held, he stops dead and stands.
		static constexpr int STOP_SPEED = 32;              //$20     0.125
		//A roll on flat ground ends when it is slower than this.
		static constexpr int ROLL_STOP_SPEED = 16;         //$10     0.0625
		//He must be at least this fast for down to start a roll, and for the other direction to start a skid.
		static constexpr int MIN_ROLL_SPEED = 256;         //$0100   1.0
		static constexpr int MIN_SKID_SPEED = 256;         //$0100   1.0
		//A slope pushes downhill by a fixed amount for its class, every update, whatever his speed. It does
		//not push a player who is standing still.
		static constexpr int SLOPE_PUSH_1_1 = 31;          //31      0.121: one up for one along (45 degrees)
		static constexpr int SLOPE_PUSH_1_2 = 24;          //24      0.094: one up for two along
		static constexpr int SLOPE_PUSH_1_4 = 16;          //16      0.0625: one up for four along
		//On the ground he is driven this far down every update and lifted back to the surface: that is all
		//that keeps him on a slope going down.
		static constexpr int GROUND_STICK = 1792;          //$0700   7.0

		//In the air                                         raw     in pixels
		static constexpr int JUMP_SPEED = 1088;            //$FBC0   4.25 upwards
		//While the jump key stays down the rise is held at JUMP_SPEED, for the first updates of the jump up
		//to this count: 13 of them. That is what makes a held key a higher jump (95 pixels against 46).
		static constexpr int JUMP_HOLD_UPDATES = 14;       //below 14
		static constexpr int GRAVITY = 48;                 //$30     0.1875
		static constexpr int RAMP_GRAVITY = 36;            //$24     0.1406: after a roll leaves the end of a ramp
		static constexpr int MAX_FALL_SPEED = 1792;        //$0700   7.0
		static constexpr int AIR_ACCELERATION = 16;        //16      0.0625: holding the way he moves
		static constexpr int AIR_BRAKING = 64;             //64      0.25: holding against it, in a jump
		static constexpr int AIR_DRAG = 16;                //16      0.0625: no direction held
		static constexpr int WALK_OFF_SPEED = 256;         //$0100   1.0 downwards: walking off a ledge
		static constexpr int CEILING_BUMP_SPEED = 256;     //$0100   1.0 downwards: after his head meets a ceiling

		//Being hit                                          raw     in pixels
		static constexpr int HURT_SPEED_X = 256;           //$0100   1.0, away from what hit him
		static constexpr int HURT_SPEED_Y = 1024;          //$FC00   4.0 upwards
		static constexpr int HURT_UPDATES = 120;           //$78     updates out of harm, drawn 2 updates in every 4

		//Where he feels the level, in whole pixels from the middle of his feet (see Player.h)
		static constexpr int WALL_PROBE_X = 10;            //10 to each side
		static constexpr int WALL_PROBE_Y = 12;            //12 above the feet: anything lower is a step he walks up
		static constexpr int CEILING_PROBE_Y = 24;         //24 above the feet
		//A one-way platform catches him only on the way down, and only if his feet are less than this, plus
		//the whole pixels of his speed downwards, into it.
		static constexpr int ONE_WAY_REACH = 9;            //9
		//His box against things that are not the level: 4 to each side of the middle, and this high from the
		//feet up, standing or curled into a ball.
		static constexpr int BOX_HALF_WIDTH = 4;           //4
		static constexpr int BOX_HEIGHT = 28;              //28
		static constexpr int BALL_HEIGHT = 18;             //18
		//He cannot leave the picture: the middle of his feet stays between these, from the picture's left.
		static constexpr int SCREEN_LEFT = 16;             //16
		static constexpr int SCREEN_RIGHT = 247;           //247

		//The camera, in whole pixels (see Game::moveCamera)
		static constexpr int CAMERA_CENTRE_X = 128;        //128: where the player is kept, from the picture's left,
		static constexpr int CAMERA_WINDOW_X = 8;          //8:   give or take this
		static constexpr int CAMERA_WINDOW_Y = 16;         //16:  his feet are kept this near the line below
		static constexpr int CAMERA_MAX_STEP = 8;          //8:   the most the camera moves in an update
		//Going left or up, a move of exactly 9 is let through whole: a quirk of the reference, kept as it is
		//part of how the camera moves. Set it to CAMERA_MAX_STEP to be rid of it.
		static constexpr int CAMERA_QUIRK_STEP = 9;        //9
		//The line his feet are kept near, from the picture's top: where it is when a level starts, where it
		//goes in play, and where looking up and crouching send it, one pixel an update.
		static constexpr int CAMERA_LINE_START = 120;      //120
		static constexpr int CAMERA_LINE = 128;            //128
		static constexpr int CAMERA_LINE_LOOK_UP = 184;    //184: the view goes 56 up
		static constexpr int CAMERA_LINE_CROUCH = 48;      //48:  the view goes 80 down

		//The clock of the level counts this many updates to a second it shows, at 50 updates a second as at
		//60: at 50 a second on the clock lasts 1.2 seconds.
		static constexpr int CLOCK_UPDATES_PER_SECOND = 60; //60

		//What the floor check finds at a point.
		enum FloorCheck {
			NO_GROUND,     //an empty block: nothing here to stand on
			ABOVE_GROUND,  //a block of ground, but the point is not down to its surface (or the platform lets it by)
			ON_GROUND      //at the surface or under it: whoever asked stands on the surface
		};

		//Is this pixel of the level inside solid ground? Slopes count from their surface down; one-way
		//platforms do not count. Walls and ceilings are made of this.
		bool IsSolid(int const& xPixel, int const& yPixel);
		//The floor at a pixel, read from the height of the ground in that column of its block; when the
		//column is ground to the top of the block, the block above is read too and its height added, which is
		//how a slope carries on upwards from one block into the next. ySpeed is the speed downwards of
		//whoever asks, in sub: a one-way platform needs it. Gives the row of the surface (surfaceY, in pixels
		//down the level) and the push of the slope there (slopePush, in sub: positive pushes to the right).
		FloorCheck CheckFloor(int const& xPixel, int const& yPixel, int const& ySpeed, int* surfaceY, int* slopePush);

		//Sub to whole pixels, rounding down also left of and above the level's corner; and the other way,
		//moving something to a pixel while keeping the fraction of a pixel it had.
		static int ToPixel(int sub) { return sub >= 0 ? sub / SUB : -((-sub + SUB - 1) / SUB); };
		static int AtPixel(int sub, int pixel) { return pixel * SUB + (sub - ToPixel(sub) * SUB); };
		//The block a pixel is in.
		static int ToBlock(int pixel) { return pixel >= 0 ? pixel / 32 : -((-pixel + 31) / 32); };
	private:
		static PhysicsManager* instance;

	};


	typedef PhysicsManager _PhysicsManager;

}



#endif
