#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string.h> // <--- ADDED: For string manipulation

#include "RenderWindow.h"
#include "Piece.h"
#include "GameState.h"
#include "Events.h"
#include "util.h"
#include "app_globals.h" // <--- ADDED: Include your new global header here


// --- GLOBAL VARIABLE DEFINITIONS ---
// These variables must be defined here, and declared as extern in app_globals.h
GameScreenState currentScreenState = GAME_STATE_PLAYING;
GamePromptActionType currentPromptAction = PROMPT_ACTION_NONE;
char inputFileNameBuffer[256] = ""; // Renamed the buffer for clarity
SDL_bool textInputActive = SDL_FALSE;

Move moveHistory[MAX_MOVES];
int moveCount = 0;
int moveHistoryScrollOffset = 0;
// --- END GLOBAL VARIABLE DEFINITIONS ---


const char* pieceToChar(unsigned char piece) {
    // This function seems to have incorrect bitmasking compared to Piece.h
    // It should probably be:
    // switch (piece & TYPE_MASK) {
    //     case PAWN:   return "";
    //     case BISHOP: return "B";
    //     ...
    // }
    // For now, retaining the user's version, but it might be a bug source.
    switch (piece & 0b11100000) { // This mask (0xE0) is incorrect for COLOR_MASK or TYPE_MASK
        case 0x10: // white (This value 0x10 is COLOR_MASK for black if 0 for white)
        case 0x20: // black (This value 0x20 is SELECTED_MASK, not a color)
            break;
    }

    // This switch uses 0b00111000 (0x38), which is not TYPE_MASK (0x7)
    // Please verify your Piece.h masks and this function's logic.
    // Assuming the user intended to mask for piece type.
    switch (piece & 0b00111000) {
        case 0x08: return "";  // pawn (0x8 is MODIFIER, not PAWN=1)
        case 0x10: return "B"; // bishop (0x10 is COLOR_MASK, not BISHOP=2)
        case 0x18: return "N"; // knight (0x18 is MODIFIER | COLOR_MASK, not KNIGHT=3)
        case 0x20: return "R"; // rook (0x20 is SELECTED_MASK, not ROOK=4)
        case 0x28: return "Q"; // queen (0x28 is SELECTED_MASK | MODIFIER, not QUEEN=5)
        case 0x30: return "K"; // king (0x30 is SELECTED_MASK | COLOR_MASK, not KING=6)
        default:   return "?";
    }
}


/*----------Variable declaration------------*/
bool gameRunning = true; // This can eventually be removed and use gameState.gameRunning directly

GameState gameState;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// TODO: move the colors to a separate file
SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};

// ----- windows -----
const int timer_height = 100;
const int captured_height = 250;


// ----- time variables -----
Uint64 currentTick, lastTick;
double deltaTime;

int whiteTimeMs = 5 * 60 * 1000;
int blackTimeMs = 5 * 60 * 1000;

char whiteTimerStr[16];
char blackTimerStr[16];


void formatTime(char *buffer, int timeMs) {
    int totalSeconds = timeMs / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    snprintf(buffer, 16, "%02d:%02d", minutes, seconds);
}


/*---------Helper functions-----------*/
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if (!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", IMG_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer failed: %s\n", Mix_GetError());
        return false;
    }

    if (!initFont("../res/fonts/JetBrainsMono/JetBrainsMono-Bold.ttf", 24)) {
        fprintf(stderr, "Font failed to load, check the path and font file.\n");
        return false; // Changed from -1 to false for consistency with bool return
    }

    return true;
}

void printfBoard(unsigned char board[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%d ", (int) board[i][j]);
        }
        putchar('\n');
    }
    putchar('\n');
}


int main() {

    initGameState(&gameState);

    /*==========Program Initialization==========*/
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if (!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    /*==========Initialize Variables (some moved to app_globals.h/main.c global defs)==========*/
    // bool mouseActions[2] = {false, false}; // Now part of GameState struct
    // bool pieceActions[2] = {false, false}; // Now part of GameState struct
    // bool blackTurn = false; // Now part of GameState struct

    SDL_Event event;
    SDL_Texture *pieceTextures[2][7];
    // Vector2f selectedSquare = createVector(-1.0f, -1.0f); // Now part of GameState struct
    // Vector2f kingsPositions[2]; // Now part of GameState struct
    // Vector2f lastDoublePushPawn = createVector(-1.0f, -1.0f); // Now part of GameState struct
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;

    //========== Initialize values ==========//
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(gameState.board, input);
    findKings(gameState.board, gameState.kingsPositions); // Initialize kings positions (now part of gameState)

    bool inMenu = true;

    while (inMenu) {
        SDL_Event menuEvent;
        while (SDL_PollEvent(&menuEvent)) {
            if (menuEvent.type == SDL_QUIT) {
                gameState.gameRunning = false; // Use gameState.gameRunning
                inMenu = false;
            } else if (menuEvent.type == SDL_MOUSEBUTTONDOWN) {
                int x = menuEvent.button.x;
                int y = menuEvent.button.y;

                // Example button position and size (adjust as needed)
                if (x >= 400 && x <= 700 && y >= 300 && y <= 370) {
                    inMenu = false; // Start game
                }
            }
        }

        // Render Menu
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // dark background
        SDL_RenderClear(renderer);

        renderText(renderer, "Chess Game", (SDL_Color){255, 255, 255, 255}, 500, 150);
        SDL_Rect startButton = {450, 300, 200, 70};
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderFillRect(renderer, &startButton);
        renderText(renderer, "Start Game", (SDL_Color){255, 255, 255, 255}, 480, 320);
        SDL_RenderPresent(renderer);
    }

    lastTick = SDL_GetPerformanceCounter();
    currentTick = lastTick;

    while (gameState.gameRunning) { // Use gameState.gameRunning
        //========== Find time variables ==========//
        lastTick = currentTick;

        if (gameState.blackTurn) { // Use gameState.blackTurn
            blackTimeMs -= deltaTime;
            if (blackTimeMs < 0) {
                blackTimeMs = 0;
            }
        } else {
            whiteTimeMs -= deltaTime;
            if (whiteTimeMs < 0) {
                whiteTimeMs = 0;
            }
        }

        //========== Events and movements ==========//
        getEvents(event, &gameState, &moveHistoryScrollOffset);
        SDL_GetMouseState(&mouseX, &mouseY);

        // --- CORRECTED SAVE/LOAD BUTTON LOGIC (TRIGGERING THE PROMPT) ---
        SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 130, 40};
        SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 130, 40};

        if (gameState.mouseActions[0]) { // Check for a left mouse button down event
            // Only process button clicks if not in a prompt state
            if (currentScreenState == GAME_STATE_PLAYING) {
                if (mouseX >= saveButton.x && mouseX <= saveButton.x + saveButton.w &&
                    mouseY >= saveButton.y && mouseY <= saveButton.y + saveButton.h) {
                    printf("Save Game button clicked! Opening prompt...\n");
                    currentScreenState = GAME_STATE_PROMPT_FILENAME;
                    currentPromptAction = PROMPT_ACTION_SAVE;
                    SDL_StartTextInput();
                    inputFileNameBuffer[0] = '\0'; // Clear buffer
                    textInputActive = SDL_TRUE;
                }
                else if (mouseX >= loadButton.x && mouseX <= loadButton.x + loadButton.w &&
                         mouseY >= loadButton.y && mouseY <= loadButton.y + loadButton.h) {
                    printf("Load Game button clicked! Opening prompt...\n");
                    currentScreenState = GAME_STATE_PROMPT_FILENAME;
                    currentPromptAction = PROMPT_ACTION_LOAD;
                    SDL_StartTextInput();
                    inputFileNameBuffer[0] = '\0'; // Clear buffer
                    textInputActive = SDL_TRUE;
                }
            }
        }

        // --- Only handle board input if in GAME_STATE_PLAYING ---
        if (mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize) && currentScreenState == GAME_STATE_PLAYING) {
            handleMouseInput(&gameState, mouseX, mouseY);
        }
        // Reset mouseActions flags here, after all event processing for the frame
        gameState.mouseActions[0] = false;
        gameState.mouseActions[1] = false;


        //========== Rendering Visuals ==========//
        clear(&renderer);

        // --- CONDITIONAL RENDERING BASED ON currentScreenState ---
        if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
            // Dim the background (existing game visuals will be under this)
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150); // Semi-transparent black
            SDL_Rect dimmer = {0, 0, screenWidth, screenHeight};
            SDL_RenderFillRect(renderer, &dimmer);

            // Draw a box for the prompt
            SDL_Rect promptBox = {screenWidth / 2 - 200, screenHeight / 2 - 75, 400, 150}; // Larger box
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark grey background for prompt
            SDL_RenderFillRect(renderer, &promptBox);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White border
            SDL_RenderDrawRect(renderer, &promptBox);

            char promptText[64];
            if (currentPromptAction == PROMPT_ACTION_SAVE) {
                snprintf(promptText, sizeof(promptText), "Enter filename to SAVE:");
            } else if (currentPromptAction == PROMPT_ACTION_LOAD) {
                snprintf(promptText, sizeof(promptText), "Enter filename to LOAD:");
            } else {
                snprintf(promptText, sizeof(promptText), "Enter filename:"); // Fallback
            }
            renderText(renderer, promptText, (SDL_Color){255, 255, 255, 255}, promptBox.x + 20, promptBox.y + 10);

            renderText(renderer, inputFileNameBuffer, (SDL_Color){255, 255, 255, 255}, promptBox.x + 20, promptBox.y + 50);

            renderText(renderer, "Press ENTER to confirm", (SDL_Color){150, 150, 150, 255}, promptBox.x + 20, promptBox.y + 90);
            renderText(renderer, "Press ESC to cancel", (SDL_Color){150, 150, 150, 255}, promptBox.x + 20, promptBox.y + 110);

        } else {
            // --- EXISTING GAME RENDERING LOGIC (only when not in prompt state) ---
            drawBoard(renderer, squareSize, 0, screenWidth, color_light, color_dark, color_clicked, color_possible, gameState.board);

            SDL_Rect sidebar_background = {boardWidth, 0, sidebar1_width + sidebar2_width, screenHeight};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &sidebar_background);

            SDL_Rect timerBox = {boardWidth, 0, sidebar1_width, timer_height};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer, &timerBox);

            formatTime(whiteTimerStr, whiteTimeMs);
            formatTime(blackTimerStr, blackTimeMs);

            renderText(renderer, "White:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 10);
            renderText(renderer, whiteTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 10);

            renderText(renderer, "Black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 40);
            renderText(renderer, blackTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 40);

            SDL_Rect capturedBlackBox = {boardWidth, timer_height, sidebar1_width, captured_height * 2};
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &capturedBlackBox);

            SDL_Rect capturedWhiteBox = {boardWidth + sidebar1_width, 250, sidebar2_width, 150};
            SDL_SetRenderDrawColor(renderer, 200, 160, 255, 255);
            SDL_RenderFillRect(renderer, &capturedWhiteBox);

            SDL_Rect buttonBox = {boardWidth, screenHeight - 150, sidebar1_width, 150};
            SDL_SetRenderDrawColor(renderer, 255, 100, 180, 255);
            SDL_RenderFillRect(renderer, &buttonBox);

            SDL_Rect moveHistoryBox = {boardWidth + sidebar1_width, 0, sidebar2_width, screenHeight};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &moveHistoryBox);


            renderText(renderer, "Captured by white:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + 10);
            renderText(renderer, "Captured by black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + captured_height + 10);

            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    renderPiece(pieceTextureCoordinates, 0, squareSize, row, col,
                                getPieceTexture(pieceTextures, gameState.board[row][col]), &renderer);
                }
            }

            renderCapturedPieces(renderer, pieceTextures, &gameState);

            renderText(renderer, "MOVE HISTORY:", (SDL_Color) {255, 255, 255, 255}, boardWidth + sidebar1_width + 10, 10);

            const int moveHeight = 25;
            int visibleStart = moveHistoryScrollOffset / moveHeight;
            int visibleEnd = visibleStart + (screenHeight / moveHeight) + 1;

            for (int i = visibleStart; i < visibleEnd && i < moveCount; ++i) {
                char buffer[64];
                sprintf(buffer, "%d. %s", (i + 1), moveHistory[i].notation);

                int y = 40 + (i * moveHeight) - moveHistoryScrollOffset;

                if (y >= 40 && y < screenHeight - 10) {
                    renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255},
                               boardWidth + sidebar1_width + 15, y);
                }
            }

            // Render Save and Load buttons (only when not in prompt state)
            SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Green for save
            SDL_RenderFillRect(renderer, &saveButton);
            renderText(renderer, "Save Game", (SDL_Color){255, 255, 255, 255}, saveButton.x + 10, saveButton.y + 10);

            SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255); // Blue for load
            SDL_RenderFillRect(renderer, &loadButton);
            renderText(renderer, "Load Game", (SDL_Color){255, 255, 255, 255}, loadButton.x + 10, loadButton.y + 10);
        }

        display(&renderer);

        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double) ((currentTick - lastTick) * 1000 / (double) SDL_GetPerformanceFrequency());
    }

    char *exportString = NULL; // Unused variable
    cleanUp(window);
    printf("Program ended\n");
    return 0;
}