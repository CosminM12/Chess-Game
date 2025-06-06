// src/app_globals.h
#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#include <SDL2/SDL.h> // Required for SDL_bool

// Enum for the overall screen state of the application
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PROMPT_FILENAME // Unified state for both save and load prompts
} GameScreenState;

// Enum to specify the action associated with the filename prompt
typedef enum {
    PROMPT_ACTION_NONE, // No prompt active
    PROMPT_ACTION_SAVE, // Prompt for saving
    PROMPT_ACTION_LOAD  // Prompt for loading
} GamePromptActionType;


// Declare global variables using 'extern'
extern GameScreenState currentScreenState;
extern GamePromptActionType currentPromptAction;
extern char inputFileNameBuffer[256]; // Renamed for clarity, handles both save and load input
extern SDL_bool textInputActive;

#endif // APP_GLOBALS_H