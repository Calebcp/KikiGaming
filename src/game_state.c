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

const char *LEVEL_THREE_QUESTIONS[3] = {
    "What rises each morning without speaking, but tells the whole jungle when to wake?",
    "What answers your shout in a stone chamber, but has no mouth of its own?",
    "What holds a tree upright, hidden underground, though no traveler sees it first?"
};

const char *LEVEL_THREE_OPTIONS[3][3] = {
    {"1  The moon", "2  The sun", "3  The river"},
    {"1  Echo", "2  Spider", "3  Torch"},
    {"1  Root", "2  Wind", "3  Feather"}
};

const int LEVEL_THREE_CORRECT[3] = {2, 1, 1};

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
}

void ResetGame(GameData *g) {
    *g = (GameData){0};
    g->unlocked[0] = true;
    g->hearts = 5;
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
        g->enemyCount = 5;
        g->enemies[0] = (Enemy){(Vector2){690, 560}, Vector2Zero(), 24, 110.0f, true, ENEMY_HUNTER, 0.0f};
        g->enemies[1] = (Enemy){(Vector2){870, 505}, Vector2Zero(), 22, 138.0f, true, ENEMY_RUSHER, 1.3f};
        g->enemies[2] = (Enemy){(Vector2){980, 610}, Vector2Zero(), 20, 105.0f, true, ENEMY_ORBITER, 2.1f};
        g->enemies[3] = (Enemy){(Vector2){570, 430}, Vector2Zero(), 21, 122.0f, true, ENEMY_HUNTER, 0.8f};
        g->enemies[4] = (Enemy){(Vector2){760, 360}, Vector2Zero(), 20, 150.0f, true, ENEMY_RUSHER, 2.8f};
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
