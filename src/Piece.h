#ifndef PIECE_H
#define PIECE_H

#define NONE   0
#define PAWN   1
#define BISHOP 2
#define KNIGHT 3
#define ROOK   4
#define QUEEN  5
#define KING   6

#define WHITE 8
#define BLACK 16

#define TYPE_MASK 0x7
#define SELECTED_MASK 0x20
#define MOVABLE_MASK 0x40

void loadPieceTextures(SDL_Texture* textures[2][7], SDL_Renderer** renderer);

SDL_Texture* getPieceTexture(SDL_Texture* textures[2][7], unsigned char piece);

void placePieces(unsigned char board[8][8], char* startPosition);

bool inBounds(int y);

bool opposingColor(unsigned char piece, int color);

void clearPossibleBoard(unsigned char board[8][8]);

void generatePossibleMoves(unsigned char board[8][8], int x, int y);
#endif