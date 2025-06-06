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
    if (file == NULL) {
        printf("Error opening file for loading, creating new game.\n");
        initGameState(state);
        return;
    }

    char line[256];
    char boardString[73];
    int tempInt;
    float tempFloatX, tempFloatY;
    char tempMoveNotation[10];

    state->numWhiteCapturedPieces = 0;
    state->numBlackCapturedPieces = 0;
    state->moveCount = 0; // Initialize before loading

    while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "BOARD:%s", boardString) == 1) {
            placePieces(state->board, boardString);
        } else if (sscanf(line, "BLACK_TURN:%d", &tempInt) == 1) {
            state->blackTurn = (bool)tempInt;
        } else if (sscanf(line, "WHITE_TIME:%d", &state->whiteTimeMs) == 1) {
            // Value loaded
        } else if (sscanf(line, "BLACK_TIME:%d", &state->blackTimeMs) == 1) {
            // Value loaded
        } else if (sscanf(line, "SELECTED_SQUARE_X:%.2f", &tempFloatX) == 1) {
            state->selectedSquare.x = tempFloatX;
        } else if (sscanf(line, "SELECTED_SQUARE_Y:%.2f", &tempFloatY) == 1) {
            state->selectedSquare.y = tempFloatY;
        } else if (sscanf(line, "WHITE_KING_POS_X:%.2f", &tempFloatX) == 1) {
            state->kingsPositions[0].x = tempFloatX;
        } else if (sscanf(line, "WHITE_KING_POS_Y:%.2f", &tempFloatY) == 1) {
            state->kingsPositions[0].y = tempFloatY;
        } else if (sscanf(line, "BLACK_KING_POS_X:%.2f", &tempFloatX) == 1) {
            state->kingsPositions[1].x = tempFloatX;
        } else if (sscanf(line, "BLACK_KING_POS_Y:%.2f", &tempFloatY) == 1) {
            state->kingsPositions[1].y = tempFloatY;
        } else if (sscanf(line, "LAST_DOUBLE_PUSH_PAWN_X:%.2f", &tempFloatX) == 1) {
            state->lastDoublePushPawn.x = tempFloatX;
        } else if (sscanf(line, "LAST_DOUBLE_PUSH_PAWN_Y:%.2f", &tempFloatY) == 1) {
            state->lastDoublePushPawn.y = tempFloatY;
        } else if (sscanf(line, "NUM_WHITE_CAPTURED:%d", &state->numWhiteCapturedPieces) == 1) {
            // Count loaded
        } else if (sscanf(line, "W_CAPTURED_PIECE_%*d:%d", &tempInt) == 1) {
            if (state->numWhiteCapturedPieces > 0 && state->numWhiteCapturedPieces <= MAX_CAPTURED) {
                state->whiteCapturedPieces[state->numWhiteCapturedPieces - 1] = (unsigned char)tempInt;
            }
        } else if (sscanf(line, "NUM_BLACK_CAPTURED:%d", &state->numBlackCapturedPieces) == 1) {
            // Count loaded
        } else if (sscanf(line, "B_CAPTURED_PIECE_%*d:%d", &tempInt) == 1) {
            if (state->numBlackCapturedPieces > 0 && state->numBlackCapturedPieces <= MAX_CAPTURED) {
                state->blackCapturedPieces[state->numBlackCapturedPieces - 1] = (unsigned char)tempInt;
            }
        }
            // --- ADDED: Load move history ---
        else if (sscanf(line, "MOVE_COUNT:%d", &state->moveCount) == 1) {
            // Count loaded
        } else if (sscanf(line, "MOVE_%*d:%s", tempMoveNotation) == 1) {
            // Using a temporary string for sscanf, then copy to struct
            if (state->moveCount > 0 && state->moveCount <= MAX_MOVES) {
                // Adjusting for 0-indexed array vs 1-indexed count.
                // Need to find the correct index for this move.
                // This sscanf approach might load moves out of order or overwrite.
                // A better approach would be to parse the index from the line:
                int moveIndex;
                if (sscanf(line, "MOVE_%d:%s", &moveIndex, tempMoveNotation) == 2) {
                    if (moveIndex >= 0 && moveIndex < MAX_MOVES) {
                        strncpy(state->moveHistory[moveIndex].notation, tempMoveNotation, sizeof(state->moveHistory[moveIndex].notation) - 1);
                        state->moveHistory[moveIndex].notation[sizeof(state->moveHistory[moveIndex].notation) - 1] = '\0';
                    }
                }
            }
        }
        // --- END ADDED ---
    }
    fclose(file);
    printf("Game loaded from %s\n", filePath);
}