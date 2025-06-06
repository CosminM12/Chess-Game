#include "app_globals.h"

// Screen dimensions
const int screenWidth = 1250;  // Increased from 1200 to accommodate wider sidebar
const int screenHeight = 800;
const int squareSize = 100;
const int boardWidth = 800;  // 8 squares of 100px each
const int sidebar1_width = 200;  // First sidebar width
const int sidebar2_width = 250;  // Second sidebar width (increased from 200 to 250)

// Global variables definitions
GameScreenState currentScreenState = GAME_STATE_PLAYING;
GamePromptActionType currentPromptAction = PROMPT_ACTION_NONE;
char inputFileNameBuffer[256] = "";
SDL_bool textInputActive = SDL_FALSE; 