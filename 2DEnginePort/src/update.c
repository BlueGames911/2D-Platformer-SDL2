#include "update.h"
#include "camera.h"
#include "player.h"
#include "gameManager.h"
#include "level.h"
#include "enemies.h"

void Update(GameManager *gm)
{
    Camera_Update(&gm->camera, gm->player, gm->level);
    Level_Update(gm);
    UpdateAllEnemies(gm->level, gm->deltaTime, gm->player);
    Player_Update(gm->player, gm->deltaTime, gm->level);
}