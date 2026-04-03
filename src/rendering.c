#include "game.h"

#include <math.h>
#include <stdio.h>
#include <raymath.h>

static void DrawFullscreenTexture(Texture2D texture, float alpha) {
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

    DrawTexturePro(texture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, alpha));
}

static void DrawTextureAnchored(Texture2D texture, Rectangle dest, float alpha) {
    Rectangle source = {0, 0, (float)texture.width, (float)texture.height};
    DrawTexturePro(texture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, alpha));
}

static void DrawHumanoidTexture(Texture2D texture, float centerX, float baselineY, float height, float alpha, bool facingRight) {
    float width = height * ((float)texture.width / (float)texture.height);
    Rectangle source = {
        facingRight ? 0.0f : (float)texture.width,
        0.0f,
        facingRight ? (float)texture.width : -(float)texture.width,
        (float)texture.height
    };
    Rectangle dest = {centerX - width * 0.5f, baselineY - height, width, height};
    DrawTexturePro(texture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, alpha));
}

static void DrawMistBand(float y, float alpha, float t) {
    for (int i = 0; i < 7; i++) {
        float x = 80.0f + i * 190.0f + sinf(t * 0.5f + i) * 45.0f;
        DrawEllipse((int)x, (int)y, 120, 34, Fade(RAYWHITE, alpha));
    }
}

static void DrawCanopyCluster(Vector2 center, float scale, float t, Color base, Color accent) {
    float sway = sinf(t * 0.45f + center.x * 0.01f) * 10.0f * scale;
    DrawCircleV((Vector2){center.x - 72.0f * scale + sway, center.y + 6.0f * scale}, 72.0f * scale, Fade(base, 0.94f));
    DrawCircleV((Vector2){center.x + sway, center.y - 18.0f * scale}, 92.0f * scale, Fade(accent, 0.96f));
    DrawCircleV((Vector2){center.x + 76.0f * scale + sway, center.y + 10.0f * scale}, 66.0f * scale, Fade(base, 0.92f));
    DrawCircleV((Vector2){center.x - 10.0f * scale + sway, center.y + 52.0f * scale}, 82.0f * scale, Fade((Color){18, 58, 42, 255}, 0.92f));
}

static void DrawJungleTree(float x, float baseY, float scale, float t, Color trunk, Color foliageA, Color foliageB) {
    float sway = sinf(t * 0.6f + x * 0.015f) * 7.0f * scale;
    Rectangle trunkRect = {x - 20.0f * scale + sway, baseY - 245.0f * scale, 40.0f * scale, 270.0f * scale};
    DrawRectangleRounded(trunkRect, 0.28f, 6, trunk);
    DrawRectangleRounded((Rectangle){trunkRect.x + 8.0f * scale, trunkRect.y + 24.0f * scale, 10.0f * scale, 214.0f * scale}, 0.4f, 4, Fade((Color){88, 56, 30, 255}, 0.55f));
    DrawLineEx((Vector2){trunkRect.x + 12.0f * scale, baseY - 6.0f * scale}, (Vector2){trunkRect.x - 28.0f * scale, baseY + 22.0f * scale}, 8.0f * scale, Fade(trunk, 0.95f));
    DrawLineEx((Vector2){trunkRect.x + trunkRect.width - 10.0f * scale, baseY - 2.0f * scale}, (Vector2){trunkRect.x + trunkRect.width + 32.0f * scale, baseY + 20.0f * scale}, 7.0f * scale, Fade(trunk, 0.92f));

    DrawCanopyCluster((Vector2){x + sway, baseY - 248.0f * scale}, 1.02f * scale, t, foliageA, foliageB);
    DrawCanopyCluster((Vector2){x - 58.0f * scale + sway, baseY - 220.0f * scale}, 0.72f * scale, t + 0.3f, foliageB, foliageA);
    DrawCanopyCluster((Vector2){x + 66.0f * scale + sway, baseY - 210.0f * scale}, 0.68f * scale, t + 0.6f, foliageA, foliageB);
}

void DrawJungleBackdrop(float t) {
    if (HasBackgroundTexture(BG_INTRO)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_INTRO), 0.42f);
    }
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, SKY_TOP, SKY_BOTTOM);
    DrawCircleGradient(1020, 135, 150, Fade(ACCENT_GOLD, 0.35f), BLANK);
    DrawCircleGradient(220, 90, 110, Fade(CURSE_BLUE, 0.16f), BLANK);

    DrawRectangleGradientV(0, 166, SCREEN_W, 280, Fade((Color){10, 32, 24, 255}, 0.22f), Fade((Color){6, 18, 14, 255}, 0.68f));
    for (int i = 0; i < 11; i++) {
        float ridgeX = -30.0f + i * 132.0f;
        DrawTriangle(
            (Vector2){ridgeX, 408},
            (Vector2){ridgeX + 88.0f, 278.0f + sinf(t * 0.15f + i) * 12.0f},
            (Vector2){ridgeX + 176.0f, 408},
            Fade((Color){14, 36, 30, 255}, 0.68f));
    }

    for (int i = 0; i < 9; i++) {
        float x = 70.0f + i * 145.0f;
        float y = 232.0f + (i % 2) * 18.0f;
        DrawCanopyCluster((Vector2){x, y}, 0.95f, t + i * 0.2f, (Color){20, 60, 43, 255}, (Color){28, 82, 58, 255});
    }

    DrawJungleTree(126.0f, 584.0f, 1.18f, t, (Color){70, 44, 23, 255}, (Color){18, 70, 49, 255}, (Color){33, 102, 71, 255});
    DrawJungleTree(326.0f, 578.0f, 0.94f, t + 0.3f, (Color){78, 48, 29, 255}, (Color){22, 74, 53, 255}, (Color){42, 112, 78, 255});
    DrawJungleTree(574.0f, 588.0f, 1.08f, t + 0.5f, (Color){66, 40, 21, 255}, (Color){17, 66, 47, 255}, (Color){36, 98, 68, 255});
    DrawJungleTree(816.0f, 582.0f, 0.92f, t + 0.9f, (Color){72, 44, 25, 255}, (Color){24, 76, 54, 255}, (Color){39, 109, 75, 255});
    DrawJungleTree(1088.0f, 590.0f, 1.24f, t + 1.1f, (Color){62, 38, 20, 255}, (Color){15, 62, 45, 255}, (Color){32, 92, 66, 255});

    for (int i = 0; i < 15; i++) {
        float vineX = 16.0f + i * 86.0f;
        float swing = cosf(t * 0.85f + i * 0.65f) * (12.0f + (i % 3) * 5.0f);
        float drop = 170.0f + (i % 4) * 22.0f;
        DrawLineEx((Vector2){vineX, -8.0f}, (Vector2){vineX + swing, drop}, 4.0f, Fade((Color){72, 132, 78, 255}, 0.34f));
        DrawCircleV((Vector2){vineX + swing, drop}, 7.0f, Fade((Color){92, 168, 94, 255}, 0.22f));
    }

    for (int i = 0; i < 24; i++) {
        float leafX = 18.0f + i * 54.0f + sinf(t * 0.4f + i) * 10.0f;
        float leafY = 470.0f + (i % 5) * 22.0f;
        DrawEllipse((int)leafX, (int)leafY, 26, 10, Fade((Color){38, 92, 58, 255}, 0.78f));
    }

    DrawRectangleGradientV(0, 548, SCREEN_W, 172, (Color){28, 82, 48, 255}, (Color){16, 44, 27, 255});
    DrawRectangleGradientV(0, 602, SCREEN_W, 118, Fade((Color){18, 54, 31, 255}, 0.98f), Fade((Color){8, 24, 15, 255}, 0.98f));
    DrawMistBand(530.0f, 0.07f, t);
    DrawMistBand(590.0f, 0.05f, t + 0.8f);
}

void DrawTempleFrame(float t) {
    DrawRectangleRounded((Rectangle){810, 145, 330, 320}, 0.05f, 8, TEMPLE_STONE);
    DrawRectangle(860, 188, 230, 245, TEMPLE_DARK);
    DrawRectangleLinesEx((Rectangle){860, 188, 230, 245}, 6.0f, Fade(ACCENT_GOLD, 0.42f));
    DrawTriangle((Vector2){785, 148}, (Vector2){975, 40}, (Vector2){1165, 148}, (Color){115, 106, 87, 255});
    DrawCircle(975, 314, 30 + (int)(sinf(t * 2.1f) * 5.0f), Fade(CURSE_BLUE, 0.30f));
    DrawRing((Vector2){975, 314}, 46, 54, 0, 360, 60, Fade(ACCENT_GOLD, 0.28f));
}

void DrawHero(Vector2 pos, bool facingRight, bool attacking, float t) {
    if (attacking) {
        Texture2D attackTexture = {0};
        bool hasAttackTexture = false;
        if (facingRight && HasHeroAttackRightTexture()) {
            attackTexture = GetHeroAttackRightTexture();
            hasAttackTexture = true;
        } else if (!facingRight && HasHeroAttackLeftTexture()) {
            attackTexture = GetHeroAttackLeftTexture();
            hasAttackTexture = true;
        }

        if (hasAttackTexture) {
            float width = 182.0f;
            float height = width * ((float)attackTexture.height / (float)attackTexture.width);
            Rectangle source = {0, 0, (float)attackTexture.width, (float)attackTexture.height};
            Rectangle dest = facingRight
                ? (Rectangle){pos.x - 40.0f, pos.y - 72.0f, width, height}
                : (Rectangle){pos.x - 106.0f, pos.y - 72.0f, width, height};
            DrawTexturePro(attackTexture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, 0.96f));
            return;
        }
    }

    if (HasHeroTexture()) {
        DrawHumanoidTexture(GetHeroTexture(), pos.x + 18.0f, pos.y + 56.0f, 138.0f, attacking ? 0.95f : 0.90f, facingRight);

        if (attacking) {
            int dir = facingRight ? 1 : -1;
            Vector2 slashCenter = {pos.x + 18 + dir * 24.0f, pos.y + 34};
            float sweep = 0.5f + 0.5f * sinf(t * 34.0f);
            DrawRing(slashCenter, 18, 38, facingRight ? -44 : 136, facingRight ? 80 : 260, 28, Fade(ACCENT_GOLD, 0.92f));
            DrawRing(slashCenter, 30, 48, facingRight ? -58 : 122, facingRight ? 58 : 238, 22, Fade(RAYWHITE, 0.32f));
            DrawLineEx(
                (Vector2){slashCenter.x - dir * 4.0f, slashCenter.y - 18.0f},
                (Vector2){slashCenter.x + dir * (42.0f + sweep * 10.0f), slashCenter.y + 7.0f},
                6.0f, Fade(ACCENT_GOLD, 0.72f));
            DrawLineEx(
                (Vector2){slashCenter.x - dir * 10.0f, slashCenter.y - 2.0f},
                (Vector2){slashCenter.x + dir * (28.0f + sweep * 8.0f), slashCenter.y + 24.0f},
                4.0f, Fade(RAYWHITE, 0.65f));
            DrawCircleV((Vector2){slashCenter.x + dir * 18.0f, slashCenter.y + 2.0f}, 8.0f + sweep * 4.0f, Fade(ACCENT_GOLD, 0.42f));
        }
        return;
    }

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
        float sweep = 0.5f + 0.5f * sinf(t * 34.0f);
        DrawRing(slashCenter, 18, 38, facingRight ? -44 : 136, facingRight ? 80 : 260, 28, Fade(ACCENT_GOLD, 0.92f));
        DrawRing(slashCenter, 30, 48, facingRight ? -58 : 122, facingRight ? 58 : 238, 22, Fade(RAYWHITE, 0.32f));
        DrawLineEx(
            (Vector2){slashCenter.x - dir * 4.0f, slashCenter.y - 18.0f},
            (Vector2){slashCenter.x + dir * (42.0f + sweep * 10.0f), slashCenter.y + 7.0f},
            6.0f, Fade(ACCENT_GOLD, 0.72f));
        DrawLineEx(
            (Vector2){slashCenter.x - dir * 10.0f, slashCenter.y - 2.0f},
            (Vector2){slashCenter.x + dir * (28.0f + sweep * 8.0f), slashCenter.y + 24.0f},
            4.0f, Fade(RAYWHITE, 0.65f));
        DrawCircleV((Vector2){slashCenter.x + dir * 18.0f, slashCenter.y + 2.0f}, 8.0f + sweep * 4.0f, Fade(ACCENT_GOLD, 0.42f));
    }
}

void DrawCharacter(Vector2 pos, Color body, Color head, bool staff) {
    if (staff && HasWizardTexture()) {
        DrawHumanoidTexture(GetWizardTexture(), pos.x, pos.y + 54.0f, 188.0f, 0.92f, false);
        return;
    }

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

void DrawDragon(Vector2 pos, bool facingRight, float scale, float alpha) {
    if (HasDragonTexture()) {
        Texture2D texture = GetDragonTexture();
        float width = 430.0f * scale;
        float height = width * ((float)texture.height / (float)texture.width);
        Rectangle source = {
            facingRight ? 0.0f : (float)texture.width,
            0.0f,
            facingRight ? (float)texture.width : -(float)texture.width,
            (float)texture.height
        };
        Rectangle dest = {pos.x - width * 0.5f, pos.y - height * 0.60f, width, height};
        DrawTexturePro(texture, source, dest, Vector2Zero(), 0.0f, Fade(RAYWHITE, alpha));
        return;
    }

    DrawSnakeBoss(pos, GetTime());
}

void DrawSpider(Vector2 pos, float scale, float t, Color tint) {
    if (HasSpiderTexture()) {
        Texture2D texture = GetSpiderTexture();
        float width = 92.0f * scale;
        float height = width * ((float)texture.height / (float)texture.width);
        Rectangle dest = {pos.x - width * 0.5f, pos.y - height * 0.45f, width, height};
        DrawTextureAnchored(texture, dest, 0.94f);
        return;
    }

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

void DrawSnakeBoss(Vector2 pos, float t) {
    for (int i = 0; i < 6; i++) {
        float wave = sinf(t * 6.0f + i * 0.5f) * 10.0f;
        DrawCircleV((Vector2){pos.x - i * 18.0f, pos.y + wave}, 18 - i, (Color){87, 164, 89, 255});
    }
    DrawCircleV(pos, 24, (Color){62, 129, 74, 255});
    DrawCircleV((Vector2){pos.x + 12, pos.y - 6}, 4, BLACK);
    DrawCircleV((Vector2){pos.x + 12, pos.y + 6}, 4, BLACK);
    DrawRectangle((int)pos.x + 20, (int)pos.y - 2, 16, 4, DANGER_RED);
}

void DrawHearts(int hearts) {
    hearts = Clamp(hearts, 0, MAX_HEARTS);
    DrawText("HEARTS", 24, 18, 20, Fade(RAYWHITE, 0.88f));

    for (int i = 0; i < MAX_HEARTS; i++) {
        int row = i / 5;
        int col = i % 5;
        int x = 28 + col * 42;
        int y = 44 + row * 36;
        Color fill = i < hearts ? DANGER_RED : Fade((Color){68, 38, 44, 255}, 0.95f);
        Color outline = i < hearts ? Fade(RAYWHITE, 0.16f) : Fade(RAYWHITE, 0.10f);

        if (HasHeartTexture()) {
            Texture2D heart = GetHeartTexture();
            Rectangle source = {0, 0, (float)heart.width, (float)heart.height};
            Rectangle dest = {(float)x, (float)y, 28.0f, 28.0f};
            Color tint = i < hearts ? RAYWHITE : Fade((Color){92, 56, 62, 255}, 0.58f);
            DrawTexturePro(heart, source, dest, Vector2Zero(), 0.0f, tint);
            if (i >= hearts) {
                DrawRectangleRounded((Rectangle){(float)x, (float)y, 28.0f, 28.0f}, 0.3f, 4, Fade(BLACK, 0.28f));
            }
        } else {
            DrawCircle(x, y, 9, fill);
            DrawCircle(x + 12, y, 9, fill);
            DrawTriangle((Vector2){x - 8, y + 2}, (Vector2){x + 20, y + 2}, (Vector2){x + 6, y + 22}, fill);
            DrawCircleLines(x, y, 9, outline);
            DrawCircleLines(x + 12, y, 9, outline);
        }
    }
}

void DrawBanner(const char *title, const char *subtitle) {
    DrawRectangleRounded((Rectangle){18, 80, SCREEN_W - 36, 78}, 0.14f, 8, Fade(BLACK, 0.45f));
    DrawText(title, 34, 92, 30, RAYWHITE);
    DrawText(subtitle, 34, 126, 20, Fade(RAYWHITE, 0.82f));
}

void DrawWrappedTextBlock(const char *text, Rectangle bounds, float fontSize, float spacing, Color color) {
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

static void DrawFittedText(const char *text, Rectangle bounds, float fontSize, float spacing, Color color) {
    Font font = GetFontDefault();
    float size = fontSize;
    Vector2 measured = MeasureTextEx(font, text, size, spacing);

    while (measured.x > bounds.width && size > 12.0f) {
        size -= 1.0f;
        measured = MeasureTextEx(font, text, size, spacing);
    }

    DrawTextEx(font, text, (Vector2){bounds.x, bounds.y}, size, spacing, color);
}

void DrawTitleScene(const GameData *g) {
    float t = (float)GetTime();
    bool audioEnabled = IsAudioEnabled();
    bool audioAvailable = IsAudioAvailable();
    bool toggleHovered = CheckCollisionPointRec(GetMousePosition(), (Rectangle){314, 628, 132, 36});
    DrawJungleBackdrop(t);
    if (HasBackgroundTexture(BG_INTRO)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_INTRO), 0.74f);
    } else if (HasBackgroundTexture(BG_ENDING_GOOD)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_ENDING_GOOD), 0.60f);
    }
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
        DrawFittedText(MENU_ITEMS[i], (Rectangle){158, 378 + i * 82, 290, 28}, 28.0f, 1.0f, selected ? BLACK : RAYWHITE);
    }

    Rectangle audioPanel = {130, 616, 344, 62};
    Rectangle togglePill = {314, 628, 132, 36};
    DrawRectangleRounded(audioPanel, 0.24f, 8, Fade((Color){18, 21, 28, 255}, 0.94f));
    DrawRectangleLinesEx(audioPanel, 2.0f, Fade(audioEnabled ? ACCENT_GOLD : RAYWHITE, toggleHovered ? 0.45f : 0.18f));
    DrawText("Audio", 154, 634, 28, RAYWHITE);
    DrawRectangleRounded(togglePill, 0.5f, 8,
                         !audioAvailable ? Fade((Color){82, 82, 82, 255}, 0.92f)
                                         : (audioEnabled ? Fade((Color){60, 146, 94, 255}, 0.98f)
                                                         : Fade((Color){112, 42, 48, 255}, 0.98f)));
    DrawRectangleLinesEx(togglePill, 2.0f, Fade(RAYWHITE, toggleHovered ? 0.48f : 0.22f));
    DrawCircleV((Vector2){audioEnabled && audioAvailable ? 425.0f : 335.0f, 646.0f}, 13.0f, Fade(RAYWHITE, 0.95f));
    DrawText(!audioAvailable ? "N/A" : (audioEnabled ? "ON" : "OFF"), 346, 635, 22, BLACK);
    DrawWrappedTextBlock(!audioAvailable ? "Audio device did not initialize"
                                         : (audioEnabled ? "Jungle ambience and combat audio enabled" : "All game audio muted"),
                         (Rectangle){488, 632, 300, 40}, 18.0f, 1.0f, Fade(RAYWHITE, 0.82f));

    DrawText("W/S or arrows to select, ENTER to confirm", 132, 576, 20, Fade(RAYWHITE, 0.82f));
    DrawText("Click the audio toggle or press M to mute/unmute", 132, 688, 18, Fade(RAYWHITE, 0.74f));
    DrawText("Temple build: substantial combat and story pass", 790, 630, 24, Fade(RAYWHITE, 0.72f));
}

void DrawStoryScene(const GameData *g) {
    float t = (float)GetTime();
    int page = g->storyPage;

    DrawJungleBackdrop(t);
    if (HasBackgroundTexture(BG_INTRO)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_INTRO), 0.45f);
    }
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.10f), Fade(BLACK, 0.68f));

    if (page == 0) {
        DrawHero((Vector2){228, 382}, true, false, t);
        if (HasAuntTexture()) {
            DrawHumanoidTexture(GetAuntTexture(), 420.0f, 520.0f, 180.0f, 0.95f, true);
        } else {
            DrawCharacter((Vector2){416, 470}, (Color){132, 82, 56, 255}, PLAYER_SKIN, false);
        }
        DrawRectangleRounded((Rectangle){126, 430, 210, 34}, 0.3f, 6, Fade(RAYWHITE, 0.88f));
        DrawText("Expedition camp", 152, 438, 20, TEMPLE_DARK);
    } else if (page == 1) {
        DrawHero((Vector2){286, 388}, true, false, t);
        if (HasAuntTexture()) {
            DrawHumanoidTexture(GetAuntTexture(), 480.0f, 520.0f, 176.0f, 0.95f, true);
        } else {
            DrawCharacter((Vector2){475, 468}, (Color){132, 82, 56, 255}, PLAYER_SKIN, false);
        }
        DrawLineEx((Vector2){452, 406}, (Vector2){360, 368}, 5.0f, DANGER_RED);
        DrawText("Do not cross that arch.", 306, 332, 30, DANGER_RED);
    } else if (page == 2) {
        DrawHero((Vector2){538, 392}, true, false, t);
        DrawRectangle(860, 192, 20, 242, GRAY);
        DrawRectangle(1070, 192, 20, 242, GRAY);
        DrawText("The temple seals shut.", 790, 116, 30, ACCENT_GOLD);
    } else if (page == 3) {
        DrawHero((Vector2){286, 396}, true, false, t);
        DrawCharacter((Vector2){970, 446}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
        DrawCircleGradient(970, 314, 130, Fade(CURSE_BLUE, 0.15f), BLANK);
    } else {
        DrawHero((Vector2){190, 398}, true, true, t);
        DrawSpider((Vector2){575, 570}, 1.1f, t, (Color){90, 55, 126, 255});
        DrawSnakeBoss((Vector2){920, 565}, t);
    }

    DrawRectangleRounded((Rectangle){52, 498, 1176, 138}, 0.08f, 8, Fade(BLACK, 0.72f));
    DrawText(STORY_HEADINGS[page], 82, 518, 32, ACCENT_GOLD);
    DrawWrappedTextBlock(STORY_LINES[page][0], (Rectangle){82, 556, 1100, 24}, 22.0f, 1.0f, RAYWHITE);
    DrawWrappedTextBlock(STORY_LINES[page][1], (Rectangle){82, 586, 1100, 24}, 22.0f, 1.0f, RAYWHITE);
    DrawWrappedTextBlock(STORY_LINES[page][2], (Rectangle){82, 616, 1100, 24}, 22.0f, 1.0f, RAYWHITE);
    DrawText(TextFormat("%d / 5", page + 1), 1122, 520, 22, Fade(RAYWHITE, 0.80f));
    DrawText("ENTER: continue", 1066, 608, 20, Fade(ACCENT_GOLD, 0.95f));
}

void DrawDialogueScene(const GameData *g) {
    float t = (float)GetTime();
    int page = g->dialoguePage;

    DrawJungleBackdrop(t);
    if (HasBackgroundTexture(BG_INTRO)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_INTRO), 0.40f);
    }
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.08f), Fade(BLACK, 0.70f));
    DrawHero((Vector2){238, 384}, true, false, t);
    DrawCharacter((Vector2){980, 438}, (Color){91, 52, 129, 255}, LIGHTGRAY, true);
    DrawCircleGradient(982, 315, 120, Fade(CURSE_BLUE, 0.14f), BLANK);

    DrawRectangleRounded((Rectangle){70, 516, 1140, 114}, 0.08f, 8, Fade(BLACK, 0.78f));
    DrawRectangleRounded((Rectangle){70, 474, 210, 40}, 0.28f, 6, page % 2 == 0 ? Fade(ACCENT_GOLD, 0.95f) : Fade(CURSE_BLUE, 0.95f));
    if (g->postLevelDialogue) {
        int levelIndex = g->postLevelLevel - 1;
        DrawText(POST_LEVEL_SPEAKERS[levelIndex][page], 92, 481, 28, BLACK);
        DrawWrappedTextBlock(POST_LEVEL_LINES[levelIndex][page][0], (Rectangle){96, 544, 980, 30}, 24.0f, 1.0f, RAYWHITE);
        DrawWrappedTextBlock(POST_LEVEL_LINES[levelIndex][page][1], (Rectangle){96, 580, 980, 30}, 24.0f, 1.0f, RAYWHITE);
        DrawText(TextFormat("%d / 2", page + 1), 1120, 482, 22, Fade(RAYWHITE, 0.82f));
    } else {
        DrawText(DIALOGUE_SPEAKERS[page], 92, 481, 28, BLACK);
        DrawWrappedTextBlock(DIALOGUE_LINES[page][0], (Rectangle){96, 544, 980, 30}, 24.0f, 1.0f, RAYWHITE);
        DrawWrappedTextBlock(DIALOGUE_LINES[page][1], (Rectangle){96, 580, 980, 30}, 24.0f, 1.0f, RAYWHITE);
        DrawText(TextFormat("%d / 4", page + 1), 1120, 482, 22, Fade(RAYWHITE, 0.82f));
    }
    DrawText("ENTER to continue", 1016, 598, 20, Fade(ACCENT_GOLD, 0.90f));
}

void DrawMapScene(const GameData *g) {
    float t = (float)GetTime();
    DrawJungleBackdrop(t);
    if (HasBackgroundTexture(BG_INTRO)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_INTRO), 0.38f);
    }
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.15f), Fade(BLACK, 0.52f));
    DrawText("Temple Trial Map", 440, 54, 46, RAYWHITE);
    DrawFittedText("Press 1-5 to enter any unlocked chamber", (Rectangle){330, 106, 620, 28}, 28.0f, 1.0f, Fade(RAYWHITE, 0.82f));
    DrawFittedText("The chambers now demand movement, combat, and decisions under pressure.", (Rectangle){180, 144, 900, 24}, 24.0f, 1.0f, Fade(ACCENT_GOLD, 0.92f));

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
        DrawFittedText(LEVEL_NAMES[i], (Rectangle){nodes[i].x - 110.0f, nodes[i].y + 72.0f, 220.0f, 24.0f}, 24.0f, 1.0f, RAYWHITE);
        DrawWrappedTextBlock(LEVEL_GOALS[i], (Rectangle){nodes[i].x - 124.0f, nodes[i].y + 102.0f, 248.0f, 48.0f}, 18.0f, 1.0f, Fade(RAYWHITE, 0.72f));
    }

    if (g->compassEarned) {
        Rectangle relicBox = {360, 598, 560, 72};
        DrawRectangleRounded(relicBox, 0.3f, 6, Fade(ACCENT_GOLD, 0.94f));
        DrawWrappedTextBlock("Relic secured: Jungle Compass. Final chamber route is now visible.", (Rectangle){384, 614, 510, 42}, 20.0f, 1.0f, BLACK);
    }
}

void DrawEndScene(const GameData *g) {
    float t = (float)GetTime();
    DrawJungleBackdrop(t);
    if (g->win && HasBackgroundTexture(BG_ENDING_GOOD)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_ENDING_GOOD), 0.62f);
    } else if (HasBackgroundTexture(BG_LEVEL_FIVE)) {
        DrawFullscreenTexture(GetBackgroundTexture(BG_LEVEL_FIVE), 0.52f);
    }
    DrawTempleFrame(t);
    DrawRectangleGradientV(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.18f), Fade(BLACK, 0.70f));
    DrawRectangleRounded((Rectangle){132, 126, 1016, 368}, 0.05f, 8, Fade(BLACK, 0.42f));

    if (g->win) {
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
}
