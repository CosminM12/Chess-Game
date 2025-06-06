#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"
#include "GameState.h"

#define MAX_MOVES 1024


void getEvents(SDL_Event event, GameState *state, int *scrollOffset);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(GameState* state, int squareX, int squareY);

void makeMove(GameState* state, int destX, int destY);

void deselectPiece(GameState* state);

//void handleMouseInput(GameState* state, int mouseX, int mouseY, int screenWidth, int squareSize);

void handleMouseInput(GameState* state, int mouseX, int mouseY);

void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece);


typedef struct {
    char notation[10];
} Move;

extern Move moveHistory[MAX_MOVES];
extern int moveCount;

#endif