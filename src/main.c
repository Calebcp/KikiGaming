#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_W 1280
#define SCREEN_H 720
#define MAX_LEVELS 5
#define MAX_ENEMIES 8
#define MAX_ORBS 10
#define MAX_GLYPHS 4

typedef enum {
    SCENE_TITLE = 0,
    SCENE_STORY,
    SCENE_DIALOGUE,
    SCENE_MAP,
    SCENE_LEVEL,
    SCENE_END
} Scene;

typedef enum {
    ENEMY_HUNTER = 0,
    ENEMY_RUSHER,
    ENEMY_ORBITER
} EnemyKind;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    float speed;
    bool active;
    EnemyKind kind;
    float wobble;
} Enemy;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float radius;
    bool active;
} Orb;

typedef struct {
    Rectangle rect;
    bool active;
} Glyph;

typedef struct {
    int currentLevel;
    bool unlocked[MAX_LEVELS];
    bool completed[MAX_LEVELS];
    int menuSelection;
    int storyPage;
    int dialoguePage;

    int hearts;
    bool win;
    bool endingChoiceMercy;
    bool compassEarned;

    float levelTimer;
    float damageCooldown;
    float attackTimer;
    float dashTimer;
    float runeFlash;
    float endingDelay;

    Vector2 playerPos;
    Vector2 playerVel;
    Vector2 playerAim;
    bool facingRight;
    bool onGround;
    bool canDoubleJump;
    bool choiceSubmitted;
    int puzzleChoice;

    Enemy enemies[MAX_ENEMIES];
    int enemyCount;
    Orb orbs[MAX_ORBS];
    int orbCount;
    Glyph glyphs[MAX_GLYPHS];
    int glyphCount;
    int glyphsCollected;
    int runeOrder[3];
    int runeProgress;
    int finalChoice;
} GameData;

static const Color SKY_TOP = {16, 42, 58, 255};
static const Color SKY_BOTTOM = {6, 14, 20, 255};
static const Color CANOPY_A = {28, 84, 60, 255};
static const Color CANOPY_B = {16, 58, 42, 255};
static const Color TEMPLE_STONE = {99, 92, 74, 255};
static const Color TEMPLE_DARK = {43, 39, 32, 255};
static const Color PLAYER_SKIN = {255, 220, 177, 255};
static const Color PLAYER_JACKET = {51, 120, 196, 255};
static const Color ACCENT_GOLD = {224, 191, 87, 255};
static const Color CURSE_BLUE = {72, 194, 216, 255};
static const Color DANGER_RED = {195, 67, 58, 255};

static const char *MENU_ITEMS[3] = {
    "Start Expedition",
    "Read Controls",
    "Quit"
};

static const char *LEVEL_NAMES[MAX_LEVELS] = {
    "Flood Run",
    "Nest Breaker",
    "Rune Trial",
    "Serpent Maze",
    "Heart Of The Gate"
};

static const char *LEVEL_GOALS[MAX_LEVELS] = {
    "Platform through the flood and outrun hunters.",
    "Fight mobile spiders and survive the swarm.",
    "Read the clue, touch runes in order, dodge cursed orbs.",
    "Use dash and route planning to escape the serpent.",
    "Collect temple sigils, then choose mercy or escape."
};

static const char *STORY_HEADINGS[5] = {
    "Storm Over The Dig Site",
    "The Warning He Ignores",
    "A Door That Breathes",
    "The Prisoner Wizard",
    "Five Trials, One Debt"
};

static const char *STORY_LINES[5][3] = {
    {
        "Kiki arrives with his aunt's expedition just as thunder rolls over the jungle canopy.",
        "The crew maps ruins by torchlight, but a buried temple arch glows beneath the vines.",
        "Curiosity beats caution before the rain even starts."
    },
    {
        "His aunt catches him staring at the arch and orders him to stay with the camp.",
        "She says the carvings describe a place that punishes thieves, wanderers, and fools alike.",
        "Kiki hears the warning and walks toward the stone anyway."
    },
    {
        "Inside, the air turns cold, the walls pulse with blue fire, and the entrance seals behind him.",
        "The temple is not empty; it is awake, listening, and built to test anyone desperate enough to enter.",
        "Every corridor ahead looks less like architecture and more like a trap with a memory."
    },
    {
        "A wizard appears, not as a master of the temple, but as another survivor trapped by it years ago.",
        "He says the temple feeds on fear and offers freedom only to someone who can move, think, and choose under pressure.",
        "He cannot leave unless Kiki breaks the curse at its center."
    },
    {
        "To see his aunt again, Kiki must cross floodwater, shatter the nest, read the runes, outpace the serpent, and reach the heart gate.",
        "Each trial changes the temple, making the final chamber harder the longer he hesitates.",
        "The only way out is to become stronger than the place trying to keep him."
    }
};

static const char *DIALOGUE_SPEAKERS[4] = {
    "Wizard",
    "Kiki",
    "Wizard",
    "Kiki"
};

static const char *DIALOGUE_LINES[4][2] = {
    {
        "The gate did not choose a victim. It chose a will to test.",
        "If you want out, stop behaving like a tourist and start surviving like a thief."
    },
    {
        "Then tell me the rules. I am not dying in here because I touched the wrong stone.",
        "My aunt is outside. I am getting back to her."
    },
    {
        "Good. The flood breaks cowards, the nest punishes hesitation, and the heart gate judges selfishness.",
        "Move fast, fight hard, and do not trust stillness. In this place, stillness is usually bait."
    },
    {
        "Then open the first chamber. I will earn my own way out.",
        "And when this is over, you are leaving with me."
    }
};

static Rectangle GetPlayerRect(const GameData *g) {
    return (Rectangle){g->playerPos.x, g->playerPos.y, 36, 56};
}

static Rectangle GetEnemyRect(const Enemy *enemy) {
    return (Rectangle){enemy->pos.x - enemy->radius, enemy->pos.y - enemy->radius, enemy->radius * 2.0f, enemy->radius * 2.0f};
}

static void ResetTransientState(GameData *g) {
    g->levelTimer = 0.0f;
    g->damageCooldown = 0.0f;
    g->attackTimer = 0.0f;
    g->dashTimer = 0.0f;
    g->runeFlash = 0.0f;
    g->playerVel = Vector2Zero();
    g->choiceSubmitted = false;
    g->puzzleChoice = 0;
    g->glyphsCollected = 0;
    g->runeProgress = 0;
    g->finalChoice = 0;
    g->orbCount = 0;
    g->enemyCount = 0;
    g->glyphCount = 0;
    g->onGround = false;
    g->canDoubleJump = false;
}

static void ResetGame(GameData *g) {
    *g = (GameData){0};
    g->unlocked[0] = true;
    g->hearts = 5;
    g->facingRight = true;
    g->playerAim = (Vector2){1.0f, 0.0f};
}

static void StartLevel(GameData *g, int level) {
    ResetTransientState(g);
    g->currentLevel = level;

    if (level == 1) {
        g->playerPos = (Vector2){70, 548};
        g->facingRight = true;
        g->onGround = true;
        g->canDoubleJump = true;
        g->enemyCount = 4;
        g->enemies[0] = (Enemy){(Vector2){310, 590}, Vector2Zero(), 24, 150.0f, true, ENEMY_HUNTER, 0.0f};
        g->enemies[1] = (Enemy){(Vector2){585, 482}, Vector2Zero(), 24, 185.0f, true, ENEMY_RUSHER, 0.7f};
        g->enemies[2] = (Enemy){(Vector2){860, 590}, Vector2Zero(), 24, 170.0f, true, ENEMY_HUNTER, 1.4f};
        g->enemies[3] = (Enemy){(Vector2){1065, 398}, Vector2Zero(), 22, 205.0f, true, ENEMY_RUSHER, 2.2f};
    } else if (level == 2) {
        g->playerPos = (Vector2){130, 560};
        g->facingRight = true;
        g->onGround = true;
        g->enemyCount = 5;
        g->enemies[0] = (Enemy){(Vector2){690, 560}, Vector2Zero(), 24, 120.0f, true, ENEMY_HUNTER, 0.0f};
        g->enemies[1] = (Enemy){(Vector2){870, 505}, Vector2Zero(), 22, 150.0f, true, ENEMY_RUSHER, 1.3f};
        g->enemies[2] = (Enemy){(Vector2){980, 610}, Vector2Zero(), 20, 105.0f, true, ENEMY_ORBITER, 2.1f};
        g->enemies[3] = (Enemy){(Vector2){570, 430}, Vector2Zero(), 21, 135.0f, true, ENEMY_HUNTER, 0.8f};
        g->enemies[4] = (Enemy){(Vector2){760, 360}, Vector2Zero(), 20, 165.0f, true, ENEMY_RUSHER, 2.8f};
    } else if (level == 3) {
        g->playerPos = (Vector2){110, 570};
        g->facingRight = true;
        g->glyphCount = 3;
        g->glyphs[0] = (Glyph){(Rectangle){260, 160, 110, 110}, true};
        g->glyphs[1] = (Glyph){(Rectangle){620, 420, 110, 110}, true};
        g->glyphs[2] = (Glyph){(Rectangle){1010, 170, 110, 110}, true};
        g->runeOrder[0] = 2;
        g->runeOrder[1] = 0;
        g->runeOrder[2] = 1;
        g->orbCount = 5;
        for (int i = 0; i < g->orbCount; i++) {
            g->orbs[i] = (Orb){(Vector2){260.0f + i * 180.0f, 350.0f + ((i % 2) * 70.0f)}, Vector2Zero(), 14.0f, true};
        }
    } else if (level == 4) {
        g->playerPos = (Vector2){52, 78};
        g->facingRight = true;
        g->enemyCount = 0;
    } else if (level == 5) {
        g->playerPos = (Vector2){88, 560};
        g->facingRight = true;
        g->glyphCount = 3;
        g->glyphs[0] = (Glyph){(Rectangle){270, 208, 44, 44}, true};
        g->glyphs[1] = (Glyph){(Rectangle){690, 518, 44, 44}, true};
        g->glyphs[2] = (Glyph){(Rectangle){1060, 218, 44, 44}, true};
        g->orbCount = 6;
        for (int i = 0; i < g->orbCount; i++) {
            g->orbs[i] = (Orb){(Vector2){420.0f + i * 110.0f, 210.0f + (i % 3) * 120.0f}, Vector2Zero(), 12.0f, true};
        }
    }
}

static void DrawMistBand(float y, float alpha, float t) {
    for (int i = 0; i < 7; i++) {
        float x = 80.0f + i * 190.0f + sinf(t * 0.5f + i) * 45.0f;
        DrawEllipse((int)x, (int)y, 120, 34, Fade(RAYWHITE, alpha));
    }
}

static void DrawJungleBackdrop(float t) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, SKY_TOP, SKY_BOTTOM);
    DrawCircleGradient(1020, 135, 150, Fade(ACCENT_GOLD, 0.35f), BLANK);
    DrawCircleGradient(220, 90, 110, Fade(CURSE_BLUE, 0.16f), BLANK);

    for (int i = 0; i < 10; i++) {
        int x = 40 + i * 130;
        int sway = (int)(sinf(t * 0.9f + i * 0.6f) * 16.0f);
        DrawRectangle(x + sway, 220, 28, 340, (Color){52, 34, 18, 255});
        DrawCircle(x + 12 + sway, 205, 82, CANOPY_A);
        DrawCircle(x - 22 + sway, 240, 64, CANOPY_B);
        DrawCircle(x + 46 + sway, 238, 58, Fade((Color){62, 129, 88, 255}, 0.95f));
    }

    for (int i = 0; i < 14; i++) {
        float vineX = 25.0f + i * 93.0f;
        float swing = cosf(t * 0.85f + i) * 18.0f;
        DrawLineEx((Vector2){vineX, 0}, (Vector2){vineX + swing, 245}, 4.0f, Fade(LIME, 0.28f));
        DrawCircleV((Vector2){vineX + swing, 245}, 8, Fade(LIME, 0.20f));
    }

    DrawRectangle(0, 565, SCREEN_W, 155, (Color){31, 78, 44, 255});
    DrawRectangle(0, 600, SCREEN_W, 120, (Color){22, 55, 31, 255});
    DrawMistBand(530.0f, 0.07f, t);
}

static void DrawTempleFrame(float t) {
    DrawRectangleRounded((Rectangle){810, 145, 330, 320}, 0.05f, 8, TEMPLE_STONE);
    DrawRectangle(860, 188, 230, 245, TEMPLE_DARK);
    DrawRectangleLinesEx((Rectangle){860, 188, 230, 245}, 6.0f, Fade(ACCENT_GOLD, 0.42f));
    DrawTriangle((Vector2){785, 148}, (Vector2){975, 40}, (Vector2){1165, 148}, (Color){115, 106, 87, 255});
    DrawCircle(975, 314, 30 + (int)(sinf(t * 2.1f) * 5.0f), Fade(CURSE_BLUE, 0.30f));
    DrawRing((Vector2){975, 314}, 46, 54, 0, 360, 60, Fade(ACCENT_GOLD, 0.28f));
}

static void DrawHero(Vector2 pos, bool facingRight, bool attacking, float t) {
    int dir = facingRight ? 1 : -1;
    float stride = sinf(t * 12.0f) * 2.0f;

    DrawCircleV((Vector2){pos.x + 18, pos.y + 12}, 12, PLAYER_SKIN);
    DrawRectangleRounded((Rectangle){pos.x + 8, pos.y + 24, 20, 24}, 0.3f, 4, PLAYER_JACKET);
    DrawRectangle((int)pos.x + 11, (int)pos.y + 18, 14, 4, (Color){40, 28, 21, 255});
    DrawLineEx((Vector2){pos.x + 13, pos.y + 48}, (Vector2){pos.x + 10 + stride, pos.y + 60}, 4.0f, DARKBLUE);
    DrawLineEx((Vector2){pos.x + 23, pos.y + 48}, (Vector2){pos.x + 25 - stride, pos.y + 60}, 4.0f, DARKBLUE);
    DrawLineEx((Vector2){pos.x + 11, pos.y + 30}, (Vector2){pos.x + 3, pos.y + 43}, 4.0f, PLAYER_JACKET);
    DrawLineEx((Vector2){pos.x + 25, pos.y + 30},
               (Vector2){pos.x + 30 + (attacking ? dir * 10.0f : dir * 2.0f), pos.y + 42},
               4.0f, PLAYER_JACKET);
    DrawRectangle((int)pos.x + 6, (int)pos.y + 58, 8, 3, DARKBROWN);
    DrawRectangle((int)pos.x + 22, (int)pos.y + 58, 8, 3, DARKBROWN);

    if (attacking) {
        Vector2 slashCenter = {pos.x + 18 + dir * 24.0f, pos.y + 34};
        DrawRing(slashCenter, 18, 29, facingRight ? -35 : 145, facingRight ? 65 : 245, 16, Fade(ACCENT_GOLD, 0.80f));
    }
}

static void DrawCharacter(Vector2 pos, Color body, Color head, bool staff) {
    DrawCircleV((Vector2){pos.x, pos.y - 78}, 22, head);
    DrawRectangleRounded((Rectangle){pos.x - 16, pos.y - 60, 32, 70}, 0.3f, 6, body);
    DrawLineEx((Vector2){pos.x - 10, pos.y + 10}, (Vector2){pos.x - 22, pos.y + 48}, 4.0f, body);
    DrawLineEx((Vector2){pos.x + 10, pos.y + 10}, (Vector2){pos.x + 22, pos.y + 48}, 4.0f, body);
    DrawLineEx((Vector2){pos.x - 14, pos.y - 35}, (Vector2){pos.x - 32, pos.y - 2}, 4.0f, body);
    DrawLineEx((Vector2){pos.x + 14, pos.y - 35}, (Vector2){pos.x + 32, pos.y - 2}, 4.0f, body);

    if (staff) {
        DrawLineEx((Vector2){pos.x + 28, pos.y - 55}, (Vector2){pos.x + 32, pos.y + 55}, 5.0f, ACCENT_GOLD);
        DrawCircle((int)pos.x + 30, (int)pos.y - 62, 8, Fade(CURSE_BLUE, 0.9f));
    }
}

static void DrawSpider(Vector2 pos, float scale, float t, Color tint) {
    float legSwing = sinf(t * 9.0f + pos.x * 0.01f) * 6.0f;
    DrawCircleV(pos, 18 * scale, tint);
    DrawCircleV((Vector2){pos.x - 18 * scale, pos.y - 10 * scale}, 11 * scale, Fade(BLACK, 0.9f));
    DrawCircleV((Vector2){pos.x + 7 * scale, pos.y - 5 * scale}, 3 * scale, RAYWHITE);
    DrawCircleV((Vector2){pos.x + 16 * scale, pos.y - 5 * scale}, 3 * scale, RAYWHITE);

    for (int i = 0; i < 4; i++) {
        float yOff = -10.0f + i * 8.0f;
        DrawLineEx((Vector2){pos.x - 4, pos.y + yOff}, (Vector2){pos.x - 25 - legSwing, pos.y + yOff - 12}, 3.0f, BLACK);
        DrawLineEx((Vector2){pos.x + 4, pos.y + yOff}, (Vector2){pos.x + 25 + legSwing, pos.y + yOff - 12}, 3.0f, BLACK);
    }
}

static void DrawSnakeBoss(Vector2 pos, float t) {
    for (int i = 0; i < 6; i++) {
        float wave = sinf(t * 6.0f + i * 0.5f) * 10.0f;
        DrawCircleV((Vector2){pos.x - i * 18.0f, pos.y + wave}, 18 - i, (Color){87, 164, 89, 255});
    }
    DrawCircleV(pos, 24, (Color){62, 129, 74, 255});
    DrawCircleV((Vector2){pos.x + 12, pos.y - 6}, 4, BLACK);
    DrawCircleV((Vector2){pos.x + 12, pos.y + 6}, 4, BLACK);
    DrawRectangle((int)pos.x + 20, (int)pos.y - 2, 16, 4, DANGER_RED);
}

static void DrawHearts(int hearts) {
    DrawText("SPIRIT", 24, 18, 18, Fade(RAYWHITE, 0.82f));
    for (int i = 0; i < hearts; i++) {
        int x = 28 + i * 34;
        DrawCircle(x, 48, 9, DANGER_RED);
        DrawCircle(x + 12, 48, 9, DANGER_RED);
        DrawTriangle((Vector2){x - 8, 50}, (Vector2){x + 20, 50}, (Vector2){x + 6, 70}, DANGER_RED);
    }
}

static void DrawBanner(const char *title, const char *subtitle) {
    DrawRectangleRounded((Rectangle){18, 80, SCREEN_W - 36, 78}, 0.14f, 8, Fade(BLACK, 0.45f));
    DrawText(title, 34, 92, 30, RAYWHITE);
    DrawText(subtitle, 34, 126, 20, Fade(RAYWHITE, 0.82f));
}

static void DrawWrappedTextBlock(const char *text, Rectangle bounds, float fontSize, float spacing, Color color) {
    Font font = GetFontDefault();
    const char *cursor = text;
    char line[512] = {0};
    int lineLen = 0;
    float y = bounds.y;

    while (*cursor != '\0') {
        while (*cursor == ' ') {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        char word[128] = {0};
        int wordLen = 0;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\n' && wordLen < (int)sizeof(word) - 1) {
            word[wordLen++] = *cursor++;
        }
        word[wordLen] = '\0';

        char candidate[512] = {0};
        if (lineLen == 0) {
            snprintf(candidate, sizeof(candidate), "%s", word);
        } else {
            snprintf(candidate, sizeof(candidate), "%s %s", line, word);
        }

        float width = MeasureTextEx(font, candidate, fontSize, spacing).x;
        if ((width > bounds.width && lineLen > 0) || *cursor == '\n') {
            DrawTextEx(font, line, (Vector2){bounds.x, y}, fontSize, spacing, color);
            y += fontSize + 8.0f;
            snprintf(line, sizeof(line), "%s", word);
            lineLen = wordLen;
        } else {
            snprintf(line, sizeof(line), "%s", candidate);
            lineLen = (int)TextLength(line);
        }

        if (*cursor == '\n') {
            cursor++;
            DrawTextEx(font, line, (Vector2){bounds.x, y}, fontSize, spacing, color);
            y += fontSize + 8.0f;
            line[0] = '\0';
            lineLen = 0;
        }
    }

    if (lineLen > 0) {
        DrawTextEx(font, line, (Vector2){bounds.x, y}, fontSize, spacing, color);
    }
}

static void DamagePlayer(GameData *g) {
    if (g->damageCooldown > 0.0f) {
        return;
    }

    g->hearts--;
    g->damageCooldown = 1.1f;
}

static void CompleteLevel(GameData *g) {
    int index = g->currentLevel - 1;
    g->completed[index] = true;
    if (index + 1 < MAX_LEVELS) {
        g->unlocked[index + 1] = true;
    }
}

static void DrawTitleScene(const GameData *g) {
    float t = (float)GetTime();
    DrawJungleBackdrop(t);
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.10f), Fade(BLACK, 0.72f));
    DrawHero((Vector2){220, 540}, true, false, t);
    DrawCharacter((Vector2){1000, 540}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);

    DrawRectangleRounded((Rectangle){86, 108, 480, 506}, 0.06f, 8, Fade(BLACK, 0.58f));
    DrawText("KIKI", 128, 142, 72, RAYWHITE);
    DrawText("Lost In The Jungle", 130, 214, 42, ACCENT_GOLD);
    DrawText("A faster, harder, more active temple escape.", 132, 272, 24, Fade(RAYWHITE, 0.85f));
    DrawText("Move, fight, solve, dash, decide.", 132, 306, 24, Fade(RAYWHITE, 0.70f));

    for (int i = 0; i < 3; i++) {
        Rectangle box = {130, 362 + i * 82, 344, 58};
        bool selected = g->menuSelection == i;
        DrawRectangleRounded(box, 0.25f, 6, selected ? ACCENT_GOLD : Fade((Color){41, 31, 20, 255}, 0.92f));
        DrawText(MENU_ITEMS[i], 158, 380 + i * 82, 28, selected ? BLACK : RAYWHITE);
    }

    DrawText("W/S or arrows to select, ENTER to confirm", 132, 576, 20, Fade(RAYWHITE, 0.82f));
    DrawText("Temple build: substantial combat and story pass", 790, 630, 24, Fade(RAYWHITE, 0.72f));
}

static void DrawStoryScene(const GameData *g) {
    float t = (float)GetTime();
    int page = g->storyPage;

    DrawJungleBackdrop(t);
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.10f), Fade(BLACK, 0.68f));

    if (page == 0) {
        DrawCharacter((Vector2){268, 530}, PLAYER_JACKET, PLAYER_SKIN, false);
        DrawCharacter((Vector2){416, 530}, (Color){132, 82, 56, 255}, PLAYER_SKIN, false);
        DrawRectangleRounded((Rectangle){140, 454, 210, 34}, 0.3f, 6, Fade(RAYWHITE, 0.88f));
        DrawText("Expedition camp", 166, 462, 20, TEMPLE_DARK);
    } else if (page == 1) {
        DrawCharacter((Vector2){315, 536}, PLAYER_JACKET, PLAYER_SKIN, false);
        DrawCharacter((Vector2){475, 530}, (Color){132, 82, 56, 255}, PLAYER_SKIN, false);
        DrawLineEx((Vector2){455, 435}, (Vector2){360, 378}, 5.0f, DANGER_RED);
        DrawText("Do not cross that arch.", 312, 342, 30, DANGER_RED);
    } else if (page == 2) {
        DrawHero((Vector2){556, 540}, true, false, t);
        DrawRectangle(860, 192, 20, 242, GRAY);
        DrawRectangle(1070, 192, 20, 242, GRAY);
        DrawText("The temple seals shut.", 790, 116, 30, ACCENT_GOLD);
    } else if (page == 3) {
        DrawHero((Vector2){305, 545}, true, false, t);
        DrawCharacter((Vector2){970, 530}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
        DrawCircleGradient(970, 314, 130, Fade(CURSE_BLUE, 0.15f), BLANK);
    } else {
        DrawHero((Vector2){205, 545}, true, true, t);
        DrawSpider((Vector2){575, 570}, 1.1f, t, (Color){90, 55, 126, 255});
        DrawSnakeBoss((Vector2){920, 565}, t);
    }

    DrawRectangleRounded((Rectangle){56, 468, 1168, 182}, 0.08f, 8, Fade(BLACK, 0.67f));
    DrawText(STORY_HEADINGS[page], 86, 500, 36, ACCENT_GOLD);
    DrawText(STORY_LINES[page][0], 86, 548, 24, RAYWHITE);
    DrawText(STORY_LINES[page][1], 86, 582, 24, RAYWHITE);
    DrawText(STORY_LINES[page][2], 86, 616, 24, RAYWHITE);
    DrawText(TextFormat("%d / 5", page + 1), 1118, 500, 24, Fade(RAYWHITE, 0.80f));
    DrawText("ENTER: continue", 1070, 616, 22, Fade(ACCENT_GOLD, 0.95f));
}

static void DrawDialogueScene(const GameData *g) {
    float t = (float)GetTime();
    int page = g->dialoguePage;

    DrawJungleBackdrop(t);
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.08f), Fade(BLACK, 0.70f));
    DrawHero((Vector2){250, 542}, true, false, t);
    DrawCharacter((Vector2){980, 534}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
    DrawCircleGradient(982, 315, 120, Fade(CURSE_BLUE, 0.14f), BLANK);

    DrawRectangleRounded((Rectangle){70, 470, 1140, 170}, 0.08f, 8, Fade(BLACK, 0.75f));
    DrawRectangleRounded((Rectangle){70, 426, 210, 42}, 0.28f, 6, page % 2 == 0 ? Fade(ACCENT_GOLD, 0.95f) : Fade(CURSE_BLUE, 0.95f));
    DrawText(DIALOGUE_SPEAKERS[page], 92, 435, 30, BLACK);
    DrawText(DIALOGUE_LINES[page][0], 100, 516, 28, RAYWHITE);
    DrawText(DIALOGUE_LINES[page][1], 100, 558, 28, RAYWHITE);
    DrawText(TextFormat("%d / 4", page + 1), 1120, 436, 22, Fade(RAYWHITE, 0.82f));
    DrawText("ENTER to continue", 1016, 598, 22, Fade(ACCENT_GOLD, 0.90f));
}

static void DrawMapScene(const GameData *g) {
    float t = (float)GetTime();
    DrawJungleBackdrop(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.15f), Fade(BLACK, 0.52f));
    DrawText("Temple Trial Map", 440, 54, 46, RAYWHITE);
    DrawText("Press 1-5 to enter any unlocked chamber", 380, 106, 28, Fade(RAYWHITE, 0.82f));
    DrawText("The chambers now demand movement, combat, and decisions under pressure.", 225, 144, 24, Fade(ACCENT_GOLD, 0.92f));

    Vector2 nodes[MAX_LEVELS] = {
        {145, 300}, {390, 470}, {640, 245}, {885, 455}, {1110, 240}
    };

    for (int i = 0; i < MAX_LEVELS - 1; i++) {
        DrawLineEx(nodes[i], nodes[i + 1], 5.0f, Fade(ACCENT_GOLD, 0.33f));
    }

    for (int i = 0; i < MAX_LEVELS; i++) {
        bool unlocked = g->unlocked[i];
        bool done = g->completed[i];
        Color core = !unlocked ? Fade(GRAY, 0.75f) : (done ? Fade(GREEN, 0.88f) : Fade(CURSE_BLUE, 0.92f));
        DrawCircleV(nodes[i], 56, Fade(BLACK, 0.35f));
        DrawCircleV(nodes[i], 46, core);
        DrawRing(nodes[i], 54, 62, 0, 360, 48, Fade(ACCENT_GOLD, unlocked ? 0.55f : 0.18f));
        DrawText(TextFormat("%d", i + 1), (int)nodes[i].x - 8, (int)nodes[i].y - 16, 34, BLACK);
        DrawText(LEVEL_NAMES[i], (int)nodes[i].x - 82, (int)nodes[i].y + 72, 24, RAYWHITE);
        DrawText(LEVEL_GOALS[i], (int)nodes[i].x - 118, (int)nodes[i].y + 102, 18, Fade(RAYWHITE, 0.72f));
    }

    if (g->compassEarned) {
        DrawRectangleRounded((Rectangle){420, 610, 438, 50}, 0.3f, 6, Fade(ACCENT_GOLD, 0.92f));
        DrawText("Relic secured: Jungle Compass. Final chamber route is now visible.", 442, 625, 20, BLACK);
    }
}

static void UpdateSharedTimers(GameData *g, float dt) {
    if (g->damageCooldown > 0.0f) {
        g->damageCooldown -= dt;
    }
    if (g->attackTimer > 0.0f) {
        g->attackTimer -= dt;
    }
    if (g->dashTimer > 0.0f) {
        g->dashTimer -= dt;
    }
    if (g->runeFlash > 0.0f) {
        g->runeFlash -= dt;
    }
}

static void ResolveTopDownMovement(GameData *g, Rectangle arena, float speed, float dt) {
    Vector2 move = {
        (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))),
        (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) - (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)))
    };
    if (Vector2Length(move) > 0.0f) {
        move = Vector2Normalize(move);
        g->playerAim = move;
        if (fabsf(move.x) > 0.05f) {
            g->facingRight = move.x >= 0.0f;
        }
    }
    g->playerPos = Vector2Add(g->playerPos, Vector2Scale(move, speed * dt));
    g->playerPos.x = Clamp(g->playerPos.x, arena.x, arena.x + arena.width - 36.0f);
    g->playerPos.y = Clamp(g->playerPos.y, arena.y, arena.y + arena.height - 56.0f);
}

static Rectangle GetAttackRect(const GameData *g) {
    Rectangle player = GetPlayerRect(g);
    Vector2 dir = Vector2Length(g->playerAim) > 0.0f ? Vector2Normalize(g->playerAim) : (Vector2){g->facingRight ? 1.0f : -1.0f, 0.0f};
    return (Rectangle){player.x + 18 + dir.x * 22.0f - 22.0f, player.y + 18 + dir.y * 18.0f - 18.0f, 44, 36};
}

static void UpdateEnemySeek(Enemy *enemy, Vector2 target, float speedScale, float dt) {
    Vector2 delta = Vector2Subtract(target, enemy->pos);
    if (Vector2Length(delta) > 1.0f) {
        delta = Vector2Normalize(delta);
        enemy->vel = Vector2Scale(delta, enemy->speed * speedScale);
        enemy->pos = Vector2Add(enemy->pos, Vector2Scale(enemy->vel, dt));
    }
}

static void UpdateLevelOne(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    float groundY = 604.0f;
    Rectangle platforms[4] = {
        {215, 610, 170, 18},
        {472, 516, 170, 18},
        {734, 610, 170, 18},
        {1008, 430, 170, 18}
    };
    Rectangle flood = {0, 640, SCREEN_W, 80};

    DrawJungleBackdrop(t);
    DrawRectangleGradientV(0, 580, SCREEN_W, 140, (Color){34, 78, 58, 255}, (Color){16, 31, 26, 255});
    DrawRectangleGradientH(0, 620, SCREEN_W, 70, Fade(CURSE_BLUE, 0.55f), Fade(BLUE, 0.25f));
    for (int i = 0; i < 4; i++) {
        DrawRectangleRounded(platforms[i], 0.35f, 4, (Color){128, 96, 65, 255});
    }
    DrawBanner("LV1: Flood Run", "Jump across broken ground, keep moving, and reach the gate before the flood closes in.");

    float moveX = ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))) * 290.0f * dt;
    g->playerPos.x += moveX;
    if (fabsf(moveX) > 0.01f) {
        g->facingRight = moveX >= 0.0f;
        g->playerAim = (Vector2){g->facingRight ? 1.0f : -1.0f, 0.0f};
    }

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP)) && (g->onGround || g->canDoubleJump)) {
        g->playerVel.y = g->onGround ? -505.0f : -450.0f;
        if (!g->onGround) {
            g->canDoubleJump = false;
        }
        g->onGround = false;
    }

    g->playerVel.y += 980.0f * dt;
    g->playerPos.y += g->playerVel.y * dt;
    g->playerPos.x = Clamp(g->playerPos.x, 12, SCREEN_W - 48);

    Rectangle player = GetPlayerRect(g);
    bool landed = false;
    float bestY = groundY - player.height;
    if (player.y + player.height >= groundY) {
        landed = true;
    }
    for (int i = 0; i < 4; i++) {
        if (player.x + player.width > platforms[i].x &&
            player.x < platforms[i].x + platforms[i].width &&
            player.y + player.height >= platforms[i].y &&
            player.y + player.height <= platforms[i].y + 28 &&
            g->playerVel.y >= 0.0f) {
            landed = true;
            bestY = platforms[i].y - player.height;
        }
    }

    if (landed) {
        g->playerPos.y = bestY;
        g->playerVel.y = 0.0f;
        g->onGround = true;
        g->canDoubleJump = true;
        player.y = g->playerPos.y;
    } else {
        g->onGround = false;
    }

    Rectangle hitbox = player;
    for (int i = 0; i < g->enemyCount; i++) {
        Enemy *enemy = &g->enemies[i];
        if (!enemy->active) {
            continue;
        }

        float laneY = (i == 1 || i == 3) ? 498.0f : 610.0f;
        enemy->pos.y = laneY + sinf(t * 4.0f + enemy->wobble) * 10.0f;
        enemy->pos.x += (i % 2 == 0 ? 1.0f : -1.0f) * enemy->speed * dt;

        if (enemy->pos.x < 140 || enemy->pos.x > SCREEN_W - 110) {
            enemy->speed *= -1.0f;
        }

        DrawSpider(enemy->pos, 1.0f, t, (Color){91, 48, 124, 255});
        if (CheckCollisionRecs(hitbox, GetEnemyRect(enemy))) {
            DamagePlayer(g);
        }
    }

    if (CheckCollisionRecs(hitbox, flood)) {
        DamagePlayer(g);
        g->playerPos = (Vector2){70, 548};
        g->playerVel = Vector2Zero();
    }

    g->levelTimer += dt;
    DrawHero(g->playerPos, g->facingRight, false, t);
    DrawRectangleRounded((Rectangle){1142, 336, 82, 132}, 0.2f, 4, Fade((Color){38, 84, 56, 255}, 0.95f));
    DrawRectangleRounded((Rectangle){1162, 356, 42, 94}, 0.2f, 4, Fade(ACCENT_GOLD, 0.78f));
    DrawText("Time", 24, 172, 20, RAYWHITE);
    DrawText(TextFormat("%.1f / 24", g->levelTimer), 24, 196, 26, ACCENT_GOLD);
    DrawText("Run, jump, and use the second jump when the high ledge forces it.", 24, 228, 20, Fade(RAYWHITE, 0.76f));

    if (g->playerPos.x >= 1160 && g->playerPos.y <= 504) {
        CompleteLevel(g);
        *scene = SCENE_MAP;
    }

    if (g->levelTimer > 24.0f) {
        DamagePlayer(g);
        StartLevel(g, 1);
    }
}

static void UpdateLevelTwo(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle arena = {74, 168, 1134, 478};

    DrawJungleBackdrop(t);
    DrawRectangleRounded(arena, 0.05f, 8, Fade((Color){25, 24, 30, 255}, 0.88f));
    DrawRectangleLinesEx(arena, 6.0f, Fade(ACCENT_GOLD, 0.38f));
    DrawBanner("LV2: Nest Breaker", "Fight through a moving swarm. Attack with J or K. Standing still will get you surrounded.");
    ResolveTopDownMovement(g, arena, 255.0f, dt);

    if ((IsKeyPressed(KEY_J) || IsKeyPressed(KEY_K) || IsKeyPressed(KEY_SPACE)) && g->attackTimer <= 0.0f) {
        g->attackTimer = 0.26f;
    }

    Rectangle attackRect = GetAttackRect(g);
    int alive = 0;

    for (int i = 0; i < g->enemyCount; i++) {
        Enemy *enemy = &g->enemies[i];
        if (!enemy->active) {
            continue;
        }
        alive++;

        if (enemy->kind == ENEMY_HUNTER) {
            UpdateEnemySeek(enemy, Vector2Add(g->playerPos, (Vector2){18, 28}), 1.0f, dt);
        } else if (enemy->kind == ENEMY_RUSHER) {
            float pulse = 0.7f + 0.7f * (0.5f + 0.5f * sinf(t * 3.0f + enemy->wobble));
            UpdateEnemySeek(enemy, Vector2Add(g->playerPos, (Vector2){18, 28}), pulse, dt);
        } else {
            Vector2 center = Vector2Add(g->playerPos, (Vector2){18, 28});
            Vector2 offset = {cosf(t * 1.6f + enemy->wobble) * 120.0f, sinf(t * 1.6f + enemy->wobble) * 90.0f};
            UpdateEnemySeek(enemy, Vector2Add(center, offset), 1.0f, dt);
        }

        enemy->pos.x = Clamp(enemy->pos.x, arena.x + 25, arena.x + arena.width - 25);
        enemy->pos.y = Clamp(enemy->pos.y, arena.y + 25, arena.y + arena.height - 25);

        DrawSpider(enemy->pos, 1.0f, t, (Color){104, 52, 135, 255});
        if (g->attackTimer > 0.10f && CheckCollisionRecs(attackRect, GetEnemyRect(enemy))) {
            enemy->active = false;
            alive--;
            continue;
        }
        if (CheckCollisionRecs(GetPlayerRect(g), GetEnemyRect(enemy))) {
            DamagePlayer(g);
        }
    }

    DrawHero(g->playerPos, g->facingRight, g->attackTimer > 0.0f, t);
    if (g->attackTimer > 0.0f) {
        DrawRectangleRounded(attackRect, 0.25f, 4, Fade(ACCENT_GOLD, 0.18f));
    }

    DrawText(TextFormat("Swarm Remaining: %d", alive), 24, 172, 24, RAYWHITE);
    DrawText("Move with WASD or arrows. Attack with J, K, or SPACE.", 24, 202, 20, Fade(RAYWHITE, 0.76f));

    if (alive <= 0) {
        g->compassEarned = true;
        CompleteLevel(g);
        *scene = SCENE_MAP;
    }
}

static void UpdateOrbs(GameData *g, float t, float speed, Rectangle bounds) {
    Vector2 center = {bounds.x + bounds.width * 0.5f, bounds.y + bounds.height * 0.5f};
    Rectangle player = GetPlayerRect(g);

    for (int i = 0; i < g->orbCount; i++) {
        Orb *orb = &g->orbs[i];
        if (!orb->active) {
            continue;
        }

        float angle = t * speed + i * 0.7f;
        float rx = bounds.width * 0.32f + (i % 3) * 45.0f;
        float ry = bounds.height * 0.24f + (i % 2) * 30.0f;
        orb->pos.x = center.x + cosf(angle + i) * rx;
        orb->pos.y = center.y + sinf(angle * 1.3f + i) * ry;

        DrawCircleV(orb->pos, orb->radius + 8.0f, Fade(CURSE_BLUE, 0.16f));
        DrawCircleV(orb->pos, orb->radius, CURSE_BLUE);
        if (CheckCollisionCircleRec(orb->pos, orb->radius, player)) {
            DamagePlayer(g);
        }
    }
}

static void UpdateLevelThree(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle room = {54, 150, 1170, 520};

    DrawJungleBackdrop(t);
    DrawRectangleRounded(room, 0.05f, 8, Fade((Color){28, 25, 36, 255}, 0.90f));
    DrawRectangleLinesEx(room, 6.0f, Fade(CURSE_BLUE, 0.40f));
    DrawBanner("LV3: Rune Trial", "Clue: sunrise, echo, root. Touch runes in the order West, East, South while dodging the curse.");
    ResolveTopDownMovement(g, room, 250.0f, dt);
    UpdateOrbs(g, t, 0.7f, room);

    for (int i = 0; i < g->glyphCount; i++) {
        Glyph *glyph = &g->glyphs[i];
        Color fill = Fade((Color){65, 55, 90, 255}, 0.95f);
        if (!glyph->active) {
            fill = Fade(GREEN, 0.80f);
        } else if (g->runeProgress < 3 && g->runeOrder[g->runeProgress] == i) {
            fill = Fade(ACCENT_GOLD, 0.90f);
        }

        DrawRectangleRounded(glyph->rect, 0.18f, 6, fill);
        DrawRectangleLinesEx(glyph->rect, 4.0f, Fade(CURSE_BLUE, 0.45f));
        DrawText(TextFormat("%d", i + 1), (int)glyph->rect.x + 42, (int)glyph->rect.y + 32, 36, BLACK);

        if (glyph->active && CheckCollisionRecs(GetPlayerRect(g), glyph->rect)) {
            if (g->runeOrder[g->runeProgress] == i) {
                glyph->active = false;
                g->runeProgress++;
                g->runeFlash = 0.35f;
            } else {
                DamagePlayer(g);
                StartLevel(g, 3);
                return;
            }
        }
    }

    DrawHero(g->playerPos, g->facingRight, false, t);
    if (g->runeFlash > 0.0f) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(ACCENT_GOLD, g->runeFlash * 0.25f));
    }

    DrawText("Correct order: 2 -> 3 -> 1", 24, 174, 24, RAYWHITE);
    DrawText("Touching the wrong rune resets the chamber and costs a heart.", 24, 206, 20, Fade(RAYWHITE, 0.78f));

    if (g->runeProgress >= 3) {
        CompleteLevel(g);
        *scene = SCENE_MAP;
    }
}

static bool CollidesWall(Rectangle rect, const Rectangle *walls, int count) {
    for (int i = 0; i < count; i++) {
        if (CheckCollisionRecs(rect, walls[i])) {
            return true;
        }
    }
    return false;
}

static void UpdateLevelFour(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle walls[10] = {
        {200, 160, 28, 488},
        {392, 24, 28, 456},
        {584, 160, 28, 488},
        {776, 24, 28, 456},
        {968, 160, 28, 488},
        {228, 160, 110, 24},
        {420, 456, 110, 24},
        {612, 160, 110, 24},
        {804, 456, 110, 24},
        {996, 160, 96, 24}
    };
    Rectangle arena = {24, 24, 1232, 652};
    Rectangle exitDoor = {1166, 66, 56, 72};
    Rectangle routeMarkers[9] = {
        {40, 74, 140, 58},
        {236, 74, 128, 58},
        {236, 564, 128, 42},
        {430, 564, 128, 42},
        {430, 74, 128, 58},
        {624, 74, 128, 58},
        {624, 564, 128, 42},
        {818, 564, 128, 42},
        {1012, 74, 186, 58}
    };

    DrawJungleBackdrop(t);
    DrawRectangleRounded(arena, 0.03f, 8, Fade((Color){23, 29, 21, 255}, 0.60f));
    for (int i = 0; i < 9; i++) {
        DrawRectangleRounded(routeMarkers[i], 0.35f, 6, Fade(CURSE_BLUE, 0.14f));
        DrawRectangleLinesEx(routeMarkers[i], 2.0f, Fade(ACCENT_GOLD, 0.28f));
    }
    for (int i = 0; i < 18; i++) {
        float rockX = 70.0f + i * 68.0f + sinf(t * 1.2f + i) * 8.0f;
        float rockY = 32.0f + fmodf(t * (80.0f + i * 3.0f), 120.0f);
        DrawCircleV((Vector2){rockX, rockY}, 6.0f + (i % 3), Fade((Color){140, 128, 103, 255}, 0.65f));
    }
    DrawRectangleGradientV(24, 24, 1232, 26, Fade((Color){99, 92, 74, 255}, 0.92f), Fade((Color){99, 92, 74, 255}, 0.12f));
    DrawBanner("LV4: Collapse Maze", "Find the route through the cave before the ceiling gives way. The glowing floor marks the intended path.");

    float baseSpeed = g->dashTimer > 0.0f ? 430.0f : 245.0f;
    if (IsKeyPressed(KEY_LEFT_SHIFT) && g->dashTimer <= 0.0f) {
        g->dashTimer = 0.22f;
    }

    Vector2 move = {
        (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))),
        (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) - (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)))
    };
    if (Vector2Length(move) > 0.0f) {
        move = Vector2Normalize(move);
        g->playerAim = move;
        if (fabsf(move.x) > 0.05f) {
            g->facingRight = move.x >= 0.0f;
        }
    }

    Rectangle current = GetPlayerRect(g);
    Rectangle nextX = current;
    nextX.x += move.x * baseSpeed * dt;
    if (!CollidesWall(nextX, walls, 10)) {
        g->playerPos.x = Clamp(nextX.x, arena.x + 8, arena.x + arena.width - current.width - 8);
    }
    Rectangle nextY = GetPlayerRect(g);
    nextY.y += move.y * baseSpeed * dt;
    if (!CollidesWall(nextY, walls, 10)) {
        g->playerPos.y = Clamp(nextY.y, arena.y + 8, arena.y + arena.height - current.height - 8);
    }

    for (int i = 0; i < 10; i++) {
        DrawRectangleRounded(walls[i], 0.15f, 4, TEMPLE_STONE);
    }
    DrawRectangleRounded(exitDoor, 0.2f, 4, Fade(GREEN, 0.92f));
    DrawRectangleLinesEx(exitDoor, 4.0f, Fade(ACCENT_GOLD, 0.65f));
    DrawHero(g->playerPos, g->facingRight, g->dashTimer > 0.0f, t);

    if (CheckCollisionRecs(GetPlayerRect(g), exitDoor)) {
        CompleteLevel(g);
        *scene = SCENE_MAP;
    }

    g->levelTimer += dt;
    DrawText("Dash: LEFT SHIFT", 24, 172, 22, RAYWHITE);
    DrawText(TextFormat("Collapse: %.1f / 36.0", g->levelTimer), 24, 202, 24, ACCENT_GOLD);
    DrawText("Route: start in the top lane, drop after pillar two, rise after pillar four, then sprint to the gate.", 24, 232, 20, Fade(RAYWHITE, 0.76f));

    if (g->levelTimer >= 36.0f) {
        DamagePlayer(g);
        StartLevel(g, 4);
        return;
    }

}

static void UpdateLevelFive(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle room = {50, 150, 1180, 520};
    Rectangle choiceMercy = {400, 585, 200, 44};
    Rectangle choiceEscape = {690, 585, 200, 44};

    DrawJungleBackdrop(t);
    DrawRectangleRounded(room, 0.05f, 8, Fade((Color){26, 20, 27, 255}, 0.93f));
    DrawRectangleLinesEx(room, 6.0f, Fade(ACCENT_GOLD, 0.48f));
    DrawTempleFrame(t);
    DrawBanner("LV5: Heart Of The Gate", "Collect the three sigils while the chamber attacks. Then choose: break the curse or escape alone.");
    ResolveTopDownMovement(g, room, 260.0f, dt);
    UpdateOrbs(g, t, 0.95f, room);

    for (int i = 0; i < g->glyphCount; i++) {
        if (!g->glyphs[i].active) {
            continue;
        }
        DrawRectangleRounded(g->glyphs[i].rect, 0.25f, 4, Fade(ACCENT_GOLD, 0.92f));
        DrawRectangleLinesEx(g->glyphs[i].rect, 3.0f, Fade(RAYWHITE, 0.55f));
        if (CheckCollisionRecs(GetPlayerRect(g), g->glyphs[i].rect)) {
            g->glyphs[i].active = false;
            g->glyphsCollected++;
        }
    }

    DrawHero(g->playerPos, g->facingRight, false, t);
    DrawCharacter((Vector2){964, 508}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
    DrawText(TextFormat("Sigils: %d / 3", g->glyphsCollected), 24, 172, 24, RAYWHITE);

    if (g->glyphsCollected >= 3) {
        DrawRectangleRounded(choiceMercy, 0.25f, 6, Fade(CURSE_BLUE, 0.92f));
        DrawRectangleRounded(choiceEscape, 0.25f, 6, Fade(DANGER_RED, 0.88f));
        DrawText("1  Break the curse", 422, 597, 24, BLACK);
        DrawText("2  Escape alone", 724, 597, 24, BLACK);
        DrawText("The temple opens because you proved skill. The ending depends on what you do with that power.", 224, 544, 22, Fade(RAYWHITE, 0.82f));

        if (IsKeyPressed(KEY_ONE)) {
            g->endingChoiceMercy = true;
            g->win = true;
            CompleteLevel(g);
            *scene = SCENE_END;
        } else if (IsKeyPressed(KEY_TWO)) {
            g->endingChoiceMercy = false;
            g->win = false;
            CompleteLevel(g);
            *scene = SCENE_END;
        }
    }
}

static void UpdateLevel(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    UpdateSharedTimers(g, dt);
    DrawHearts(g->hearts);
    DrawText(TextFormat("Level %d", g->currentLevel), SCREEN_W - 164, 18, 26, RAYWHITE);
    DrawText(LEVEL_NAMES[g->currentLevel - 1], SCREEN_W - 288, 48, 24, ACCENT_GOLD);
    DrawText("ESC: map", 24, SCREEN_H - 32, 20, Fade(RAYWHITE, 0.66f));

    if (g->currentLevel == 1) {
        UpdateLevelOne(g, scene);
    } else if (g->currentLevel == 2) {
        UpdateLevelTwo(g, scene);
    } else if (g->currentLevel == 3) {
        UpdateLevelThree(g, scene);
    } else if (g->currentLevel == 4) {
        UpdateLevelFour(g, scene);
    } else if (g->currentLevel == 5) {
        UpdateLevelFive(g, scene);
    }

    if (g->hearts <= 0) {
        g->win = false;
        *scene = SCENE_END;
    }
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "KiKi - Temple Breaker");
    SetTargetFPS(60);

    GameData game = {0};
    Scene scene = SCENE_TITLE;
    ResetGame(&game);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (scene == SCENE_TITLE) {
            DrawTitleScene(&game);

            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                game.menuSelection = (game.menuSelection + 2) % 3;
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                game.menuSelection = (game.menuSelection + 1) % 3;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                if (game.menuSelection == 0) {
                    game.storyPage = 0;
                    scene = SCENE_STORY;
                } else if (game.menuSelection == 1) {
                    game.dialoguePage = 0;
                    scene = SCENE_DIALOGUE;
                } else {
                    break;
                }
            }
        } else if (scene == SCENE_STORY) {
            DrawStoryScene(&game);
            if (IsKeyPressed(KEY_ENTER)) {
                if (game.storyPage < 4) {
                    game.storyPage++;
                } else {
                    game.dialoguePage = 0;
                    scene = SCENE_DIALOGUE;
                }
            }
        } else if (scene == SCENE_DIALOGUE) {
            DrawDialogueScene(&game);
            if (IsKeyPressed(KEY_ENTER)) {
                if (game.dialoguePage < 3) {
                    game.dialoguePage++;
                } else {
                    scene = SCENE_MAP;
                }
            }
        } else if (scene == SCENE_MAP) {
            DrawMapScene(&game);
            if (IsKeyPressed(KEY_ONE) && game.unlocked[0]) {
                StartLevel(&game, 1);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_TWO) && game.unlocked[1]) {
                StartLevel(&game, 2);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_THREE) && game.unlocked[2]) {
                StartLevel(&game, 3);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_FOUR) && game.unlocked[3]) {
                StartLevel(&game, 4);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_FIVE) && game.unlocked[4]) {
                StartLevel(&game, 5);
                scene = SCENE_LEVEL;
            }
        } else if (scene == SCENE_LEVEL) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                scene = SCENE_MAP;
            } else {
                UpdateLevel(&game, &scene);
            }
        } else if (scene == SCENE_END) {
            float t = (float)GetTime();
            DrawJungleBackdrop(t);
            DrawTempleFrame(t);
            DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.18f), Fade(BLACK, 0.70f));
            DrawRectangleRounded((Rectangle){132, 126, 1016, 368}, 0.05f, 8, Fade(BLACK, 0.42f));

            if (game.win) {
                DrawText("Curse Broken", 440, 152, 60, RAYWHITE);
                DrawWrappedTextBlock(
                    "Kiki drags the wizard into the rain, and the temple finally goes silent.",
                    (Rectangle){190, 252, 700, 120},
                    28.0f, 1.0f, ACCENT_GOLD);
                DrawWrappedTextBlock(
                    "His aunt finds him at dawn with the compass in one hand and a new scar across his courage.",
                    (Rectangle){190, 334, 730, 140},
                    24.0f, 1.0f, Fade(RAYWHITE, 0.84f));
            } else {
                DrawText("Gate Claimed", 458, 152, 60, RAYWHITE);
                DrawWrappedTextBlock(
                    "Kiki reaches freedom, but the curse keeps the wizard and the temple remembers his choice.",
                    (Rectangle){190, 252, 700, 120},
                    28.0f, 1.0f, DANGER_RED);
                DrawWrappedTextBlock(
                    "He gets back to his aunt alive, though some doors do not stay closed for long.",
                    (Rectangle){190, 334, 730, 140},
                    24.0f, 1.0f, Fade(RAYWHITE, 0.84f));
            }

            DrawText("Press R to restart the expedition", 388, 540, 30, Fade(RAYWHITE, 0.82f));
            if (IsKeyPressed(KEY_R)) {
                ResetGame(&game);
                scene = SCENE_TITLE;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
