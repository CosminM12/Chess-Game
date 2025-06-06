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

// UPDATED PROTOYPE: add GameState* state parameter
void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece);

#endif