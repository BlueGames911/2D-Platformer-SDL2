#include "player.h"
#include "level.h"
#include "entity.h"  
#include "goblin.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


void Goblin_Init(enemy* e, const char* config_json)
{
    if (!e) return;
    GoblinData* data = calloc(1, sizeof(GoblinData));
    e->behaviorData = data;
    e->newState = GOBLIN_STATE_IDLE;
    data->attackCooldownTimer = 1.0f;
    data->bombCooldownTimer = 10.0f;
    data->meleeRange = e->attackRange;           
    data->projectileRange = 300.0f;
    data->state = GOBLIN_STATE_IDLE;
    data->hurtTimer = 0.0f;
    e->tookHit = false;

    // find attack indices by name (if attacks loaded)
    data->meleeAttackIndex = Entity_FindAttack(&e->core, "slash");
    data->bombAttackIndex = Entity_FindAttack(&e->core, "bomb");

    printf("[Goblin] Initialized.\n");
}

void Goblin_Update(enemy* enemy, float dt, Player* player, Level* lvl)
{
    GoblinData* data = (GoblinData*)enemy->behaviorData;

    if (!enemy->core.isDead)
    {

        // If in hurt state, decrement timer and block other logic
        if (data->state == GOBLIN_STATE_HURT || enemy->newState == GOBLIN_STATE_HURT) {
            if (data->hurtTimer > 0.0f) {
                data->hurtTimer -= dt;
                if (data->attackCooldownTimer > 0.0f) {
                    data->attackCooldownTimer -= dt;
                }
                enemy->newState = GOBLIN_STATE_HURT;
                enemy->isMoving = false;
                enemy->isChasing = false;
                enemy->isPatrolling = false;
                enemy->tookHit = true;

                Animation* anim = Entity_GetCurrentAnimation(&enemy->core);
                if (strcmp(anim->name, "hurt") == 0)
                {
                    if (enemy->core.currentFrame == 2)
                    {
                        enemy->core.hitStopTimer = data->recordedHitStop;
                    }
                }
                return;
            } else {
                enemy->core.body.velocity_x = 0;
                enemy->isMoving = false;
                enemy->isChasing = false;
                enemy->isPatrolling = false;
                enemy->tookHit = false;
                enemy->lastHitAttackIndex = -1;
                enemy->lastHitAttackStage = -1;
                if (enemy->core.currentHealth <= 0) {
                    enemy->newState = GOBLIN_STATE_DEAD;
                } else {
                    enemy->newState = GOBLIN_STATE_IDLE;
                }
            }
        }

        bool meleeTimerFinished = false;
        bool bombTimerFinished = false;

        if (data->attackCooldownTimer > 0.0f) {
            data->attackCooldownTimer -= dt;
        }
        else
        {
            meleeTimerFinished = true;
        }

        if (data->bombCooldownTimer > 0.0f) {
            data->bombCooldownTimer -= dt;
        }
        else
        {
            bombTimerFinished = true;
        }

        enemy->newState = GOBLIN_STATE_IDLE;

        bool inMeleeRange = false; 
        if (enemy->distanceFromPlayer <= data->meleeRange) inMeleeRange = true;

        if (inMeleeRange && enemy->playerVisible && meleeTimerFinished && !enemy->tookHit && !player->core.isDead) {
            enemy->newState = GOBLIN_STATE_MELEE_ATTACK;
            enemy->isChasing = false;
            enemy->isMoving = false;
            enemy->isPatrolling = false;
        }
        /* projectile logic left commented out for future use
        else if (enemy->distanceFromPlayer <= data->projectileRange && data->bombCooldownTimer <= 0.0f) {
            enemy->newState = GOBLIN_STATE_THROW_BOMB;
            enemy->isChasing = false;
            enemy->isPatrolling = false;
        } */
        else if ((enemy->distanceFromPlayer <= enemy->aggroRange) || (enemy->playerVisible) && !player->core.isDead) {
            enemy->isChasing = true;
            enemy->isPatrolling = false;
        }
        else {
            enemy->isPatrolling = true;
            enemy->isChasing = false;
        }

        if (enemy->isChasing) Enemy_RunTowardsPlayer(enemy, lvl, dt, player, enemy->distanceFromPlayer);
        else if (enemy->isPatrolling) Enemy_Patrol(enemy, lvl, dt);

        if (enemy->isMoving && !enemy->core.isAttacking)
            enemy->newState = GOBLIN_STATE_RUN;
        if (enemy->tookHit) enemy->newState = GOBLIN_STATE_HURT;

    }
    else
    {
        Animation *anim = Entity_GetCurrentAnimation(&enemy->core);
        if (anim && strcmp(anim->name, "die") == 0)
        {
            if (anim->frameCount > 0 && enemy->core.currentFrame == (anim->frameCount - 1))
            {
                printf("Enemy has Died\n");
                printf("Enemy Address: %p\n", (void*)enemy);
                enemy->core.visible = false;
                enemy->core.isActive = false;
            }
        }
    }
}

void Goblin_SetState(enemy* e, GoblinState newState)
{
    GoblinData* data = (GoblinData*)e->behaviorData;

    if (data->state == GOBLIN_STATE_HURT && data->hurtTimer > 0.0f && newState != GOBLIN_STATE_DEAD) {
        return;
    }

    if (!e->core.canBeInterrupted && !e->tookHit && !e->core.isDead) {
        return;
    }

    // When leaving attack states
    if (data->state == GOBLIN_STATE_MELEE_ATTACK || data->state == GOBLIN_STATE_THROW_BOMB) {
        if (newState != GOBLIN_STATE_MELEE_ATTACK && newState != GOBLIN_STATE_THROW_BOMB) {
            e->core.currentAttackIndex = -1;
            e->core.currentAttackStage = 0;
            e->core.hitboxActive = false;
            e->core.canBeInterrupted = true;
            e->core.isAttacking = false;
        }
    }

    // Apply new state
    data->state = newState;

    switch (newState) {

    case GOBLIN_STATE_RUN:
        Entity_PlayAnimation(&e->core, "run");
        break;

    case GOBLIN_STATE_MELEE_ATTACK:
        if (e->behavior && e->behavior->attack) e->behavior->attack(e);
        break;

    case GOBLIN_STATE_THROW_BOMB:
        if (e->behavior && e->behavior->attack) e->behavior->attack(e);
        break;

    case GOBLIN_STATE_HURT:
        Entity_PlayAnimation(&e->core, "hurt");
        break;
    case GOBLIN_STATE_DEAD:
        Entity_PlayAnimation(&e->core, "die");
        break;
    default:
        Entity_PlayAnimation(&e->core, "idle");
        e->tookHit = false;
        break;
    }
}

void Goblin_Attack(enemy* e)
{
    if (!e) return;
    GoblinData* data = (GoblinData*)e->behaviorData;
    

    // Check cooldown
    if (data->attackCooldownTimer > 0 || (data->state == GOBLIN_STATE_THROW_BOMB && data->bombCooldownTimer > 0))
    {
        e->core.isAttacking = false;
        return;
    }

    if (data->state == GOBLIN_STATE_MELEE_ATTACK)
    {
        
        int attackIdx = data->meleeAttackIndex;
        if (attackIdx >= 0) {
            e->core.currentAttackIndex = attackIdx;
            e->core.currentAttackStage = 0;
            e->core.isAttacking = true;
            Entity_StartAttackStage(&e->core, attackIdx, 0);

            float baseCooldown = (e->attackCoolDown > 0.0f) ? e->attackCoolDown : 2.0f;
            float levelFactor = 1.0f - (0.03f * (e->level - 1));
            if (levelFactor < 0.0f) levelFactor = 0.0f; // Prevent negative factor
            float scaledCooldown = baseCooldown * levelFactor;
            if (scaledCooldown > 2.0f) scaledCooldown = 2.0f; // Clamping to max 2 seconds
            if (scaledCooldown < 0.1f) scaledCooldown = 0.1f; // Clamping to a reasonable minimum
            data->attackCooldownTimer = scaledCooldown;

        }
        else {
            data->state = GOBLIN_STATE_IDLE;
            e->core.isAttacking = false;
        }
    }
    else if (data->state == GOBLIN_STATE_THROW_BOMB)
    {
        int attackIdx = data->bombAttackIndex;
        if (attackIdx >= 0) {
            e->core.currentAttackIndex = attackIdx;
            e->core.currentAttackStage = 0;
            e->core.isAttacking = true;
            Entity_StartAttackStage(&e->core, attackIdx, 0);

            data->bombCooldownTimer = 10.0f;
        }
        else {
            data->state = GOBLIN_STATE_IDLE;
            e->core.isAttacking = false;
        }
    }

}


void Goblin_OnHit(enemy* e, float damage, float knockBack, float hitStop)
{
    if (!e) return;
    GoblinData* data = (GoblinData*)e->behaviorData;

    e->newState = GOBLIN_STATE_HURT;
    int animIdx = Entity_FindAnimation(&e->core, "hurt");
    if (animIdx >= 0) {
        Animation* anim = &e->core.anims.animations[animIdx];
        data->hurtTimer = (anim->frameCount * anim->frameDuration) + 0.1;
        e->core.currentAnimIndex = animIdx;
        e->core.currentFrame = 0;
        e->core.frameTimer = 0.0f;
    } else {
        data->hurtTimer = 0.44f;
    }

    e->tookHit = true;

    printf("Enemy has taken %f damage\n", damage);
    e->core.currentHealth -= damage;

    if (e->core.currentHealth <= 0 && !e->core.isDead)
    {
        if (e->behavior->onDeath)
        {
            e->behavior->onDeath(e);
        }
    }
    e->isMoving = false;
    e->isChasing = false;
    e->isPatrolling = false;
    e->core.body.velocity_x = knockBack;
    data->recordedHitStop = hitStop;
}


void Goblin_OnDeath(enemy* e)
{
    if (!e) return;
    printf("[Goblin] Died.\n");
    e->core.isDead = true;
    e->newState = GOBLIN_STATE_DEAD;
    Entity_PlayAnimation(&e->core, "die");

}

void Goblin_Free(enemy* e)
{
    if (!e || !e->behaviorData) return;
    free(e->behaviorData);
    e->behaviorData = NULL;
}

const EnemyBehavior GOBLIN_BEHAVIOR = {
    .init = Goblin_Init,
    .update = Goblin_Update,
    .attack = Goblin_Attack,
    .onHit = Goblin_OnHit,
    .onDeath = Goblin_OnDeath,
    .setState = Goblin_SetState,
    .free = Goblin_Free
};

