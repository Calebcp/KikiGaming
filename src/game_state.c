#include "game.h"

#include <raymath.h>

const Color SKY_TOP = {16, 42, 58, 255};
const Color SKY_BOTTOM = {6, 14, 20, 255};
const Color CANOPY_A = {28, 84, 60, 255};
const Color CANOPY_B = {16, 58, 42, 255};
const Color TEMPLE_STONE = {99, 92, 74, 255};
const Color TEMPLE_DARK = {43, 39, 32, 255};
const Color PLAYER_SKIN = {255, 220, 177, 255};
const Color PLAYER_JACKET = {51, 120, 196, 255};
const Color ACCENT_GOLD = {224, 191, 87, 255};
const Color CURSE_BLUE = {72, 194, 216, 255};
const Color DANGER_RED = {195, 67, 58, 255};

const char *MENU_ITEMS[3] = {
    "Start Expedition",
    "Read Controls",
    "Quit"
};

const char *LEVEL_NAMES[MAX_LEVELS] = {
    "Flood Run",
    "Nest Breaker",
    "Rune Trial",
    "Serpent Maze",
    "Heart Of The Gate"
};

const char *LEVEL_GOALS[MAX_LEVELS] = {
    "Platform through the flood and outrun hunters.",
    "Fight mobile spiders and survive the swarm.",
    "Answer rune riddles in order while dodging cursed orbs.",
    "Use dash and route planning to escape the collapsing maze.",
    "Collect temple sigils, then choose mercy or escape."
};

const char *STORY_HEADINGS[5] = {
    "Storm Over The Dig Site",
    "The Warning He Ignores",
    "A Door That Breathes",
    "The Prisoner Wizard",
    "Five Trials, One Debt"
};

const char *STORY_LINES[5][3] = {
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

const char *DIALOGUE_SPEAKERS[4] = {
    "Wizard",
    "Kiki",
    "Wizard",
    "Kiki"
};

const char *DIALOGUE_LINES[4][2] = {
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

const char *POST_LEVEL_SPEAKERS[4][2] = {
    {"Wizard", "Kiki"},
    {"Wizard", "Kiki"},
    {"Wizard", "Kiki"},
    {"Wizard", "Kiki"}
};

const char *POST_LEVEL_LINES[4][2][2] = {
    {
        {
            "You outran the flood. Good. Panic did not get to decide for you.",
            "The next chamber will punish hesitation more than fear."
        },
        {
            "Then I keep moving. I did not come this far to drown in the first test.",
            "Open the nest."
        }
    },
    {
        {
            "Steel, timing, pressure. That is how the nest breaks.",
            "The temple sees you as a threat now, not a guest."
        },
        {
            "Let it. If it wants me scared, it should stop sending things I can kill.",
            "What comes next?"
        }
    },
    {
        {
            "Mind before muscle. You read the room before it read you.",
            "Hold on to that. The collapsing maze gives no second thought."
        },
        {
            "Then I run smart, not just fast.",
            "Show me the route."
        }
    },
    {
        {
            "Now you understand the last rule. Escape is never only speed.",
            "At the heart gate, power without mercy becomes another curse."
        },
        {
            "Then I finish this my way.",
            "Open the final chamber."
        }
    }
};

const char *LEVEL_THREE_QUESTIONS[3][4] = {
    {
        "What rises each morning without speaking, but tells the whole jungle when to wake?",
        "What leaves first light on every stone, though no hand carries it through the ruins?",
        "What returns after every night, impossible to cage and impossible to bargain with?",
        "What climbs the horizon with no footsteps, yet every shadow obeys it?"
    },
    {
        "What answers your shout in a stone chamber, but has no mouth of its own?",
        "What repeats your fear inside the temple, yet never thinks a thought itself?",
        "What follows a voice through empty halls, but disappears when the speaker stops?",
        "What sounds alive in the dark for a moment, though it owns no lungs and no tongue?"
    },
    {
        "What holds a tree upright, hidden underground, though no traveler sees it first?",
        "What drinks in silence beneath the earth, anchoring the giant above it?",
        "What grips the soil like a buried hand, though wind and rain cannot pull it loose?",
        "What hides below the forest floor, carrying weight without ever seeing the sun?"
    }
};

const char *LEVEL_THREE_OPTIONS[3][4][3] = {
    {
        {"1  The moon", "2  The sun", "3  The river"},
        {"1  The sun", "2  The vine", "3  The drum"},
        {"1  The thunder", "2  The tide", "3  The sun"},
        {"1  The flame", "2  The sun", "3  The owl"}
    },
    {
        {"1  Echo", "2  Spider", "3  Torch"},
        {"1  Reflection", "2  Echo", "3  Smoke"},
        {"1  Footstep", "2  Rain", "3  Echo"},
        {"1  Echo", "2  Drum", "3  Wind"}
    },
    {
        {"1  Root", "2  Wind", "3  Feather"},
        {"1  Root", "2  River", "3  Ember"},
        {"1  Moss", "2  Root", "3  Cloud"},
        {"1  Root", "2  Stone", "3  Shadow"}
    }
};

const int LEVEL_THREE_CORRECT[3][4] = {
    {2, 1, 3, 2},
    {1, 2, 3, 1},
    {1, 1, 2, 1}
};

Rectangle GetPlayerRect(const GameData *g) {
    return (Rectangle){g->playerPos.x + 6.0f, g->playerPos.y + 10.0f, 24.0f, 46.0f};
}

Rectangle GetEnemyRect(const Enemy *enemy) {
    return (Rectangle){enemy->pos.x - enemy->radius, enemy->pos.y - enemy->radius, enemy->radius * 2.0f, enemy->radius * 2.0f};
}

static void ResetTransientState(GameData *g) {
    g->levelTimer = 0.0f;
    g->damageCooldown = 0.0f;
    g->attackTimer = 0.0f;
    g->attackImpactTimer = 0.0f;
    g->dragonStaggerTimer = 0.0f;
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
    g->activeQuizIndex = -1;
    g->dragonIntroActive = false;
    g->dragonPhaseActive = false;
    g->dragonDefeated = false;
    g->dragonOrbsCollected = 0;
    g->dragonFacingRight = true;
    g->wizardPhase = 0.0f;
    g->attackImpactPos = Vector2Zero();
}

static void ConfigureLevelThreeRun(GameData *g) {
    int previousOrder[3] = {g->lastRuneOrder[0], g->lastRuneOrder[1], g->lastRuneOrder[2]};
    int previousVariants[3] = {
        g->lastRuneQuestionVariant[0],
        g->lastRuneQuestionVariant[1],
        g->lastRuneQuestionVariant[2]
    };

    do {
        g->runeOrder[0] = 0;
        g->runeOrder[1] = 1;
        g->runeOrder[2] = 2;

        for (int i = 2; i > 0; i--) {
            int swapIndex = GetRandomValue(0, i);
            int temp = g->runeOrder[i];
            g->runeOrder[i] = g->runeOrder[swapIndex];
            g->runeOrder[swapIndex] = temp;
        }

        for (int i = 0; i < 3; i++) {
            g->runeQuestionVariant[i] = GetRandomValue(0, 3);
        }
    } while (g->hasLastRuneSetup &&
             g->runeOrder[0] == previousOrder[0] &&
             g->runeOrder[1] == previousOrder[1] &&
             g->runeOrder[2] == previousOrder[2] &&
             g->runeQuestionVariant[0] == previousVariants[0] &&
             g->runeQuestionVariant[1] == previousVariants[1] &&
             g->runeQuestionVariant[2] == previousVariants[2]);

    for (int i = 0; i < 3; i++) {
        g->lastRuneOrder[i] = g->runeOrder[i];
        g->lastRuneQuestionVariant[i] = g->runeQuestionVariant[i];
    }
    g->hasLastRuneSetup = true;
}

void ResetGame(GameData *g) {
    *g = (GameData){0};
    g->unlocked[0] = true;
    g->hearts = MAX_HEARTS;
    g->menuSelection = 0;
    g->storyPage = 0;
    g->dialoguePage = 0;
    g->postLevelDialogue = false;
    g->facingRight = true;
    g->playerAim = (Vector2){1.0f, 0.0f};
    g->activeQuizIndex = -1;
}

void StartLevel(GameData *g, int level) {
    ResetTransientState(g);
    g->currentLevel = level;

    if (level == 1) {
        g->playerPos = (Vector2){70, 548};
        g->facingRight = true;
        g->onGround = true;
        g->canDoubleJump = true;
        g->enemyCount = 4;
        g->enemies[0] = (Enemy){(Vector2){310, 590}, Vector2Zero(), 24, 140.0f, true, ENEMY_HUNTER, 0.0f};
        g->enemies[1] = (Enemy){(Vector2){585, 482}, Vector2Zero(), 24, 170.0f, true, ENEMY_RUSHER, 0.7f};
        g->enemies[2] = (Enemy){(Vector2){860, 590}, Vector2Zero(), 24, 160.0f, true, ENEMY_HUNTER, 1.4f};
        g->enemies[3] = (Enemy){(Vector2){1065, 398}, Vector2Zero(), 22, 190.0f, true, ENEMY_RUSHER, 2.2f};
    } else if (level == 2) {
        g->playerPos = (Vector2){130, 560};
        g->facingRight = true;
        g->onGround = true;
        g->enemyCount = 8;
        g->enemies[0] = (Enemy){(Vector2){170, 205}, Vector2Zero(), 24, 128.0f, true, ENEMY_HUNTER, 0.0f};
        g->enemies[1] = (Enemy){(Vector2){630, 192}, Vector2Zero(), 22, 172.0f, true, ENEMY_RUSHER, 1.1f};
        g->enemies[2] = (Enemy){(Vector2){1100, 225}, Vector2Zero(), 20, 120.0f, true, ENEMY_ORBITER, 2.0f};
        g->enemies[3] = (Enemy){(Vector2){1104, 585}, Vector2Zero(), 23, 134.0f, true, ENEMY_HUNTER, 2.8f};
        g->enemies[4] = (Enemy){(Vector2){710, 618}, Vector2Zero(), 21, 182.0f, true, ENEMY_RUSHER, 3.4f};
        g->enemies[5] = (Enemy){(Vector2){208, 610}, Vector2Zero(), 20, 118.0f, true, ENEMY_ORBITER, 4.2f};
        g->enemies[6] = (Enemy){(Vector2){115, 430}, Vector2Zero(), 23, 138.0f, true, ENEMY_HUNTER, 5.0f};
        g->enemies[7] = (Enemy){(Vector2){1180, 420}, Vector2Zero(), 22, 176.0f, true, ENEMY_RUSHER, 5.8f};
    } else if (level == 3) {
        g->playerPos = (Vector2){110, 570};
        g->facingRight = true;
        g->glyphCount = 3;
        g->glyphs[0] = (Glyph){(Rectangle){260, 160, 110, 110}, true};
        g->glyphs[1] = (Glyph){(Rectangle){620, 420, 110, 110}, true};
        g->glyphs[2] = (Glyph){(Rectangle){1010, 170, 110, 110}, true};
        ConfigureLevelThreeRun(g);
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
        g->wizardPos = (Vector2){960, 506};
        g->wizardPhase = GetRandomValue(0, 628) / 100.0f;
        g->dragonIntroActive = false;
        g->dragonPhaseActive = false;
        g->dragonDefeated = false;
        g->dragonOrbsCollected = 0;
        g->dragonFacingRight = false;
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
