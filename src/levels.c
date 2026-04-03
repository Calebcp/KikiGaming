#include "game.h"

#include <math.h>
#include <raymath.h>

static void DrawLevelBackground(BackgroundId id, float tintAlpha) {
    if (HasBackgroundTexture(id)) {
        Texture2D texture = GetBackgroundTexture(id);
        Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
        Rectangle dest = {0, 0, (float)SCREEN_W, (float)SCREEN_H};
        float textureAspect = (float)texture.width / (float)texture.height;
        float screenAspect = (float)SCREEN_W / (float)SCREEN_H;

        if (textureAspect > screenAspect) {
            float cropWidth = (float)texture.height * screenAspect;
            source.x = ((float)texture.width - cropWidth) * 0.5f;
            source.width = cropWidth;
        } else {
            float cropHeight = (float)texture.width / screenAspect;
            source.y = ((float)texture.height - cropHeight) * 0.5f;
            source.height = cropHeight;
        }

        DrawTexturePro(texture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, tintAlpha));
    }
}

static void DamagePlayer(GameData *g) {
    if (g->damageCooldown > 0.0f) {
        return;
    }

    g->hearts--;
    g->damageCooldown = 1.35f;
}

static void CompleteLevel(GameData *g) {
    int index = g->currentLevel - 1;
    g->completed[index] = true;
    if (index + 1 < MAX_LEVELS) {
        g->unlocked[index + 1] = true;
    }
}

static void BeginPostLevelDialogue(GameData *g, Scene *scene) {
    g->postLevelDialogue = true;
    g->postLevelLevel = g->currentLevel;
    g->dialoguePage = 0;
    *scene = SCENE_DIALOGUE;
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
    const float playerHitboxYOffset = 10.0f;
    Rectangle platforms[4] = {
        {215, 610, 170, 18},
        {472, 516, 170, 18},
        {734, 610, 170, 18},
        {1008, 430, 170, 18}
    };
    Rectangle flood = {0, 640, SCREEN_W, 80};

    DrawJungleBackdrop(t);
    DrawLevelBackground(BG_LEVEL_ONE, 0.55f);
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
        g->playerPos.y = bestY - playerHitboxYOffset;
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
    DrawText(TextFormat("%.1f / 28", g->levelTimer), 24, 196, 26, ACCENT_GOLD);
    DrawText("Run, jump, and save the second jump for the high ledge near the exit.", 24, 228, 20, Fade(RAYWHITE, 0.76f));

    if (g->playerPos.x >= 1160 && g->playerPos.y <= 504) {
        CompleteLevel(g);
        BeginPostLevelDialogue(g, scene);
    }

    if (g->levelTimer > 28.0f) {
        DamagePlayer(g);
        StartLevel(g, 1);
    }
}

static void UpdateLevelTwo(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle arena = {74, 168, 1134, 478};

    DrawJungleBackdrop(t);
    DrawLevelBackground(BG_LEVEL_TWO, 0.46f);
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
        BeginPostLevelDialogue(g, scene);
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
    DrawLevelBackground(BG_LEVEL_THREE, 0.44f);
    DrawRectangleRounded(room, 0.05f, 8, Fade((Color){28, 25, 36, 255}, 0.90f));
    DrawRectangleLinesEx(room, 6.0f, Fade(CURSE_BLUE, 0.40f));
    DrawBanner("LV3: Rune Trial", "Reach each rune and answer the riddle with keys 1, 2, or 3 while the curse keeps moving.");
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

        if (glyph->active && CheckCollisionRecs(GetPlayerRect(g), glyph->rect) && g->activeQuizIndex < 0) {
            int expectedGlyph = g->runeOrder[g->runeProgress];
            if (i == expectedGlyph) {
                g->activeQuizIndex = i;
            } else {
                DamagePlayer(g);
                StartLevel(g, 3);
                return;
            }
        }
    }

    if (g->activeQuizIndex >= 0) {
        int expectedGlyph = g->activeQuizIndex;
        int answer = 0;
        if (IsKeyPressed(KEY_ONE)) {
            answer = 1;
        } else if (IsKeyPressed(KEY_TWO)) {
            answer = 2;
        } else if (IsKeyPressed(KEY_THREE)) {
            answer = 3;
        }

        DrawRectangleRounded((Rectangle){180, 208, 920, 244}, 0.08f, 8, Fade(BLACK, 0.86f));
        DrawText("Rune Question", 216, 234, 34, ACCENT_GOLD);
        DrawText(LEVEL_THREE_QUESTIONS[expectedGlyph], 216, 286, 26, RAYWHITE);
        DrawText(LEVEL_THREE_OPTIONS[expectedGlyph][0], 240, 338, 24, Fade(RAYWHITE, 0.90f));
        DrawText(LEVEL_THREE_OPTIONS[expectedGlyph][1], 240, 374, 24, Fade(RAYWHITE, 0.90f));
        DrawText(LEVEL_THREE_OPTIONS[expectedGlyph][2], 240, 410, 24, Fade(RAYWHITE, 0.90f));

        if (answer != 0) {
            if (answer == LEVEL_THREE_CORRECT[expectedGlyph]) {
                g->glyphs[expectedGlyph].active = false;
                g->runeProgress++;
                g->runeFlash = 0.35f;
                g->activeQuizIndex = -1;
            } else {
                DamagePlayer(g);
                g->activeQuizIndex = -1;
                StartLevel(g, 3);
                return;
            }
        }
    }

    DrawHero(g->playerPos, g->facingRight, false, t);
    if (g->runeFlash > 0.0f) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(ACCENT_GOLD, g->runeFlash * 0.25f));
    }

    DrawText("Visit runes in the order 2 -> 3 -> 1", 24, 174, 24, RAYWHITE);
    DrawText("Touch only the glowing rune. Wrong rune choices or wrong answers cost a heart and restart the chamber.", 24, 206, 20, Fade(RAYWHITE, 0.78f));

    if (g->runeProgress >= 3) {
        CompleteLevel(g);
        BeginPostLevelDialogue(g, scene);
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
    const float playerHitboxXOffset = 6.0f;
    const float playerHitboxYOffset = 10.0f;
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
    DrawLevelBackground(BG_LEVEL_FOUR, 0.40f);
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
        g->playerPos.x = Clamp(nextX.x - playerHitboxXOffset, arena.x + 8, arena.x + arena.width - 36.0f - 8.0f);
    }
    Rectangle nextY = GetPlayerRect(g);
    nextY.y += move.y * baseSpeed * dt;
    if (!CollidesWall(nextY, walls, 10)) {
        g->playerPos.y = Clamp(nextY.y - playerHitboxYOffset, arena.y + 8, arena.y + arena.height - 56.0f - 8.0f);
    }

    for (int i = 0; i < 10; i++) {
        DrawRectangleRounded(walls[i], 0.15f, 4, TEMPLE_STONE);
    }
    DrawRectangleRounded(exitDoor, 0.2f, 4, Fade(GREEN, 0.92f));
    DrawRectangleLinesEx(exitDoor, 4.0f, Fade(ACCENT_GOLD, 0.65f));
    DrawHero(g->playerPos, g->facingRight, g->dashTimer > 0.0f, t);

    if (CheckCollisionRecs(GetPlayerRect(g), exitDoor)) {
        CompleteLevel(g);
        BeginPostLevelDialogue(g, scene);
    }

    g->levelTimer += dt;
    DrawText("Dash: LEFT SHIFT", 24, 172, 22, RAYWHITE);
    DrawText(TextFormat("Collapse: %.1f / 42.0", g->levelTimer), 24, 202, 24, ACCENT_GOLD);
    DrawText("Route: start in the top lane, drop after pillar two, rise after pillar four, then sprint to the gate.", 24, 232, 20, Fade(RAYWHITE, 0.76f));

    if (g->levelTimer >= 42.0f) {
        DamagePlayer(g);
        StartLevel(g, 4);
    }
}

static void UpdateLevelFive(GameData *g, Scene *scene) {
    float dt = GetFrameTime();
    float t = (float)GetTime();
    Rectangle room = {50, 150, 1180, 520};
    Rectangle choiceMercy = {400, 585, 200, 44};
    Rectangle choiceEscape = {690, 585, 200, 44};

    DrawJungleBackdrop(t);
    DrawLevelBackground(BG_LEVEL_FIVE, 0.42f);
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

void UpdateLevel(GameData *g, Scene *scene) {
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
        g->hearts = 5;
        g->activeQuizIndex = -1;
        *scene = SCENE_MAP;
    }
}
