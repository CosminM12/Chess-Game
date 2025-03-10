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

void loadPieceTextures(SDL_Texture* textures[2][7], SDL_Renderer** renderer);

SDL_Texture* getPieceTexture(SDL_Texture* textures[2][7], unsigned char piece);

void placePieces(unsigned char board[8][8], char* startPosition);
#endif