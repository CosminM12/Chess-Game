// src/app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <SDL2/SDL.h> // Include SDL2 for SDL_bool if it's used with global variables

// Define the enum for game screen states
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PROMPT_FILENAME
} GameScreenState;

typedef enum {
    PROMPT_ACTION_NONE, // No prompt active
    PROMPT_ACTION_SAVE, // Prompt for saving
    PROMPT_ACTION_LOAD  // Prompt for loading
} GamePromptActionType;

// Declare global variables using 'extern'
// This tells the compiler that these variables are defined elsewhere
extern GameScreenState currentScreenState;
extern GamePromptActionType currentPromptAction;
extern char inputFileNameBuffer[256]; // Renamed for clarity, handles both save and load input
extern SDL_bool textInputActive;

#endif // APP_GLOBALS_H