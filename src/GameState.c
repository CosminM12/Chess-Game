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
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filePath);
        return;
    }
    
    // Save board state
    fprintf(file, "BOARD:");
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            fprintf(file, "%d", state->board[row][col]);
            if (row < 7 || col < 7) {
                fprintf(file, ",");
            }
        }
    }
    fprintf(file, "\n");
    
    // Save game state variables
    fprintf(file, "BLACK_TURN:%d\n", state->blackTurn);
    fprintf(file, "WHITE_TIME:%d\n", state->whiteTimeMs);
    fprintf(file, "BLACK_TIME:%d\n", state->blackTimeMs);
    fprintf(file, "SELECTED_SQUARE_X:%d\n", state->selectedSquare.x);
    fprintf(file, "SELECTED_SQUARE_Y:%d\n", state->selectedSquare.y);
    fprintf(file, "LAST_DOUBLE_PAWN_X:%d\n", state->lastDoublePushPawn.x);
    fprintf(file, "LAST_DOUBLE_PAWN_Y:%d\n", state->lastDoublePushPawn.y);
    fprintf(file, "WHITE_KING_X:%d\n", state->kingsPositions[0].x);
    fprintf(file, "WHITE_KING_Y:%d\n", state->kingsPositions[0].y);
    fprintf(file, "BLACK_KING_X:%d\n", state->kingsPositions[1].x);
    fprintf(file, "BLACK_KING_Y:%d\n", state->kingsPositions[1].y);
    
    // Save move history
    fprintf(file, "MOVE_COUNT:%d\n", state->moveCount);
    for (int i = 0; i < state->moveCount; i++) {
        fprintf(file, "MOVE_%d:%s\n", i, state->moveHistory[i].notation);
    }
    
    // Save captured pieces
    fprintf(file, "NUM_WHITE_CAPTURED:%d\n", state->numWhiteCapturedPieces);
    fprintf(file, "NUM_BLACK_CAPTURED:%d\n", state->numBlackCapturedPieces);
    
    fprintf(file, "WHITE_CAPTURED:");
    for (int i = 0; i < state->numWhiteCapturedPieces; i++) {
        fprintf(file, "%d", state->whiteCapturedPieces[i]);
        if (i < state->numWhiteCapturedPieces - 1) {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");
    
    fprintf(file, "BLACK_CAPTURED:");
    for (int i = 0; i < state->numBlackCapturedPieces; i++) {
        fprintf(file, "%d", state->blackCapturedPieces[i]);
        if (i < state->numBlackCapturedPieces - 1) {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");
    
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
    char tempBuffer[256]; // Buffer for string values
    
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
            
            char* token = strtok(boardData, ",");
            while (token != NULL && row < 8) {
                state->board[row][col] = (unsigned char)atoi(token);
                
                col++;
                if (col >= 8) {
                    col = 0;
                    row++;
                }
                
                token = strtok(NULL, ",");
            }
        }
        else if (sscanf(line, "BLACK_TURN:%d", (int*)&state->blackTurn) == 1) {
            // Black turn already parsed
        }
        else if (sscanf(line, "WHITE_TIME:%d", &state->whiteTimeMs) == 1) {
            // White time already parsed
        }
        else if (sscanf(line, "BLACK_TIME:%d", &state->blackTimeMs) == 1) {
            // Black time already parsed
        }
        else if (sscanf(line, "SELECTED_SQUARE_X:%f", &tempFloatX) == 1) {
            state->selectedSquare.x = (int)tempFloatX;
        }
        else if (sscanf(line, "SELECTED_SQUARE_Y:%f", &tempFloatY) == 1) {
            state->selectedSquare.y = (int)tempFloatY;
        }
        else if (sscanf(line, "LAST_DOUBLE_PAWN_X:%f", &tempFloatX) == 1) {
            state->lastDoublePushPawn.x = (int)tempFloatX;
        }
        else if (sscanf(line, "LAST_DOUBLE_PAWN_Y:%f", &tempFloatY) == 1) {
            state->lastDoublePushPawn.y = (int)tempFloatY;
        }
        else if (sscanf(line, "WHITE_KING_X:%f", &tempFloatX) == 1) {
            state->kingsPositions[0].x = (int)tempFloatX;
        }
        else if (sscanf(line, "WHITE_KING_Y:%f", &tempFloatY) == 1) {
            state->kingsPositions[0].y = (int)tempFloatY;
        }
        else if (sscanf(line, "BLACK_KING_X:%f", &tempFloatX) == 1) {
            state->kingsPositions[1].x = (int)tempFloatX;
        }
        else if (sscanf(line, "BLACK_KING_Y:%f", &tempFloatY) == 1) {
            state->kingsPositions[1].y = (int)tempFloatY;
        }
        // Handle move history
        else if (sscanf(line, "MOVE_COUNT:%d", &state->moveCount) == 1) {
            // Ensure move count is within bounds
            if (state->moveCount > MAX_MOVES) {
                state->moveCount = MAX_MOVES;
                printf("Warning: Move count exceeds maximum, truncating to %d\n", MAX_MOVES);
            }
        }
        else if (sscanf(line, "MOVE_%*d:%255[^\n]", tempBuffer) == 1) {
            // Only add the move if we haven't reached the maximum
            if (state->moveCount < MAX_MOVES) {
                strncpy(state->moveHistory[state->moveCount].notation, tempBuffer, 255);
                state->moveHistory[state->moveCount].notation[255] = '\0'; // Ensure null termination
                state->moveCount++;
            }
        }
        // Handle captured pieces
        else if (sscanf(line, "NUM_WHITE_CAPTURED:%d", &state->numWhiteCapturedPieces) == 1) {
            // White captured count already parsed
        }
        else if (sscanf(line, "NUM_BLACK_CAPTURED:%d", &state->numBlackCapturedPieces) == 1) {
            // Black captured count already parsed
        }
        else if (strncmp(line, "WHITE_CAPTURED:", 15) == 0) {
            char* capturedData = line + 15;
            char* token = strtok(capturedData, ",");
            int i = 0;
            while (token != NULL && i < state->numWhiteCapturedPieces) {
                state->whiteCapturedPieces[i++] = (unsigned char)atoi(token);
                token = strtok(NULL, ",");
            }
        }
        else if (strncmp(line, "BLACK_CAPTURED:", 15) == 0) {
            char* capturedData = line + 15;
            char* token = strtok(capturedData, ",");
            int i = 0;
            while (token != NULL && i < state->numBlackCapturedPieces) {
                state->blackCapturedPieces[i++] = (unsigned char)atoi(token);
                token = strtok(NULL, ",");
            }
        }
    }
    
    fclose(file);
    
    // Validate king positions after loading
    bool kingFound[2] = {false, false};
    
    // Scan the board to find kings if positions are invalid
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            unsigned char piece = state->board[row][col];
            if ((piece & TYPE_MASK) == KING) {
                unsigned char color = (piece & COLOR_MASK) >> 4;
                state->kingsPositions[color].x = row;
                state->kingsPositions[color].y = col;
                kingFound[color] = true;
            }
        }
    }
    
    // If kings weren't found, reset to default positions
    if (!kingFound[0]) {
        state->kingsPositions[0].x = 7;
        state->kingsPositions[0].y = 4;
        printf("White king position was invalid, reset to default\n");
    }
    
    if (!kingFound[1]) {
        state->kingsPositions[1].x = 0;
        state->kingsPositions[1].y = 4;
        printf("Black king position was invalid, reset to default\n");
    }
    
    printf("Game loaded from %s\n", filePath);
}