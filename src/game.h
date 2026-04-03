#ifndef KIKIGAME_GAME_H
#define KIKIGAME_GAME_H

#include <stdbool.h>
#include <raylib.h>

#include "assets.h"

#define SCREEN_W 1280
#define SCREEN_H 720
#define MAX_LEVELS 5
#define MAX_ENEMIES 8
#define MAX_ORBS 10
#define MAX_GLYPHS 4
#define MAX_HEARTS 10

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
    bool postLevelDialogue;
    int postLevelLevel;

    int hearts;
    bool win;
    bool endingChoiceMercy;
    bool compassEarned;

    float levelTimer;
    float damageCooldown;
    float attackTimer;
    float attackImpactTimer;
    float dragonStaggerTimer;
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
    int activeQuizIndex;
    int runeQuestionVariant[3];
    int lastRuneOrder[3];
    int lastRuneQuestionVariant[3];
    bool hasLastRuneSetup;
    bool dragonIntroActive;
    bool dragonPhaseActive;
    bool dragonDefeated;
    int dragonOrbsCollected;
    bool dragonFacingRight;
    Vector2 wizardPos;
    Vector2 attackImpactPos;
    float wizardPhase;

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

extern const Color SKY_TOP;
extern const Color SKY_BOTTOM;
extern const Color CANOPY_A;
extern const Color CANOPY_B;
extern const Color TEMPLE_STONE;
extern const Color TEMPLE_DARK;
extern const Color PLAYER_SKIN;
extern const Color PLAYER_JACKET;
extern const Color ACCENT_GOLD;
extern const Color CURSE_BLUE;
extern const Color DANGER_RED;

extern const char *MENU_ITEMS[3];
extern const char *LEVEL_NAMES[MAX_LEVELS];
extern const char *LEVEL_GOALS[MAX_LEVELS];
extern const char *STORY_HEADINGS[5];
extern const char *STORY_LINES[5][3];
extern const char *DIALOGUE_SPEAKERS[4];
extern const char *DIALOGUE_LINES[4][2];
extern const char *POST_LEVEL_SPEAKERS[4][2];
extern const char *POST_LEVEL_LINES[4][2][2];
extern const char *LEVEL_THREE_QUESTIONS[3][4];
extern const char *LEVEL_THREE_OPTIONS[3][4][3];
extern const int LEVEL_THREE_CORRECT[3][4];

Rectangle GetPlayerRect(const GameData *g);
Rectangle GetEnemyRect(const Enemy *enemy);

void ResetGame(GameData *g);
void StartLevel(GameData *g, int level);
void UpdateLevel(GameData *g, Scene *scene);

void DrawJungleBackdrop(float t);
void DrawTempleFrame(float t);
void DrawHero(Vector2 pos, bool facingRight, bool attacking, float t);
void DrawCharacter(Vector2 pos, Color body, Color head, bool staff);
void DrawDragon(Vector2 pos, bool facingRight, float scale, float alpha);
void DrawSpider(Vector2 pos, float scale, float t, Color tint);
void DrawSnakeBoss(Vector2 pos, float t);
void DrawHearts(int hearts);
void DrawBanner(const char *title, const char *subtitle);
void DrawWrappedTextBlock(const char *text, Rectangle bounds, float fontSize, float spacing, Color color);

void DrawTitleScene(const GameData *g);
void DrawStoryScene(const GameData *g);
void DrawDialogueScene(const GameData *g);
void DrawMapScene(const GameData *g);
void DrawEndScene(const GameData *g);

#endif
