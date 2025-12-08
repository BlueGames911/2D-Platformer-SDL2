#include "player.h"
#include "utils.h"
#include "texture.h"
#include "settings.h"
#include "gameManager.h"
#include "collision.h"
#include "camera.h"
#include "entity.h"
#include "enemies.h"
#include "level.h"

#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMBO_WINDOW_FRAMES 2 // Planning to put this into a json file, so each attack can have their own combo window
#define PLAYER_ATTACK_COOLDOWN 0.35f

bool Player_LoadConfig(Player *player, struct mainSystems *systems, const char *filePath)
{
    //Read the JSON file into a string
    char *file = read_whole_file(filePath);
    if (!file)
    {
        fprintf(stderr, "[PLAYER] Failed to load %s\n", filePath);
        return false;
    }
    printf("Loaded player.json into memory\n");
    
    // Parse the JSON string into a cJSON root object
    cJSON *configFile = cJSON_Parse(file);
    if (!configFile) 
    {
        free(file);
        fprintf(stderr, "[PLAYER] Failed to parse %s into json file\n", filePath);
        return false;
    }
    free(file);
    printf("Parsed player.json\n");

    //Read sprite scale
    cJSON *scale = cJSON_GetObjectItemCaseSensitive(configFile, "spriteScale");
    if (cJSON_IsNumber(scale)) {
        player->core.spriteScale = scale->valuedouble;
        printf("Successfully read spriteScale: %.2f\n", player->core.spriteScale);
    } else {
        fprintf(stderr, "[PLAYER] Warning: 'spriteScale' missing or invalid in %s, defaulting to 1.0\n", filePath);
        player->core.spriteScale = 1.0f; // default scale
    }
    printf("Loaded Sprite Scale\n");

    // Load sheets array
    cJSON *sheets = cJSON_GetObjectItemCaseSensitive(configFile, "sheets");
    int numOfSheets = cJSON_GetArraySize(sheets);
    player->core.sheetCount = numOfSheets;
    SpriteSheet *spriteSheets = calloc(numOfSheets, sizeof(SpriteSheet));

    int idx = 0;
    cJSON *sheets_idx = NULL;
    cJSON_ArrayForEach(sheets_idx, sheets)
    {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(sheets_idx, "name");
        cJSON *path = cJSON_GetObjectItemCaseSensitive(sheets_idx, "path");
        cJSON *frameHeight = cJSON_GetObjectItemCaseSensitive(sheets_idx, "frameHeight");
        cJSON *frameWidth = cJSON_GetObjectItemCaseSensitive(sheets_idx, "frameWidth");

        if (cJSON_IsString(name)) {
            spriteSheets[idx].name = _strdup(name->valuestring);
        }

        if (cJSON_IsString(path)) {
            spriteSheets[idx].texture = loadTexture(path->valuestring, systems->renderer);
            if (!spriteSheets[idx].texture) {
                fprintf(stderr, "[PLAYER] Failed to create texture from %s\n", path->valuestring);
                cJSON_Delete(configFile);
                return false;    
            }
        }

        if (!cJSON_IsString(name) || !cJSON_IsString(path)) {
            fprintf(stderr, "[PLAYER] Missing or invalid 'name' or 'path' in sheet entry\n");
            cJSON_Delete(configFile);
            return false;
        }



        if (cJSON_IsNumber(frameHeight) && cJSON_IsNumber(frameWidth)) {
            spriteSheets[idx].frameHeight = frameHeight->valueint;
            spriteSheets[idx].frameWidth = frameWidth->valueint;
        }
        else {
            fprintf(stderr, "[PLAYER] Invalid Value for frameWidth/frameHeight!\n");
            cJSON_Delete(configFile);
            return false;
        }

        idx++;
    }

    player->core.sheets = spriteSheets;
    printf("Loaded Sprite Sheets\n");

    // Load animations array
    // Would you look at that, I found a better way

    cJSON* animationsArr = cJSON_GetObjectItemCaseSensitive(configFile, "animations");
    if (!cJSON_IsArray(animationsArr)) {
        fprintf(stderr, "[PLAYER] 'animations' is not a JSON array!\n");
        cJSON_Delete(configFile);
        return false;
    }

    player->core.anims.animCount = cJSON_GetArraySize(animationsArr);
    player->core.anims.animations = calloc(player->core.anims.animCount, sizeof(Animation));

    int animIndex = 0;
    cJSON* animObj;
    cJSON_ArrayForEach(animObj, animationsArr) {
        Animation* anim = &player->core.anims.animations[animIndex];

        // MetaData
        cJSON* name = cJSON_GetObjectItemCaseSensitive(animObj, "name");
        cJSON* sheet = cJSON_GetObjectItemCaseSensitive(animObj, "sheet");
        cJSON* frameDuration = cJSON_GetObjectItemCaseSensitive(animObj, "frameDuration");
        cJSON* loop = cJSON_GetObjectItemCaseSensitive(animObj, "loop");

        anim->name = cJSON_IsString(name) ? _strdup(name->valuestring) : NULL;

        // Find sheet index
        int sheetIndex = -1;
        if (cJSON_IsString(sheet)) {
            for (int i = 0; i < player->core.sheetCount; i++) {
                if (strcmp(player->core.sheets[i].name, sheet->valuestring) == 0) {
                    sheetIndex = i;
                    break;
                }
            }
        }
        if (sheetIndex == -1) {
            fprintf(stderr, "[PLAYER] Unknown sheet name '%s' in animation '%s'\n",
                cJSON_IsString(sheet) ? sheet->valuestring : "NULL",
                anim->name ? anim->name : "unnamed");
            cJSON_Delete(configFile);
            return false;
        }
        anim->sheetIndex = sheetIndex;

        anim->frameDuration = cJSON_IsNumber(frameDuration) ? (float)frameDuration->valuedouble : 0.1f;
        anim->loop = cJSON_IsBool(loop) ? cJSON_IsTrue(loop) : true;

        // Frames
        cJSON* framesArray = cJSON_GetObjectItemCaseSensitive(animObj, "frames");
        int frameCount = cJSON_GetArraySize(framesArray);
        anim->frameCount = frameCount;
        anim->frames = calloc(frameCount, sizeof(SDL_Rect));

        int frameIdx = 0;
        cJSON* frameObj;
        cJSON_ArrayForEach(frameObj, framesArray) {
            cJSON* x = cJSON_GetObjectItemCaseSensitive(frameObj, "x");
            cJSON* y = cJSON_GetObjectItemCaseSensitive(frameObj, "y");

            if (cJSON_IsNumber(x) && cJSON_IsNumber(y)) {
                anim->frames[frameIdx].x = x->valueint * player->core.sheets[sheetIndex].frameWidth;
                anim->frames[frameIdx].y = y->valueint * player->core.sheets[sheetIndex].frameHeight;
                anim->frames[frameIdx].w = player->core.sheets[sheetIndex].frameWidth;
                anim->frames[frameIdx].h = player->core.sheets[sheetIndex].frameHeight;
            }
            else {
                fprintf(stderr, "[PLAYER] Invalid frame coordinates in animation '%s'\n", anim->name);
                cJSON_Delete(configFile);
                return false;
            }
            frameIdx++;
        }

        // CollisionBox
        cJSON* collisionBox = cJSON_GetObjectItemCaseSensitive(animObj, "collisionBox");
        if (collisionBox) {
            cJSON* w = cJSON_GetObjectItemCaseSensitive(collisionBox, "w");
            cJSON* h = cJSON_GetObjectItemCaseSensitive(collisionBox, "h");
            cJSON* offsetX = cJSON_GetObjectItemCaseSensitive(collisionBox, "offsetX");
            cJSON* offsetY = cJSON_GetObjectItemCaseSensitive(collisionBox, "offsetY");

            if (cJSON_IsNumber(w) && cJSON_IsNumber(h) && cJSON_IsNumber(offsetX) && cJSON_IsNumber(offsetY)) {
                anim->collisionProfile = malloc(sizeof(CollisionProfile));
                anim->collisionProfile->widthScale = w->valuedouble / player->core.sheets[sheetIndex].frameWidth;
                anim->collisionProfile->heightScale = h->valuedouble / player->core.sheets[sheetIndex].frameHeight;

                anim->collisionProfile->offsetX = (float)offsetX->valuedouble;
                anim->collisionProfile->offsetY = (float)offsetY->valuedouble;
            }
        }

        // Events
        cJSON* eventsArr = cJSON_GetObjectItemCaseSensitive(animObj, "events");
        if (cJSON_IsArray(eventsArr)) {
            int eventCount = cJSON_GetArraySize(eventsArr);
            anim->eventCount = eventCount;
            anim->events = calloc(eventCount, sizeof(AnimationEvent));
            int eventIdx = 0;
            cJSON* eventObj;
            cJSON_ArrayForEach(eventObj, eventsArr) {
                cJSON* frameIndex = cJSON_GetObjectItemCaseSensitive(eventObj, "frameIndex");
                cJSON* type = cJSON_GetObjectItemCaseSensitive(eventObj, "type");
                cJSON* value = cJSON_GetObjectItemCaseSensitive(eventObj, "value");

                if (cJSON_IsNumber(frameIndex) && cJSON_IsString(type)) {
                    anim->events[eventIdx].frameIndex = frameIndex->valueint;
                    anim->events[eventIdx].type = _strdup(type->valuestring);
                    if (cJSON_IsString(value)) {
                        anim->events[eventIdx].value = _strdup(value->valuestring);
                    }
                }
                eventIdx++;
            }
        }

        // Flags 
        cJSON* invulnerable = cJSON_GetObjectItemCaseSensitive(animObj, "invulnerable");
        anim->invulnerable = cJSON_IsBool(invulnerable) ? cJSON_IsTrue(invulnerable) : false;

        cJSON* canBeInterrupted = cJSON_GetObjectItemCaseSensitive(animObj, "canBeInterrupted");
        anim->canBeInterrupted = cJSON_IsBool(canBeInterrupted) ? cJSON_IsTrue(canBeInterrupted) : true;

        animIndex++;
    }

    // Parse "attacks" array (multi-stage attacks)
    cJSON* attacksArr = cJSON_GetObjectItemCaseSensitive(configFile, "attacks");
    if (cJSON_IsArray(attacksArr)) {
        player->core.anims.attackCount = cJSON_GetArraySize(attacksArr);
        player->core.anims.attacks = calloc(player->core.anims.attackCount, sizeof(AttackDef));

        int attackIndex = 0;
        cJSON* atkObj;
        cJSON_ArrayForEach(atkObj, attacksArr) {
            AttackDef* attack = &player->core.anims.attacks[attackIndex];

            // MetaData
            cJSON* atkName = cJSON_GetObjectItemCaseSensitive(atkObj, "name");

            attack->name = cJSON_IsString(atkName) ? _strdup(atkName->valuestring) : NULL;

            // Stages 
            cJSON* stagesArr = cJSON_GetObjectItemCaseSensitive(atkObj, "stages");
            attack->stageCount = cJSON_GetArraySize(stagesArr);
            attack->stages = calloc(attack->stageCount, sizeof(AttackStage));
            attack->currentStage = 0;

            int stageIndex = 0;
            cJSON* stageObj;
            cJSON_ArrayForEach(stageObj, stagesArr) {
                AttackStage* stage = &attack->stages[stageIndex];

                // Link to Animation 
                cJSON* animationName = cJSON_GetObjectItemCaseSensitive(stageObj, "animation");
                if (!cJSON_IsString(animationName)) {
                    fprintf(stderr, "[PLAYER] Missing animation name for attack '%s' stage %d\n",
                        attack->name, stageIndex);
                    cJSON_Delete(configFile);
                    return false;
                }

                // Find the animation by name and store pointer
                stage->animation = NULL;
                for (int i = 0; i < player->core.anims.animCount; i++) {
                    if (player->core.anims.animations[i].name &&
                        strcmp(player->core.anims.animations[i].name, animationName->valuestring) == 0) {
                        stage->animation = &player->core.anims.animations[i];
                        break;
                    }
                }

                if (!stage->animation) {
                    fprintf(stderr, "[PLAYER] Animation '%s' not found for attack '%s' stage %d\n",
                        animationName->valuestring, attack->name, stageIndex);
                    cJSON_Delete(configFile);
                    return false;
                }

                // Extract Hitbox from Animation Events
                bool hitboxFound = false;
                Animation* anim = stage->animation;

                for (int eventIdx = 0; eventIdx < anim->eventCount; eventIdx++) {
                    AnimationEvent* event = &anim->events[eventIdx];
                    if (event->type && strcmp(event->type, "hitbox") == 0) {
                        // Find the animation JSON object again to extract hitbox data
                        cJSON* animJsonObj = NULL;
                        cJSON* animObj;
                        cJSON_ArrayForEach(animObj, animationsArr) {
                            cJSON* name = cJSON_GetObjectItemCaseSensitive(animObj, "name");
                            if (cJSON_IsString(name) && strcmp(name->valuestring, animationName->valuestring) == 0) {
                                animJsonObj = animObj;
                                break;
                            }
                        }

                        if (animJsonObj) {
                            cJSON* eventsArr = cJSON_GetObjectItemCaseSensitive(animJsonObj, "events");
                            if (cJSON_IsArray(eventsArr)) {
                                cJSON* eventObj;
                                cJSON_ArrayForEach(eventObj, eventsArr) {
                                    cJSON* type = cJSON_GetObjectItemCaseSensitive(eventObj, "type");
                                    cJSON* frameIndex = cJSON_GetObjectItemCaseSensitive(eventObj, "frameIndex");

                                    if (cJSON_IsString(type) && strcmp(type->valuestring, "hitbox") == 0 &&
                                        cJSON_IsNumber(frameIndex) && frameIndex->valueint == event->frameIndex) {

                                        // Extract hitbox dimensions
                                        cJSON* w = cJSON_GetObjectItemCaseSensitive(eventObj, "w");
                                        cJSON* h = cJSON_GetObjectItemCaseSensitive(eventObj, "h");
                                        cJSON* offsetX = cJSON_GetObjectItemCaseSensitive(eventObj, "offsetX");
                                        cJSON* offsetY = cJSON_GetObjectItemCaseSensitive(eventObj, "offsetY");

                                        if (cJSON_IsNumber(w) && cJSON_IsNumber(h) &&
                                            cJSON_IsNumber(offsetX) && cJSON_IsNumber(offsetY)) {

                                            stage->hitbox.w = (int)(w->valueint * player->core.spriteScale);
                                            stage->hitbox.h = (int)(h->valueint * player->core.spriteScale);
                                            stage->hitbox.offsetX = (int)(offsetX->valueint * player->core.spriteScale);
                                            stage->hitbox.offsetY = (int)(offsetY->valueint * player->core.spriteScale);
                                            stage->hitbox.x = 0; 
                                            stage->hitbox.y = 0; 

                                            hitboxFound = true;
                                            break;
                                        }
                                        else {
                                            fprintf(stderr, "[PLAYER] Invalid hitbox parameters for animation '%s'\n",
                                                animationName->valuestring);
                                            cJSON_Delete(configFile);
                                            return false;
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                }

                if (!hitboxFound) {
                    fprintf(stderr, "[PLAYER] Attack stage animation '%s' must have a 'hitbox' event for attack '%s' stage %d\n",
                        animationName->valuestring, attack->name, stageIndex);
                    cJSON_Delete(configFile);
                    return false;
                }

                cJSON* damage = cJSON_GetObjectItemCaseSensitive(stageObj, "damage");
                cJSON* knockBack = cJSON_GetObjectItem(stageObj, "knockBack");
                cJSON* hitStop = cJSON_DetachItemFromObjectCaseSensitive(stageObj, "hitStop");
                cJSON* canBeComboed = cJSON_GetObjectItemCaseSensitive(stageObj, "canBeComboed");

                if (cJSON_IsNumber(damage) && cJSON_IsNumber(knockBack) && cJSON_IsNumber(hitStop))
                {
                    stage->baseDamage = damage->valueint;
                    stage->knockBack = knockBack->valueint;
                    stage->hitStop = hitStop->valuedouble;
                }
                else
                {
                    fprintf(stderr, "[PLAYER] Attack Stage from attack %s has invalid numeric propertie(s)\n", atkName->valuestring);
                    stage->baseDamage = 10;
                    stage->knockBack = 2;
                    stage->hitStop = 0.03;
                }
                stage->canBeComboed = cJSON_IsBool(canBeComboed) ? cJSON_IsTrue(canBeComboed) : false;

                printf("[PLAYER] Loaded attack stage '%s': hitbox(%d,%d) offset(%d,%d) damage=%f knockback%f\n",
                    animationName->valuestring, stage->hitbox.w, stage->hitbox.h,
                    stage->hitbox.offsetX, stage->hitbox.offsetY, stage->baseDamage, stage->knockBack);

                stageIndex++;
            }
            attackIndex++;
        }
    }
    printf("Loaded player animations and attacks\n");


    // Load physics object
    cJSON *physics = cJSON_GetObjectItemCaseSensitive(configFile, "physics");
    if (physics) {
        cJSON *runSpeed = cJSON_GetObjectItemCaseSensitive(physics, "runSpeed");
        cJSON *walkSpeed = cJSON_GetObjectItemCaseSensitive(physics, "walkSpeed");
        cJSON *jumpForce = cJSON_GetObjectItemCaseSensitive(physics, "jumpForce");
        cJSON *gravity = cJSON_GetObjectItemCaseSensitive(physics, "gravity");
        cJSON *maxFallSpeed = cJSON_GetObjectItemCaseSensitive(physics, "maxFallSpeed");
        cJSON *crouchHeightOffset = cJSON_GetObjectItemCaseSensitive(physics, "crouchHeightOffset");
        cJSON *spriteScale = cJSON_GetObjectItemCaseSensitive(physics, "spriteScale");
        cJSON *acceleration = cJSON_GetObjectItemCaseSensitive(physics, "acceleration");
        cJSON *decceleration = cJSON_GetObjectItemCaseSensitive(physics, "decceleration");
        cJSON *crouchLag = cJSON_GetObjectItemCaseSensitive(physics, "crouchLag");
        cJSON *crouchSpeed = cJSON_GetObjectItemCaseSensitive(physics, "crouchSpeed");

        if (cJSON_IsNumber(crouchLag)) {
            player->crouchLag = crouchLag->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing crouchLag in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(decceleration)) {
            player->core.body.deceleration = decceleration->valueint;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing decceleration in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(acceleration)) {
            player->core.body.acceleration = acceleration->valueint;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing acceleration in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(runSpeed)) {
            player->core.body.runSpeed = runSpeed->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing runSpeed in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(crouchSpeed)) {
            player->crouchSpeed = crouchSpeed->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing crouchSpeed in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(walkSpeed)) {
            player->walkSpeed = walkSpeed->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing walkSpeed in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(jumpForce)) {
            player->core.body.jumpForce = jumpForce->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing jumpForce in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(gravity)) {
            player->core.body.gravity = gravity->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing gravity in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(maxFallSpeed)) {
            player->core.body.maxFallSpeed = maxFallSpeed->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing maxFallSpeed in physics\n");
            cJSON_Delete(configFile);
            return false;
        }

        if (cJSON_IsNumber(crouchHeightOffset)) {
            player->crouchHeightOffset = crouchHeightOffset->valuedouble;
        } else {
            fprintf(stderr, "[PLAYER] Invalid or missing crouchHeightOffset in physics\n");
            cJSON_Delete(configFile);
            return false;
        }


    } else {
        fprintf(stderr, "[PLAYER] Missing physics object in config\n");
        cJSON_Delete(configFile);
        return false;
    }
    printf("Loaded Player Physics\n");

    // Note from CoPilot: (Optional) Load "stateMap"
    //    - If you add this to JSON:
    //        * For each PlayerState enum name in JSON, find the animation index and store in player->stateToAnim[state]

    cJSON_Delete(configFile);

    //  Return true if everything loaded successfully
    printf("Player spriteScale: %.2f\n", player->core.spriteScale);
    player->state = PLAYER_IDLE;

    player->core.currentAttackIndex = -1;
    player->core.currentAttackStage = 0;
    player->core.currentAnimIndex = Entity_FindAnimation(&player->core, "idle");
    player->core.currentFrame = 0;
    player->core.frameTimer = 0.0f;
    player->core.hitboxActive = false;
    player->comboExtendRequested = false;
    player->core.canBeInterrupted = true;

    return true;
}

void Player_Destroy(Player* player)
{
    if (!player) return;

    // Use shared entity destroy
    Entity_Destroy(&player->core);
}

void Player_Update(Player* player, float deltaTime, Level* lvl)
{
    if (!player || !lvl) return;

    if (player->core.currentHealth <= 0 && !player->core.isDead)
    {
        printf("Player has Died\n");
        Player_OnDeath(player);
    }

    if (!player->core.isDead)
    {
        if (player->attackCoolDown > 0.0f) {
            player->attackCoolDown -= deltaTime;
            if (player->attackCoolDown < 0.0f)
                player->attackCoolDown = 0.0f;
        }

        // hurt state
        if (player->isHurt)
            if (player->hurtTimer > 0.0f) {
                player->hurtTimer -= deltaTime;
                player->isHurt = true;
                player->core.body.velocity_x = 0;
            }
            else {
                player->isHurt = false;
                if (player->core.currentHealth <= 0) {
                    player->state = PLAYER_DIE;
                    player->core.isDead = true;
                }
                else {
                    player->state = PLAYER_IDLE;
                }
            }
    

        for (int i = 0; i < lvl->enemyCount; i++)
        {
            enemy* e = lvl->enemies[i];
            if (e->core.hitStopTimer > 0) {
                e->core.hitStopTimer -= deltaTime;
                return;
            }
        }

        if (player->state == PLAYER_GROUND_ATTACKING)
        {
            player->core.body.velocity_x = 0;
        }
        Player_UpdatePhysics(player, deltaTime, lvl);

        PlayerState newState = PLAYER_IDLE;

        if (player->isHurt)
        {
            newState = PLAYER_HURT;
        }
        else if (player->core.isAttacking && player->core.body.isOnGround) {
            newState = PLAYER_GROUND_ATTACKING;
            player->core.body.isMovingLeft = false;
            player->core.body.isMovingRight = false;
        }
        else if (player->core.isAttacking && !player->core.body.isOnGround) {
            newState = PLAYER_AIR_ATTACKING;
        }
        else if (!player->core.body.isOnGround && player->core.body.velocity_y < 0) {
            newState = PLAYER_JUMPING;
        }
        else if (!player->core.body.isOnGround && player->core.body.velocity_y > 0) {
            newState = PLAYER_FALLING;
        }
        else if (player->isCrouching && (player->core.body.isMovingLeft || player->core.body.isMovingRight)) {
            newState = PLAYER_CROUCH_WALKING;
        }
        else if (player->isCrouching) {
            newState = PLAYER_CROUCH;
        }
        else if (player->core.body.isMovingLeft || player->core.body.isMovingRight) {
            newState = PLAYER_RUNNING;
        }

        if (newState == PLAYER_GROUND_ATTACKING)
        {
            player->core.body.velocity_x = 0;
        }

        Player_SetState(player, newState);

        Player_UpdateAnimation(player, deltaTime);

        Player_UpdateCollisionBox(player, lvl);

        Entity_UpdateAttackHitbox(&player->core);

        for (int i = 0; i < lvl->enemyCount; i++)
        {
            enemy* e = lvl->enemies[i];
            Player_CheckAttackHits(player, e, deltaTime);
        }

        HealthBar_Update(&player->core, deltaTime);

    } // <-- This closing brace fixes the orphaned "else"

    else 
    {
        int animIndex = Entity_FindAnimation(&player->core, "die");
        if (player->core.currentAnimIndex == animIndex)
        {
            Animation* anim = Entity_GetCurrentAnimation(&player->core);
            if (player->core.currentFrame >= anim->frameCount - 3)
            {
                player->fadeStarted = true;
            }
        }
        if (player->fadeStarted)
        {
            player->fadeAlpha -= 200 * deltaTime;   // fade speed
            if (player->fadeAlpha <= 0)
            {
                player->fadeAlpha = 0;
                player->deathFinished = true;
            }
            
        }
        Player_SetState(player, PLAYER_DIE);
        Player_UpdateAnimation(player, deltaTime);
    }
    

}

void Player_UpdatePhysics(Player *player, float deltaTime, Level *lvl)
{
    if (!player || !lvl) return;

    // Handle jump
    if (player->jumpRequested && player->core.body.isOnGround) {
        player->core.body.velocity_y = player->core.body.jumpForce;
        player->core.body.isOnGround = false;
        player->jumpRequested = false;
    }

    // Handle crouch speed/lag
    float originalAccel = player->core.body.acceleration;
    float originalMaxSpeed = player->core.body.runSpeed;
    if (player->isCrouching) {
        player->core.body.acceleration /= player->crouchLag;
        player->core.body.runSpeed = player->crouchSpeed;
    }

    // Handle fall speed manipulation
    float fallMultiplier = 1.20f;
    if (player->state == PLAYER_FALLING) {
        player->core.body.velocity_y += player->core.body.gravity * (fallMultiplier - 1.0f) * deltaTime;
    }
    if ((!player->core.body.isOnGround) && player->state == PLAYER_AIR_ATTACKING) {
        player->core.body.velocity_x /= 1.3f;
    }

    // base entity physics
    Entity_UpdatePhysics(&player->core, deltaTime, lvl);

    // Restore original values
    player->core.body.acceleration = originalAccel;
    player->core.body.runSpeed = originalMaxSpeed;
}

void Player_UpdateCollisionBox(Player* player, Level* lvl) {

    int oldHeight = player->core.body.collisionRect.h;

    Entity_UpdateCollisionBox(&player->core, lvl);

    int newHeight = player->core.body.collisionRect.h;

    if (player->isCrouching) {
        player->core.body.y += (oldHeight - newHeight);
    }
    else {
        int extraHeight = newHeight - oldHeight;
        if (extraHeight > 0) {
            if (!hasCeilingAbove(&player->core.body, lvl, extraHeight)) {
                player->core.body.y += (oldHeight - newHeight);
            }
            else {
                player->isCrouching = true;
                player->core.body.collisionRect.h = oldHeight;
            }
        }
    }
}

void Player_CheckAttackHits(Player* player, enemy* enemy, float deltaTime)
{
    if (!player || !enemy || !player->core.isAttacking || !player->core.hitboxActive) return;

    SDL_Rect hitbox;
    hitbox.x = player->core.activeHitbox.x;
    hitbox.y = player->core.activeHitbox.y;
    hitbox.h = player->core.activeHitbox.h;
    hitbox.w = player->core.activeHitbox.w;

    int atkIdx = player->core.currentAttackIndex;
    int stageIdx = player->core.currentAttackStage;

    if (collisionCheck(hitbox, enemy->core.body.collisionRect))
    {
        if (enemy->behavior && !enemy->tookHit)
        {
            printf("Enemy Hit\n");
            enemy->tookHit = true;
            AttackStage* stage = &player->core.anims.attacks[atkIdx].stages[stageIdx];
            if (!stage) return;

            int direction = 1;
            if (player->core.body.x > enemy->core.body.x) direction = -1;
            float knockBack = (stage->knockBack * 10) * direction;
            enemy->behavior->onHit(enemy, stage->baseDamage, knockBack, stage->hitStop);
        }
    }

}

void Player_OnHit(Player* player, enemy* enemy, float damage, float knockBack)
{
    if (!player || !enemy) return;

    player->state == PLAYER_HURT;

    int animIdx = Entity_FindAnimation(&player->core, "hurt");
    if (animIdx >= 0) {
        Animation* anim = &player->core.anims.animations[animIdx];
        player->hurtTimer = (anim->frameCount * anim->frameDuration) + 0.1;
        player->core.currentAnimIndex = animIdx;
        player->core.currentFrame = 0;
        player->core.frameTimer = 0.0f;
    }
    else {
      player->hurtTimer = 0.44f;
    }

    player->isHurt = true;
    player->core.isAttacking = false;
    player->comboExtendRequested = false;

    if (player->core.currentHealth > 0 && !player->core.isDead)
    {
        player->core.currentHealth -= damage;
        printf("Player has taken %f damage\nCurrent Player health: %f\n", damage, player->core.currentHealth);
        //KnockBack and Hitstop are a simple TODO for now
    }
}

void Player_OnDeath(Player* player)
{
    player->core.isDead = true;
    player->state = PLAYER_DIE;
    Entity_PlayAnimation(&player->core, "die");
    player->onDeathDelay = 5;

}

void Player_SetState(Player *player, PlayerState newState)
{
    if (!player) return;
    if (player->state == newState) return;

    // If current animation is uninterruptible, block state changes.
    if (!player->core.canBeInterrupted) {
        return;
    }

    // When leaving attack states
    if (player->state == PLAYER_GROUND_ATTACKING || player->state == PLAYER_AIR_ATTACKING) {
        if (newState != PLAYER_GROUND_ATTACKING && newState != PLAYER_AIR_ATTACKING) {
            player->core.currentAttackIndex = -1;
            player->core.currentAttackStage = 0;
            player->core.attackTimer = 0.0f;
            player->comboExtendRequested = false;
            player->core.hitboxActive = false;
            player->core.canBeInterrupted = true;
            player->core.isAttacking = false;
        }
    }

    // Apply new state
    player->state = newState;

    switch (newState) {
        case PLAYER_JUMPING:
            Entity_PlayAnimation(&player->core, "jump");
            break;

        case PLAYER_FALLING:
            Entity_PlayAnimation(&player->core, "fall");
            break;

        case PLAYER_CROUCH:
            Entity_PlayAnimation(&player->core, "crouch");
            break;

        case PLAYER_CROUCH_WALKING:
            Entity_PlayAnimation(&player->core, "crouch-walk");
            break;

        case PLAYER_RUNNING:
            Entity_PlayAnimation(&player->core, "run");
            break;

        case PLAYER_HURT:
            Entity_PlayAnimation(&player->core, "hurt");
            break;

        case PLAYER_GROUND_ATTACKING: {
            int attackIdx = Entity_FindAttack(&player->core, "ground_slash");
            if (attackIdx >= 0) {
                player->core.currentAttackIndex = attackIdx;
                player->core.currentAttackStage = 0;
                player->core.isAttacking = true;
                Entity_StartAttackStage(&player->core, attackIdx, 0);
            } else {
                // fallback: ensure we don't remain in an invalid attacking state
                player->state = PLAYER_IDLE;
                player->core.isAttacking = false;
            }
            break;
        }

        case PLAYER_AIR_ATTACKING: {
            int attackIdx = Entity_FindAttack(&player->core, "air_slash");
            if (attackIdx >= 0) {
                player->core.currentAttackIndex = attackIdx;
                player->core.currentAttackStage = 0;
                player->core.isAttacking = true;
                Entity_StartAttackStage(&player->core, attackIdx, 0);
            } else {
                player->state = PLAYER_FALLING;
                player->core.isAttacking = false;
            }
            break;
        }

        default:
            Entity_PlayAnimation(&player->core, "idle");
            break;
    }

    // debug
    //printf("Player_SetState -> %d (attackIndex=%d stage=%d)\n", player->state, player->core.currentAttackIndex, player->core.currentAttackStage);
}

void Player_UpdateAnimation(Player* player, float deltaTime)
{
    if (!player) return;

    Animation* anim = Entity_GetCurrentAnimation(&player->core);
    if (!anim) return;

    player->core.frameTimer += deltaTime;

    if (player->core.frameTimer >= anim->frameDuration) {
        player->core.frameTimer -= anim->frameDuration;
        player->core.currentFrame++;


        // Attack stage handling

        if (player->state == PLAYER_GROUND_ATTACKING || player->state == PLAYER_AIR_ATTACKING) {
            AttackDef* attack = &player->core.anims.attacks[player->core.currentAttackIndex];
            AttackStage* stage = &attack->stages[player->core.currentAttackStage];

            if (player->core.currentFrame >= stage->animation->frameCount) {
                // Stage finished
                bool grounded = player->core.body.isOnGround;

                if (player->comboExtendRequested && stage->canBeComboed &&
                    player->core.currentAttackStage + 1 < attack->stageCount) {
                    // Chain to next stage
                    player->comboExtendRequested = false;
                    Entity_StartAttackStage(&player->core, player->core.currentAttackIndex, player->core.currentAttackStage + 1);
                }
                else {
                    // Reset to movement state
                    player->core.currentAttackIndex = -1;
                    player->core.currentAttackStage = 0;
                    player->comboExtendRequested = false;
                    player->core.isAttacking = false;
                    player->core.canBeInterrupted = true;

                    player->attackCoolDown = PLAYER_ATTACK_COOLDOWN;
                    if (player->state == PLAYER_AIR_ATTACKING && !grounded)
                    {
                        Player_SetState(player, PLAYER_FALLING);
                        printf("Attack finished state=%d\n", player->state);
                    }
                    else {
                        Player_SetState(player, PLAYER_IDLE);
                        printf("Attack finished state=%d\n", player->state);
                    }
                }
            }
        }
        else {
            // Non-attack animations
            if (player->core.currentFrame >= anim->frameCount) {
                player->core.currentFrame = anim->loop ? 0 : anim->frameCount - 1;
            }
        }
    }
}

void Player_Render(GameManager* gm)
{

    if (gm->player->core.isDead) {
        Animation* anim = Entity_GetCurrentAnimation(&gm->player->core);
        SDL_SetTextureAlphaMod(
            gm->player->core.sheets[anim->sheetIndex].texture,
            (Uint8)gm->player->fadeAlpha
        );
    }
    else
    {
        Animation* anim = Entity_GetCurrentAnimation(&gm->player->core);
        SDL_SetTextureAlphaMod(gm->player->core.sheets[anim->sheetIndex].texture, 255);
    }

    Entity_Render(&gm->player->core, gm->mainSystems.renderer, &gm->camera, gm->settings.gameplay.debugMode);
}

void Player_HandleInput(Player *player) 
{
    if (!player) return;

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    #define SAFE_STATE(scancode) ((scancode >= 0 && scancode < SDL_NUM_SCANCODES) ? state[scancode] : 0)

    // If attack animation is active, only allow combo requests in the last X frames
    if ((player->state == PLAYER_GROUND_ATTACKING || player->state == PLAYER_AIR_ATTACKING) && !player->core.canBeInterrupted) {
        int attackIdx = player->core.currentAttackIndex;
        int stageIdx  = player->core.currentAttackStage;

        if (attackIdx >= 0 && attackIdx < player->core.anims.attackCount) {
            AttackDef *attack = &player->core.anims.attacks[attackIdx];
            if (stageIdx >= 0 && stageIdx < attack->stageCount) {
                AttackStage *stage = &attack->stages[stageIdx];
                int framesLeft = stage->animation->frameCount - player->core.currentFrame;

                if (framesLeft <= COMBO_WINDOW_FRAMES && stage->canBeComboed) {
                    if (SAFE_STATE(player->controls.attack)) {
                        player->comboExtendRequested = true;
                    }
                }
            }
        }
        return; // ignore all other inputs until attack ends, hopefully
    }


    // Movement inputs
    player->core.body.isMovingLeft  = SAFE_STATE(player->controls.moveLeft);
    player->core.body.isMovingRight = SAFE_STATE(player->controls.moveRight);
    player->isCrouching        = SAFE_STATE(player->controls.crouch);
    player->isDashing          = SAFE_STATE(player->controls.dash);

    // Jump input
    player->jumpRequested = SAFE_STATE(player->controls.jump);
    if (!player->core.body.isOnGround) {
        player->jumpRequested = false; // prevent double jump, for now, hehehe
    }

    // Attack input
    if (SAFE_STATE(player->controls.attack) && player->attackCoolDown <= 0.0f) {
        if (player->core.body.isOnGround) {
            player->core.isAttacking = true;
            Player_SetState(player, PLAYER_GROUND_ATTACKING);
        } else {
            player->core.isAttacking = true;
            Player_SetState(player, PLAYER_AIR_ATTACKING);
        }
    }

}




