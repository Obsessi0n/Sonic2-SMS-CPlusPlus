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
		bool CheckIfGrounded(float const& xLiteralPosition,float const& yLiteralPosition);
		int CalculateSlope(int const& xBlock, int const& yBlock);
		int CalculateSlope(float const& xLiteralPos, float const& yLiteralPos);
		bool CheckIfSlope(float const& xLiteralPos, float const& yLiteralPos);
	private:
		static PhysicsManager* instance;
		const float GRAVITY = 30;
		const float ACCELERATION = 10.0f;
		const float TERMINALVELOCITY = 2.0f;


		

	};


	typedef PhysicsManager _PhysicsManager;

}



#endif