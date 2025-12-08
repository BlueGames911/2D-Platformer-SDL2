#pragma once

#include "collision.h"
#include "animation.h"
#include "entity.h"

#include <SDL.h>
#include <stdbool.h>

typedef struct UI UI;
typedef struct enemy enemy;
typedef struct Level Level;
struct mainSystems;
typedef struct GameSettings GameSettings;
typedef struct Camera Camera;
typedef struct GameManager GameManager;


// Player States

typedef enum {
    PLAYER_IDLE,
    PLAYER_RUNNING,
    PLAYER_JUMPING,
    PLAYER_FALLING,
    PLAYER_CROUCH,
    PLAYER_CROUCH_WALKING,
    PLAYER_GROUND_ATTACKING,
    PLAYER_AIR_ATTACKING,
    PLAYER_ROLLING,
    PLAYER_HURT,
    PLAYER_DIE,
    PLAYER_STATE_COUNT
} PlayerState;

typedef struct {
    SDL_Scancode moveLeft;
    SDL_Scancode moveRight;
    SDL_Scancode jump;
    SDL_Scancode attack;
    SDL_Scancode crouch;
    SDL_Scancode dash;
} Controls;


// Player Entity

typedef struct Player {
    //Entity
    Entity core;

    // State
    PlayerState state;
    bool isCrouching;
    bool jumpRequested;
    bool isDashing;
    bool comboExtendRequested;
    float attackCoolDown;

    bool isHurt;
    float hurtTimer;
    float onDeathDelay;
    bool deathFinished;
    float fadeAlpha;
    bool fadeStarted;

    // Later
    //int stateToAnim[PLAYER_STATE_COUNT];
    Controls controls;

    // Physics config
    float walkSpeed;
    float crouchHeightOffset;
    float crouchLag;
    float crouchSpeed;


    // Audio (future use)
    char** soundMap;       // array of sound file paths per animation
    int soundCount;
} Player;


// Lifecycle

bool Player_LoadConfig(Player* player, struct mainSystems* systems, const char* filePath);
void Player_Destroy(Player* player);

// Main loop

void Player_Update(Player* player, float deltatime, Level* lvl);
void Player_Render(GameManager *gm);

// updates

void Player_HandleInput(Player* player);
void Player_UpdatePhysics(Player* player, float deltaTime, Level* lvl);
void Player_UpdateAnimation(Player* player, float deltaTime);
void Player_UpdateCollisionBox(Player* player, Level* lvl);
void Player_CheckAttackHits(Player* player, enemy* enemy, float deltaTime);
void Player_OnHit(Player* player, enemy* enemy, float damage, float knockBack);
void Player_OnDeath(Player* player);


// State control

void Player_SetState(Player* player, PlayerState state);
