// GameState.h
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "util.h" // For Vector2f structure
#include "Piece.h" // Required for MAX_CAPTURED
#include "app_globals.h" // <--- ADDED: To get MAX_MOVES and other globals if needed

// Forward declaration for Move if Events.h includes GameState.h
// Move struct is defined in Events.h, so we need its definition before using it.
// To resolve circular dependency or order issues, consider defining Move in a new common_types.h
// For now, let's assume Events.h is included after GameState.h or its contents are compatible.
// A safer way would be to move the 'Move' struct definition to app_globals.h or its own common_types.h
typedef struct {
    char notation[10];
} Move; // <--- ADDED: Define Move struct here if it's not globally available before GameState.h

typedef struct {
    // Board state
    unsigned char board[8][8];

    // Game flow control
    bool gameRunning;
    bool blackTurn; // true if it's black's turn, false for white's

    // Mouse and piece interaction states
    bool mouseActions[2];   // [0]: mouseButtonDown, [1]: mouseButtonUp
    bool pieceActions[2];   // [0]: isSelected, [1]: isHoldingPiece
    Vector2f selectedSquare; // Coordinates of the currently selected square

    // Special chess rule tracking
    Vector2f kingsPositions[2]; // Index 0 for white king, 1 for black king
    Vector2f lastDoublePushPawn; // Tracks the pawn that made a double push for en passant

    // Game timers
    int whiteTimeMs;
    int blackTimeMs;

    // Captured pieces storage
    unsigned char whiteCapturedPieces[MAX_CAPTURED];
    int numWhiteCapturedPieces;

    unsigned char blackCapturedPieces[MAX_CAPTURED];
    int numBlackCapturedPieces;

    // --- ADDED: Move History as part of GameState ---
    Move moveHistory[MAX_MOVES]; // Array to store move notations
    int moveCount;               // Number of moves currently in history
    // --- END ADDED ---

} GameState;

void initGameState(GameState* state);
void resetGameState(GameState* state);
void saveGameToFile(GameState* state, const char* filePath);
void loadGameFromFile(GameState* state, const char* filePath);

#endif // GAMESTATE_H