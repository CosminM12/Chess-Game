#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "GameState.h"
#include "Events.h"
#include "util.h"
#include "app_globals.h"


// --- GLOBAL VARIABLE DEFINITIONS (from app_globals.h) ---
GameScreenState currentScreenState = GAME_STATE_PLAYING;
GamePromptActionType currentPromptAction = PROMPT_ACTION_NONE;
char inputFileNameBuffer[256] = "";
SDL_bool textInputActive = SDL_FALSE;

// GameState history management variables - DEFINED HERE (declared in GameState.h)
GameState historyStates[MAX_HISTORY_STATES];
int historyCount = 0;
int currentHistoryIdx = -1;
// --- END GLOBAL VARIABLE DEFINITIONS ---


// REMOVED: Move moveHistory[MAX_MOVES]; and int moveCount = 0; as they are now part of GameState struct.
int moveHistoryScrollOffset = 0;


const char* pieceToChar(unsigned char piece) {
    // This function seems to have incorrect bitmasking compared to Piece.h
    // It should probably be:
    // switch (piece & TYPE_MASK) {
    //     case PAWN:   return "";
    //     case BISHOP: return "B";
    //     ...
    // }
    // For now, retaining the user's version, but it might be a bug source.
    switch (piece & 0b11100000) {
        case 0x10: // white
        case 0x20: // black
            break;
    }

    switch (piece & 0b00111000) {
        case 0x08: return "";  // pawn
        case 0x10: return "B"; // bishop
        case 0x18: return "N"; // knight
        case 0x20: return "R"; // rook
        case 0x28: return "Q"; // queen
        case 0x30: return "K"; // king
        default:   return "?";
    }
}


/*----------Variable declaration------------*/
bool gameRunning = true;

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
        return false;
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


/* --- NEW UNDO/REDO FUNCTIONS --- */
void recordGameState(GameState* state) {
    // If we are not at the latest state (i.e., some undos have happened),
    // a new move truncates the redo history.
    if (currentHistoryIdx < historyCount - 1) {
        historyCount = currentHistoryIdx + 1;
    }

    // Check for array bounds
    if (historyCount >= MAX_HISTORY_STATES) {
        // Option 1: Shift history to make space (more complex, not implemented here)
        // Option 2: Just don't record if history is full (simple)
        printf("History buffer full. Cannot record more states.\n");
        return;
    }

    currentHistoryIdx++;
    historyStates[currentHistoryIdx] = *state; // Copy the entire current game state
    historyCount = currentHistoryIdx + 1; // Update total count of states
    printf("Recorded state %d. Total states: %d\n", currentHistoryIdx, historyCount);

    // Reset scroll offset to show current moves after recording a new state
    moveHistoryScrollOffset = 0;
}

void undoGame(GameState* state) {
    if (currentHistoryIdx > 0) {
        currentHistoryIdx--;
        *state = historyStates[currentHistoryIdx]; // Revert to previous state
        printf("Undone to state %d.\n", currentHistoryIdx);
        // Reset scroll offset to show the newly reverted state's moves
        moveHistoryScrollOffset = 0;
    } else {
        printf("Cannot undo further.\n");
    }
}

void redoGame(GameState* state) {
    if (currentHistoryIdx < historyCount - 1) {
        currentHistoryIdx++;
        *state = historyStates[currentHistoryIdx]; // Re-apply next state
        printf("Redone to state %d.\n", currentHistoryIdx);
        // Reset scroll offset to show the newly redone state's moves
        moveHistoryScrollOffset = 0;
    } else {
        printf("Cannot redo further.\n");
    }
}
/* --- END NEW UNDO/REDO FUNCTIONS --- */


int main() {

    initGameState(&gameState);

    /*==========Program Initialization==========*/
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if (!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    // --- Record initial game state ---
    recordGameState(&gameState); // Capture the initial board state
    // --- End record initial game state ---

    SDL_Event event;
    SDL_Texture *pieceTextures[2][7];
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;

    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(gameState.board, input);
    findKings(gameState.board, gameState.kingsPositions);

    bool inMenu = true;
    while (inMenu) {
        SDL_Event menuEvent;
        while (SDL_PollEvent(&menuEvent)) {
            if (menuEvent.type == SDL_QUIT) {
                gameState.gameRunning = false;
                inMenu = false;
            } else if (menuEvent.type == SDL_MOUSEBUTTONDOWN) {
                int x = menuEvent.button.x;
                int y = menuEvent.button.y;

                if (x >= 400 && x <= 700 && y >= 300 && y <= 370) {
                    inMenu = false; // Start game
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
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

    while (gameState.gameRunning) {
        lastTick = currentTick;

        if (gameState.blackTurn) {
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

        getEvents(event, &gameState, &moveHistoryScrollOffset);
        SDL_GetMouseState(&mouseX, &mouseY);

        // Define positions for your buttons
        SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 130, 40};
        SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 130, 40};
        SDL_Rect undoButton = {boardWidth + 10, screenHeight - 90, 130, 40}; // Example position
        SDL_Rect redoButton = {boardWidth + 160, screenHeight - 90, 130, 40}; // Example position


        if (gameState.mouseActions[0]) {
            if (currentScreenState == GAME_STATE_PLAYING) {
                if (mouseX >= saveButton.x && mouseX <= saveButton.x + saveButton.w &&
                    mouseY >= saveButton.y && mouseY <= saveButton.y + saveButton.h) {
                    printf("Save Game button clicked! Opening prompt...\n");
                    currentScreenState = GAME_STATE_PROMPT_FILENAME;
                    currentPromptAction = PROMPT_ACTION_SAVE;
                    SDL_StartTextInput();
                    inputFileNameBuffer[0] = '\0';
                    textInputActive = SDL_TRUE;
                }
                else if (mouseX >= loadButton.x && mouseX <= loadButton.x + loadButton.w &&
                         mouseY >= loadButton.y && mouseY <= loadButton.y + loadButton.h) {
                    printf("Load Game button clicked! Opening prompt...\n");
                    currentScreenState = GAME_STATE_PROMPT_FILENAME;
                    currentPromptAction = PROMPT_ACTION_LOAD;
                    SDL_StartTextInput();
                    inputFileNameBuffer[0] = '\0';
                    textInputActive = SDL_TRUE;
                }
                    // --- UNDO/REDO BUTTON CLICK DETECTION ---
                else if (mouseX >= undoButton.x && mouseX <= undoButton.x + undoButton.w &&
                         mouseY >= undoButton.y && mouseY <= undoButton.y + undoButton.h) {
                    printf("Undo button clicked!\n");
                    undoGame(&gameState);
                }
                else if (mouseX >= redoButton.x && mouseX <= redoButton.x + redoButton.w &&
                         mouseY >= redoButton.y && mouseY <= redoButton.y + redoButton.h) {
                    printf("Redo button clicked!\n");
                    redoGame(&gameState);
                }
                // --- END UNDO/REDO BUTTON CLICK DETECTION ---
            }
        }

        if (mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize) && currentScreenState == GAME_STATE_PLAYING) {
            handleMouseInput(&gameState, mouseX, mouseY);
        }
        gameState.mouseActions[0] = false;
        gameState.mouseActions[1] = false;


        clear(&renderer);

        if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_Rect dimmer = {0, 0, screenWidth, screenHeight};
            SDL_RenderFillRect(renderer, &dimmer);

            SDL_Rect promptBox = {screenWidth / 2 - 200, screenHeight / 2 - 75, 400, 150};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &promptBox);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &promptBox);

            char promptText[64];
            if (currentPromptAction == PROMPT_ACTION_SAVE) {
                snprintf(promptText, sizeof(promptText), "Enter filename to SAVE:");
            } else if (currentPromptAction == PROMPT_ACTION_LOAD) {
                snprintf(promptText, sizeof(promptText), "Enter filename to LOAD:");
            } else {
                snprintf(promptText, sizeof(promptText), "Enter filename:");
            }
            renderText(renderer, promptText, (SDL_Color){255, 255, 255, 255}, promptBox.x + 20, promptBox.y + 10);

            renderText(renderer, inputFileNameBuffer, (SDL_Color){255, 255, 255, 255}, promptBox.x + 20, promptBox.y + 50);

            renderText(renderer, "Press ENTER to confirm", (SDL_Color){150, 150, 150, 255}, promptBox.x + 20, promptBox.y + 90);
            renderText(renderer, "Press ESC to cancel", (SDL_Color){150, 150, 150, 255}, promptBox.x + 20, promptBox.y + 110);

        } else {
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

            for (int i = visibleStart; i < visibleEnd && i < gameState.moveCount; ++i) { // Use gameState.moveCount
                char buffer[64];
                sprintf(buffer, "%d. %s", (i + 1), gameState.moveHistory[i].notation); // Use gameState.moveHistory

                int y = 40 + (i * moveHeight) - moveHistoryScrollOffset;

                if (y >= 40 && y < screenHeight - 10) {
                    renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255},
                               boardWidth + sidebar1_width + 15, y);
                }
            }

            // Render Save and Load buttons
            SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Green for save
            SDL_RenderFillRect(renderer, &saveButton);
            renderText(renderer, "Save Game", (SDL_Color){255, 255, 255, 255}, saveButton.x + 10, saveButton.y + 10);

            SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255); // Blue for load
            SDL_RenderFillRect(renderer, &loadButton);
            renderText(renderer, "Load Game", (SDL_Color){255, 255, 255, 255}, loadButton.x + 10, loadButton.y + 10);

            // --- RENDER UNDO/REDO BUTTONS ---
            SDL_SetRenderDrawColor(renderer, 150, 50, 0, 255); // Orange for undo
            SDL_RenderFillRect(renderer, &undoButton);
            renderText(renderer, "Undo", (SDL_Color){255, 255, 255, 255}, undoButton.x + 40, undoButton.y + 10);

            SDL_SetRenderDrawColor(renderer, 0, 150, 150, 255); // Cyan for redo
            SDL_RenderFillRect(renderer, &redoButton);
            renderText(renderer, "Redo", (SDL_Color){255, 255, 255, 255}, redoButton.x + 40, redoButton.y + 10);
            // --- END RENDER UNDO/REDO BUTTONS ---
        }

        display(&renderer);

        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double) ((currentTick - lastTick) * 1000 / (double) SDL_GetPerformanceFrequency());
    }

    cleanUp(window);
    printf("Program ended\n");
    return 0;
}