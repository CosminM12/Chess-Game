#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"

void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[]);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare);

void makeMove(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[]);

void deselectPiece(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]);

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], Vector2f* selectedSquare);

#endif