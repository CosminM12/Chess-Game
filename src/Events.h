#ifndef EVENTS_H
#define EVENTS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "util.h"
#include "GameState.h"

// Function declaration from main.c for position analysis
void displayPositionAnalysis(unsigned char board[8][8], bool blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

// External variables
extern bool showEvaluationBar;
extern bool showMenu;
extern int gameMode;
extern Uint32 moveTimestamp;
extern int moveHistoryScrollOffset;

// Legacy functions (for backward compatibility)
void getEvents_legacy(SDL_Event event, bool *gameRunning, bool mouseActions[]);
bool mouseInsideBoard(int mouseX, int mouseY);
void selectAndHold_legacy(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare);
void makeMove_legacy(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[], Vector2f* lastDoublePawn, Vector2f kingsPositions[], bool* blackTurn);
void deselectPiece_legacy(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]);
void handleMouseInput_legacy(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], bool* blackTurn, Vector2f* selectedSquare, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

// New GameState-based functions
void getEvents(SDL_Event event, GameState *state, int *scrollOffset);
void selectAndHold(GameState *state, int squareX, int squareY);
void makeMove(GameState *state, int destX, int destY);
void deselectPiece(GameState *state);
void handleMouseInput(GameState *state, int mouseX, int mouseY);

#endif