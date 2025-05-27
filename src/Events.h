#ifndef EVENTS_H
#define EVENTS_H

#include "util.h"

#define MAX_MOVES 1024


void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[], int *scrollOffset);

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare);

void makeMove(unsigned char board[8][8],
              int destX,
              int destY,
              Vector2f* sourceSquare,
              bool pieceActions[],
              Vector2f* lastDoublePawn,
              Vector2f kingsPositions[],
              unsigned char* capturedByWhite,
              unsigned char* capturedByBlack,
              int* capturedWhiteCount,
              int* capturedBlackCount);

void deselectPiece(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]);

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], bool* blackTurn, Vector2f* selectedSquare, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

void addMoveToHistory(int startRow, int startCol, int endRow, int endCol, unsigned char piece);


typedef struct {
    char notation[10];
} Move;

extern Move moveHistory[MAX_MOVES];
extern int moveCount;

#endif