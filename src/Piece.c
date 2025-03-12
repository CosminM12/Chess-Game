#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Piece.h"
#include "RenderWindow.h"

void loadPieceTextures(SDL_Texture* textures[2][7], SDL_Renderer** renderer) {
    textures[0][0] = NULL;
    textures[0][1] = loadTexture("../res/Pawn_white.png", renderer);
    textures[0][2] = loadTexture("../res/Bishop_white.png", renderer);
    textures[0][3] = loadTexture("../res/Knight_white.png", renderer);
    textures[0][4] = loadTexture("../res/Rook_white.png", renderer);
    textures[0][5] = loadTexture("../res/Queen_white.png", renderer);
    textures[0][6] = loadTexture("../res/King_white.png", renderer);

    textures[1][0] = NULL;
    textures[1][1] = loadTexture("../res/Pawn_black.png", renderer);
    textures[1][2] = loadTexture("../res/Bishop_black.png", renderer);
    textures[1][3] = loadTexture("../res/Knight_black.png", renderer);
    textures[1][4] = loadTexture("../res/Rook_black.png", renderer);
    textures[1][5] = loadTexture("../res/Queen_black.png", renderer);
    textures[1][6] = loadTexture("../res/King_black.png", renderer);
}

SDL_Texture* getPieceTexture(SDL_Texture* textures[2][7], unsigned char piece) {
    //Piece type is saved on the last 3 bits of piece while the color is saved on the 4th and 5th bits
    /*=================
    (unsigned char) board map
    1B -> (b1)(b2)(b3)(b4)(b5)(b6)(b7)(b8)
    b6, b7, b8: Piece Type: 1-6
    b4, b5: Piece Color (0x10 = white, 0x20 = black)
    b3: is selected 
    =================*/
    int pieceColor = (piece & 0x10) >> 4;
    int pieceType = piece & 0x7;

    return textures[pieceColor][pieceType];
}

void placePieces(unsigned char board[8][8], char* startPosition) {
    int len = strlen(startPosition);
    int pos = 0;
    int row=0, col=0;
    while(pos < len) {
        char c = startPosition[pos];
        if(isdigit(c)) {
            col += (c - '0');
        }
        else if(c == '/') {
            row++;
            col=0;
        }
        else {
            unsigned char piece = 0;
            //get the color (4th and 5th bits)
            piece = isupper(c) ? BLACK : WHITE;

            //get the type (last 3 bits)
            c = tolower(c);
            switch(c) {
                case 'p':
                    piece = piece | PAWN;
                    break;
                case 'b':
                    piece = piece | BISHOP;
                    break;
                case 'n':
                    piece = piece | KNIGHT;
                    break;
                case 'r':
                    piece = piece | ROOK;
                    break;
                case 'q':
                    piece = piece | QUEEN;
                    break;
                case 'k':
                    piece = piece | KING;
                    break;
                default:
                    piece = 0;
                    break;
            }
            //place piece on board
            board[row][col] = piece;
            col++;
        }
        pos++;
    }
}