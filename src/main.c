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
#include "util.h"
#include "engine.h"
#include "GameState.h"
#include "app_globals.h"

/*----------Variable declaration------------*/
bool gameRunning = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Screen dimensions are now defined in app_globals.c

SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};
SDL_Color color_risky = {255, 128, 0, 255}; // Orange color for risky moves

// Timer variables
int whiteTimeMs = 5 * 60 * 1000; // 5 minutes in milliseconds
int blackTimeMs = 5 * 60 * 1000;
char whiteTimerStr[16];
char blackTimerStr[16];

// Move history scroll offset
int moveHistoryScrollOffset = 0;

// Engine variables
bool engineEnabled = false;
bool showAnalysis = false;
bool computerPlaysBlack = false;
bool showEvaluationBar = false;
bool showMenu = false;
int gameMode = 0; // 0 = not selected, 1 = PvP, 2 = PvE
Uint32 moveTimestamp = 0; // Timestamp of the last player move

Uint64 currentTick, lastTick;
double deltaTime;

/*---------Helper functions-----------*/
bool init() {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(TTF_Init() == -1) {
        printf("TTF_Init has failed. Error: %s\n", TTF_GetError());
        return false;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer failed: %s\n", Mix_GetError());
        return false;
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

// Format time as MM:SS
void formatTime(char *buffer, int timeMs) {
    int totalSeconds = timeMs / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    snprintf(buffer, 16, "%02d:%02d", minutes, seconds);
}

// Function to draw the evaluation bar
void drawEvaluationBar(SDL_Renderer* renderer, int score) {
    float whitePercentage, blackPercentage;
    getScoreBar(score, &whitePercentage, &blackPercentage);
    
    // Draw the bar on the right side of the screen
    int barWidth = 30;
    int barHeight = 400;
    int barX = screenWidth - barWidth - 20;
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
    char scoreText[32];
    snprintf(scoreText, sizeof(scoreText), "%.2f", score / 100.0f);
    
    SDL_Color textColor = {255, 255, 255, 255};
    renderText(renderer, scoreText, textColor, barX - 50, barY + barHeight + 5);
}

// Function to make the computer move
void makeComputerMove(GameState* state) {
    unsigned char color = state->blackTurn ? 1 : 0;
    EngineMove bestMove = findBestMove(state->board, color, &state->lastDoublePushPawn, state->kingsPositions);
    
    if (bestMove.from.x != -1) {
        // Update the selected square to the piece we want to move
        state->selectedSquare.x = bestMove.from.y;
        state->selectedSquare.y = bestMove.from.x;
        
        // Make the move
        makeMove(state, bestMove.to.y, bestMove.to.x);
        
        // Check for checkmate or stalemate after the move
        unsigned char nextColor = state->blackTurn ? 1 : 0;
        MoveList moveList;
        generateMoves(state->board, nextColor, &moveList, &state->lastDoublePushPawn);
        
        if (moveList.count == 0) {
            // Check if the king is in check
            if (isCheck(state->board, state->kingsPositions[nextColor])) {
                printf("Checkmate! %s wins!\n", nextColor == 1 ? "White" : "Black");
            } else {
                printf("Stalemate! Game is drawn.\n");
            }
        }
    }
}

// Function to draw game status text (check, checkmate, stalemate)
void drawGameStatus(SDL_Renderer* renderer, bool isInCheck, bool isGameOver, bool isStalemate, bool blackTurn) {
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
        renderText(renderer, statusText, textColor, (boardWidth - 200) / 2, 10);
    }
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

// Function to get the renderer for external modules
SDL_Renderer* getMainRenderer() {
    return renderer;
}

// Draw sidebar with timer, captured pieces, move history, and buttons
void drawSidebar(SDL_Renderer* renderer, GameState* state) {
    // Draw sidebar background
    SDL_Rect sidebar_background = {boardWidth, 0, sidebar1_width + sidebar2_width, screenHeight};
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
    SDL_RenderFillRect(renderer, &sidebar_background);
    
    // Timer section
    SDL_Rect timerBox = {boardWidth, 0, sidebar1_width, 100};
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
    SDL_RenderFillRect(renderer, &timerBox);
    
    formatTime(whiteTimerStr, whiteTimeMs);
    formatTime(blackTimerStr, blackTimeMs);
    
    renderText(renderer, "White:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 10);
    renderText(renderer, whiteTimerStr, (SDL_Color){255, 255, 255, 255}, boardWidth + 150, 10);
    
    renderText(renderer, "Black:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 40);
    renderText(renderer, blackTimerStr, (SDL_Color){255, 255, 255, 255}, boardWidth + 150, 40);
    
    // Captured pieces section
    renderText(renderer, "Captured by white:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 100 + 10);
    renderCapturedPieces(renderer, state);
    
    renderText(renderer, "Captured by black:", (SDL_Color){255, 255, 255, 255}, boardWidth + 10, 350 + 10);
    
    // Move history section
    renderText(renderer, "MOVE HISTORY:", (SDL_Color){255, 255, 255, 255}, boardWidth + sidebar1_width + 10, 10);
    
    const int moveHeight = 25;
    int visibleStart = moveHistoryScrollOffset / moveHeight;
    int visibleEnd = visibleStart + (screenHeight / moveHeight) + 1;
    
    for (int i = visibleStart; i < visibleEnd && i < state->moveCount; ++i) {
        char buffer[64];
        sprintf(buffer, "%d. %s", (i + 1), state->moveHistory[i].notation);
        
        int y = 40 + (i * moveHeight) - moveHistoryScrollOffset;
        
        if (y >= 40 && y < screenHeight - 10) {
            renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255},
                       boardWidth + sidebar1_width + 15, y);
        }
    }
    
    // Buttons section
    SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 140, 40};
    SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 140, 40};
    SDL_Rect undoButton = {boardWidth + 10, screenHeight - 90, 140, 40};
    SDL_Rect redoButton = {boardWidth + 160, screenHeight - 90, 140, 40};
    
    // Save button
    SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255); // Green
    SDL_RenderFillRect(renderer, &saveButton);
    renderText(renderer, "Save Game", (SDL_Color){255, 255, 255, 255}, saveButton.x + 10, saveButton.y + 5);
    
    // Load button
    SDL_SetRenderDrawColor(renderer, 0, 0, 150, 255); // Blue
    SDL_RenderFillRect(renderer, &loadButton);
    renderText(renderer, "Load Game", (SDL_Color){255, 255, 255, 255}, loadButton.x + 10, loadButton.y + 5);
    
    // Undo button
    SDL_SetRenderDrawColor(renderer, 150, 50, 0, 255); // Orange
    SDL_RenderFillRect(renderer, &undoButton);
    renderText(renderer, "Undo", (SDL_Color){255, 255, 255, 255}, undoButton.x + 40, undoButton.y + 5);
    
    // Redo button
    SDL_SetRenderDrawColor(renderer, 0, 150, 150, 255); // Cyan
    SDL_RenderFillRect(renderer, &redoButton);
    renderText(renderer, "Redo", (SDL_Color){255, 255, 255, 255}, redoButton.x + 40, redoButton.y + 5);
}

// Draw filename prompt overlay
void drawFilenamePrompt(SDL_Renderer* renderer) {
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
}

// Handle button clicks in the sidebar
void handleSidebarButtons(int mouseX, int mouseY, GameState* state) {
    // Define button positions
    SDL_Rect saveButton = {boardWidth + 10, screenHeight - 140, 140, 40};
    SDL_Rect loadButton = {boardWidth + 160, screenHeight - 140, 140, 40};
    SDL_Rect undoButton = {boardWidth + 10, screenHeight - 90, 140, 40};
    SDL_Rect redoButton = {boardWidth + 160, screenHeight - 90, 140, 40};
    
    if (currentScreenState == GAME_STATE_PLAYING) {
        // Save button
        if (mouseX >= saveButton.x && mouseX <= saveButton.x + saveButton.w &&
            mouseY >= saveButton.y && mouseY <= saveButton.y + saveButton.h) {
            printf("Save Game button clicked! Opening prompt...\n");
            currentScreenState = GAME_STATE_PROMPT_FILENAME;
            currentPromptAction = PROMPT_ACTION_SAVE;
            SDL_StartTextInput();
            inputFileNameBuffer[0] = '\0';
            textInputActive = SDL_TRUE;
        }
        // Load button
        else if (mouseX >= loadButton.x && mouseX <= loadButton.x + loadButton.w &&
                 mouseY >= loadButton.y && mouseY <= loadButton.y + loadButton.h) {
            printf("Load Game button clicked! Opening prompt...\n");
            currentScreenState = GAME_STATE_PROMPT_FILENAME;
            currentPromptAction = PROMPT_ACTION_LOAD;
            SDL_StartTextInput();
            inputFileNameBuffer[0] = '\0';
            textInputActive = SDL_TRUE;
        }
        // Undo button
        else if (mouseX >= undoButton.x && mouseX <= undoButton.x + undoButton.w &&
                 mouseY >= undoButton.y && mouseY <= undoButton.y + undoButton.h) {
            printf("Undo button clicked!\n");
            undoGame(state);
        }
        // Redo button
        else if (mouseX >= redoButton.x && mouseX <= redoButton.x + redoButton.w &&
                 mouseY >= redoButton.y && mouseY <= redoButton.y + redoButton.h) {
            printf("Redo button clicked!\n");
            redoGame(state);
        }
    }
}

int main() {
    //==========Program Initialization==========//
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if(!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    if (!initFont("../res/fonts/JetBrainsMono/JetBrainsMono-Bold.ttf", 24)) {
        fprintf(stderr, "Font failed to load, check the path and font file.\n");
        return -1;
    }

    // Game mode selection menu
    bool inMenu = true;
    while (inMenu) {
        SDL_Event menuEvent;
        int mouseX, mouseY;
        
        while (SDL_PollEvent(&menuEvent)) {
            if (menuEvent.type == SDL_QUIT) {
                return 0; // Exit game if window is closed
            } else if (menuEvent.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mouseX, &mouseY);
                
                // PvP button
                if (mouseX >= 450 && mouseX <= 800 && mouseY >= 300 && mouseY <= 370) {
                    gameMode = 1; // Player vs Player
                    inMenu = false;
                }
                // PvE button
                else if (mouseX >= 450 && mouseX <= 800 && mouseY >= 400 && mouseY <= 470) {
                    gameMode = 2; // Player vs Engine
                    inMenu = false;
                }
            }
        }
        
        // Draw menu
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        
        // Title
        renderText(renderer, "Chess Game", (SDL_Color){255, 255, 255, 255}, screenWidth / 2 - 100, 150);
        
        // PvP button
        SDL_Rect pvpButton = {450, 300, 350, 70};
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderFillRect(renderer, &pvpButton);
        renderText(renderer, "Player vs Player", (SDL_Color){255, 255, 255, 255}, pvpButton.x + 80, pvpButton.y + 20);
        
        // PvE button
        SDL_Rect pveButton = {450, 400, 350, 70};
        SDL_SetRenderDrawColor(renderer, 100, 0, 0, 255);
        SDL_RenderFillRect(renderer, &pveButton);
        renderText(renderer, "Player vs Computer", (SDL_Color){255, 255, 255, 255}, pveButton.x + 70, pveButton.y + 20);
        
        SDL_RenderPresent(renderer);
    }

    //==========Initialize Game State==========//
    GameState gameState;
    initGameState(&gameState);
    gameState.gameRunning = true;
    
    //==========Initialize Values==========//
    SDL_Event event;
    SDL_Texture* pieceTextures[2][7];
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;
    
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(gameState.board, input);
    initCastlingRights(gameState.board); // Initialize castling rights (sets MODIFIER flags)
    findKings(gameState.board, gameState.kingsPositions); //Initialize kings positions
    
    // Record initial game state
    recordGameState(&gameState);
    
    // Game state variables for check/checkmate/stalemate
    bool isInCheck = false;
    bool isGameOver = false;
    bool isStalemate = false;
    
    // Initialize time tracking
    lastTick = SDL_GetPerformanceCounter();
    currentTick = lastTick;
    
    while(gameState.gameRunning) {
        //==========Find time variables==========//
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());
        
        // Update timers
        if (gameState.blackTurn) {
            blackTimeMs -= deltaTime;
            if (blackTimeMs < 0) blackTimeMs = 0;
        } else {
            whiteTimeMs -= deltaTime;
            if (whiteTimeMs < 0) whiteTimeMs = 0;
        }
        
        //==========Events and movements==========//
        getEvents(event, &gameState, &moveHistoryScrollOffset);
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Handle mouse click
        if (gameState.mouseActions[0]) {
            handleSidebarButtons(mouseX, mouseY, &gameState);
        }
        
        if (mouseInsideBoard(mouseX, mouseY) && currentScreenState == GAME_STATE_PLAYING) {
            handleMouseInput(&gameState, mouseX, mouseY);
        }
        
        gameState.mouseActions[0] = false;
        gameState.mouseActions[1] = false;
        
        // Check for check, checkmate, or stalemate
        unsigned char currentColor = gameState.blackTurn ? 1 : 0;
        isInCheck = isCheck(gameState.board, gameState.kingsPositions[currentColor]);
        
        MoveList moveList;
        generateMoves(gameState.board, currentColor, &moveList, &gameState.lastDoublePushPawn);
        
        isGameOver = moveList.count == 0;
        isStalemate = isGameOver && !isInCheck;
        
        //==========Render Visuals==========//
        clear(&renderer);
        
        // Draw the chess board
        drawBoard(renderer, squareSize, 0, screenWidth, color_light, color_dark, color_clicked, color_possible, color_risky, gameState.board);
        
        // Draw pieces
        for(int row=0;row<8;row++) {
            for(int col=0;col<8;col++) {
                renderPiece(pieceTextureCoordinates, 0, squareSize, row, col, getPieceTexture(pieceTextures, gameState.board[row][col]), &renderer);
            }
        }
        
        // Draw sidebar UI
        drawSidebar(renderer, &gameState);
        
        // Draw game status
        drawGameStatus(renderer, isInCheck, isGameOver, isStalemate, gameState.blackTurn);
        
        // Draw filename prompt if active
        if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
            drawFilenamePrompt(renderer);
        }
        
        display(&renderer);
    }

    // Cleanup
    cleanUp(window);
    printf("Program ended\n");
    return 0;
}