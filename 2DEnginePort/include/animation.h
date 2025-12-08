#pragma once

#include <SDL.h>
#include <stdbool.h>

typedef struct Projectile Projectile;

typedef struct {
    int frameIndex;     // Which frame triggers the event
    char* value;
    char* type;
} AnimationEvent;

typedef struct {
    float widthScale;       // relative to frameWidth*scale
    float heightScale;      // relative to frameHeight*scale
    float offsetX;          // fine-tune placement cause of some weird animation sprites
    float offsetY;
} CollisionProfile;


typedef struct Animation{
    char* name;             
    int sheetIndex;         // which sprite sheet to use
    SDL_Rect* frames;       // array of src rects
    int frameCount;
    float frameDuration;
    bool loop;
    CollisionProfile* collisionProfile; // optional, can be NULL

    // events (e.g., hit detection, sound cues)
    AnimationEvent* events;
    int eventCount;

    // Flags
    bool invulnerable;      // used for roll/dodge frames
    bool canBeInterrupted;  // false if animation must finish 
} Animation;

typedef struct {
    char* name;             
    SDL_Texture* texture;
    int frameWidth;
    int frameHeight;
} SpriteSheet;


typedef struct {
    int w, h;           
    int offsetX;        // relative to collision rect (mirrored if flipped)
    int offsetY;
    int x;
    int y;
} AttackHitbox; 

typedef struct {
    Animation* animation;
    AttackHitbox hitbox;
    Projectile* projectile;
    char* projectileName;
    bool canBeComboed;
    float baseDamage;
    float knockBack;
    float hitStop;
} AttackStage;

typedef struct {
    char* name;
    char* type;
    AttackStage* stages;
    int stageCount;
    int currentStage;
} AttackDef;