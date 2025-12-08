#pragma once

#include <stdbool.h>
#include "ui.h"

typedef struct Entity Entity;
typedef struct Camera Camera;

typedef enum HealthBarType {
	HEALTHBAR_SCREEN,
	HEALTHBAR_WORLD
} HealthBarType;


typedef struct healthBar
{
	float maxHealth;
	float currentHealth;
	float percentage;
	float displayedHealth;

	bool healthBarTop;
	float x, y;
	int width, height;
	int offsetX;
	int offsetY;

	SDL_Rect rect;
	SDL_Color backColor;
	SDL_Color fillColor;
	SDL_Color borderColor;

	Label hpLabel;
	Label percLabel;
	bool shouldBeChanged;

	HealthBarType type;
	bool isVisible;
	bool hasFont;
}healthBar;

bool HealthBar_Init(Entity* entity, UI* ui);
void HealthBar_Free(healthBar* hb);

void HealthBar_Update(Entity* entity, float delatTime);
void HealthBar_Render(healthBar* hb, SDL_Renderer* renderer, Camera* camera);