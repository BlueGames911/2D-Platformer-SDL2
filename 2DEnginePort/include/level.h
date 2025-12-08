#pragma once

#include "constants.h"
#include "enemies.h"

#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <cJSON.h>

struct mainSystems;
typedef struct UI UI;
typedef struct Player Player;
typedef struct GameManager GameManager;
typedef struct Label Label;

typedef struct {
    char *name;  // e.g., "level1"
    char *path;  
} LevelEntry;

typedef struct LevelPaths{
    LevelEntry *levels;      // array of levels
    int count;               
    char *defaultLevel;      
    char *lastLoadedLevel;   
} LevelPaths;

typedef struct {
    bool enabled;
    int tileX, tileY;        
    int width, height;       
    char* targetLevel;       
    Label promptLabel;

    // Runtime
    bool playerInRange;
    SDL_Rect worldRect;      
} LevelTrigger;

typedef struct {
    char *id;              
    SDL_Texture *tex;
    int tileSize;           // in pixels 
    int tilesPerRow;        // how many tiles in source image row
    float scale;            // how much to scale when rendering
} Tileset;

typedef struct Layer {
    char *csvPath;          
    int *tiles;             // Array of integers used to store CSV contents
    char *tileset_id;       // which tileset to use for this layer
    bool collidable;         // Is the layer collidable
    int *solidTiles;        // list of tile indices that are considered solid for this layer
    int solidCount;         // number of items in solidTiles
} Layer;

typedef struct {
    char *imagePath;
    SDL_Texture *tex;
    float scrollSpeed;
    float scale;
    float offsetY;          // vertical placement
} BackgroundLayer;

 struct Level{
    char *name;             // level name
    Tileset *tilesets;     
    int tilesetCount;
    Layer *layers;        
    int layerCount;
    BackgroundLayer *bgs;
    int bgCount;
    int spawnColumn;
    int levelRows;
    int levelColumns;
    LevelTrigger trigger;

    enemy** enemies;
    int enemyCount;
} typedef Level;

 // Load and Unload
LevelPaths *levelPaths(char *path);
Level* loadLevelFromJSON(const char* jsonPath, GameManager* gm);
int* loadLayerCSV(const char* csvPath, int levelRows, int levelCols);
void unloadLevel(Level* level);
void freeLevelPaths(LevelPaths* lp);

// Utils
int getLevelTileWidth(Level *lvl);
bool level_isTileSolid(Level *lvl, int worldX, int worldY);
void spawnPlayerOnAnyCollidable(Player* p, int spawnCol, Level* lvl, bool searchFromTop, UI* ui);
Tileset* findTileset(Level* lvl, const char* id);

//Update (Yeah only one function for now lol)
void Level_Update(GameManager* gm);
void Level_UpdateTrigger(Level* lvl, Player* player);

// Renderering
void renderLevel(GameManager *gm);
void Level_RenderTriggerPrompt(Level* lvl, UI* ui, Camera* camera, Player* player);
void debug_draw_collidable_tiles(Level* lvl, SDL_Renderer* renderer, int cameraX, int cameraY);