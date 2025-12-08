#pragma once

#include "SDL_render.h"
#include <SDL.h>
#include <SDL_image.h>
#include "SDL_surface.h"
#include "constants.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct Level Level;
struct mainSystems;

typedef struct Player {
    SDL_Texture* texture;
    SDL_Texture *texture2;      
    SDL_Rect srcRect;           
    SDL_Rect destRect;          
    int frameWidth, frameHeight;
    int currentFrame;
    int totalFrames;
    int animationRow;           
    float frameTimer;           
    float frameDuration;        
    float x, y;
    int x_velocity;
    int y_velocity; 
    bool isMovingRight, isMovingLeft;   
    bool isOnGround;    
    bool jumpRequested;  // set by input, consumed by update 
    bool isCrouching;
    float originalHeight;
    float crouchHeight;
    bool isAttacking;
    int attackStage;          
    bool attackQueued;        // Player input a follow-up attack
    float attackTimer;        // How long current attack has played
    float attackWindow;       // Time after which I allow chaining
    SDL_RendererFlip flip; 
    SDL_Rect collisionRect;
    SDL_Rect attackRect1;           
} Player;

void loadPlayer(Player *player, struct mainSystems *systems, Level *lvl);
void updatePlayer(Player *player, float deltaTime, Level *lvl);
void renderPlayer(Player *player, struct mainSystems *systems);
void updateAnimation(Player *player, float deltaTime);
void destroyPlayer(Player *player);