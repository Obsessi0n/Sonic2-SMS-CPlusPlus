#include "Background.h"



Sanic::Background::Background() {
	
	empty = new SDL_Rect();
	empty->x = 0;
	empty->y = 0;
	empty->w = 32;
	empty->h = 32;
	
}


Sanic::Background::~Background() {

	delete empty;
}


void Sanic::Background::DrawBackground(SDL_Renderer* _render) {

	SDL_SetRenderDrawColor(_render, 65, 0, 0, 255);
	SDL_RenderFillRect(_render, empty);
	SDL_RenderClear(_render);
}