#include "tilemap.h"
#include <SDL_image.h>
#include <stdio.h>
#include <string.h>


SDL_Texture *tileMapTexture;
SDL_Texture *sunnyLandsTexture;

// Sample level data (temporary, for testing)
int level[LEVEL_ROWS][LEVEL_COLS] = {0};
int sunnyLandLevel[LEVEL_ROWS][LEVEL_COLS] = {0};
int levelBackground[LEVEL_ROWS][LEVEL_COLS] = {0};
int levelBackground2[LEVEL_ROWS][LEVEL_COLS] = {0};

void loadTileMap(struct mainSystems *systems)
{
    SDL_Surface *tileMapSurface = IMG_Load("assets/mapFiles/Tileset.png");
    if (!tileMapSurface)
    {
        fprintf(stderr, "Failed to load tilemap: %s\n", IMG_GetError());
        cleanUp(systems, EXIT_FAILED);
    }

    SDL_Surface *sunnyLandsSurface = IMG_Load("assets/mapFiles/props.png");
    if (!sunnyLandsSurface) 
    {
        fprintf(stderr, "Failed to load sunnyLandsFile: %s\n", IMG_GetError());
        cleanUp(systems, EXIT_FAILED);
    }

    sunnyLandsTexture = NULL;
    tileMapTexture = NULL;

    tileMapTexture = SDL_CreateTextureFromSurface(systems->renderer, tileMapSurface);
    if (!tileMapTexture)
    {
        fprintf(stderr, "Failed to create tilemap texture: %s\n", SDL_GetError());
        SDL_FreeSurface(tileMapSurface);
        cleanUp(systems, EXIT_FAILED);
    }

    sunnyLandsTexture = SDL_CreateTextureFromSurface(systems->renderer, sunnyLandsSurface);
    if (!sunnyLandsTexture) 
    {
        fprintf(stderr, "Failed to create sunnyLands texture texture: %s\n", SDL_GetError());
        SDL_FreeSurface(tileMapSurface);
        cleanUp(systems, EXIT_FAILED);
    }

    loadMapLayer1("assets/maps/level1/TmpLevel._Tile Layer 1.csv", systems);
    loadSunnyLandsLayer("assets/maps/level1/TmpLevel._Sunny Lands layer.csv", systems);
    loadBackgroundLayer("assets/maps/level1/TmpLevel._background.csv", systems);
    loadBackgroundLayer2("assets/maps/level1/TmpLevel._Backgroung -1.csv", systems);

    SDL_FreeSurface(tileMapSurface);
    SDL_FreeSurface(sunnyLandsSurface);
}

void freeTileMap()
{
    SDL_DestroyTexture(tileMapTexture);
}

void renderMap(SDL_Renderer *renderer, int cameraX, int cameraY)
{
    renderBackground2(renderer, cameraX, cameraY);
    renderBackground(renderer, cameraX, cameraY);
    rendersunnyLandsLayer(renderer, cameraX, cameraY);
    renderTilemap(renderer, cameraX, cameraY);
}

void renderBackground2(SDL_Renderer *renderer, int cameraX, int cameraY)
{
    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            int tileIndex = levelBackground2[row][col];
            if (tileIndex >= 0)
            {
                SDL_Rect srcRect;
                srcRect.x = (tileIndex % 8) * TILE_SIZE; // tiles per row in tilesheet
                srcRect.y = (tileIndex / 8) * TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;

                SDL_Rect destRect;
                destRect.x = col * TILE_SIZE * TILE_SCALE - cameraX;
                destRect.y = row * TILE_SIZE * TILE_SCALE - cameraY;
                destRect.w = TILE_SIZE * TILE_SCALE;
                destRect.h = TILE_SIZE * TILE_SCALE;

                SDL_RenderCopy(renderer, tileMapTexture, &srcRect, &destRect);
            }
            
        }
    }   

}

void renderBackground(SDL_Renderer *renderer, int cameraX, int cameraY)
{
    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            int tileIndex = levelBackground[row][col];
            if (tileIndex >= 0)
            {
                SDL_Rect srcRect;
                srcRect.x = (tileIndex % 8) * TILE_SIZE; //  tiles per row in tilesheet
                srcRect.y = (tileIndex / 8) * TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;

                SDL_Rect destRect;
                destRect.x = col * TILE_SIZE * TILE_SCALE - cameraX;
                destRect.y = row * TILE_SIZE * TILE_SCALE - cameraY;
                destRect.w = TILE_SIZE * TILE_SCALE;
                destRect.h = TILE_SIZE * TILE_SCALE;

                SDL_RenderCopy(renderer, tileMapTexture, &srcRect, &destRect);
            }
            
        }
    }   

}

void rendersunnyLandsLayer(SDL_Renderer *renderer, int cameraX, int cameraY)
{
    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            int tileIndex = sunnyLandLevel[row][col];
            if (tileIndex >= 0)
            {
                SDL_Rect srcRect;
                srcRect.x = (tileIndex % 26) * TILE_SIZE; // tiles per row in tilesheet
                srcRect.y = (tileIndex / 26) * TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;


                SDL_Rect destRect;
                destRect.x = col * TILE_SIZE * TILE_SCALE - cameraX;
                destRect.y = row * TILE_SIZE * TILE_SCALE - cameraY;
                destRect.w = TILE_SIZE * TILE_SCALE;
                destRect.h = TILE_SIZE * TILE_SCALE;

                SDL_RenderCopy(renderer, sunnyLandsTexture, &srcRect, &destRect);
            }
            
        }
    }
}

void renderTilemap(SDL_Renderer *renderer, int cameraX, int cameraY)
{
    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            int tileIndex = level[row][col];

            if (tileIndex >= 0)
            {
                // TODO: Calculate tile source rect (on the tile sheet)
                SDL_Rect srcRect;
                srcRect.x = (tileIndex % 8) * TILE_SIZE; // tiles per row in tilesheet
                srcRect.y = (tileIndex / 8) * TILE_SIZE;
                srcRect.w = TILE_SIZE;
                srcRect.h = TILE_SIZE;
    
    
                SDL_Rect destRect;
                destRect.x = col * TILE_SIZE * TILE_SCALE - cameraX;
                destRect.y = row * TILE_SIZE * TILE_SCALE - cameraY;
                destRect.w = TILE_SIZE * TILE_SCALE;
                destRect.h = TILE_SIZE * TILE_SCALE;
    
                SDL_RenderCopy(renderer, tileMapTexture, &srcRect, &destRect);

                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for collision boxes
                SDL_RenderDrawRect(renderer, &destRect);
            }
        }
    }

}

void loadMapLayer1(const char *filename, struct mainSystems *systems)
{
    FILE *mapFile = fopen(filename, "r");
    if (!mapFile) 
    {
        fprintf(stderr, "Failed to open map file: %s\n", filename);
        cleanUp(systems, EXIT_FAILED);
        return;
    }

    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        char line[1024];
        if (!fgets(line, sizeof(line), mapFile)) 
        {
            fprintf(stderr, "Error reading line %d from map file: %s\n", row, filename);
            fclose(mapFile);
            cleanUp(systems, EXIT_FAILED);
            return;
        }

        char *token = strtok(line, ",");
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            if (token == NULL) 
            {
                fprintf(stderr, "Not enough data in row %d of map file: %s\n", row, filename);
                fclose(mapFile);
                cleanUp(systems, EXIT_FAILED);
                return;
            }

            int tileNumber = atoi(token);
            level[row][col] = tileNumber;

            token = strtok(NULL, ",");
        }
    }

    fclose(mapFile);
}

void loadSunnyLandsLayer(const char *filename, struct mainSystems *systems)
{

    FILE *mapFile = fopen(filename, "r");
    if (!mapFile) 
    {
        fprintf(stderr, "Failed to open map file: %s\n", filename);
        cleanUp(systems, EXIT_FAILED);
        return;
    }

    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        char line[1024];
        if (!fgets(line, sizeof(line), mapFile)) 
        {
            fprintf(stderr, "Error reading line %d from map file: %s\n", row, filename);
            fclose(mapFile);
            cleanUp(systems, EXIT_FAILED);
            return;
        }

        char *token = strtok(line, ",");
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            if (token == NULL) 
            {
                fprintf(stderr, "Not enough data in row %d of map file: %s\n", row, filename);
                fclose(mapFile);
                cleanUp(systems, EXIT_FAILED);
                return;
            }

            int tileNumber = atoi(token);
            sunnyLandLevel[row][col] = tileNumber;

            token = strtok(NULL, ",");
        }
    }

    fclose(mapFile);
}
void loadBackgroundLayer(const char *filename, struct mainSystems *systems)
{
    FILE *mapBackgroundFile = fopen(filename, "r");
    if (!mapBackgroundFile)
    {
        fprintf(stderr, "Failed to load %s", filename);
        cleanUp(systems, EXIT_FAILED);
    }

    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        char line[1024];
        if (!fgets(line, sizeof(line), mapBackgroundFile)) 
        {
            fprintf(stderr, "Error reading line %d from map file: %s\n", row, filename);
            fclose(mapBackgroundFile);
            cleanUp(systems, EXIT_FAILED);
            return;
        }

        char *token = strtok(line, ",");
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            if (token == NULL) 
            {
                fprintf(stderr, "Not enough data in row %d of map file: %s\n", row, filename);
                fclose(mapBackgroundFile);
                cleanUp(systems, EXIT_FAILED);
                return;
            }

            int tileNumber = atoi(token);
            levelBackground[row][col] = tileNumber;

            token = strtok(NULL, ",");
        }
    }
    fclose(mapBackgroundFile);
}

void loadBackgroundLayer2(const char *filename, struct mainSystems *systems)
{
    FILE *mapBackgroundFile2 = fopen(filename, "r");
    if (!mapBackgroundFile2)
    {
        fprintf(stderr, "Failed to load %s", filename);
        cleanUp(systems, EXIT_FAILED);
    }

    for (int row = 0; row < LEVEL_ROWS; row++)
    {
        char line[1024];
        if (!fgets(line, sizeof(line), mapBackgroundFile2)) 
        {
            fprintf(stderr, "Error reading line %d from map file: %s\n", row, filename);
            fclose(mapBackgroundFile2);
            cleanUp(systems, EXIT_FAILED);
            return;
        }

        char *token = strtok(line, ",");
        for (int col = 0; col < LEVEL_COLS; col++)
        {
            if (token == NULL) 
            {
                fprintf(stderr, "Not enough data in row %d of map file: %s\n", row, filename);
                fclose(mapBackgroundFile2);
                cleanUp(systems, EXIT_FAILED);
                return;
            }

            int tileNumber = atoi(token);
            levelBackground2[row][col] = tileNumber;

            token = strtok(NULL, ",");
        }
    }
    fclose(mapBackgroundFile2);
}



