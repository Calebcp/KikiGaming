#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_W 1100
#define SCREEN_H 700
#define MAX_LEVELS 5

typedef enum {
    SCENE_TITLE = 0,
    SCENE_STORY,
    SCENE_MAP,
    SCENE_LEVEL,
    SCENE_END
} Scene;

typedef struct {
    Rectangle body;
    float speed;
} Enemy;

typedef struct {
    int currentLevel;
    bool unlocked[MAX_LEVELS];
    bool completed[MAX_LEVELS];

    int hearts;
    bool win;

    float levelTimer;
    int puzzleChoice;
    bool choiceSubmitted;
    bool gotReward;

    Vector2 playerPos;
    Enemy enemies[4];
    int enemyCount;
    int spidersLeft;
    Vector2 snakePos;
} GameData;

static const char *LEVEL_NAMES[MAX_LEVELS] = {
    "River Escape",
    "Spider Nest",
    "Rune Riddle",
    "Snake Maze",
    "Temple Gate"
};

static const char *LEVEL_GOALS[MAX_LEVELS] = {
    "Cross the flooded trail.",
    "Defeat the spiders guarding the compass.",
    "Answer the rune gate riddle.",
    "Escape the serpent maze.",
    "Choose the path that leads home."
};

static void StartLevel(GameData *g, int level) {
    g->currentLevel = level;
    g->levelTimer = 0.0f;
    g->choiceSubmitted = false;
    g->puzzleChoice = 0;

    if (level == 1) {
        g->playerPos = (Vector2){80, SCREEN_H - 160};
        g->enemyCount = 3;
        g->enemies[0] = (Enemy){(Rectangle){220, SCREEN_H - 160, 120, 24}, 210.0f};
        g->enemies[1] = (Enemy){(Rectangle){450, SCREEN_H - 250, 140, 24}, -240.0f};
        g->enemies[2] = (Enemy){(Rectangle){760, SCREEN_H - 160, 130, 24}, 190.0f};
    }

    if (level == 2) {
        g->spidersLeft = 4;
    }

    if (level == 4) {
        g->playerPos = (Vector2){80, 100};
        g->snakePos = (Vector2){SCREEN_W - 130, SCREEN_H - 130};
    }
}

static void ResetGame(GameData *g) {
    *g = (GameData){0};
    g->unlocked[0] = true;
    g->hearts = 3;
}

static bool TouchesAnyEnemy(const GameData *g, Rectangle player) {
    for (int i = 0; i < g->enemyCount; i++) {
        if (CheckCollisionRecs(player, g->enemies[i].body)) {
            return true;
        }
    }
    return false;
}

static void DrawHearts(int hearts) {
    for (int i = 0; i < hearts; i++) {
        DrawCircle(32 + i * 30, 32, 12, RED);
    }
    DrawText("HEARTS", 20, 50, 16, MAROON);
}

static void DrawBanner(const char *title, const char *subtitle) {
    DrawRectangleRounded((Rectangle){18, 72, SCREEN_W - 36, 70}, 0.15f, 8, Fade(BEIGE, 0.92f));
    DrawText(title, 32, 86, 28, DARKGREEN);
    DrawText(subtitle, 32, 118, 20, DARKGRAY);
}

static void CompleteLevel(GameData *g) {
    const int i = g->currentLevel - 1;
    g->completed[i] = true;
    if (i + 1 < MAX_LEVELS) {
        g->unlocked[i + 1] = true;
    }
}

static void UpdateLevel(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    DrawHearts(g->hearts);
    DrawText(TextFormat("Level %d", g->currentLevel), SCREEN_W - 190, 16, 24, DARKGREEN);
    DrawText(LEVEL_NAMES[g->currentLevel - 1], SCREEN_W - 240, 44, 20, DARKBROWN);

    if (g->currentLevel == 1) {
        DrawBanner("LV1: Escape the flooded jungle trail",
                   "Cross the river before the current pulls you deeper into the ruins.");
        DrawRectangle(0, SCREEN_H - 120, SCREEN_W, 120, BEIGE);
        DrawRectangle(0, SCREEN_H - 240, SCREEN_W, 80, Fade(SKYBLUE, 0.45f));

        g->playerPos.x += (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * 280.0f * dt;
        g->playerPos.x = Clamp(g->playerPos.x, 20, SCREEN_W - 60);

        Rectangle player = {g->playerPos.x, SCREEN_H - 170, 40, 60};
        DrawRectangleRec(player, SKYBLUE);

        for (int i = 0; i < g->enemyCount; i++) {
            g->enemies[i].body.x += g->enemies[i].speed * dt;
            if (g->enemies[i].body.x < 0 || g->enemies[i].body.x + g->enemies[i].body.width > SCREEN_W) {
                g->enemies[i].speed *= -1.0f;
            }
            DrawRectangleRec(g->enemies[i].body, DARKBROWN);
        }

        if (TouchesAnyEnemy(g, player)) {
            g->hearts--;
            StartLevel(g, 1);
        }

        g->levelTimer += dt;
        DrawText(TextFormat("Time: %.1f / 20", g->levelTimer), 24, 154, 20, BLACK);
        DrawText("Controls: A/D to move", 24, 182, 20, DARKBLUE);

        if (g->playerPos.x > SCREEN_W - 80) {
            CompleteLevel(g);
            *scene = SCENE_MAP;
        }

        if (g->levelTimer > 20.0f) {
            g->hearts--;
            StartLevel(g, 1);
        }
    }

    if (g->currentLevel == 2) {
        DrawBanner("LV2: Clear the spider nest",
                   "Fight through the first dungeon chamber and claim the jungle compass.");

        DrawCircle(250, 300, 70, DARKBLUE);
        DrawText("Boy", 230, 295, 24, WHITE);

        for (int i = 0; i < g->spidersLeft; i++) {
            DrawCircle(500 + i * 110, 320, 35, PURPLE);
        }

        DrawText(TextFormat("Spiders Left: %d", g->spidersLeft), 24, 154, 24, BLACK);
        DrawText("Press SPACE to attack", 24, 186, 20, DARKBLUE);

        if (IsKeyPressed(KEY_SPACE) && g->spidersLeft > 0) {
            g->spidersLeft--;
        }

        if (GetTime() - (int)GetTime() == 0) {
            // Light pressure loop so players cannot idle forever.
            g->levelTimer += dt;
        }

        if (g->levelTimer > 10.0f) {
            g->hearts--;
            g->levelTimer = 0.0f;
        }

        if (g->spidersLeft == 0) {
            g->gotReward = true;
            CompleteLevel(g);
            *scene = SCENE_MAP;
        }
    }

    if (g->currentLevel == 3) {
        DrawBanner("LV3: Solve the rune gate",
                   "The wizard leaves a clue. Answer correctly to unlock the deep dungeon.");
        DrawText("Riddle: What is full of keys but cannot open doors?", 24, 170, 28, BLACK);
        DrawText("1) Piano  2) Temple  3) River", 24, 220, 26, DARKGRAY);

        if (IsKeyPressed(KEY_ONE)) g->puzzleChoice = 1;
        if (IsKeyPressed(KEY_TWO)) g->puzzleChoice = 2;
        if (IsKeyPressed(KEY_THREE)) g->puzzleChoice = 3;
        if (IsKeyPressed(KEY_ENTER)) g->choiceSubmitted = true;

        DrawText(TextFormat("Selected: %d (press ENTER)", g->puzzleChoice), 24, 270, 24, DARKBLUE);

        if (g->choiceSubmitted) {
            if (g->puzzleChoice == 1) {
                CompleteLevel(g);
                *scene = SCENE_MAP;
            } else {
                g->hearts--;
                StartLevel(g, 3);
            }
        }
    }

    if (g->currentLevel == 4) {
        DrawBanner("LV4: Escape the serpent maze",
                   "Move with WASD. Reach the gate before the guardian snake catches you.");

        Rectangle player = {g->playerPos.x, g->playerPos.y, 34, 34};
        Rectangle exitDoor = {SCREEN_W - 90, SCREEN_H - 80, 56, 56};

        Rectangle walls[] = {
            {170, 70, 26, 520},
            {340, 120, 26, 520},
            {510, 70, 26, 520},
            {680, 120, 26, 520},
            {850, 70, 26, 520}
        };

        Vector2 step = {
            (IsKeyDown(KEY_D) - IsKeyDown(KEY_A)) * 250.0f * dt,
            (IsKeyDown(KEY_S) - IsKeyDown(KEY_W)) * 250.0f * dt
        };

        Rectangle next = {player.x + step.x, player.y + step.y, player.width, player.height};

        bool blocked = false;
        for (int i = 0; i < 5; i++) {
            if (CheckCollisionRecs(next, walls[i])) {
                blocked = true;
            }
        }

        if (!blocked) {
            g->playerPos.x = Clamp(next.x, 10, SCREEN_W - 44);
            g->playerPos.y = Clamp(next.y, 10, SCREEN_H - 44);
            player = (Rectangle){g->playerPos.x, g->playerPos.y, 34, 34};
        }

        Vector2 toPlayer = Vector2Subtract(g->playerPos, g->snakePos);
        float length = Vector2Length(toPlayer);
        if (length > 1.0f) {
            Vector2 dir = Vector2Scale(toPlayer, 1.0f / length);
            g->snakePos = Vector2Add(g->snakePos, Vector2Scale(dir, 110.0f * dt));
        }

        Rectangle snake = {g->snakePos.x, g->snakePos.y, 42, 42};

        DrawRectangleRec(exitDoor, GREEN);
        for (int i = 0; i < 5; i++) DrawRectangleRec(walls[i], DARKBROWN);
        DrawRectangleRec(player, BLUE);
        DrawRectangleRec(snake, RED);

        if (CheckCollisionRecs(player, snake)) {
            g->hearts--;
            StartLevel(g, 4);
        }

        if (CheckCollisionRecs(player, exitDoor)) {
            CompleteLevel(g);
            *scene = SCENE_MAP;
        }
    }

    if (g->currentLevel == 5) {
        DrawBanner("LV5: The temple gate",
                   "Beyond this gate is the path home to your aunt. Choose who you become.");
        DrawText("1) Run for the exit alone", 24, 170, 28, BLACK);
        DrawText("2) Help the wizard and end the curse", 24, 210, 28, BLACK);
        DrawText("Press 1 or 2", 24, 260, 24, DARKBLUE);

        if (IsKeyPressed(KEY_ONE)) {
            g->win = false;
            CompleteLevel(g);
            *scene = SCENE_END;
        }

        if (IsKeyPressed(KEY_TWO)) {
            g->win = true;
            CompleteLevel(g);
            *scene = SCENE_END;
        }
    }

    if (g->hearts <= 0) {
        g->win = false;
        *scene = SCENE_END;
    }
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "KiKi 2D Story Game - Foundation");
    SetTargetFPS(60);

    Scene scene = SCENE_TITLE;
    GameData game = {0};
    ResetGame(&game);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (scene == SCENE_TITLE) {
            DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(DARKGREEN, 0.85f), Fade(BLACK, 0.85f));
            DrawCircle(930, 120, 160, Fade(GOLD, 0.18f));
            DrawText("KIKI: LOST IN THE JUNGLE", 195, 150, 50, RAYWHITE);
            DrawText("A 2D dungeon escape story built in C with raylib", 240, 225, 28, Fade(RAYWHITE, 0.9f));
            DrawText("Find the road back to your aunt.", 335, 270, 28, GOLD);
            DrawText("Press ENTER to start", 385, 350, 30, RAYWHITE);
            if (IsKeyPressed(KEY_ENTER)) scene = SCENE_STORY;
        }

        if (scene == SCENE_STORY) {
            DrawRectangleRounded((Rectangle){70, 90, 960, 300}, 0.08f, 10, Fade(BEIGE, 0.95f));
            DrawText("A young man is separated from his aunt during a jungle expedition.", 100, 140, 28, BLACK);
            DrawText("He follows a false trail into ancient dungeon ruins hidden in the trees.", 100, 190, 28, BLACK);
            DrawText("A trapped wizard offers help: survive five trials and the jungle will release you.", 100, 240, 28, BLACK);
            DrawText("Press ENTER for the level map", 100, 315, 32, DARKGREEN);
            if (IsKeyPressed(KEY_ENTER)) scene = SCENE_MAP;
        }

        if (scene == SCENE_MAP) {
            DrawText("LEVEL MAP", 440, 60, 44, DARKGREEN);
            DrawText("Press 1-5 to enter unlocked levels", 320, 110, 28, DARKGRAY);
            DrawText("Goal: clear all five trials and find the path back to your aunt", 210, 150, 24, DARKBROWN);

            for (int i = 0; i < MAX_LEVELS; i++) {
                Rectangle box = {180 + i * 170, 250 + (i % 2) * 70, 120, 72};
                Color c = game.unlocked[i] ? (game.completed[i] ? GREEN : SKYBLUE) : LIGHTGRAY;
                DrawRectangleRounded(box, 0.2f, 8, c);
                DrawText(TextFormat("LV %d", i + 1), (int)box.x + 26, (int)box.y + 20, 30, BLACK);
                DrawText(LEVEL_NAMES[i], (int)box.x - 10, (int)box.y + 84, 18, DARKGRAY);
                DrawText(LEVEL_GOALS[i], (int)box.x - 30, (int)box.y + 106, 15, GRAY);
            }

            if (game.gotReward) {
                DrawText("Reward unlocked: Jungle Compass", 385, 520, 28, DARKPURPLE);
            }

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
        }

        if (scene == SCENE_LEVEL) {
            DrawText("ESC: back to map", 20, SCREEN_H - 30, 20, DARKGRAY);
            if (IsKeyPressed(KEY_ESCAPE)) scene = SCENE_MAP;
            UpdateLevel(&game, &scene);
        }

        if (scene == SCENE_END) {
            if (game.win) {
                DrawText("GOOD ENDING", 410, 160, 54, DARKGREEN);
                DrawText("You broke the curse and found the trail back to your aunt.", 145, 250, 34, BLACK);
            } else {
                DrawText("BAD ENDING", 430, 160, 54, MAROON);
                DrawText("The temple sealed behind you before you reached your aunt.", 130, 250, 34, BLACK);
            }

            DrawText("Press R to restart", 410, 340, 32, DARKGRAY);
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
