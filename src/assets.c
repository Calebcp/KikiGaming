#include "assets.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Texture2D backgrounds[BG_COUNT];
    bool backgroundLoaded[BG_COUNT];
    Texture2D hero;
    bool heroLoaded;
    Texture2D heroAttackLeft;
    bool heroAttackLeftLoaded;
    Texture2D heroAttackRight;
    bool heroAttackRightLoaded;
    Texture2D heart;
    bool heartLoaded;
    Texture2D wizard;
    bool wizardLoaded;
    Texture2D aunt;
    bool auntLoaded;
    Texture2D spider;
    bool spiderLoaded;
    Texture2D dragon;
    bool dragonLoaded;
} GameAssets;

static GameAssets g_assets = {0};
static Sound g_attackSound = {0};
static bool g_attackSoundLoaded = false;
static Sound g_hitSound = {0};
static bool g_hitSoundLoaded = false;
static AudioStream g_jungleStream = {0};
static bool g_jungleStreamLoaded = false;
static bool g_audioEnabled = true;
static bool g_audioAvailable = false;
static short g_jungleBuffer[2048];
static float g_junglePhaseA = 0.0f;
static float g_junglePhaseB = 0.0f;
static float g_junglePhaseC = 0.0f;

static void FillJungleAudioBuffer(short *buffer, int frameCount) {
    const float sampleRate = 22050.0f;
    for (int i = 0; i < frameCount; i++) {
        float chirp = sinf(g_junglePhaseA) * 0.18f + sinf(g_junglePhaseB) * 0.08f;
        float rumble = sinf(g_junglePhaseC) * 0.10f;
        float pulse = sinf(g_junglePhaseA * 0.17f) * sinf(g_junglePhaseB * 0.09f) * 0.12f;
        float sample = chirp + rumble + pulse;
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        buffer[i] = (short)(sample * 4200.0f);

        g_junglePhaseA += 2.0f * PI * 480.0f / sampleRate;
        g_junglePhaseB += 2.0f * PI * 760.0f / sampleRate;
        g_junglePhaseC += 2.0f * PI * 52.0f / sampleRate;
        if (g_junglePhaseA > 2.0f * PI) g_junglePhaseA -= 2.0f * PI;
        if (g_junglePhaseB > 2.0f * PI) g_junglePhaseB -= 2.0f * PI;
        if (g_junglePhaseC > 2.0f * PI) g_junglePhaseC -= 2.0f * PI;
    }
}

static bool InitAttackSound(void) {
    const int sampleRate = 22050;
    const int sampleCount = 2600;
    short *samples = (short *)malloc(sizeof(short) * sampleCount);
    if (samples == NULL) {
        return false;
    }

    float phaseA = 0.0f;
    float phaseB = 0.0f;
    for (int i = 0; i < sampleCount; i++) {
        float t = (float)i / (float)sampleRate;
        float envelope = (1.0f - (float)i / (float)sampleCount);
        float freqA = 420.0f + 560.0f * (1.0f - (float)i / (float)sampleCount);
        float freqB = 120.0f + 180.0f * sinf(t * 18.0f);
        phaseA += 2.0f * PI * freqA / (float)sampleRate;
        phaseB += 2.0f * PI * freqB / (float)sampleRate;
        float sample = sinf(phaseA) * 0.65f + sinf(phaseB) * 0.25f;
        sample *= envelope * envelope;
        samples[i] = (short)(sample * 18000.0f);
    }

    Wave wave = {
        .frameCount = sampleCount,
        .sampleRate = sampleRate,
        .sampleSize = 16,
        .channels = 1,
        .data = samples
    };
    g_attackSound = LoadSoundFromWave(wave);
    g_attackSoundLoaded = g_attackSound.frameCount > 0;
    UnloadWave(wave);
    if (g_attackSoundLoaded) {
        SetSoundVolume(g_attackSound, 0.78f);
    }
    return g_attackSoundLoaded;
}

static bool InitHitSound(void) {
    const int sampleRate = 22050;
    const int sampleCount = 1800;
    short *samples = (short *)malloc(sizeof(short) * sampleCount);
    if (samples == NULL) {
        return false;
    }

    float phaseA = 0.0f;
    float phaseB = 0.0f;
    for (int i = 0; i < sampleCount; i++) {
        float progress = (float)i / (float)sampleCount;
        float envelope = (1.0f - progress);
        float freqA = 920.0f - 520.0f * progress;
        float freqB = 180.0f + 120.0f * sinf(progress * 14.0f);
        phaseA += 2.0f * PI * freqA / (float)sampleRate;
        phaseB += 2.0f * PI * freqB / (float)sampleRate;
        float click = ((i % 23) < 7 ? 1.0f : -1.0f) * 0.12f;
        float sample = sinf(phaseA) * 0.58f + sinf(phaseB) * 0.26f + click;
        sample *= envelope * envelope;
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        samples[i] = (short)(sample * 18000.0f);
    }

    Wave wave = {
        .frameCount = sampleCount,
        .sampleRate = sampleRate,
        .sampleSize = 16,
        .channels = 1,
        .data = samples
    };
    g_hitSound = LoadSoundFromWave(wave);
    g_hitSoundLoaded = g_hitSound.frameCount > 0;
    UnloadWave(wave);
    if (g_hitSoundLoaded) {
        SetSoundVolume(g_hitSound, 0.86f);
    }
    return g_hitSoundLoaded;
}

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
    g_assets.heroAttackLeftLoaded = LoadTextureIfPresent(
        &g_assets.heroAttackLeft,
        TextFormat("%sassets/sprites/hitting_left.PNG", appDir),
        "assets/sprites/hitting_left.PNG");
    g_assets.heroAttackRightLoaded = LoadTextureIfPresent(
        &g_assets.heroAttackRight,
        TextFormat("%sassets/sprites/hitting_right.PNG", appDir),
        "assets/sprites/hitting_right.PNG");
    g_assets.heartLoaded = LoadTextureIfPresent(
        &g_assets.heart,
        TextFormat("%sassets/sprites/heart_kiki.PNG", appDir),
        "assets/sprites/heart_kiki.PNG");
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
    g_assets.dragonLoaded = LoadTextureIfPresent(
        &g_assets.dragon,
        TextFormat("%sassets/sprites/dragon_kiki.PNG", appDir),
        "assets/sprites/dragon_kiki.PNG");

    g_audioAvailable = IsAudioDeviceReady();
    if (g_audioAvailable) {
        SetMasterVolume(g_audioEnabled ? 1.0f : 0.0f);
        InitAttackSound();
        InitHitSound();
        g_jungleStream = LoadAudioStream(22050, 16, 1);
        g_jungleStreamLoaded = g_jungleStream.sampleRate > 0;
        if (g_jungleStreamLoaded) {
            SetAudioStreamVolume(g_jungleStream, 0.62f);
            FillJungleAudioBuffer(g_jungleBuffer, 2048);
            UpdateAudioStream(g_jungleStream, g_jungleBuffer, 2048);
            PlayAudioStream(g_jungleStream);
        }
        g_audioAvailable = g_attackSoundLoaded || g_hitSoundLoaded || g_jungleStreamLoaded;
    }

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
    if (g_assets.heroAttackLeftLoaded) {
        UnloadTexture(g_assets.heroAttackLeft);
    }
    if (g_assets.heroAttackRightLoaded) {
        UnloadTexture(g_assets.heroAttackRight);
    }
    if (g_assets.heartLoaded) {
        UnloadTexture(g_assets.heart);
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
    if (g_assets.dragonLoaded) {
        UnloadTexture(g_assets.dragon);
    }
    if (g_attackSoundLoaded) {
        UnloadSound(g_attackSound);
    }
    if (g_hitSoundLoaded) {
        UnloadSound(g_hitSound);
    }
    if (g_jungleStreamLoaded) {
        StopAudioStream(g_jungleStream);
        UnloadAudioStream(g_jungleStream);
    }

    memset(&g_assets, 0, sizeof(g_assets));
    g_attackSound = (Sound){0};
    g_attackSoundLoaded = false;
    g_hitSound = (Sound){0};
    g_hitSoundLoaded = false;
    g_jungleStream = (AudioStream){0};
    g_jungleStreamLoaded = false;
    g_audioAvailable = false;
}

void UpdateGameAudio(void) {
    if (g_jungleStreamLoaded && IsAudioStreamProcessed(g_jungleStream)) {
        FillJungleAudioBuffer(g_jungleBuffer, 2048);
        UpdateAudioStream(g_jungleStream, g_jungleBuffer, 2048);
    }
}

void PlayAttackSound(void) {
    if (g_audioEnabled && g_attackSoundLoaded) {
        PlaySound(g_attackSound);
    }
}

void PlayHitSound(void) {
    if (g_audioEnabled && g_hitSoundLoaded) {
        PlaySound(g_hitSound);
    }
}

void SetAudioEnabled(bool enabled) {
    g_audioEnabled = enabled;
    if (IsAudioDeviceReady()) {
        SetMasterVolume(g_audioEnabled ? 1.0f : 0.0f);
    }
    if (g_audioEnabled && g_audioAvailable && g_hitSoundLoaded) {
        PlaySound(g_hitSound);
    }
}

bool IsAudioEnabled(void) {
    return g_audioEnabled;
}

bool IsAudioAvailable(void) {
    return g_audioAvailable;
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

bool HasHeroAttackLeftTexture(void) {
    return g_assets.heroAttackLeftLoaded;
}

Texture2D GetHeroAttackLeftTexture(void) {
    return g_assets.heroAttackLeft;
}

bool HasHeroAttackRightTexture(void) {
    return g_assets.heroAttackRightLoaded;
}

Texture2D GetHeroAttackRightTexture(void) {
    return g_assets.heroAttackRight;
}

bool HasHeartTexture(void) {
    return g_assets.heartLoaded;
}

Texture2D GetHeartTexture(void) {
    return g_assets.heart;
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

bool HasDragonTexture(void) {
    return g_assets.dragonLoaded;
}

Texture2D GetDragonTexture(void) {
    return g_assets.dragon;
}
