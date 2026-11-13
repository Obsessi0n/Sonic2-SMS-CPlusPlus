#pragma once
namespace Sanic {
	class UserInterface
	{
	public:
		UserInterface();
		void TopUI();
		void BottomUI();
		void Render();
	private:
		int m_width = 16;
		int m_height = 16;
		void DrawRings(const int &xPosition, const int &yPosition);
		void DrawTimer(const int &xPosition, const int &yPosition);
	};
}


