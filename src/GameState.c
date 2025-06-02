// GameState.c
#include "GameState.h"
#include "Piece.h" // Required for placePieces and findKings

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

    // Initialize other state variables as needed
}

void resetGameState(GameState* state) {
    // For a game reset, you can simply call initGameState again
    initGameState(state);
    // Or selectively reset specific parts if a full re-initialization is not desired
}