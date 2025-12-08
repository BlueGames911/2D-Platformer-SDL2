#include "ui.h"
#include "healthbar.h"
#include "utils.h"
#include "gameManager.h"
#include "player.h"
#include "texture.h"

#include <cJSON.h>

bool UI_Init(const char* path, UI* ui, int resHeight, int resWidth, SDL_Renderer* renderer)
{
    char* file = read_whole_file(path);
    if (!file)
    {
        fprintf(stderr, "[UI] Failed to read %s\n", path);
        return false;
    }

    cJSON* configFile = cJSON_Parse(file);
    free(file);

    if (!configFile)
    {
        fprintf(stderr, "[UI] Failed to parse %s\n", path);
        return false;
    }
    ui->renderer = renderer;

    cJSON* fontsArray = cJSON_GetObjectItemCaseSensitive(configFile, "fonts");
    ui->numOfFonts = cJSON_GetArraySize(fontsArray);
    ui->fonts = calloc(ui->numOfFonts, sizeof(Fonts));

    cJSON* fontObj;
    int idx = 0;
    cJSON_ArrayForEach(fontObj, fontsArray)
    {
        cJSON* name = cJSON_GetObjectItemCaseSensitive(fontObj, "name");
        cJSON* fontPath = cJSON_GetObjectItemCaseSensitive(fontObj, "path");
        cJSON* fontSize = cJSON_GetObjectItemCaseSensitive(fontObj, "fontSize");

        if (cJSON_IsString(name) && cJSON_IsString(fontPath) && cJSON_IsNumber(fontSize))
        {
            ui->fonts[idx].name = SDL_strdup(name->valuestring);
            ui->fonts[idx].fontSize = fontSize->valueint;

            ui->fonts[idx].font = TTF_OpenFont(fontPath->valuestring, fontSize->valueint);
            if (!ui->fonts[idx].font)
            {
                fprintf(stderr, "[UI] Failed to open font from %s\n", fontPath->valuestring);
                cJSON_Delete(configFile);
                UI_Free(ui);
                return false;
            }
        }
        else
        {
            fprintf(stderr, "[UI] Invalid font data at index %d\n", idx);
            cJSON_Delete(configFile);
            return false;
        }

        idx++;
    }

    cJSON* FPath = cJSON_GetObjectItemCaseSensitive(configFile, "FKeyPath");
    if (cJSON_IsString(FPath))
    {
        SDL_Texture* tex = loadTexture(FPath->valuestring, ui->renderer);
        if (tex)
        {
            ui->fkeyTex = tex;
        }
        else
        {
            ui->fkeyTex = NULL;
        }
    }

    cJSON_Delete(configFile);
    ui->resHeight = resHeight;
    ui->resWidth = resWidth;
    return true;
}

void UI_Free(UI* ui)
{
    if (!ui) return;

    if (ui->fonts)
    {
        for (int i = 0; i < ui->numOfFonts; i++)
        {
            if (ui->fonts[i].name) SDL_free(ui->fonts[i].name);
            if (ui->fonts[i].font) TTF_CloseFont(ui->fonts[i].font);
        }
    }
    if (ui->fkeyTex) SDL_DestroyTexture(ui->fkeyTex);
}

void UI_Update(UI* ui)
{

}

void UI_Render(GameManager* gm)
{
    HealthBar_Render(&gm->player->core.healthBar, gm->mainSystems.renderer, &gm->camera);
}

void UI_RenderLabel(SDL_Renderer* renderer, Label* label)
{
    if (!label || !label->text || !label->font.font) return;

    if (!label->wordTexture)
    {
        SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(label->font.font,
            label->text,
            label->colour, 0);
        if (!surface) return;

        int surfW = surface->w;
        int surfH = surface->h;

        label->wordRect.w = surfW;
        label->wordRect.h = surfH;
        label->wordRect.x = label->x - label->wordRect.w / 2;
        label->wordRect.y = label->y;

        label->wordTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!label->wordTexture) return;
    }

    SDL_Rect shadow = label->wordRect;
    shadow.x += 2;
    shadow.y += 2;
    SDL_SetTextureAlphaMod(label->wordTexture, 160);
    SDL_SetTextureColorMod(label->wordTexture, 0, 0, 0);
    SDL_RenderCopy(renderer, label->wordTexture, NULL, &shadow);

    SDL_SetTextureColorMod(label->wordTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(label->wordTexture, 255);
    SDL_RenderCopy(renderer, label->wordTexture, NULL, &label->wordRect);
}
