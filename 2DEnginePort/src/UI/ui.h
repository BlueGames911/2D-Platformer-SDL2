#pragma once

#include <SDL_ttf.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

typedef struct GameManager GameManager;
typedef struct Fonts
{
	char* name;
	TTF_Font* font;
	int fontSize;
}Fonts;

typedef struct Label
{
	int x, y;
	char* text;
	SDL_Color colour;
	Fonts font;
	SDL_Texture* wordTexture;
	SDL_Rect wordRect;
}Label;

typedef struct Button
{
	int x, y;
	int width, height;
	Label buttonLabel;

	bool isHovered;
	bool isPressed;

	SDL_Color normal;
	SDL_Color hovered;
	SDL_Color pressed;

	SDL_Texture* Buttontexture;
	SDL_Rect ButtonRect;

	//function onClick();
}Button;

typedef struct UI
{
	Fonts* fonts;
	int numOfFonts;
	int resWidth;
	int resHeight;
	SDL_Texture* fkeyTex; // I wanted to make a whole UI icons system, but for one button, this will do
	SDL_Renderer* renderer;
}UI;

bool UI_Init(const char* path, UI* ui, int resHeight, int resWidth, SDL_Renderer* renderer);
void UI_Free(UI* ui);

void UI_Update(UI* ui);
void UI_Render(GameManager *gm);
void UI_RenderLabel(SDL_Renderer* renderer, Label* label);
