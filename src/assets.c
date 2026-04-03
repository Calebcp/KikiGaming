#include "assets.h"

#include <string.h>

typedef struct {
    Texture2D backgrounds[BG_COUNT];
    bool backgroundLoaded[BG_COUNT];
    Texture2D hero;
    bool heroLoaded;
    Texture2D wizard;
    bool wizardLoaded;
    Texture2D aunt;
    bool auntLoaded;
    Texture2D spider;
    bool spiderLoaded;
} GameAssets;

static GameAssets g_assets = {0};

static bool LoadTextureIfPresent(Texture2D *texture, const char *primaryPath, const char *fallbackPath) {
    const char *path = NULL;

    if (FileExists(primaryPath)) {
        path = primaryPath;
    } else if (fallbackPath != NULL && FileExists(fallbackPath)) {
        path = fallbackPath;
    }

    if (path == NULL) {
        return false;
    }

    *texture = LoadTexture(path);
    return texture->id != 0;
}

bool InitGameAssets(void) {
    const char *appDir = GetApplicationDirectory();

    g_assets.backgroundLoaded[BG_INTRO] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_INTRO],
        TextFormat("%sassets/backgrounds/intro.jpg", appDir),
        "assets/backgrounds/intro.jpg");
    g_assets.backgroundLoaded[BG_LEVEL_ONE] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_LEVEL_ONE],
        TextFormat("%sassets/backgrounds/level_one.png", appDir),
        "assets/backgrounds/level_one.png");
    g_assets.backgroundLoaded[BG_LEVEL_TWO] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_LEVEL_TWO],
        TextFormat("%sassets/backgrounds/level_two.jpg", appDir),
        "assets/backgrounds/level_two.jpg");
    g_assets.backgroundLoaded[BG_LEVEL_THREE] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_LEVEL_THREE],
        TextFormat("%sassets/backgrounds/level_three.jpg", appDir),
        "assets/backgrounds/level_three.jpg");
    g_assets.backgroundLoaded[BG_LEVEL_FOUR] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_LEVEL_FOUR],
        TextFormat("%sassets/backgrounds/level_four.jpg", appDir),
        "assets/backgrounds/level_four.jpg");
    g_assets.backgroundLoaded[BG_LEVEL_FIVE] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_LEVEL_FIVE],
        TextFormat("%sassets/backgrounds/level_five.jpg", appDir),
        "assets/backgrounds/level_five.jpg");
    g_assets.backgroundLoaded[BG_ENDING_GOOD] = LoadTextureIfPresent(
        &g_assets.backgrounds[BG_ENDING_GOOD],
        TextFormat("%sassets/backgrounds/good_ending.jpg", appDir),
        "assets/backgrounds/good_ending.jpg");

    g_assets.heroLoaded = LoadTextureIfPresent(
        &g_assets.hero,
        TextFormat("%sassets/sprites/hero.png", appDir),
        "assets/sprites/hero.png");
    g_assets.wizardLoaded = LoadTextureIfPresent(
        &g_assets.wizard,
        TextFormat("%sassets/sprites/evil_wizard.png", appDir),
        "assets/sprites/evil_wizard.png");
    g_assets.auntLoaded = LoadTextureIfPresent(
        &g_assets.aunt,
        TextFormat("%sassets/sprites/aunt.png", appDir),
        "assets/sprites/aunt.png");
    g_assets.spiderLoaded = LoadTextureIfPresent(
        &g_assets.spider,
        TextFormat("%sassets/sprites/spider_enemy.png", appDir),
        "assets/sprites/spider_enemy.png");

    return true;
}

void UnloadGameAssets(void) {
    for (int i = 0; i < BG_COUNT; i++) {
        if (g_assets.backgroundLoaded[i]) {
            UnloadTexture(g_assets.backgrounds[i]);
        }
    }

    if (g_assets.heroLoaded) {
        UnloadTexture(g_assets.hero);
    }
    if (g_assets.wizardLoaded) {
        UnloadTexture(g_assets.wizard);
    }
    if (g_assets.auntLoaded) {
        UnloadTexture(g_assets.aunt);
    }
    if (g_assets.spiderLoaded) {
        UnloadTexture(g_assets.spider);
    }

    memset(&g_assets, 0, sizeof(g_assets));
}

bool HasBackgroundTexture(BackgroundId id) {
    return id >= 0 && id < BG_COUNT && g_assets.backgroundLoaded[id];
}

Texture2D GetBackgroundTexture(BackgroundId id) {
    return g_assets.backgrounds[id];
}

bool HasHeroTexture(void) {
    return g_assets.heroLoaded;
}

Texture2D GetHeroTexture(void) {
    return g_assets.hero;
}

bool HasWizardTexture(void) {
    return g_assets.wizardLoaded;
}

Texture2D GetWizardTexture(void) {
    return g_assets.wizard;
}

bool HasAuntTexture(void) {
    return g_assets.auntLoaded;
}

Texture2D GetAuntTexture(void) {
    return g_assets.aunt;
}

bool HasSpiderTexture(void) {
    return g_assets.spiderLoaded;
}

Texture2D GetSpiderTexture(void) {
    return g_assets.spider;
}
