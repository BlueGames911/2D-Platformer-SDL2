#pragma once
typedef struct enemy enemy;
typedef struct Level Level;
typedef struct Player Player;

// Behavior "vtable"
typedef struct EnemyBehavior {
    void (*init)(enemy* e, const char* config_json);
    void (*update)(enemy* e, float dt, Player* player, Level* level);
    void (*attack)(enemy* e);
    void (*onHit)(enemy* e, float damage, float knockBack, float hitStop);
    void (*onDeath)(enemy* e);
    void (*setState)(enemy* e, int newState);
    void (*free)(enemy* e);
} EnemyBehavior;


