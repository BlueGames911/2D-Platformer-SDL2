#pragma once

#include "gameManager.h"
#include "collision.h"
#include "animation.h"
#include "enemy_behaviour.h"
#include "entity.h"

#include <SDL.h>

typedef enum state
{
	ENEMY_IDLE,
	ENEMY_FALLING,
	ENEMY_SCOUTING,
	ENEMY_WALKING,
	ENEMY_ATTACKING,
	ENEMY_HURT,
	ENEMY_DIE,
	ENEMY_STATE_COUNT
}state;

typedef struct enemy
{
	// Shared entity core (position, physics, sheets, animations, attacks, health, etc.)
	Entity core;

	// Enemy-only tuning
	int level;
	int id;
	char* enemyType;
	bool isFlyingType;

	int spawnTileX;
	int spawnTileY;

	float runSpeedMultiplier;
	float dodgeChance;
	float poise;

	int patrolLeftDistance;
	int patrolRightDistance;
	int patrolOriginX;
	int patroldTraveled;
	int patrolDirection;
	float distanceFromPlayer;
	bool isChasing;
	bool isPatrolling;
	bool playerVisible;
	bool playerAhead;
	bool tookHit;
	float growthRate;

	bool isTurning;
	bool hasFlippedYet;
	float turnTimer;
	float turnDelay;
	bool isMoving;

	int aggroRange;
	int visualRange;
	float attackRange;
	int chaseRange;
	float attackCoolDown;
	int lastHitAttackIndex;   
	int lastHitAttackStage;

	bool isOnScreen;
	state EnemyState;
	int newState;

	// Enemy behavior vtable and per-enemy behavior data
	const EnemyBehavior* behavior;
	void* behaviorData;
} enemy;

// Loads config for an enemy type and fills the enemy struct
// Returns true on success, false on failure
bool Enemy_LoadConfigFromFile(const char* type, const char* path, enemy* enemy, mainSystems systems);
enemy** LoadEnemiesFromFile(const char* path, mainSystems systems, Level* lvl);
bool Enemy_Spawn(Level *lvl, enemy *enemy, mainSystems systems, UI* ui);
bool Enemy_Respawn(Level* lvl, enemy* enemy, UI* ui);
void Enemy_ResetState(enemy* e);

void Enemy_Update(enemy* enemy, float deltaTime, Player* player, Level* lvl);
void UpdateAllEnemies(Level* lvl, float deltaTime, Player* player);
void Enemy_SetState(enemy* enemy, state newState);

void Enemy_Patrol(enemy* enemy, Level* lvl, float deltaTime);
void Enemy_RunTowardsPlayer(enemy* enemy, Level* lvl, float deltaTime, Player* player, int distance);

void Enemy_CheckAttackHits(enemy* enemy, Player* player);

void RenderAllEnemies(Level* lvl, SDL_Renderer* r, Camera* cam, bool debug);

void Enemy_Unload(enemy* enemy);
