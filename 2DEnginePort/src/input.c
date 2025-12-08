#include "input.h"
#include "init.h"
#include "gameManager.h"
#include "player.h"
#include "level.h"

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

MouseState gMouseState;


void processInput(SDL_Event *event, GameManager *gm)
{
    if (!gm || !event) return;

    SDL_GetMouseState(&gMouseState.x, &gMouseState.y);
    gMouseState.leftPressed = false;
    gMouseState.leftReleased = false;


    // Handle all SDL events
    while (SDL_PollEvent(event))
    {
        switch (event->type)
        {
            case SDL_QUIT:
                gm->running = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    gMouseState.leftDown = true;
                    gMouseState.leftPressed = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event->button.button == SDL_BUTTON_LEFT) {
                    gMouseState.leftDown = false;
                    gMouseState.leftReleased = true;
                }
                break;

            case SDL_KEYDOWN:
                switch (event->key.keysym.scancode)
                {
                    case SDL_SCANCODE_ESCAPE:
                        /* Toggle pause state
                        if (gm->state == GAMESTATE_RUNNING)
                            gm->state = GAMESTATE_PAUSED;
                        else if (gm->state == GAMESTATE_PAUSED)
                            gm->state = GAMESTATE_RUNNING;
                        break;
                        */
                        gm->running = false;

                    case SDL_SCANCODE_F11:
                        {
                            Uint32 flags = SDL_GetWindowFlags(gm->mainSystems.window);
                            if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                            {
                                SDL_SetWindowFullscreen(gm->mainSystems.window, 0);
                                gm->settings.video.fullscreen = false;
                            }
                            else
                            {
                                SDL_SetWindowFullscreen(gm->mainSystems.window, SDL_WINDOW_FULLSCREEN);
                                gm->settings.video.fullscreen = true;
                            }
                        }
                        break;

                    case SDL_SCANCODE_F1:
                        gm->settings.gameplay.debugMode = !gm->settings.gameplay.debugMode;
                        break;
                    default:
                        if (event->key.keysym.scancode == SDL_SCANCODE_F4 &&
                            (SDL_GetModState() & KMOD_ALT)) {
                            gm->running = false;
                        }
                        break;
                }
                break;

            default:
                break;
        }
    }

    if (gm->state == GAMESTATE_RUNNING)
    {
        if (gm->player != NULL && !gm->player->core.isDead && !gm->player->isHurt)
        {
            Player_HandleInput(gm->player);
            if (gm->level->trigger.enabled && gm->level->trigger.playerInRange)
            {
                const Uint8* keys = SDL_GetKeyboardState(NULL);
                if (keys[SDL_SCANCODE_F]) {
                    fprintf(stderr, "[TRIGGER] Loading level: %s\n",
                        gm->level->trigger.targetLevel);
                    GameManager_LoadLevel(gm, gm->level->trigger.targetLevel);
                    gm->level->trigger.playerInRange = false;
                }
            }
        }
    }
}
