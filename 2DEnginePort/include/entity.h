#pragma once

#include "collision.h"
#include "animation.h"
#include "healthbar.h"


typedef struct healthBar healthBar;
typedef struct Camera Camera;
// Generic pack for animations + attacks used by both player and enemies
typedef struct EntityAnims {
	Animation* animations;
	int animCount;

	AttackDef* attacks;
	int attackCount;
} EntityAnims;

typedef struct Entity {
	// Position & movement
	PhysicsBody body;           // includes accel/decel/runSpeed/gravity and flip

	//Health
	float maxHealth;
	float currentHealth;
	int baseHealth;
	healthBar healthBar;

	// Animation
	SpriteSheet* sheets;
	int sheetCount;
	float spriteScale;
	EntityAnims anims;
	int currentAnimIndex;
	int currentFrame;
	float frameTimer;
	bool visible;
	bool isActive;

	// Collision
	CollisionProfile* collisionProfiles;
	int profileCount;

	// Attacks
	int currentAttackIndex;    
	int currentAttackStage;    
	float attackTimer;
	AttackHitbox activeHitbox;
	bool hitboxActive;

	// State
	bool isAttacking, isDead, canBeInterrupted, invulnerable, mustBeFreed;
	float hurtTimer;            // brief i-frames or stun window
	float hitStopTimer;

	// Identity
	char* typeName;
	//int faction;             
	bool canBeHit;
} Entity;

void Entity_UpdatePhysics(Entity* e, float dt, struct Level* lvl);
void Entity_UpdateAnimation(Entity* e, float dt);
void Entity_UpdateCollisionBox(Entity* e, struct Level* lvl);
void Entity_UpdateAttackHitbox(Entity* e);

int Entity_FindAnimation(const Entity* e, const char* name);
Animation* Entity_GetCurrentAnimation(Entity* e);
int Entity_FindAttack(const Entity* e, const char* name);
void Entity_StartAttackStage(struct Entity* e, int attackIndex, int stageIdx);
bool Entity_PlayAnimation(Entity* e, const char* name);

void Entity_Render(const Entity* entity, SDL_Renderer* renderer, const Camera* camera, bool debug);

void Entity_Destroy(Entity* e); 
