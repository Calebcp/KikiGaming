#ifndef KIKIGAME_ASSETS_H
#define KIKIGAME_ASSETS_H

#include <raylib.h>
#include <stdbool.h>

typedef enum {
    BG_INTRO = 0,
    BG_LEVEL_ONE,
    BG_LEVEL_TWO,
    BG_LEVEL_THREE,
    BG_LEVEL_FOUR,
    BG_LEVEL_FIVE,
    BG_ENDING_GOOD,
    BG_COUNT
} BackgroundId;

bool InitGameAssets(void);
void UnloadGameAssets(void);

bool HasBackgroundTexture(BackgroundId id);
Texture2D GetBackgroundTexture(BackgroundId id);

bool HasHeroTexture(void);
Texture2D GetHeroTexture(void);

bool HasWizardTexture(void);
Texture2D GetWizardTexture(void);

bool HasAuntTexture(void);
Texture2D GetAuntTexture(void);

bool HasSpiderTexture(void);
Texture2D GetSpiderTexture(void);

#endif
