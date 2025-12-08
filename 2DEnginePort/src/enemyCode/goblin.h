#pragma once
#include "enemies.h"

typedef enum {
    GOBLIN_STATE_IDLE,
    GOBLIN_STATE_RUN,
    GOBLIN_STATE_MELEE_ATTACK,
    GOBLIN_STATE_THROW_BOMB,
    GOBLIN_STATE_HURT,
    GOBLIN_STATE_DEAD
} GoblinState;

typedef struct GoblinData {
    float attackCooldownTimer;
    float bombCooldownTimer;
    float projectileRange;
    float meleeRange;
    float counterChance;
    float hurtTimer;
    float recordedHitStop;

    bool hasThrownBomb;
    bool isPreparingAttack;
    bool inMeleeRange;
    bool inBombRange;

    GoblinState state;          
    int meleeAttackIndex;       
    int bombAttackIndex;         
} GoblinData;

void Goblin_Init(enemy* e, const char* config_json);
void Goblin_Update(enemy* enemy, float dt, Player* player, Level* lvl);
void Goblin_Attack(enemy* e);
void Goblin_OnHit(enemy* e, int damage);
void Goblin_OnDeath(enemy* e);
void Goblin_Free(enemy* e);

extern const EnemyBehavior GOBLIN_BEHAVIOR;
