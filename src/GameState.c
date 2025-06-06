#include <stdio.h>
#include <string.h>
#include "GameState.h"
#include "Piece.h"

// Define the global variables for history management
GameState historyStates[MAX_HISTORY_STATES];
int historyCount = 0;
int currentHistoryIdx = -1;

// Initialize a new game state
void initGameState(GameState* state) {
    // Clear the board
    memset(state->board, 0, sizeof(state->board));
    
    // Set initial game state
    state->blackTurn = false;
    state->gameRunning = true;
    state->mouseActions[0] = false;
    state->mouseActions[1] = false;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
    
    // Initialize selected square to invalid position
    state->selectedSquare.x = -1;
    state->selectedSquare.y = -1;
    
    // Initialize kings positions
    state->kingsPositions[0].x = -1;
    state->kingsPositions[0].y = -1;
    state->kingsPositions[1].x = -1;
    state->kingsPositions[1].y = -1;
    
    // Initialize last double push pawn
    state->lastDoublePushPawn.x = -1;
    state->lastDoublePushPawn.y = -1;
    
    // Clear captured pieces
    state->numWhiteCapturedPieces = 0;
    state->numBlackCapturedPieces = 0;
    memset(state->whiteCapturedPieces, 0, sizeof(state->whiteCapturedPieces));
    memset(state->blackCapturedPieces, 0, sizeof(state->blackCapturedPieces));
    
    // Clear move history
    state->moveCount = 0;
    memset(state->moveHistory, 0, sizeof(state->moveHistory));
}

// Convert piece type to character for notation
char getPieceChar(unsigned char piece) {
    unsigned char type = piece & TYPE_MASK;
    switch (type) {
        case PAWN: return 'p';
        case KNIGHT: return 'n';
        case BISHOP: return 'b';
        case ROOK: return 'r';
        case QUEEN: return 'q';
        case KING: return 'k';
        default: return '?';
    }
}

// Add a move to the history
void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece) {
    if (state->moveCount >= MAX_MOVES) {
        printf("Move history is full\n");
        return;
    }
    
    // Create a new move entry
    Move newMove;
    newMove.startRow = startRow;
    newMove.startCol = startCol;
    newMove.endRow = endRow;
    newMove.endCol = endCol;
    
    // Generate algebraic notation
    char pieceChar = ' ';
    switch (piece & TYPE_MASK) {
        case BISHOP: pieceChar = 'B'; break;
        case KNIGHT: pieceChar = 'N'; break;
        case ROOK:   pieceChar = 'R'; break;
        case QUEEN:  pieceChar = 'Q'; break;
        case KING:   pieceChar = 'K'; break;
        default:     pieceChar = ' '; break; // Pawn
    }
    
    // Convert board coordinates to algebraic notation
    char fromFile = 'a' + startCol;
    int fromRank = 8 - startRow;
    char toFile = 'a' + endCol;
    int toRank = 8 - endRow;
    
    if (pieceChar == ' ') {
        // Pawn move
        sprintf(newMove.notation, "%c%d-%c%d", fromFile, fromRank, toFile, toRank);
    } else {
        // Piece move
        sprintf(newMove.notation, "%c%c%d-%c%d", pieceChar, fromFile, fromRank, toFile, toRank);
    }
    
    // Add to history
    state->moveHistory[state->moveCount] = newMove;
    state->moveCount++;
}

// Save game state to a file
void saveGameToFile(GameState* state, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }
    
    // Write the entire game state to the file
    fwrite(state, sizeof(GameState), 1, file);
    fclose(file);
    
    printf("Game saved to %s\n", filename);
}

// Load game state from a file
void loadGameFromFile(GameState* state, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s for reading\n", filename);
        return;
    }
    
    // Read the entire game state from the file
    fread(state, sizeof(GameState), 1, file);
    fclose(file);
    
    printf("Game loaded from %s\n", filename);
}

// Record the current game state in history
void recordGameState(GameState* state) {
    // If we are not at the latest state (i.e., some undos have happened),
    // a new move truncates the redo history.
    if (currentHistoryIdx < historyCount - 1) {
        historyCount = currentHistoryIdx + 1;
    }

    // Check for array bounds
    if (historyCount >= MAX_HISTORY_STATES) {
        // Option 1: Shift history to make space
        for (int i = 0; i < historyCount - 1; i++) {
            historyStates[i] = historyStates[i + 1];
        }
        historyCount--;
        currentHistoryIdx--;
    }

    currentHistoryIdx++;
    historyStates[currentHistoryIdx] = *state; // Copy the entire current game state
    historyCount = currentHistoryIdx + 1; // Update total count of states
    printf("Recorded state %d. Total states: %d\n", currentHistoryIdx, historyCount);
}

// Undo the last move
void undoGame(GameState* state) {
    if (currentHistoryIdx > 0) {
        currentHistoryIdx--;
        *state = historyStates[currentHistoryIdx]; // Revert to previous state
        printf("Undone to state %d.\n", currentHistoryIdx);
    } else {
        printf("Cannot undo further.\n");
    }
}

// Redo a previously undone move
void redoGame(GameState* state) {
    if (currentHistoryIdx < historyCount - 1) {
        currentHistoryIdx++;
        *state = historyStates[currentHistoryIdx]; // Re-apply next state
        printf("Redone to state %d.\n", currentHistoryIdx);
    } else {
        printf("Cannot redo further.\n");
    }
} 