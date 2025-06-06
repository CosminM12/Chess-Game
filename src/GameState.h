#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "Piece.h"
#include "app_globals.h"
#include "util.h"

// GameState structure to hold all game state information
typedef struct {
    unsigned char board[8][8];
    bool blackTurn;
    bool gameRunning;
    bool mouseActions[2];
    bool pieceActions[2];
    Vector2f selectedSquare;
    Vector2f kingsPositions[2];
    Vector2f lastDoublePushPawn;
    
    // Captured pieces tracking
    unsigned char whiteCapturedPieces[16];
    unsigned char blackCapturedPieces[16];
    int numWhiteCapturedPieces;
    int numBlackCapturedPieces;
    
    // Move history
    Move moveHistory[MAX_MOVES];
    int moveCount;
} GameState;

// Functions for GameState management
void initGameState(GameState* state);
void saveGameToFile(GameState* state, const char* filename);
void loadGameFromFile(GameState* state, const char* filename);
void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece);

// History state management
extern GameState historyStates[MAX_HISTORY_STATES];
extern int historyCount;
extern int currentHistoryIdx;

void recordGameState(GameState* state);
void undoGame(GameState* state);
void redoGame(GameState* state);

#endif // GAMESTATE_H 