#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <SDL2/SDL.h>

// Screen dimensions
extern const int screenWidth;
extern const int screenHeight;
extern const int squareSize;
extern const int boardWidth;
extern const int sidebar1_width;
extern const int sidebar2_width;

// Game screen states
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PROMPT_FILENAME
} GameScreenState;

// Game prompt action types
typedef enum {
    PROMPT_ACTION_NONE,
    PROMPT_ACTION_SAVE,
    PROMPT_ACTION_LOAD
} GamePromptActionType;

// Move structure for move history
typedef struct {
    int startRow;
    int startCol;
    int endRow;
    int endCol;
    char notation[10];
} Move;

// Maximum number of moves and history states
#define MAX_MOVES 1000
#define MAX_HISTORY_STATES 100

// Global variables declarations
extern GameScreenState currentScreenState;
extern GamePromptActionType currentPromptAction;
extern char inputFileNameBuffer[256];
extern SDL_bool textInputActive;

#endif // APP_GLOBALS_H 