#include "game.h"

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "KiKi - Temple Breaker");
    SetTargetFPS(60);
    InitAudioDevice();
    InitGameAssets();

    GameData game = {0};
    Scene scene = SCENE_TITLE;
    ResetGame(&game);

    while (!WindowShouldClose()) {
        UpdateGameAudio();
        BeginDrawing();
        ClearBackground(BLACK);

        if (scene == SCENE_TITLE) {
            DrawTitleScene(&game);
            Rectangle audioToggle = {314, 628, 132, 36};
            bool titleConfirm = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE);

            if (IsKeyPressed(KEY_M)) {
                SetAudioEnabled(!IsAudioEnabled());
            }
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), audioToggle)) {
                SetAudioEnabled(!IsAudioEnabled());
            }

            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                game.menuSelection = (game.menuSelection + 2) % 3;
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                game.menuSelection = (game.menuSelection + 1) % 3;
            }
            if (titleConfirm) {
                if (game.menuSelection == 0) {
                    game.storyPage = 0;
                    game.postLevelDialogue = false;
                    scene = SCENE_STORY;
                } else if (game.menuSelection == 1) {
                    game.dialoguePage = 0;
                    game.postLevelDialogue = false;
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
                int maxPage = game.postLevelDialogue ? 1 : 3;
                if (game.dialoguePage < maxPage) {
                    game.dialoguePage++;
                } else {
                    game.postLevelDialogue = false;
                    scene = SCENE_MAP;
                }
            }
        } else if (scene == SCENE_MAP) {
            DrawMapScene(&game);
            if (IsKeyPressed(KEY_ONE) && game.unlocked[0]) {
                game.postLevelDialogue = false;
                StartLevel(&game, 1);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_TWO) && game.unlocked[1]) {
                game.postLevelDialogue = false;
                StartLevel(&game, 2);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_THREE) && game.unlocked[2]) {
                game.postLevelDialogue = false;
                StartLevel(&game, 3);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_FOUR) && game.unlocked[3]) {
                game.postLevelDialogue = false;
                StartLevel(&game, 4);
                scene = SCENE_LEVEL;
            }
            if (IsKeyPressed(KEY_FIVE) && game.unlocked[4]) {
                game.postLevelDialogue = false;
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
            DrawEndScene(&game);
            if (IsKeyPressed(KEY_R)) {
                ResetGame(&game);
                scene = SCENE_TITLE;
            }
        }

        EndDrawing();
    }

    UnloadGameAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
