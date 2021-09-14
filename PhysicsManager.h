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
		bool IsColliding(int x, int y);

	private:
		static PhysicsManager* instance;
		const float GRAVITY = 9.80665f;
		const float ACCELERATION = 10.0f;
		const float TERMINALVELOCITY = 2.0f;
		

	};


	typedef PhysicsManager _PhysicsManager;

}



#endif