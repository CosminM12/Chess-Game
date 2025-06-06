#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "Events.h"
#include "util.h" // Now includes screen dimension constants
#include "engine.h"
#include "GameState.h"
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

int moveHistoryScrollOffset = 0;

/*----------Variable declaration------------*/
bool gameRunning = true; // This should be state->gameRunning after initGameState

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};
SDL_Color color_risky = {255, 128, 0, 255}; // Orange color for risky moves

// REMOVED: const int screenWidth, screenHeight, squareSize, boardWidth, sidebar1_width, sidebar2_width, etc.
// These are now external constants from util.h/util.c

Uint64 currentTick, lastTick;
double deltaTime;

// Engine variables
bool engineEnabled = false;
bool showAnalysis = false;
bool computerPlaysBlack = false;
bool showEvaluationBar = true; // New variable to control evaluation bar visibility
bool showMenu = true; // Variable to control menu visibility
int gameMode = 0; // 0 = not selected, 1 = PvP, 2 = PvE
Uint32 moveTimestamp = 0; // Timestamp of the last player move

// REMOVED: int whiteTimeMs, blackTimeMs; These are now part of GameState struct.

char whiteTimerStr[16];
char blackTimerStr[16];

// Updated layout constants (These are now external from util.c)
const int timer_height = 80;
const int captured_area_startY = 100; // Starting Y for captured pieces labels
const int captured_area_height = 250;
const int buttons_area_height = 100;

// Function to format time for display
void formatTime(char *buffer, int timeMs) {
    int totalSeconds = timeMs / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    snprintf(buffer, 16, "%02d:%02d", minutes, seconds);
}

/*---------Helper functions-----------*/
bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(TTF_Init() == -1) {
        printf("TTF_Init has failed. Error: %s\n", TTF_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer failed: %s\n", Mix_GetError());
        return false;
    }

    // Corrected font paths to ../res/fonts/
    if (!initFont("../res/fonts/DejaVuSans-Bold.ttf", 24)) {
        if (!initFont("../res/fonts/FreeSans.ttf", 24)) {
            if (!initFont("../res/fonts/LiberationSans-Regular.ttf", 24)) {
                fprintf(stderr, "Font failed to load, check the path and font file.\n");
                return false;
            }
        }
    }

    return true;
}

void printfBoard(unsigned char board[8][8]) {
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            printf("%d ", (int)board[i][j]);
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

// Function to draw the evaluation bar
void drawEvaluationBar(SDL_Renderer* renderer, int score) {
    float whitePercentage, blackPercentage;
    getScoreBar(score, &whitePercentage, &blackPercentage);

    // Draw the bar on the right side of the screen (within the second sidebar)
    int barWidth = 20;
    int barHeight = 400;
    // Position relative to the second sidebar
    int barX = boardWidth + sidebar1_width + (sidebar2_width - barWidth) / 2 + 125;
    int barY = (screenHeight - barHeight) / 2;

    // Black portion (top)
    SDL_Rect blackRect = {barX, barY, barWidth, (int)(barHeight * blackPercentage)};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &blackRect);

    // White portion (bottom)
    SDL_Rect whiteRect = {barX, barY + (int)(barHeight * blackPercentage),
                          barWidth, (int)(barHeight * whitePercentage)};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &whiteRect);

    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect borderRect = {barX, barY, barWidth, barHeight};
    SDL_RenderDrawRect(renderer, &borderRect);

    // Draw evaluation value
    TTF_Font* font = TTF_OpenFont("../res/fonts/DejaVuSans.ttf", 16); // Corrected font path
    if (font) {
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "%.2f", score / 100.0f);

        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {
                    barX - textSurface->w - 5, // Position to the left of the bar
                    barY + barHeight / 2 - textSurface->h / 2,
                    textSurface->w,
                    textSurface->h
            };
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
            SDL_FreeSurface(textSurface);
        }
        TTF_CloseFont(font);
    }
}

// Function to draw game status text (check, checkmate, stalemate)
void drawGameStatus(SDL_Renderer* renderer, bool isInCheck, bool isGameOver, bool isStalemate, bool blackTurn) {
    TTF_Font* font = TTF_OpenFont("../res/fonts/DejaVuSans-Bold.ttf", 24); // Corrected font path
    if (!font) return;

    SDL_Color textColor = {255, 0, 0, 255}; // Red for check/checkmate
    if (isStalemate) textColor = (SDL_Color){255, 255, 0, 255}; // Yellow for stalemate

    char statusText[32] = "";
    if (isGameOver) {
        if (isStalemate) {
            strcpy(statusText, "Stalemate! Draw");
        } else {
            sprintf(statusText, "Checkmate! %s wins", blackTurn ? "White" : "Black");
        }
    } else if (isInCheck) {
        sprintf(statusText, "%s is in Check", blackTurn ? "Black" : "White");
    }

    if (statusText[0] != '\0') {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, statusText, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                        (screenWidth - textSurface->w) / 2,
                        10,  // Top of the screen
                        textSurface->w,
                        textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }

    TTF_CloseFont(font);
}

// Function to analyze the current position for display
void displayPositionAnalysis(unsigned char board[8][8], bool blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    // Evaluate the current position
    unsigned char currentColor = blackTurn ? 1 : 0;
    int score = evaluatePosition(board, currentColor);

    printf("Current evaluation: %.2f\n", score / 100.0f);

    // Generate and evaluate legal moves
    MoveList legalMoves;
    generateMoves(board, currentColor, &legalMoves, lastDoublePawn);

    if (legalMoves.count == 0) {
        if (isCheck(board, kingsPositions[currentColor])) {
            printf("Checkmate! %s wins!\n", blackTurn ? "White" : "Black");
        } else {
            printf("Stalemate! Game is drawn.\n");
        }
        return;
    }

    // Evaluate move safety
    evaluateMovesSafety(board, currentColor, &legalMoves);

    // Find any risky moves
    bool hasRiskyMoves = false;
    for (int i = 0; i < legalMoves.count; i++) {
        if (legalMoves.moves[i].safetyScore < 0) {
            if (!hasRiskyMoves) {
                printf("Risky moves detected:\n");
                hasRiskyMoves = true;
            }

            // Convert board coordinates to algebraic notation
            char fromFile = 'a' + legalMoves.moves[i].from.y;
            int fromRank = 8 - legalMoves.moves[i].from.x;
            char toFile = 'a' + legalMoves.moves[i].to.y;
            int toRank = 8 - legalMoves.moves[i].to.x;

            unsigned char pieceType = board[legalMoves.moves[i].from.x][legalMoves.moves[i].from.y] & TYPE_MASK;
            const char* pieceName = "Unknown";
            switch (pieceType) {
                case PAWN: pieceName = "Pawn"; break;
                case KNIGHT: pieceName = "Knight"; break;
                case BISHOP: pieceName = "Bishop"; break;
                case ROOK: pieceName = "Rook"; break;
                case QUEEN: pieceName = "Queen"; break;
                case KING: pieceName = "King"; break;
            }

            printf("  %s %c%d -> %c%d: Risk score %d\n",
                   pieceName, fromFile, fromRank, toFile, toRank,
                   legalMoves.moves[i].safetyScore);
        }
    }
}

// Function to draw the menu overlay
bool drawMenu(SDL_Renderer* renderer, int screenWidth_local, int screenHeight_local, int* selectedMode) {
    // Semi-transparent background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Dark semi-transparent background
    SDL_Rect overlay = {0, 0, screenWidth_local, screenHeight_local};
    SDL_RenderFillRect(renderer, &overlay);

    // Load font
    TTF_Font* font = TTF_OpenFont("../res/fonts/DejaVuSans-Bold.ttf", 32); // Corrected font path
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }

    // Title
    SDL_Color titleColor = {255, 215, 0, 255}; // Gold color
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Chess Game", titleColor);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {
            (screenWidth_local - titleSurface->w) / 2,
            screenHeight_local / 4 - titleSurface->h / 2,
            titleSurface->w,
            titleSurface->h
    };
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);

    // Button dimensions
    int buttonWidth = 200;
    int buttonHeight = 60;
    int buttonSpacing = 40;

    // PvP Button
    SDL_Color buttonColor = {255, 255, 255, 255}; // White text
    SDL_Surface* pvpSurface = TTF_RenderText_Solid(font, "Player vs Player", buttonColor);
    SDL_Texture* pvpTexture = SDL_CreateTextureFromSurface(renderer, pvpSurface);

    SDL_Rect pvpButtonRect = {
            (screenWidth_local - buttonWidth) / 2,
            screenHeight_local / 2 - buttonHeight - buttonSpacing / 2,
            buttonWidth,
            buttonHeight
    };

    SDL_SetRenderDrawColor(renderer, 50, 100, 150, 255); // Button background color
    SDL_RenderFillRect(renderer, &pvpButtonRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Button border color
    SDL_RenderDrawRect(renderer, &pvpButtonRect);

    SDL_Rect pvpTextRect = {
            (screenWidth_local - pvpSurface->w) / 2,
            pvpButtonRect.y + (buttonHeight - pvpSurface->h) / 2,
            pvpSurface->w,
            pvpSurface->h
    };
    SDL_RenderCopy(renderer, pvpTexture, NULL, &pvpTextRect);

    // PvE Button
    SDL_Surface* pveSurface = TTF_RenderText_Solid(font, "Player vs Computer", buttonColor);
    SDL_Texture* pveTexture = SDL_CreateTextureFromSurface(renderer, pveSurface);

    SDL_Rect pveButtonRect = {
            (screenWidth_local - buttonWidth) / 2,
            screenHeight_local / 2 + buttonSpacing / 2,
            buttonWidth,
            buttonHeight
    };

    SDL_SetRenderDrawColor(renderer, 50, 100, 150, 255); // Button background color
    SDL_RenderFillRect(renderer, &pveButtonRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Button border color
    SDL_RenderDrawRect(renderer, &pveButtonRect);

    SDL_Rect pveTextRect = {
            (screenWidth_local - pveSurface->w) / 2,
            pveButtonRect.y + (buttonHeight - pveSurface->h) / 2,
            pvpSurface->w,
            pvpSurface->h
    };
    SDL_RenderCopy(renderer, pveTexture, NULL, &pveTextRect);

    // Clean up
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);
    SDL_FreeSurface(pvpSurface);
    SDL_DestroyTexture(pvpTexture);
    SDL_FreeSurface(pveSurface);
    SDL_DestroyTexture(pveTexture);
    TTF_CloseFont(font);

    // Check for button clicks
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (mouseX >= pvpButtonRect.x && mouseX <= pvpButtonRect.x + pvpButtonRect.w &&
            mouseY >= pvpButtonRect.y && mouseY <= pvpButtonRect.y + pvpButtonRect.h) {
            // PvP button clicked
            *selectedMode = 1;
            printf("Player vs Player mode selected\n");
            return true;
        }

        if (mouseX >= pveButtonRect.x && mouseX <= pveButtonRect.x + pveButtonRect.w &&
            mouseY >= pveButtonRect.y && mouseY <= pveButtonRect.y + pveButtonRect.h) {
            // PvE button clicked
            *selectedMode = 2;
            printf("Player vs Computer mode selected\n");
            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[]) {
    // Initialize game state
    GameState gameState;
    initGameState(&gameState);

    // Initialize SDL
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if (!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    // Record initial game state
    recordGameState(&gameState);

    // Load piece textures
    SDL_Texture* pieceTextures[2][7];
    loadPieceTextures(pieceTextures, &renderer);
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};

    // Initialize game variables
    SDL_Event event;
    int mouseX, mouseY;
    bool isInCheck = false;
    bool isGameOver = false;
    bool isStalemate = false;

    // Initialize engine
    initializeEngine();

    // Main menu loop
    bool inMenu = true;
    while (inMenu && gameState.gameRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState.gameRunning = false;
                inMenu = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x = event.button.x;
                int y = event.button.y;

                // Check if PvP button clicked
                if (x >= (screenWidth - 200) / 2 && x <= (screenWidth + 200) / 2 &&
                    y >= screenHeight / 2 - 60 - 20 && y <= screenHeight / 2 - 20) {
                    inMenu = false;
                    gameMode = 1; // PvP mode
                }
                    // Check if PvE button clicked
                else if (x >= (screenWidth - 200) / 2 && x <= (screenWidth + 200) / 2 &&
                         y >= screenHeight / 2 + 20 && y <= screenHeight / 2 + 60 + 20) {
                    inMenu = false;
                    gameMode = 2; // PvE mode
                    computerPlaysBlack = true;
                }
            }
        }

        // Render menu
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        // Pass global screenWidth, screenHeight
        drawMenu(renderer, screenWidth, screenHeight, &gameMode);
        SDL_RenderPresent(renderer);
    }

    // Initialize game time tracking
    lastTick = SDL_GetPerformanceCounter();
    currentTick = lastTick;

    // Main game loop
    while (gameState.gameRunning) {
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick) * 1000 / (double)SDL_GetPerformanceFrequency());

        // Update timers
        if (gameState.blackTurn) {
            gameState.blackTimeMs -= deltaTime;
            if (gameState.blackTimeMs < 0) gameState.blackTimeMs = 0;
        } else {
            gameState.whiteTimeMs -= deltaTime;
            if (gameState.whiteTimeMs < 0) gameState.whiteTimeMs = 0;
        }

        // Process events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                gameState.gameRunning = false;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        // Toggle menu or cancel filename prompt
                        if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                            currentScreenState = GAME_STATE_PLAYING;
                            SDL_StopTextInput();
                            textInputActive = SDL_FALSE;
                        } else if (currentScreenState == GAME_STATE_PLAYING) {
                            inMenu = !inMenu;
                        }
                        break;
                    case SDLK_e:
                        // Toggle evaluation bar
                        showEvaluationBar = !showEvaluationBar;
                        break;
                    case SDLK_a:
                        // Toggle analysis
                        showAnalysis = !showAnalysis;
                        break;
                    case SDLK_RETURN:
                        // Handle filename prompt confirmation
                        if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                            if (currentPromptAction == PROMPT_ACTION_SAVE) {
                                saveGameToFile(&gameState, inputFileNameBuffer);
                            } else if (currentPromptAction == PROMPT_ACTION_LOAD) {
                                loadGameFromFile(&gameState, inputFileNameBuffer);
                                recordGameState(&gameState); // Record the loaded state
                            }
                            currentScreenState = GAME_STATE_PLAYING;
                            SDL_StopTextInput();
                            textInputActive = SDL_FALSE;
                        }
                        break;
                    case SDLK_BACKSPACE:
                        // Handle backspace in text input
                        if (currentScreenState == GAME_STATE_PROMPT_FILENAME && strlen(inputFileNameBuffer) > 0) {
                            inputFileNameBuffer[strlen(inputFileNameBuffer) - 1] = '\0';
                        }
                        break;
                }
            } else if (event.type == SDL_TEXTINPUT) {
                // Handle text input for filename
                if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                    if (strlen(inputFileNameBuffer) < sizeof(inputFileNameBuffer) - 1) {
                        strcat(inputFileNameBuffer, event.text.text);
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    gameState.mouseActions[0] = true;

                    // Get mouse position
                    SDL_GetMouseState(&mouseX, &mouseY);

                    // Check for button clicks in sidebar
                    if (currentScreenState == GAME_STATE_PLAYING) {
                        // Define positions for buttons in the first sidebar
                        SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 140, 40};
                        SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 140, 40};
                        SDL_Rect undoButton = {boardWidth + 10, screenHeight - 90, 140, 40}; // Example position
                        SDL_Rect redoButton = {boardWidth + 160, screenHeight - 90, 140, 40}; // Example position

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
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    gameState.mouseActions[1] = true;
                }
            }
        }

        // Handle mouse input for chess moves
        SDL_GetMouseState(&mouseX, &mouseY);
        // Pass &gameState directly to handleMouseInput
        if (mouseX < boardWidth && mouseY < boardWidth && currentScreenState == GAME_STATE_PLAYING && !inMenu) {
            handleMouseInput(&gameState, mouseX, mouseY, squareSize);

            // Record game state after a move is made (if mouse button was released AND a move happened)
            // The check for gameState.mouseActions[1] needs to be re-evaluated as makeMove now sets blackTurn
            // Let's assume recordGameState should always be called after handleMouseInput returns
            // if a move was successfully made (indicated by a turn change)
            // Or simpler: recordGameState is called within makeMove if you refactor it that way.
            // For now, keep it here assuming handleMouseInput properly causes a turn change for real moves.
            if (gameState.mouseActions[1]) {
                recordGameState(&gameState);
            }

            // Set timestamp for computer's move in PvE mode
            // This logic needs to consider if a player move just finished
            // and it's now the computer's turn.
            // The turn change is now handled inside makeMove.
            if (gameMode == 2 && gameState.blackTurn && computerPlaysBlack && !gameState.mouseActions[0]) {
                moveTimestamp = SDL_GetTicks();
            }
        }

        // Reset mouse actions
        gameState.mouseActions[0] = false;
        gameState.mouseActions[1] = false;

        // Make computer move in PvE mode after delay
        if (gameMode == 2 && gameState.blackTurn && computerPlaysBlack && SDL_GetTicks() - moveTimestamp > 500) {
            unsigned char color = gameState.blackTurn ? 1 : 0;
            EngineMove bestMove = findBestMove(gameState.board, color, &gameState.lastDoublePushPawn, gameState.kingsPositions);

            if (bestMove.from.x != -1) {
                engineMakeMove(gameState.board, bestMove, &gameState.lastDoublePushPawn, gameState.kingsPositions, 1);
                gameState.blackTurn = !gameState.blackTurn; // Computer made its move, change turn
                recordGameState(&gameState); // Record computer's move
            }
        }

        // Check game status
        isInCheck = isCheck(gameState.board, gameState.kingsPositions[gameState.blackTurn ? 1 : 0]);

        MoveList moveList;
        generateMoves(gameState.board, gameState.blackTurn ? 1 : 0, &moveList, &gameState.lastDoublePushPawn);
        isGameOver = (moveList.count == 0 && isInCheck);
        isStalemate = (moveList.count == 0 && !isInCheck);

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (inMenu) {
            // Render menu overlay
            drawMenu(renderer, screenWidth, screenHeight, &gameMode);
        } else if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
            // Render the main game in the background
            drawBoard(renderer, squareSize, 0, color_light, color_dark, color_clicked, color_possible, color_risky, gameState.board);

            // Render pieces
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    if (gameState.board[row][col] & TYPE_MASK) {
                        renderPiece(pieceTextureCoordinates, 0, squareSize, row, col,
                                    getPieceTexture(pieceTextures, gameState.board[row][col]), &renderer);
                    }
                }
            }

            // Render the prompt overlay
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
            // Render the main game
            // 1. Draw the board
            drawBoard(renderer, squareSize, 0, color_light, color_dark, color_clicked, color_possible, color_risky, gameState.board);

            // 2. Render first sidebar background
            SDL_Rect sidebar1_background = {boardWidth, 0, sidebar1_width, screenHeight};
            SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
            SDL_RenderFillRect(renderer, &sidebar1_background);

            // 3. Render second sidebar background
            SDL_Rect sidebar2_background = {boardWidth + sidebar1_width, 0, sidebar2_width, screenHeight};
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255); // Slightly different color for distinction
            SDL_RenderFillRect(renderer, &sidebar2_background);

            // Render elements within the first sidebar (timers, captured pieces, buttons)
            // Format and render timers
            renderText(renderer, "White:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 10);
            formatTime(whiteTimerStr, gameState.whiteTimeMs);
            renderText(renderer, whiteTimerStr, (SDL_Color){255, 255, 255, 255}, boardWidth + 100, 10);

            renderText(renderer, "Black:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 40);
            formatTime(blackTimerStr, gameState.blackTimeMs);
            renderText(renderer, blackTimerStr, (SDL_Color){255, 255, 255, 255}, boardWidth + 100, 40);

            // Labels for captured pieces in the first sidebar
            renderText(renderer, "Captured by white:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + 10);
            renderText(renderer, "Captured by black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + captured_area_height + 10);

            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    renderPiece(pieceTextureCoordinates, 0, squareSize, row, col,
                                getPieceTexture(pieceTextures, gameState.board[row][col]), &renderer);
                }
            }

            renderCapturedPieces(renderer, pieceTextures, &gameState);

            // Render buttons at the bottom of the first sidebar
            SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 140, 40};
            SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 140, 40};
            SDL_Rect undoButton = {boardWidth + 10, screenHeight - 90, 140, 40}; // Example position
            SDL_Rect redoButton = {boardWidth + 160, screenHeight - 90, 140, 40}; // Example position

            SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Green for save
            SDL_RenderFillRect(renderer, &saveButton);
            renderText(renderer, "Save", (SDL_Color){255, 255, 255, 255}, saveButton.x + 10, saveButton.y + 5);

            SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255); // Blue for load
            SDL_RenderFillRect(renderer, &loadButton);
            renderText(renderer, "Load", (SDL_Color){255, 255, 255, 255}, loadButton.x + 10, loadButton.y + 5);

            // --- RENDER UNDO/REDO BUTTONS ---
            SDL_SetRenderDrawColor(renderer, 150, 50, 0, 255); // Orange for undo
            SDL_RenderFillRect(renderer, &undoButton);
            renderText(renderer, "Undo", (SDL_Color){255, 255, 255, 255}, undoButton.x + 40, undoButton.y + 5);

            SDL_SetRenderDrawColor(renderer, 0, 150, 150, 255); // Cyan for redo
            SDL_RenderFillRect(renderer, &redoButton);
            renderText(renderer, "Redo", (SDL_Color){255, 255, 255, 255}, redoButton.x + 40, redoButton.y + 5);

            // Render elements within the second sidebar (move history, evaluation bar, analysis)
            renderText(renderer, "MOVE HISTORY:", (SDL_Color){255, 255, 255, 255}, boardWidth + sidebar1_width + 10, 10); // Placed at top of second sidebar

            const int moveHeight = 25;
            int visibleStart = moveHistoryScrollOffset / moveHeight;
            // Adjust visibleEnd calculation to fit within the second sidebar's height (e.g., 800 - 10 - 40 - 20 = 730 / 25 = ~29 moves)
            int visibleEnd = visibleStart + (screenHeight - 80) / moveHeight;

            for (int i = visibleStart; i < visibleEnd && i < gameState.moveCount; ++i) {
                char buffer[64];
                // Display moves in a single column, numbered sequentially
                sprintf(buffer, "%d. %s", (i + 1), gameState.moveHistory[i].notation); // Number all moves sequentially

                int y = 40 + (i * moveHeight) - moveHistoryScrollOffset; // Adjusted Y to start closer to "MOVE HISTORY:" label

                renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255}, boardWidth + sidebar1_width + 15, y);
            }

            // Draw game status (check, checkmate, stalemate) - This remains centrally at the top, overlaying the board area
            drawGameStatus(renderer, isInCheck, isGameOver, isStalemate, gameState.blackTurn);

            // Draw evaluation bar if enabled
            if (showEvaluationBar) {
                int score = evaluatePosition(gameState.board, gameState.blackTurn ? 1 : 0);
                drawEvaluationBar(renderer, score); // Now positioned within the second sidebar
            }

            // Show position analysis if enabled
            if (showAnalysis) {
                displayPositionAnalysis(gameState.board, gameState.blackTurn, &gameState.lastDoublePushPawn, gameState.kingsPositions);
            }
        }

        // Present the renderer
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    destroyFont();
    cleanUp(window);
    printf("Program ended\n");
    return 0;
}