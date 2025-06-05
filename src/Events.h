#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"
#include "engine.h"

// Function declaration from main.c for position analysis
void displayPositionAnalysis(unsigned char board[8][8], bool blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[]);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare);

void makeMove(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[], Vector2f* lastDoublePawn, Vector2f kingsPositions[], bool* blackTurn);

void deselectPiece(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]);

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], bool* blackTurn, Vector2f* selectedSquare, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

#endif