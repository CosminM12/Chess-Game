#ifndef PIECE_H
#define PIECE_H

#include <SDL2/SDL_image.h>
#include "util.h"

#define NONE   0
#define PAWN   1
#define BISHOP 2
#define KNIGHT 3
#define ROOK   4
#define QUEEN  5
#define KING   6
#define MODIFIER 0x8
/*==========
--Modifier keep extra data:
Pawn: remembers if it has move before (for double push)
King: remembers if it has move before (for castling)
Rook: remembers if it has move before (for castling)
==========*/

#define TYPE_MASK 0x7
#define COLOR_MASK 0x10  //0 = white, 1 = black

#define SELECTED_MASK 0x20
#define MOVABLE_MASK 0x40
#define RISKY_MOVE_MASK 0x80

// Maximum number of pieces that can be captured
#define MAX_CAPTURED 16

void loadPieceTextures(SDL_Texture* textures[2][7], SDL_Renderer** renderer);

SDL_Texture* getPieceTexture(SDL_Texture* textures[2][7], unsigned char piece);

void placePieces(unsigned char board[8][8], char* startPosition);

void exportPosition(unsigned char board[8][8], char **exportString);

void findKings(unsigned char board[8][8], Vector2f kingsPositions[]);

void initCastlingRights(unsigned char board[8][8]);

bool isCheck(unsigned char board[8][8], Vector2f kingPosition);

bool isSquareAttacked(unsigned char board[8][8], Vector2f position, unsigned char attackerColor);

bool inBounds(int y);

bool opposingColor(unsigned char piece, int color);

void generateLongMoves(unsigned char board[8][8], int x, int y, int dx[], int dy[], int color, int directions);

void generateAllPossibleMoves(unsigned char board[8][8], unsigned char color, Vector2f *lastDoublePawn);

void clearPossibleBoard(unsigned char board[8][8]);

void generatePossibleMoves(unsigned char board[8][8], int x, int y, Vector2f *lastDoublePawn);

#endif