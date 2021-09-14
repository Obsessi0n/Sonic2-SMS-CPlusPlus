#pragma once
namespace Sanic {
	class Interface
	{
	public:
		Interface();
		void TopUI();
		void BottomUI();
		void Render();
	private:
		int m_width = 16;
		int m_height = 16;
	};
}


