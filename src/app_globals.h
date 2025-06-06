// src/app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <SDL2/SDL.h>
#include "GameState.h" // Ensure GameState is included here to declare its struct properly

// --- Moved from Events.h ---
#define MAX_MOVES 1024 // Maximum number of moves to store in history
// --- End Moved ---

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

// --- ADDED FOR UNDO/REDO (History States) ---
#define MAX_HISTORY_STATES 200

extern GameState historyStates[MAX_HISTORY_STATES];
extern int historyCount;
extern int currentHistoryIdx;

// Function Prototypes for Undo/Redo operations (these are implemented in main.c)
void recordGameState(GameState* state);
void undoGame(GameState* state);
void redoGame(GameState* state);
// --- END ADDED FOR UNDO/REDO ---

#endif // APP_GLOBALS_H