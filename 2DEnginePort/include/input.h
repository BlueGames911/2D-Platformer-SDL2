#pragma once
#include "gameManager.h"

#include <SDL.h>

typedef struct MouseState
{
	int x, y;
	bool leftDown;
	bool leftPressed;
	bool leftReleased;
} MouseState;

extern MouseState gMouseState;

void processInput(SDL_Event *event, GameManager *gm);


