#include "render.h"
#include "player.h"
#include "gameManager.h"
#include "ui.h"
#include "camera.h"

void render(GameManager *gm)
{
    SDL_SetRenderDrawColor(gm->mainSystems.renderer, 0, 0, 0, 255);
    SDL_RenderClear(gm->mainSystems.renderer);
    renderLevel(gm);
    Player_Render(gm);
    UI_Render(gm);
}