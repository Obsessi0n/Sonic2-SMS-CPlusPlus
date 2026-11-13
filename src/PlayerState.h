#pragma once
//What the player is doing. The state decides what the keys do to his speed and which picture he is drawn with.
enum class PlayerState
{
	//On the ground
	Standing,
	Walking,      //and running: the same rules, faster
	Skidding,     //braking against his own movement
	Rolling,      //curled into a ball along the ground
	Crouching,
	LookingUp,
	//In the air
	Jumping,      //as a ball: from a jump, or from rolling off a ledge
	Falling,      //walked off a ledge
	RampJumping,  //rolled off the end of a ramp, uphill
	Hurt          //thrown back by a hit, with no say in it until he lands
};

