#include "init.h"
#include "level.h"
#include "player.h"
#include "camera.h"
#include "collision.h"

extern Level *currentLevel;
float moveSpeed;

void loadPlayer(Player *player, struct mainSystems *systems, Level *lvl)
{
    SDL_Surface *surface = IMG_Load("assets/characters/player.png");
    if (!surface)
    {
        fprintf(stderr, "Failed To Load Player\nError: %s\n", IMG_GetError());
        SDL_FreeSurface(surface);
        cleanUp(systems, EXIT_FAILED, currentLevel);
    }
    SDL_Surface *surface2 = IMG_Load("assets/characters/player2.png");
    if (!surface2)
    {
        SDL_FreeSurface(surface);
        SDL_FreeSurface(surface2);
        fprintf(stderr, "Failed To Load Player\nError %s\n", IMG_GetError());
        cleanUp(systems, EXIT_FAILED, currentLevel);
    }

    player->texture = SDL_CreateTextureFromSurface(systems->renderer, surface);
    if (!player->texture) 
    {
        fprintf(stderr, "Failed To Create Texture\nError: %s\n", SDL_GetError());
        cleanUp(systems, EXIT_FAILED, currentLevel);
    }

    player->texture2 = SDL_CreateTextureFromSurface(systems->renderer, surface2);
    if (!player->texture2)
    {
        fprintf(stderr, "Failed To Create Texture\nError: %s\n", SDL_GetError());
        cleanUp(systems, EXIT_FAILED, currentLevel);
    }

    SDL_FreeSurface(surface);
    SDL_FreeSurface(surface2);
    
    player->frameWidth = 50;
    player->frameHeight = 37;
    
    // Initial animation frame
    player->srcRect.x = 0;
    player->srcRect.y = 0;
    player->srcRect.w = player->frameWidth;
    player->srcRect.h = player->frameHeight;
    
    // Destination rect (drawn size, scaled up)
    player->destRect.w = player->frameWidth * 2;
    player->destRect.h = player->frameHeight * 2;

    //spawning the player on a solid tile
    spawnPlayerOnAnyCollidable(player, lvl->spawnColumn, lvl, true);
    
    player->jumpRequested = false;
    player->isCrouching = false;
    

    player->destRect.x = (int)player->x;
    player->destRect.y = (int)player->y;
    

    // Collision rectangle (smaller than drawn sprite for better gameplay feel)
    player->collisionRect.w = player->frameWidth * 2 * 0.3f; // scale and shrink
    player->collisionRect.h = player->frameHeight * 2 * 0.8f;


    player->y_velocity = 0;

    // Animation state
    player->currentFrame = 0;
    player->frameTimer = 0;
    player->frameDuration = 0.15f;
    player->totalFrames = 4;     
    player->animationRow = 0;


    player->originalHeight = player->collisionRect.h;
    player->crouchHeight = player->collisionRect.h - 18;
}

void updatePlayer(Player *player, float deltaTime, Level *lvl)
{   
    player->x_velocity = MOVEMENT_SPEED;
    moveSpeed = player->x_velocity;
    if (player->isCrouching)
        moveSpeed /= CROUCH_LAG; // Slowing down if crouching

    // Moveing left/right
    if (player->isMovingLeft)
    {
        player->x -= moveSpeed * deltaTime;
        player->flip = SDL_FLIP_HORIZONTAL;
    }
    if (player->isMovingRight)
    {
        player->x += moveSpeed * deltaTime;
        player->flip = SDL_FLIP_NONE;
    }

    // Updating collision rect for X movement and checking collisions
    float collisionBox_X = player->x + ((player->destRect.w - player->collisionRect.w) / 2);

    //Adjusting the hitbox based on the player's running sprites
    if (player->isMovingLeft) 
    {
        collisionBox_X -= 7;
    }
    else if (player->isMovingRight) 
    {
        collisionBox_X += 7;
    }
    player->collisionRect.x = collisionBox_X;
    checkPlayerTileCollisionsX(player, lvl);

    // Clamping player to map bounds
    int mapWidth = lvl->levelColumns * TILE_SIZE * TILE_SCALE;
    if (player->x < 0) player->x = 0;
    if (player->x > mapWidth - player->destRect.w)
        player->x = mapWidth - player->destRect.w;
    player->destRect.x = (int)player->x;

    player->y_velocity += GRAVITY_FORCE * deltaTime;

    if (player->jumpRequested && player->isOnGround) 
    {
        player->y_velocity = -JUMP_FORCE;
        player->isOnGround = false;
        player->jumpRequested = false;
    }
     
    //vertical movement
    player->y += player->y_velocity * deltaTime;
    player->collisionRect.y = player->y + (player->destRect.h - player->collisionRect.h);
    checkPlayerTileCollisionsY(player, lvl);

    // Clamping fall speed
    if (player->y_velocity > MAX_FALL_SPEED)
        player->y_velocity = MAX_FALL_SPEED;

    player->destRect.y = (int)(player->y - cameraY);

    //Height Calculation for collisionRect when crouching
    if (player->isCrouching && player->isOnGround) 
    {
        player->collisionRect.h = player->crouchHeight;
    }
    else {
        player->collisionRect.h = player->originalHeight;
    }


    //Adjusting the attacking Hitbox based on the player's attacking stage
    if (player->isAttacking) 
    {
        player->attackRect1.y = player->collisionRect.y + 3;
        if (player->flip) 
        {
            player->attackRect1.x = player->collisionRect.x - 33;
        }
        else {
            player->attackRect1.x = player->collisionRect.x + 33;
        }
        if (player->attackStage == 3) 
        {
            player->attackRect1.y = player->collisionRect.y +15;
            player->attackRect1.w = 35; 
            player->attackRect1.h = 40;
        }
        else {
            player->attackRect1.w = 35; 
            player->attackRect1.h = 50;
        }
    }
    else 
    {
        player->attackRect1.x = 0;
        player->attackRect1.y = 0;
        player->attackRect1.w = 0;
        player->attackRect1.h = 0;
    }
    
    static int lastAnimationRow = -1;

    // Jumping animation (player moving up)
    if (!player->isOnGround && player->y_velocity < 0) {
        player->animationRow = 2; // 3rd row
        player->totalFrames = 4;
        player->frameDuration = 0.095f;
        player->srcRect.x = player->currentFrame * player->frameWidth;
        player->srcRect.y = player->animationRow * player->frameHeight;

        // Reset frame if animation changed
        if (player->animationRow != lastAnimationRow) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = player->animationRow;

        updateAnimation(player, deltaTime);
        return;
    }

    // Attacking animation 
    if (player->isAttacking && player->isOnGround) {
        // Locking movement and other actions during attack
        player->isMovingLeft = false;
        player->isMovingRight = false;
        player->isCrouching = false;
        player->jumpRequested = false;

        // Attack frame definitions 
        int atk1X[] = {0, 1, 2, 3, 4};
        int atk1Y[] = {6, 6, 6, 6, 6};
        int atk2X[] = {5, 6, 0, 1, 2, 3};
        int atk2Y[] = {6, 6, 7, 7, 7, 7};
        int atk3X[] = {4, 5, 6, 0, 1, 2};
        int atk3Y[] = {7, 7, 7, 8, 8, 8};

        int *atkX, *atkY, totalFrames;

        // Select attack stage
        switch (player->attackStage) {
            case 1:
                atkX = atk1X; atkY = atk1Y; totalFrames = 5;
                player->frameDuration = 0.06f;
                player->attackWindow = 0.3f; // Combo input window
                break;
            case 2:
                atkX = atk2X; atkY = atk2Y; totalFrames = 6;
                player->frameDuration = 0.06f;
                player->attackWindow = 0.25f;
                break;
            case 3:
                atkX = atk3X; atkY = atk3Y; totalFrames = 6;
                player->frameDuration = 0.06f;
                player->attackWindow = 0.25f;
                break;
        }

        player->srcRect.x = atkX[player->currentFrame] * player->frameWidth;
        player->srcRect.y = atkY[player->currentFrame] * player->frameHeight;

        // Advance frame
        player->frameTimer += deltaTime;
        player->attackTimer += deltaTime;

        if (player->frameTimer >= player->frameDuration) {
            player->currentFrame++;
            player->frameTimer = 0;
        }

        // Combo transitioning
        if (player->currentFrame >= totalFrames) {
            if (player->attackQueued && player->attackStage < 3) {
                // Advance to next combo stage
                player->attackStage++;
                player->currentFrame = 0;
                player->attackTimer = 0;
                player->attackQueued = false;
            } else {
                // End attack
                player->isAttacking = false;
                player->attackStage = 0;
                player->currentFrame = 0;
                player->frameTimer = 0;
            }
        }

        return; // Skip other movement/animations
    }

    // Falling animation
    if (!player->isOnGround)
    {
        player->animationRow = 3;
        player->totalFrames = 2;
        int startColumn = 1;
        player->frameDuration = 0.09f;
        player->srcRect.x = (startColumn + player->currentFrame) * player->frameWidth;
        player->srcRect.y = player->animationRow * player->frameHeight;

        if (player->animationRow != lastAnimationRow) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = player->animationRow;

        updateAnimation(player, deltaTime);
        return;
    }

    // Crouch-walk animation
    if (player->isCrouching && (player->isMovingLeft || player->isMovingRight))
    {
        player->totalFrames = 5;
        player->frameDuration = 0.15f;

        int crouchWalkX[] = {3, 4, 5, 6, 0};
        int crouchWalkY[] = {7, 7, 7, 7, 8};

        player->srcRect.x = crouchWalkX[player->currentFrame] * player->frameWidth;
        player->srcRect.y = crouchWalkY[player->currentFrame] * player->frameHeight;

        // Reset frame if animation changed
        if (lastAnimationRow != ANIM_CROUCH_WALK) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = -3; // Unique value for crouch-walk

        updateAnimation(player, deltaTime);
        return; // Skip other animations
    }

    // Crouch animation 
    if (player->isCrouching && player->isOnGround) {
        player->animationRow = -1;
        player->totalFrames = 4;
        player->frameDuration = 0.15f;

        // Frame definitions for crouch
        int crouchX[] = {4, 5, 6, 0};
        int crouchY[] = {0, 0, 0, 1};

        player->srcRect.x = crouchX[player->currentFrame] * player->frameWidth;
        player->srcRect.y = crouchY[player->currentFrame] * player->frameHeight;

        // Reset frame if animation changed
        if (lastAnimationRow != ANIM_CROUCH) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = -2; // Unique value for crouch

        updateAnimation(player, deltaTime);
        return; // Skip other animations
    }

    // Running animation
    if ((player->isMovingLeft || player->isMovingRight) && player->isOnGround) {
        player->animationRow = 1;
        player->totalFrames = 6;
        int startColumn = 1;
        player->frameDuration = 0.09f;
        player->srcRect.x = (startColumn + player->currentFrame) * player->frameWidth;
        player->srcRect.y = player->animationRow * player->frameHeight;

        // Reset frame if animation changed
        if (player->animationRow != lastAnimationRow) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = player->animationRow;

        updateAnimation(player, deltaTime);
    }
    // Idle animation
    else if (player->isOnGround && !(player->isMovingLeft || player->isMovingRight))
    {
        player->animationRow = 0;
        player->totalFrames = 4;
        player->frameDuration = 0.15f;
        player->srcRect.x = player->currentFrame * player->frameWidth;
        player->srcRect.y = player->animationRow * player->frameHeight;

        // Reset frame if animation changed
        if (player->animationRow != lastAnimationRow) {
            player->currentFrame = 0;
            player->frameTimer = 0;
        }
        lastAnimationRow = player->animationRow;

        updateAnimation(player, deltaTime);
    }
}

void renderPlayer(Player *player, struct mainSystems *systems)
{
    //Visually presenting the player's collisionBox/Hitbox
    SDL_Rect visualCollision;
    visualCollision.x = player->collisionRect.x - cameraX;
    visualCollision.y = player->collisionRect.y - cameraY;
    
    visualCollision.w = player->collisionRect.w;
    visualCollision.h = player->collisionRect.h;

    //Visually presenting the player's attacking HitBox
    SDL_Rect VissualAttacks;
    VissualAttacks.x = player->attackRect1.x - cameraX;
    VissualAttacks.y = player->attackRect1.y - cameraY;

    VissualAttacks.w = player->attackRect1.w;
    VissualAttacks.h = player->attackRect1.h;

    SDL_Rect *Rect = 0;

    // Using alternate texture for crouch-walk, its in a different imag
    if (player->isCrouching && (player->isMovingLeft || player->isMovingRight))
    {
        player->destRect.x = (int)(player->x - cameraX);
        player->destRect.y = (int)(player->y - cameraY);
        SDL_RenderCopyEx(systems->renderer, player->texture2, &player->srcRect, &player->destRect, 0, NULL, player->flip);
    }
    else
    {
        player->destRect.x = (int)(player->x - cameraX);
        player->destRect.y = (int)(player->y - cameraY);
        SDL_RenderCopyEx(systems->renderer, player->texture, &player->srcRect, &player->destRect, 0, NULL, player->flip);
    }

    SDL_SetRenderDrawColor(systems->renderer, 255, 0, 0, 255); // Red: sprite bounds
    SDL_RenderDrawRect(systems->renderer, &player->destRect);
    SDL_SetRenderDrawColor(systems->renderer, 0, 0, 255, 255); // Blue: collision bounds
    SDL_RenderDrawRect(systems->renderer, &visualCollision);
    SDL_SetRenderDrawColor(systems->renderer, 0, 255, 100, 255);
    SDL_RenderDrawRect(systems->renderer, &VissualAttacks);
}

void updateAnimation(Player *player, float deltaTime)
{
    player->frameTimer += deltaTime;
    if (player->frameTimer >= player->frameDuration) {
        player->currentFrame = (player->currentFrame + 1) % player->totalFrames;
        player->frameTimer = 0;
    }
}

void destroyPlayer(Player *player)
{
    SDL_DestroyTexture(player->texture);
    SDL_DestroyTexture(player->texture2);
}
