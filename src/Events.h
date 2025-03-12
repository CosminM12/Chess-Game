#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"

void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[]);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], Vector2f* selectedSquare);

#endif