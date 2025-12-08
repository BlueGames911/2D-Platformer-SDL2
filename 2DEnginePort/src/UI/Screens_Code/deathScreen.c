#include "ui_screens.h"
#include "collision.h"
#include "input.h"

#include <string.h>
#include <stdio.h>

void DeathScreen_Init(deathScreen* ds, UI* ui)
{
	ds->ui = ui;
	bool fontFound = false;
	for (int i = 0; i < ds->ui->numOfFonts; i++)
	{
		if (strcmp(ds->ui->fonts[i].name, "menuFont") == 0)
		{
			ds->survivedLabel.font = ds->ui->fonts[i];
			ds->thanksLabel.font = ds->ui->fonts[i];
			ds->respawnButton.buttonLabel.font = ds->ui->fonts[i];
			fontFound = true;
			break;
		}
	}

	if (!fontFound)
	{
		fprintf(stderr, "[DEATH_SCREEN] Font could not be found\n");
	}

	ds->active = false;
	ds->fullyVisible = false;
	ds->fadeAlpha = 0;
	ds->overlaySpeed = 250;
	ds->survivedSeconds = 0;

	ds->survivedLabel.x = (ds->ui->resWidth / 2);
	ds->survivedLabel.y = (ds->ui->resHeight * 0.3);
	ds->survivedLabel.colour = (SDL_Color){ 255, 255, 255, 255 };
	ds->survivedLabel.wordTexture = NULL;

	ds->thanksLabel.x = (ds->ui->resWidth / 2);
	ds->thanksLabel.y = ds->survivedLabel.y + 100;
	ds->thanksLabel.text = "Thank you for playing\n		Game by MC TheOne";
	ds->thanksLabel.colour = (SDL_Color){ 211, 211, 211, 255 };
	ds->thanksLabel.wordTexture = NULL;

	// Button sizing/positioning
	ds->respawnButton.width = (int)(ds->ui->resWidth * 0.2f);
	ds->respawnButton.height = (int)(ds->ui->resHeight * 0.06f);
	ds->respawnButton.x = (ds->ui->resWidth / 2) - (ds->respawnButton.width / 2);
	ds->respawnButton.y = ds->thanksLabel.y + 100;
	ds->respawnButton.buttonLabel.text = "Respawn";
	ds->respawnButton.isHovered = false;
	ds->respawnButton.isPressed = false;
	ds->respawnButton.Buttontexture = NULL;

	ds->respawnButton.normal = (SDL_Color){ 40, 40, 40, 255 };   // dark gray
	ds->respawnButton.hovered = (SDL_Color){ 80, 80, 80, 255 };  // lighter
	ds->respawnButton.pressed = (SDL_Color){ 20, 20, 20, 255 };  // darker when pressed
}

void DeathScreen_Show(deathScreen* ds, float survivedSeconds)
{
	ds->active = true;
	ds->survivedSeconds = survivedSeconds;
	static char survivedText[64];
	snprintf(survivedText, sizeof(survivedText), "You survived %.1f seconds", ds->survivedSeconds);
	ds->survivedLabel.text = survivedText;

}

void DeathScreen_Hide(deathScreen* ds)
{
	ds->active = false;
	ds->survivedSeconds = 0;
	ds->respawnRequested = false;
	ds->fullyVisible = false;
	ds->fadeAlpha = 0;

	if (ds->thanksLabel.wordTexture) SDL_DestroyTexture(ds->thanksLabel.wordTexture);
	if (ds->survivedLabel.wordTexture) SDL_DestroyTexture(ds->survivedLabel.wordTexture);
	if (ds->respawnButton.Buttontexture) SDL_DestroyTexture(ds->respawnButton.Buttontexture);
}

void DeathScreen_Update(deathScreen* ds, float dt)
{
	if (!ds->active) return;

	// Fade in
	if (!ds->fullyVisible)
	{
		ds->fadeAlpha += ds->overlaySpeed * dt;

		if (ds->fadeAlpha >= 205)
		{
			ds->fadeAlpha = 205;
			printf("Death Screen is now fully visble\n");
			ds->fullyVisible = true;
		}

		return;
	}

	SDL_Rect mouseRect = { gMouseState.x, gMouseState.y, 5, 5 };
	SDL_Rect buttonRect = { ds->respawnButton.x, ds->respawnButton.y,
							ds->respawnButton.width, ds->respawnButton.height };

	// Hover detection
	if (collisionCheck(mouseRect, buttonRect))
		ds->respawnButton.isHovered = true;
	else
		ds->respawnButton.isHovered = false;

	// Reset press state when leaving button
	if (!ds->respawnButton.isHovered)
		ds->respawnButton.isPressed = false;

	// Press
	if (ds->respawnButton.isHovered && gMouseState.leftPressed)
		ds->respawnButton.isPressed = true;

	// Release (valid click)
	if (ds->respawnButton.isHovered &&
		ds->respawnButton.isPressed &&
		gMouseState.leftReleased)
	{
		ds->respawnRequested = true;
	}
}

void DeathScreen_Render(deathScreen* ds)
{
	if (!ds || !ds->active)
		return;

	// Fade overlay
	SDL_SetRenderDrawBlendMode(ds->ui->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(ds->ui->renderer, 0, 0, 0, (Uint8)ds->fadeAlpha);

	SDL_Rect overlay = {
		0,
		0,
		ds->ui->resWidth,
		ds->ui->resHeight
	};

	SDL_RenderFillRect(ds->ui->renderer, &overlay);

	UI_RenderLabel(ds->ui->renderer, &ds->survivedLabel);
	UI_RenderLabel(ds->ui->renderer, &ds->thanksLabel);

	// Colour based on state
	SDL_Color color;

	if (ds->respawnButton.isPressed)
		color = ds->respawnButton.pressed;
	else if (ds->respawnButton.isHovered)
		color = ds->respawnButton.hovered;
	else
		color = ds->respawnButton.normal;

	SDL_SetRenderDrawColor(ds->ui->renderer, color.r, color.g, color.b, color.a);

	SDL_Rect btnRect = {
		ds->respawnButton.x,
		ds->respawnButton.y,
		ds->respawnButton.width,
		ds->respawnButton.height
	};

	SDL_RenderFillRect(ds->ui->renderer, &btnRect);

	if (!ds->respawnButton.Buttontexture)
	{
		SDL_Surface* bSurf = TTF_RenderText_Blended(
			ds->respawnButton.buttonLabel.font.font,
			ds->respawnButton.buttonLabel.text,
			(SDL_Color){ 255, 255, 255, 255 }
		);
		if (bSurf)
		{
			ds->respawnButton.Buttontexture = SDL_CreateTextureFromSurface(ds->ui->renderer, bSurf);

			SDL_Rect txtDst;
			txtDst.w = bSurf->w;
			txtDst.h = bSurf->h;

			txtDst.x = ds->respawnButton.x + (ds->respawnButton.width - txtDst.w) / 2;
			txtDst.y = ds->respawnButton.y + (ds->respawnButton.height - txtDst.h) / 2;
			ds->respawnButton.ButtonRect = txtDst;

			SDL_FreeSurface(bSurf);

		}
	}
	SDL_RenderCopy(ds->ui->renderer, ds->respawnButton.Buttontexture, NULL, &ds->respawnButton.ButtonRect);

}
