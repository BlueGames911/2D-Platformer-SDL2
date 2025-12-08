#include "gameManager.h"
#include "settings.h"
#include "init.h"
#include "player.h"
#include "level.h"
#include "input.h"
#include "update.h"
#include "render.h"
#include "camera.h"
#include "ui.h"
#include "ui_screens.h"

#include <stdio.h>
#include <string.h>

GameManager *GameManager_Create(const char *settingsPath, const char *windowName) {
    GameManager *gm = calloc(1, sizeof(GameManager));
    if (!gm) {
        fprintf(stderr, "Failed to allocate GameManager\n");
        return NULL;
    }

    gm->windowName = _strdup(windowName ? windowName : "Game v0.3");

    // Load settings
    printf("Loading settings\n");
    if (!Settings_Init(&gm->settings, settingsPath))
    {
        fprintf(stderr, "Failed to load settings\n");
        GameManager_Destroy(gm);
        return NULL;
    }
    printf("Intializing systems\n");

    // Initialize SDL, window, renderer
    if (!initalizeSystems(gm)) {
        fprintf(stderr, "Failed to initialize systems\n");
        GameManager_Destroy(gm);
        return NULL;
    }

    printf("Intializing UI\n");
    gm->ui = calloc(1, sizeof(UI));
    if (!UI_Init("ui.json", gm->ui, gm->settings.video.height, gm->settings.video.width, gm->mainSystems.renderer))
    {
        fprintf(stderr, "Failed to initialize UI\n");
        GameManager_Destroy(gm);
        return NULL;
    }

    gm->screens = calloc(1, sizeof(Screens));


    // TODO: Create texture cache
    // gm->cache = TextureCache_Create(...);

    printf("Loading Player\n");
    // Create and load player
    gm->player = calloc(1, sizeof(Player));
    if (!gm->player || !Player_LoadConfig(gm->player, &gm->mainSystems, "player.json")) {
        fprintf(stderr, "Failed to load player\n");
        GameManager_Destroy(gm);
        return NULL;
    }

    // Load level paths
    gm->levelPaths = levelPaths("levelPaths.json");
    if (!gm->levelPaths) {
        fprintf(stderr, "Failed to load level paths\n");
        GameManager_Destroy(gm);
        return NULL;
    }

    // Load default level
    if (!GameManager_LoadLevel(gm, gm->levelPaths->defaultLevel)) {
        fprintf(stderr, "Failed to load default level\n");
        GameManager_Destroy(gm);
        return NULL;
    }

    Camera_Init(&gm->camera, gm->settings.video.width, gm->settings.video.height);
    Settings_ApplyControls(gm->player, &gm->settings.controls);
    printf("Applied controls\n");
    gm->running = true;
    gm->state = GAMESTATE_RUNNING;
    gm->gameData.survivedSeconds = 0;
    return gm;
}


void GameManager_Destroy(GameManager *gm) {
    if (!gm) return;

    fprintf(stderr, "Destroying GameManager\n");
    // Destroy player
    if (gm->player) {
        Player_Destroy(gm->player);
        free(gm->player);
    }
    printf("Freed Player\n");

    // Unload level
    if (gm->level) {
        GameManager_UnloadLevel(gm);
    }
    printf("Unloaded Level\n");

    // Free level paths
    if (gm->levelPaths) {
        freeLevelPaths(gm->levelPaths);
    }
    printf("Freed Level paths\n");

    UI_Free(gm->ui);
    printf("Cleaned up UI\n");

    // Destroy SDL systems
    cleanUp(gm);
    printf("Cleaned up Systems\n");

    // Free window name
    free((char*)gm->windowName);
    printf("Freed Window Name\n");

    // Free GameManager
    free(gm);
    printf("Freeed GameManager\n");
}

void GameManager_HandleInput(GameManager *gm) {
    if (!gm) return;

    SDL_Event event;

    processInput(&event, gm);

}

void GameManager_Update(GameManager *gm, float deltaTime) {
    if (!gm) return;
    gm->deltaTime = deltaTime;

    GameState newState = GAMESTATE_RUNNING;

    if (gm->player->deathFinished)
    {
        newState = GAMESTATE_DEATHSCREEN;
    }
    else if (gm->paused)
    {
        newState = GAMESTATE_PAUSED;
    }

    if (newState == GAMESTATE_DEATHSCREEN)
    {
        if (!gm->screens->deathScreen.active)
        {
            DeathScreen_Init(&gm->screens->deathScreen, gm->ui);
            DeathScreen_Show(&gm->screens->deathScreen, gm->gameData.survivedSeconds);
        }
        else
        {
            DeathScreen_Update(&gm->screens->deathScreen, deltaTime);
            if (gm->screens->deathScreen.respawnRequested)
            {
                DeathScreen_Hide(&gm->screens->deathScreen);
                GameManager_Respawn(gm);
            }
        }
    }
    else
    {
        gm->gameData.survivedSeconds += deltaTime;
        Update(gm);
    }

}

void GameManager_SetState(GameManager* gm, GameState state)
{

}

void GameManager_Render(GameManager *gm) {
    if (!gm) return;

    render(gm);

    if (gm->screens->deathScreen.active)
    {
        DeathScreen_Render(&gm->screens->deathScreen);
    }

    SDL_RenderPresent(gm->mainSystems.renderer);

}

bool GameManager_LoadLevel(GameManager *gm, const char *levelName) {
    if (!gm || !levelName) return false;

    // Find the level path
    const char *foundPath = NULL;
    for (int i = 0; i < gm->levelPaths->count; i++) {
        if (strcmp(gm->levelPaths->levels[i].name, levelName) == 0) {
            foundPath = gm->levelPaths->levels[i].path;
            break;
        }
    }

    if (!foundPath) {
        fprintf(stderr, "GameManager_LoadLevel: Level '%s' not found\n", levelName);
        return false;
    }

    // Unload current 
    if (gm->level) {
        GameManager_UnloadLevel(gm);
    }

    // Load the level from JSON
    gm->level = loadLevelFromJSON(foundPath, gm);
    if (!gm->level) {
        fprintf(stderr, "GameManager_LoadLevel: Failed to load '%s'\n", levelName);
        return false;
    }

    // Update current level name
    free(gm->currentLevelName);
    gm->currentLevelName = _strdup(levelName);

    spawnPlayerOnAnyCollidable(gm->player, gm->level->spawnColumn, gm->level, true, gm->ui);

    return true;
}

void GameManager_Respawn(GameManager* gm)
{
    printf("Respawning\n");

    gm->player->deathFinished = false;
    spawnPlayerOnAnyCollidable(gm->player, gm->level->spawnColumn, gm->level, true, gm->ui);
    for (int i = 0; i < gm->level->enemyCount; i++)
    {
        enemy* e = gm->level->enemies[i];
        Enemy_Respawn(gm->level, e, gm->ui);
    }
}


void GameManager_UnloadLevel(GameManager *gm) {
    if (!gm || !gm->level) return;

    unloadLevel(gm->level);
    free(gm->level);
    gm->level = NULL;
    printf("Freed Level");

    free(gm->currentLevelName);
    gm->currentLevelName = NULL;

}