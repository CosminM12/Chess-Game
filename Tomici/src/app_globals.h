// src/app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <SDL2/SDL.h> // For SDL_bool

// --- Constants ---
#define MAX_MOVES 1024       // Maximum number of moves to store in history
#define MAX_HISTORY_STATES 200 // Max game states for undo/redo

// Enum for the overall screen state of the application
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PROMPT_FILENAME
} GameScreenState;

// Enum to specify the action associated with the filename prompt
typedef enum {
    PROMPT_ACTION_NONE,
    PROMPT_ACTION_SAVE,
    PROMPT_ACTION_LOAD
} GamePromptActionType;

// Declare global variables for UI prompts
extern GameScreenState currentScreenState;
extern GamePromptActionType currentPromptAction;
extern char inputFileNameBuffer[256];
extern SDL_bool textInputActive;

// Move struct definition (now centralized here)
typedef struct {
    char notation[10];
} Move;

#endif // APP_GLOBALS_H