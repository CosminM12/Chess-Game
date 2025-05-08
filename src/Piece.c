// bafta
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Piece.h"
#include "RenderWindow.h"


/*
==========================
=INITIALIZATION FUNCTIONS=
==========================
*/
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
    int pieceColor = (piece & COLOR_MASK) >> 4;
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
            piece = isupper(c) ? COLOR_MASK : 0;
            //get the type (last 3 bits)
            c = tolower(c);
            switch(c) {
                case 'p': {
                    piece = piece | PAWN;
                    break;
                }
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

void exportPosition(unsigned char board[8][8], char **exportString) {
    *exportString = malloc(73*sizeof(char)); //maximum 8x8 + 8 row-limiters + \0 = 73
    int numOfEmptySpaces = 0, cnt = 0;
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            unsigned char color = board[i][j] & COLOR_MASK;
            if(piece == NONE) {
                numOfEmptySpaces++;
            }
            else {
                if(numOfEmptySpaces != 0) {
                    (*exportString)[cnt++] = (char)(numOfEmptySpaces + '0');
                    numOfEmptySpaces = 0;
                }
                switch(piece) {
                    case PAWN:
                        (*exportString)[cnt++] = 'p'; 
                        break;
                    case BISHOP:
                        (*exportString)[cnt++] = 'b';
                        break;
                    case KNIGHT:
                        (*exportString)[cnt++] = 'n';
                        break;
                    case ROOK:
                        (*exportString)[cnt++] = 'r';
                        break;
                    case QUEEN:
                        (*exportString)[cnt++] = 'q';
                        break;
                    case KING:
                        (*exportString)[cnt++] = 'k';
                        break;
                    default:
                        (*exportString) = NULL;
                        return;
                }
                if(color != 0) {
                    (*exportString)[cnt-1] = toupper((*exportString)[cnt-1]);
                }
            }
        }
        if(i<7) {
            (*exportString)[cnt++] = '/';
            numOfEmptySpaces = 0;
        }
    }
    exportString[cnt] = '\0';
}

void findKings(unsigned char board[8][8], Vector2f kingsPositions[]) {
    int foundKings = 0;
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            if((board[i][j] & TYPE_MASK) == KING) {
                board[i][j] |= MODIFIER; //Initialize ability to castle

                foundKings++;
                unsigned int color = (board[i][j] & COLOR_MASK) >> 4;
                
                kingsPositions[color].x = i;
                kingsPositions[color].y = j;
                
                if(foundKings == 2) {
                    return;
                }
            }
        }
    }
}

/*
==========================
=       CHECKERS         =
==========================
*/
bool isCheck(unsigned char board[8][8], Vector2f kingPosition) {
    int kingX = kingPosition.x;
    int kingY = kingPosition.y;

    unsigned char color = (board[kingX][kingY] & COLOR_MASK) >> 4;
    unsigned char enemyColor = (board[kingX][kingY] & COLOR_MASK) ^ COLOR_MASK;
    printf("Color: 0x%X, enemyColor: 0x%X\n", color, enemyColor);

    int pawnDY[2] = {-1, 1};

    int knightDX[8] = {-2, -2, -1, -1,  1,  1,  2,  2};
    int knightDY[8] = {-1,  1, -2,  2, -2,  2, -1,  1};

    int diagDX[4] = {-1, -1,  1,  1};
    int diagDY[4] = {-1,  1, -1,  1};

    int lineDX[4] = {-1,  0,  0,  1};
    int lineDY[4] = { 0, -1,  1,  0};

    int newX, newY;
    unsigned char pieceType;

    //I. Pawn threat
    int direction = (enemyColor == 1) ? -1 : 1;
    newX = kingX + direction;
    if(inBounds(newX)) {
        for(int i=0;i<2;i++) {
            newY = kingY + pawnDY[i];
            if(inBounds(newY)) {
                if((board[newX][newY] & TYPE_MASK) == PAWN && (board[newX][newY] & COLOR_MASK) == enemyColor) {
                    printf("Dir=%d, dy[i]=%d\n", direction, pawnDY[i]);
                    printf("\nPawn threat(%d, %d): ", newX, newY);
                    return true;
                }
            }
        }
    }

    //II. Knight threat
    for(int i=0;i<8;i++) {
        newX = kingX + knightDX[i];
        newY = kingY + knightDY[i];
        if(inBounds(newX) && inBounds(newY)) {
            if((board[newX][newY] & TYPE_MASK) == KNIGHT && (board[newX][newY] & COLOR_MASK) == enemyColor) {
                printf("\nKnight threat: ");
                return true;
            }
        }
    }

    //III. Diagonal threat (bishop / queen)
    for(int i=0;i<4;i++) {
        newX = kingX + diagDX[i];
        newY = kingY + diagDY[i];

        while(inBounds(newX) && inBounds(newY)) {
            pieceType = board[newX][newY] & TYPE_MASK;
            if(pieceType != 0) {
                if((board[newX][newY] & COLOR_MASK) == enemyColor && (pieceType == BISHOP || pieceType == QUEEN)) {
                    printf("\nDiagonal threat: ");
                    return true;
                }
                
                //exit if there is another piece on diagonal
                break;
            }

            newX += diagDX[i];
            newY += diagDY[i];
        }
    }

    //IV. Line threats (rook / queen)
    for(int i=0;i<4;i++) {
        newX = kingX + lineDX[i];
        newY = kingY + lineDY[i];

        while(inBounds(newX) && inBounds(newY)) {
            pieceType = board[newX][newY] & TYPE_MASK;
            if(pieceType != 0) {
                if((board[newX][newY] & COLOR_MASK) == enemyColor && (pieceType == ROOK || pieceType == QUEEN)) {
                    printf("\nLine threat(%d, %d): ", newX, newY);
                    return true;
                }

                //exit if there is another piece on line
                break;
            }

            newX += lineDX[i];
            newY += lineDY[i];
        }
    }

    //V. King threat (for possible moves)
    for(int dx=-1;dx<2;dx++) {
        for(int dy=-1;dy<2;dy++) {
            newX = kingX + dx;
            newY = kingY + dy;
            if(inBounds(newX) && inBounds(newY)) {
                if((board[newX][newY] & TYPE_MASK) == KING && (board[newX][newY] & COLOR_MASK) == enemyColor) {
                    printf("\nKing threat: ");
                    return true;
                }
            }
        }
    }

    return false;
}

//Checks if calc position is on board
bool inBounds(int var) {
    return -1 < var && var < 8; 
}

//Checks if piece is of another color
bool opposingColor(unsigned char piece, int color) {
    return ((piece & COLOR_MASK) >> 4) != color;
}


/*
==========================
=    MOVE GENERATION     =
==========================
*/
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

void generateAllPossibleMoves(unsigned char board[8][8], unsigned char color, Vector2f *lastDoublePawn) {
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            if((board[i][j] & TYPE_MASK) != 0 && (board[i][j] & COLOR_MASK) == color) {
                generatePossibleMoves(board, i, j, lastDoublePawn);
            }
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

void generatePossibleMoves(unsigned char board[8][8], int x, int y, Vector2f *lastDoublePawn) {
    clearPossibleBoard(board);
    
    unsigned int type = board[x][y] & TYPE_MASK;
    unsigned int color = (board[x][y] & COLOR_MASK) >> 4;
    //color is 1 if piece is black, 0 if its white

    switch(type) {
        case PAWN: {
            //------Single step movement------

            //if dir is 1 -> piece is black => move down
            //if dir is 0 -> piece is white => move up
            int direction = (color == 1) ? 1 : -1;
            int newX = x + direction;

            //I.Move forward
            if ((board[newX][y] & TYPE_MASK) == 0) {
                board[newX][y] |= MOVABLE_MASK;
            }

            //II.Capture (diagonally) + En passant
            int dy[2] = {-1, 1};
            int newY;
            for (int i = 0; i < 2; i++) {
                newY = y + dy[i];
                if (inBounds(newY)) {
                    if (board[newX][newY] != 0 && opposingColor(board[newX][newY], color)) {
                        board[newX][newY] |= MOVABLE_MASK;
                    }
                }
            }


            //------En passant------
            for (int i = 0; i < 2; i++) {
                newY = y + dy[i];
                if ((board[x][newY] & TYPE_MASK) == PAWN && x == (int) (*lastDoublePawn).y &&
                    newY == (int) (*lastDoublePawn).x && opposingColor(board[x][newY], color)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
            }


            //------Double pawn push------
            if ((board[x][y] & MODIFIER) == 0) { //pawn hasn't moved
                newX = x + 2 * direction;

                if ((board[newX][y] & TYPE_MASK) == 0 &&
                    (board[newX - direction][y] & TYPE_MASK) == 0) { //no piece on route
                    board[newX][y] |= MOVABLE_MASK;
                }
            }

            break;
        }
        case KNIGHT: {
            //Possible knight moves
            int jumpDX[8] = {-2, -2, -1, -1, 1, 1, 2, 2};
            int jumpDY[8] = {-1, 1, -2, 2, -2, 2, -1, 1};
            generateStepMoves(board, x, y, jumpDX, jumpDY, color, 8);
            break;
        }
            
        case BISHOP: {
            int diagDX[4] = {-1, -1, 1, 1};
            int diagDY[4] = {-1, 1, -1, 1};
            generateLongMoves(board, x, y, diagDX, diagDY, color, 4);
            break;
        }

        case ROOK: {
            int linesDX[4] = {-1, 1, 0, 0};
            int linesDY[4] = {0, 0, 1, -1};
            generateLongMoves(board, x, y, linesDX, linesDY, color, 4);
            break;
        }

        case QUEEN: {
            int queenDX[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
            int queenDY[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
            generateLongMoves(board, x, y, queenDX, queenDY, color, 8);
            break;
        }

        case KING: {
            int kingDX[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
            int kingDY[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
            generateStepMoves(board, x, y, kingDX, kingDY, color, 8);
            break;
        }
    }
}
