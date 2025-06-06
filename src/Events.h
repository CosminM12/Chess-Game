#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"
#include "GameState.h" // This now includes app_globals.h and declares history globals/prototypes

// MAX_MOVES is defined in app_globals.h

void getEvents(SDL_Event event, GameState *state, int *scrollOffset);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(GameState* state, int squareX, int squareY);

void makeMove(GameState* state, int destX, int destY);

void deselectPiece(GameState* state);

void handleMouseInput(GameState* state, int mouseX, int mouseY);

// Prototype for addMoveToHistory (matches current implementation in Events.c)
void addMoveToHistory(int startRow, int startCol, int endRow, int endCol, unsigned char piece);

// Global move history and count (defined in main.c)
extern Move moveHistory[MAX_MOVES];
extern int moveCount;

#endif