#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

#define SCREEN_W 1100
#define SCREEN_H 700
#define MAX_LEVELS 5

static const Color SKIN_TONE = {255, 220, 177, 255};

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
    int storyPage;

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

static const char *STORY_HEADINGS[4] = {
    "Expedition at Dusk",
    "Warning at the Temple",
    "The Gate Seals Shut",
    "The Wizard's Curse"
};

static const char *STORY_LINES[4][3] = {
    {
        "A curious teenager travels with his aunt, the leader of an archaeological expedition.",
        "While the team studies ruined statues, he notices an ancient temple hidden in jungle vines.",
        "His curiosity pulls him away from the camp."
    },
    {
        "His aunt warns him not to go inside.",
        "The temple looks unstable, old, and wrong, but he sneaks forward anyway.",
        "He steps over the broken threshold alone."
    },
    {
        "Stone doors crash shut behind him.",
        "Torches flare alive and the corridor begins to tremble as if the temple has awakened.",
        "There is no path back outside."
    },
    {
        "An old wizard appears from the shadows and reveals the curse.",
        "Only someone who survives five ancient trials may leave this place alive.",
        "Fail, and he becomes the next prisoner of the temple."
    }
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

static void DrawJungleBackdrop(float t) {
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, (Color){16, 54, 47, 255}, (Color){5, 14, 18, 255});
    DrawCircle(900, 120, 170, Fade(GOLD, 0.18f));

    for (int i = 0; i < 8; i++) {
        int x = 80 + i * 140;
        int sway = (int)(sinf(t * 1.2f + i * 0.7f) * 18.0f);
        DrawRectangle(x + sway, 210, 30, 360, (Color){48, 30, 20, 255});
        DrawCircle(x + 15 + sway, 200, 70, Fade(DARKGREEN, 0.95f));
        DrawCircle(x - 20 + sway, 230, 55, Fade((Color){44, 101, 62, 255}, 0.95f));
        DrawCircle(x + 50 + sway, 235, 48, Fade((Color){32, 83, 51, 255}, 0.95f));
    }

    DrawRectangle(0, 540, SCREEN_W, 160, (Color){26, 58, 31, 255});
    for (int i = 0; i < 14; i++) {
        int vineX = 35 + i * 85;
        int vineSwing = (int)(cosf(t * 0.8f + i) * 12.0f);
        DrawLineEx((Vector2){(float)vineX, 0}, (Vector2){(float)(vineX + vineSwing), 220}, 4.0f, Fade(LIME, 0.30f));
    }
}

static void DrawTempleEntrance(float t) {
    DrawRectangleRounded((Rectangle){640, 185, 280, 260}, 0.06f, 6, (Color){84, 80, 68, 255});
    DrawRectangle(680, 215, 200, 200, (Color){45, 38, 33, 255});
    DrawRectangleLinesEx((Rectangle){680, 215, 200, 200}, 5, Fade(GOLD, 0.35f));
    DrawTriangle((Vector2){620, 190}, (Vector2){780, 75}, (Vector2){940, 190}, (Color){98, 92, 78, 255});
    DrawCircle(780, 310, 22 + (int)(sinf(t * 2.4f) * 4.0f), Fade(GOLD, 0.25f));
}

static void DrawCharacter(Vector2 pos, Color body, Color head, bool staff) {
    DrawCircleV((Vector2){pos.x, pos.y - 78}, 22, head);
    DrawRectangleRounded((Rectangle){pos.x - 16, pos.y - 60, 32, 70}, 0.3f, 6, body);
    DrawLineEx((Vector2){pos.x - 10, pos.y + 10}, (Vector2){pos.x - 22, pos.y + 48}, 4.0f, body);
    DrawLineEx((Vector2){pos.x + 10, pos.y + 10}, (Vector2){pos.x + 22, pos.y + 48}, 4.0f, body);
    DrawLineEx((Vector2){pos.x - 14, pos.y - 35}, (Vector2){pos.x - 32, pos.y - 2}, 4.0f, body);
    DrawLineEx((Vector2){pos.x + 14, pos.y - 35}, (Vector2){pos.x + 32, pos.y - 2}, 4.0f, body);

    if (staff) {
        DrawLineEx((Vector2){pos.x + 28, pos.y - 55}, (Vector2){pos.x + 32, pos.y + 55}, 5.0f, GOLD);
        DrawCircle((int)pos.x + 30, (int)pos.y - 62, 8, Fade(SKYBLUE, 0.9f));
    }
}

static void DrawStoryScene(const GameData *g) {
    float t = (float)GetTime();
    int page = g->storyPage;

    DrawJungleBackdrop(t);

    if (page >= 1) {
        DrawTempleEntrance(t);
    }

    if (page == 0) {
        DrawCharacter((Vector2){290, 500}, (Color){65, 117, 174, 255}, SKIN_TONE, false);
        DrawCharacter((Vector2){415, 495}, (Color){114, 76, 50, 255}, SKIN_TONE, false);
        DrawRectangleRounded((Rectangle){125, 410, 170, 36}, 0.3f, 6, Fade(BEIGE, 0.9f));
        DrawText("Aunt's expedition camp", 137, 420, 18, DARKBROWN);
    } else if (page == 1) {
        DrawCharacter((Vector2){320, 500}, (Color){65, 117, 174, 255}, SKIN_TONE, false);
        DrawCharacter((Vector2){470, 495}, (Color){114, 76, 50, 255}, SKIN_TONE, false);
        DrawLineEx((Vector2){440, 420}, (Vector2){340, 380}, 6.0f, Fade(MAROON, 0.9f));
        DrawText("Do not enter.", 330, 345, 26, MAROON);
    } else if (page == 2) {
        DrawTempleEntrance(t);
        DrawCharacter((Vector2){520, 500}, (Color){65, 117, 174, 255}, SKIN_TONE, false);
        DrawRectangle(665, 215, 18, 200, GRAY);
        DrawRectangle(877, 215, 18, 200, GRAY);
        DrawRectangle(680, 215, 200, 22, DARKGRAY);
        DrawRectangle(680, 393, 200, 22, DARKGRAY);
        DrawText("BOOM", 720, 150, 36, ORANGE);
    } else {
        DrawTempleEntrance(t);
        DrawCharacter((Vector2){350, 500}, (Color){65, 117, 174, 255}, SKIN_TONE, false);
        DrawCharacter((Vector2){765, 495}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
        DrawCircle(760, 300, 80, Fade(SKYBLUE, 0.12f));
        DrawText("\"Five trials. One escape.\"", 575, 165, 28, RAYWHITE);
    }

    DrawRectangleRounded((Rectangle){60, 470, 980, 170}, 0.08f, 8, Fade(BLACK, 0.62f));
    DrawText(STORY_HEADINGS[page], 92, 500, 34, GOLD);
    DrawText(STORY_LINES[page][0], 92, 548, 24, RAYWHITE);
    DrawText(STORY_LINES[page][1], 92, 582, 24, RAYWHITE);
    DrawText(STORY_LINES[page][2], 92, 616, 24, RAYWHITE);
    DrawText(TextFormat("Story %d/4", page + 1), 905, 500, 22, Fade(RAYWHITE, 0.85f));
    DrawText("ENTER: next scene", 840, 615, 22, Fade(GOLD, 0.9f));
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
            float t = (float)GetTime();
            DrawJungleBackdrop(t);
            DrawTempleEntrance(t);
            DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.15f), Fade(BLACK, 0.65f));
            DrawCharacter((Vector2){240, 520}, (Color){65, 117, 174, 255}, SKIN_TONE, false);
            DrawCharacter((Vector2){835, 520}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
            DrawRectangleRounded((Rectangle){135, 115, 830, 270}, 0.06f, 8, Fade(BLACK, 0.46f));
            DrawText("KIKI: LOST IN THE JUNGLE", 185, 155, 56, RAYWHITE);
            DrawText("A 2D story game in C + raylib", 335, 228, 30, Fade(RAYWHITE, 0.9f));
            DrawText("A teenager enters a cursed temple and must survive five trials to return to his aunt.", 150, 285, 25, GOLD);
            DrawText("Press ENTER to begin the opening cutscene", 298, 338, 28, RAYWHITE);
            DrawText("Version 1 focuses on story setup, atmosphere, and trial progression.", 215, 610, 22, Fade(RAYWHITE, 0.85f));
            if (IsKeyPressed(KEY_ENTER)) {
                game.storyPage = 0;
                scene = SCENE_STORY;
            }
        }

        if (scene == SCENE_STORY) {
            DrawStoryScene(&game);
            if (IsKeyPressed(KEY_ENTER)) {
                if (game.storyPage < 3) {
                    game.storyPage++;
                } else {
                    scene = SCENE_MAP;
                }
            }
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
