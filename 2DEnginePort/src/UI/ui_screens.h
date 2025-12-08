#pragma once

#include <ui.h>

typedef struct deathScreen
{
	bool active;
	float fadeAlpha;
	float overlaySpeed;
	float survivedSeconds;

	Label survivedLabel;
	Label thanksLabel;

	Button respawnButton;

	bool fullyVisible;
	bool respawnRequested;
	UI* ui;
} deathScreen;

typedef struct Screens
{
	deathScreen deathScreen;
} Screens;

void DeathScreen_Init(deathScreen* ds, UI* ui);
void DeathScreen_Update(deathScreen* ds, float dt);
void DeathScreen_Render(deathScreen* ds);
void DeathScreen_Show(deathScreen* ds, float survivedSeconds);
void DeathScreen_Hide(deathScreen* ds);