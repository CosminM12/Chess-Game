// GameState.h
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>
#include <SDL2/SDL.h> // Required for SDL_Color if you move colors into GameState
#include "util.h" // For Vector2f structure
#include "Piece.h" // Required for MAX_TOTAL_CAPTURED_PIECES


// Define masks and piece types if they are not already globally accessible or in Piece.h
// If they are in Piece.h, you might want to include Piece.h here, or define them in GameState.h if GameState depends on them
// For example:
// #define NONE   0
// #define PAWN   1
// ...
// #define TYPE_MASK 0x7
// #define COLOR_MASK 0x10
// #define SELECTED_MASK 0x20
// #define MOVABLE_MASK 0x40

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
    // These arrays store the type of piece (PAWN, ROOK, etc.) that has been captured.
    unsigned char whiteCapturedPieces[MAX_CAPTURED]; // Pieces captured by White (i.e., Black's pieces)
    int numWhiteCapturedPieces; // Count of pieces captured by White

    unsigned char blackCapturedPieces[MAX_CAPTURED]; // Pieces captured by Black (i.e., White's pieces)
    int numBlackCapturedPieces; // Count of pieces captured by Black

    // You might also consider moving SDL_Color definitions here from main.c if you want them
    // to be part of the dynamic game state or easily resettable.
    // SDL_Color color_light;
    // SDL_Color color_dark;
    // SDL_Color color_clicked;
    // SDL_Color color_possible;

} GameState;

/**
 * @brief Initializes the game state with starting values.
 * @param state A pointer to the GameState structure to initialize.
 */
void initGameState(GameState* state);

/**
 * @brief Resets the game state to its initial configuration (e.g., for a new game).
 * @param state A pointer to the GameState structure to reset.
 */
void resetGameState(GameState* state); // Optional: for restarting the game

#endif // GAMESTATE_H