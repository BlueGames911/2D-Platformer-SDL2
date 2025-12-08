#pragma once

#include <SDL.h>
#include <SDL_image.h>

// Simple texture cache structure
typedef struct {
    char* path;
    SDL_Texture* tex;
} TextureCacheItem;

SDL_Texture* loadTexture(const char* filepath, SDL_Renderer* renderer);

