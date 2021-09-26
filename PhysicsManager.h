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

		float CalculateGravityForce(float _fallingTimer);
		bool IsColliding(float const& xLiteralPosition, float const& yLiteralPosition);
		char CheckIfGrounded(float const& xLiteralPosition,float const& yLiteralPosition);

	private:
		static PhysicsManager* instance;
		const float GRAVITY = 30;
		const float ACCELERATION = 10.0f;
		const float TERMINALVELOCITY = 2.0f;

		int CalculateSlope(int const& xBlock, int const& yBlock);
		

	};


	typedef PhysicsManager _PhysicsManager;

}



#endif