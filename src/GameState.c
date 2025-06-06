// GameState.c
#include "GameState.h"
#include "Piece.h" // Required for placePieces and findKings
#include <string.h> // For strcmp, strcpy, etc. for move history
#include <stdio.h> // For snprintf, etc.

void initGameState(GameState* state) {
    // Initialize the board with the standard starting position
    char startPosition[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(state->board, startPosition);

    // Set initial game flags
    state->gameRunning = true;
    state->blackTurn = false; // White starts

    // Initialize mouse and piece action states
    state->mouseActions[0] = false;
    state->mouseActions[1] = false;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
    state->selectedSquare = createVector(-1.0f, -1.0f);

    // Initialize special chess rule trackers
    findKings(state->board, state->kingsPositions);
    state->lastDoublePushPawn = createVector(-1.0f, -1.0f);

    // Set initial game timers
    state->whiteTimeMs = 5 * 60 * 1000; // 5 minutes in milliseconds
    state->blackTimeMs = 5 * 60 * 1000; // 5 minutes in milliseconds

    // Initialize captured pieces counts
    state->numWhiteCapturedPieces = 0;
    state->numBlackCapturedPieces = 0;

    // --- ADDED: Initialize move history count ---
    state->moveCount = 0;
    // --- END ADDED ---
}

void resetGameState(GameState* state) {
    initGameState(state);
}

void saveGameToFile(GameState* state, const char* filePath) {
    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        printf("Error opening file for saving!\n");
        return;
    }

    char* boardString = NULL;
    exportPosition(state->board, &boardString);
    if (boardString) {
        fprintf(file, "BOARD:%s\n", boardString);
        free(boardString);
    } else {
        printf("Error exporting board position for saving!\n");
    }

    fprintf(file, "BLACK_TURN:%d\n", state->blackTurn);
    fprintf(file, "WHITE_TIME:%d\n", state->whiteTimeMs);
    fprintf(file, "BLACK_TIME:%d\n", state->blackTimeMs);
    fprintf(file, "SELECTED_SQUARE_X:%d\n", state->selectedSquare.x);
    fprintf(file, "SELECTED_SQUARE_Y:%d\n", state->selectedSquare.y);

    fprintf(file, "WHITE_KING_POS_X:%d\n", state->kingsPositions[0].x);
    fprintf(file, "WHITE_KING_POS_Y:%d\n", state->kingsPositions[0].y);
    fprintf(file, "BLACK_KING_POS_X:%d\n", state->kingsPositions[1].x);
    fprintf(file, "BLACK_KING_POS_Y:%d\n", state->kingsPositions[1].y);

    fprintf(file, "LAST_DOUBLE_PUSH_PAWN_X:%d\n", state->lastDoublePushPawn.x);
    fprintf(file, "LAST_DOUBLE_PUSH_PAWN_Y:%d\n", state->lastDoublePushPawn.y);

    fprintf(file, "NUM_WHITE_CAPTURED:%d\n", state->numWhiteCapturedPieces);
    for (int i = 0; i < state->numWhiteCapturedPieces; ++i) {
        fprintf(file, "W_CAPTURED_PIECE_%d:%d\n", i, state->whiteCapturedPieces[i]);
    }
    fprintf(file, "NUM_BLACK_CAPTURED:%d\n", state->numBlackCapturedPieces);
    for (int i = 0; i < state->numBlackCapturedPieces; ++i) {
        fprintf(file, "B_CAPTURED_PIECE_%d:%d\n", i, state->blackCapturedPieces[i]);
    }

    // --- ADDED: Save move history ---
    fprintf(file, "MOVE_COUNT:%d\n", state->moveCount);
    for (int i = 0; i < state->moveCount; ++i) {
        fprintf(file, "MOVE_%d:%s\n", i, state->moveHistory[i].notation);
    }
    // --- END ADDED ---

    fclose(file);
    printf("Game saved to %s\n", filePath);
}

void loadGameFromFile(GameState* state, const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for reading\n", filePath);
        return;
    }
    
    // Reset the game state before loading
    resetGameState(state);
    
    char line[256];
    float tempFloatX, tempFloatY;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Parse different types of data
        if (strncmp(line, "BOARD:", 6) == 0) {
            // Parse board data
            char* boardData = line + 6; // Skip "BOARD:" prefix
            int row = 0, col = 0;
            
            for (size_t i = 0; i < strlen(boardData); i++) {
                if (boardData[i] == '/') {
                    row++;
                    col = 0;
                } else if (boardData[i] >= '0' && boardData[i] <= '9') {
                    int emptyCount = boardData[i] - '0';
                    for (int j = 0; j < emptyCount; j++) {
                        state->board[row][col++] = NONE;
                    }
                } else {
                    // Parse piece code
                    state->board[row][col++] = boardData[i];
                }
            }
        } else {
            // Parse other game state variables
            if (sscanf(line, "BLACK_TURN:%d", &state->blackTurn) == 1) {
                // Value loaded
            } else if (sscanf(line, "WHITE_TIME:%d", &state->whiteTimeMs) == 1) {
                // Value loaded
            } else if (sscanf(line, "BLACK_TIME:%d", &state->blackTimeMs) == 1) {
                // Value loaded
            } else if (sscanf(line, "SELECTED_SQUARE_X:%f", &tempFloatX) == 1) {
                state->selectedSquare.x = tempFloatX;
            } else if (sscanf(line, "SELECTED_SQUARE_Y:%f", &tempFloatY) == 1) {
                state->selectedSquare.y = tempFloatY;
            } else if (sscanf(line, "WHITE_KING_POS_X:%f", &tempFloatX) == 1) {
                state->kingsPositions[0].x = tempFloatX;
            } else if (sscanf(line, "WHITE_KING_POS_Y:%f", &tempFloatY) == 1) {
                state->kingsPositions[0].y = tempFloatY;
            } else if (sscanf(line, "BLACK_KING_POS_X:%f", &tempFloatX) == 1) {
                state->kingsPositions[1].x = tempFloatX;
            } else if (sscanf(line, "BLACK_KING_POS_Y:%f", &tempFloatY) == 1) {
                state->kingsPositions[1].y = tempFloatY;
            } else if (sscanf(line, "LAST_DOUBLE_PUSH_PAWN_X:%f", &tempFloatX) == 1) {
                state->lastDoublePushPawn.x = tempFloatX;
            } else if (sscanf(line, "LAST_DOUBLE_PUSH_PAWN_Y:%f", &tempFloatY) == 1) {
                state->lastDoublePushPawn.y = tempFloatY;
            } else if (sscanf(line, "NUM_WHITE_CAPTURED:%d", &state->numWhiteCapturedPieces) == 1) {
                // Value loaded
            } else if (sscanf(line, "NUM_BLACK_CAPTURED:%d", &state->numBlackCapturedPieces) == 1) {
                // Value loaded
            } else if (strncmp(line, "WHITE_CAPTURED:", 15) == 0) {
                // Parse white captured pieces
                char* capturedData = line + 15;
                for (int i = 0; i < state->numWhiteCapturedPieces; i++) {
                    state->whiteCapturedPieces[i] = capturedData[i];
                }
            } else if (strncmp(line, "BLACK_CAPTURED:", 15) == 0) {
                // Parse black captured pieces
                char* capturedData = line + 15;
                for (int i = 0; i < state->numBlackCapturedPieces; i++) {
                    state->blackCapturedPieces[i] = capturedData[i];
                }
            }
            // --- ADDED: Load move history ---
            else if (sscanf(line, "MOVE_COUNT:%d", &state->moveCount) == 1) {
                // Count loaded
            } else if (sscanf(line, "MOVE_%*d:%[^\n]", state->moveHistory[state->moveCount].notation) == 1) {
                state->moveCount++;
            }
            // --- END ADDED ---
        }
    }
    fclose(file);
    printf("Game loaded from %s\n", filePath);
}