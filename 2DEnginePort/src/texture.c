#include "texture.h"

#include <stdlib.h>
#include <string.h>

static TextureCacheItem *textureCache = NULL;
static int textureCacheCount = 0;

// Loads a texture from file (no caching)
SDL_Texture* loadTexture(const char* filepath, SDL_Renderer* renderer) {
    SDL_Surface *surface = IMG_Load(filepath);
    if (!surface) return NULL;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
