#pragma once

#include "entity.h"
#include "healthbar.h"
#include "camera.h"
#include "GameManager.h"

#include <stdio.h>
#include <SDL_ttf.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

bool HealthBar_Init(Entity* entity, UI* ui)
{
    entity->healthBar.maxHealth = entity->maxHealth;
    entity->healthBar.currentHealth = entity->currentHealth;
    entity->healthBar.percentage = entity->healthBar.currentHealth / entity->healthBar.maxHealth * 100;
    entity->healthBar.isVisible = true;
    entity->healthBar.hasFont = false;
    entity->healthBar.displayedHealth = entity->currentHealth;
    entity->healthBar.shouldBeChanged = true;

    entity->healthBar.hpLabel.wordTexture = NULL;
    entity->healthBar.hpLabel.text = "HP:"; 
    entity->healthBar.percLabel.wordTexture = NULL;
    entity->healthBar.percLabel.text = NULL; 

    for (int i = 0; i < ui->numOfFonts; i++)
    {
        if (strcmp(ui->fonts[i].name, "menuFont") == 0)
        {
            entity->healthBar.hasFont = true;
            entity->healthBar.hpLabel.font = ui->fonts[i];
            entity->healthBar.percLabel.font = ui->fonts[i];
        }
    }

	if (entity->healthBar.type == HEALTHBAR_SCREEN)
	{
        printf("Player healthBar created\n");
        entity->healthBar.healthBarTop = true;
        entity->healthBar.width = (int)(ui->resHeight * 0.2f);
        entity->healthBar.height = 30;
        entity->healthBar.x = (ui->resWidth - entity->healthBar.width) / 2;
        entity->healthBar.y = entity->healthBar.healthBarTop ? 20 : (ui->resHeight - entity->healthBar.height - 20);
	}
	else
	{
        entity->healthBar.x = entity->body.x + entity->healthBar.offsetX;
        entity->healthBar.y = entity->body.y + entity->healthBar.offsetY;
        entity->healthBar.hasFont = false;

        entity->healthBar.width = 40;
        entity->healthBar.height = 12;
        entity->healthBar.offsetX = 5;
        entity->healthBar.offsetY = 15;
	}
}

void HealthBar_Free(healthBar* hb)
{
    if (!hb) return;

    if (hb->hpLabel.wordTexture) SDL_DestroyTexture(hb->hpLabel.wordTexture);
    if (hb->percLabel.wordTexture) SDL_DestroyTexture(hb->percLabel.wordTexture);

    if (hb->percLabel.text) {
        SDL_free(hb->percLabel.text);
        hb->percLabel.text = NULL;
    }
}

void HealthBar_Update(Entity* entity, float deltaTime)
{
	if (!entity || entity->isDead) return;

    int prevHealth = entity->healthBar.percentage;
    entity->healthBar.currentHealth = entity->currentHealth;
    entity->healthBar.percentage = entity->healthBar.currentHealth / entity->healthBar.maxHealth * 100;

    if (prevHealth != entity->healthBar.percentage) {
        entity->healthBar.shouldBeChanged = true;
    }

    // Scroll animation
    float speed = 45.0f;
    float delta = entity->healthBar.currentHealth - entity->healthBar.displayedHealth;
    if (fabs(delta) > 0.01f) {
        float step = speed * deltaTime;
        if (fabs(delta) < step) entity->healthBar.displayedHealth = entity->healthBar.currentHealth;
        else entity->healthBar.displayedHealth += (delta > 0 ? step : -step);
    }

	if (entity->healthBar.type == HEALTHBAR_WORLD)
	{
        int offsetX = entity->healthBar.offsetX;
        if (entity->body.flip == SDL_FLIP_HORIZONTAL)
        {
            offsetX *= -1;
        }

        entity->healthBar.x = entity->body.x + offsetX;
        entity->healthBar.y = entity->body.y - entity->healthBar.offsetY;
	}
}

void HealthBar_Render(healthBar* hb, SDL_Renderer* renderer, Camera* camera)
{
    if (!hb->isVisible || hb->currentHealth <= 0) return;

    float renderX = hb->x;
    float renderY = hb->y;

    if (hb->type == HEALTHBAR_WORLD)
    {
        renderX -= camera->x;
        renderY -= camera->y;
    }

    // HP Label 
    if (hb->hasFont && hb->type == HEALTHBAR_SCREEN) {
        hb->hpLabel.colour = (SDL_Color){255,255,255,255};
        // Place the centered X slightly left
        hb->hpLabel.x = (int)(renderX - 25);
        hb->hpLabel.y = (int)(renderY + (hb->height - hb->hpLabel.font.fontSize) / 2);
        UI_RenderLabel(renderer, &hb->hpLabel);
    }

    // Dynamic Color
    float percent = hb->displayedHealth / hb->maxHealth;
    SDL_Color fillColor;
    if (percent > 0.5f) {
        fillColor.r = (Uint8)(255 * (1.0f - percent) * 2);
        fillColor.g = 255;
        fillColor.b = 0;
    } else {
        fillColor.r = 255;
        fillColor.g = (Uint8)(255 * percent * 2);
        fillColor.b = 0;
    }
    fillColor.a = 255;

    // Bar Rendering
    int fillWidth = (int)(hb->width * percent);
    SDL_Rect bgRect = {(int)renderX, (int)renderY, hb->width, hb->height};
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    SDL_Rect fillRect = {(int)renderX, (int)renderY, fillWidth, hb->height};
    SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
    SDL_RenderFillRect(renderer, &fillRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);

    // Percentage Label
    if (hb->hasFont) {
        if (hb->shouldBeChanged)
        {
            // freeing previous wordTexture
            if (hb->percLabel.wordTexture) {
                SDL_DestroyTexture(hb->percLabel.wordTexture);
                hb->percLabel.wordTexture = NULL;
            }

            // freeing previous allocated text if any
            if (hb->percLabel.text) {
                SDL_free(hb->percLabel.text);
                hb->percLabel.text = NULL;
            }

            char text[8];
            snprintf(text, sizeof(text), "%.0f%%", hb->percentage);
            hb->percLabel.text = SDL_strdup(text);
            hb->percLabel.colour = (SDL_Color){220,220,220,235};

            // Slightly right of the bar
            hb->percLabel.x = (int)(renderX + hb->width + 30);
            hb->percLabel.y = (int)(renderY + (hb->height - hb->percLabel.font.fontSize) / 2);

            hb->shouldBeChanged = false;
        }
        UI_RenderLabel(renderer, &hb->percLabel);
    }
}

