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
    int pieceType = piece & TYPE_MASK;

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


//Checks if calc position is on board
bool inBounds(int var) {
    return -1 < var && var < 8; 
}

//Checks if piece is of another color
bool opposingColor(unsigned char piece, int color) {
    return ((piece & 0x10) >> 4) != color;
}

void generateStepMoves(unsigned char board[8][8], int x, int y, int dx[], int dy[], int color, int directions) {
    for(int k=0;k<directions;k++) {
        int newX = x + dx[k];
        int newY = y + dy[k];

        if(inBounds(newX) && inBounds(newY)) {
            if(board[newX][newY] == 0 || opposingColor(board[newX][newY], color)) {
                board[newX][newY] |=  MOVABLE_MASK;
            }
        }
    }
}

//Generate moves on lines, colomns and diagonals
void generateLongMoves(unsigned char board[8][8], int x, int y, int dx[], int dy[], int color, int directions) {
    for(int k=0;k<directions;k++) { //For each movable line

        //Calculate new positions
        int newX = x + dx[k];
        int newY = y + dy[k];

        while(inBounds(newX) && inBounds(newY)) {
            //Empty square => add possible + continue
            if(board[newX][newY] == 0) {
                board[newX][newY] |=  MOVABLE_MASK;
            }

            //Piece on square => add possible + break 
            else if(opposingColor(board[newX][newY], color)) {
                board[newX][newY] |=  MOVABLE_MASK;
                break;
            }
            else {
                break;
            }

            //Calculate new pos on line
            newX += dx[k];
            newY += dy[k];
        }
    }
}

//Removes all possible set moves
void clearPossibleBoard(unsigned char board[8][8]) {
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            board[i][j] = board[i][j] & (~MOVABLE_MASK);
        }
    }
}

void generatePossibleMoves(unsigned char board[8][8], int x, int y) {
    clearPossibleBoard(board);
    
    int type = board[x][y] & TYPE_MASK;
    int color = (board[x][y] & 0x10) >> 4;
    //color is 1 if piece is black, 0 if its white

    switch(type) {
        case PAWN:
            int direction = (color == 1) ? 1 : -1;
            //if dir is 1 -> piece is black => move down
            //if dir is 0 -> piece is white => move up
            int newX = x+direction;
            //I.Move forward
            if((board[newX][y] & 0x7) == 0) {
                board[newX][y] |=  MOVABLE_MASK;
            }

            //II.Capture (diagonally)
            int newY = y-1;
            if(inBounds(newY)) {
                if(board[newX][newY] != 0 && opposingColor(board[newX][newY], color)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
            }

            newY = y+1;
            if(inBounds(newY)) {
                if(board[newX][newY] != 0 && opposingColor(board[newX][newY], color)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
            }

            break;
        case KNIGHT:
            //Possible knight moves
            int jumpDX[8] = {-2, -2, -1, -1,  1,  1,  2,  2};
            int jumpDY[8] = {-1,  1, -2,  2, -2,  2, -1,  1};
            generateStepMoves(board, x, y, jumpDX, jumpDY, color, 8);
            break;
        case BISHOP:
            int diagDX[4] = {-1, -1,  1, 1};
            int diagDY[4] = {-1,  1, -1, 1};
            generateLongMoves(board, x, y, diagDX, diagDY, color, 4);
            break;
        case ROOK:
            int linesDX[4] = {-1,  1,  0,  0};
            int linesDY[4] = { 0,  0,  1, -1};
            generateLongMoves(board, x, y, linesDX, linesDY, color, 4);
            break;
        case QUEEN:
            int queenDX[8] = {-1, -1, -1,  0,  0,  1,  1,  1};
            int queenDY[8] = {-1,  0,  1, -1,  1, -1,  0,  1};
            generateLongMoves(board, x, y, queenDX, queenDY, color, 8);
            break;
        case KING:
            int kingDX[8] = {-1, -1, -1,  0,  0,  1,  1,  1};
            int kingDY[8] = {-1,  0,  1, -1,  1, -1,  0,  1};
            generateStepMoves(board, x, y, kingDX, kingDY, color, 8);
            break;
    }
}