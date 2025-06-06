#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "Piece.h"
#include "RenderWindow.h"

// Function prototypes
bool wouldLeaveKingInCheck(unsigned char board[8][8], int fromX, int fromY, int toX, int toY, Vector2f kingsPositions[]);


/*
==========================
=INITIALIZATION FUNCTIONS=
==========================
*/
void initCastlingRights(unsigned char board[8][8]) {
    // Set the MODIFIER flag on kings and rooks for castling rights
    // This should be called only once at the start of the game
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            // Initialize kings
            if((board[i][j] & TYPE_MASK) == KING) {
                board[i][j] |= MODIFIER; // Set MODIFIER for castling
            }
            
            // Initialize rooks for castling
            if((board[i][j] & TYPE_MASK) == ROOK) {
                board[i][j] |= MODIFIER; // Set MODIFIER for castling
            }
        }
    }
}

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
    
    // Find king positions without modifying any flags
    // This prevents resetting castling rights each time this function is called
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            // Find kings and update positions
            if((board[i][j] & TYPE_MASK) == KING) {
                // Get king color and update its position
                unsigned int color = (board[i][j] & COLOR_MASK) >> 4;
                kingsPositions[color].x = i;
                kingsPositions[color].y = j;
                
                foundKings++;
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
bool isCheck(unsigned char board[8][8], Vector2f kingPos) {
    int kingX = kingPos.x;
    int kingY = kingPos.y;
    
    // Check if the position is even a king
    if ((board[kingX][kingY] & TYPE_MASK) != KING) {
        return false;
    }
    
    // Get the color of the opponent
    unsigned char kingColor = (board[kingX][kingY] & COLOR_MASK) >> 4;
    unsigned char opponentColor = 1 - kingColor;
    
    // Check if any enemy piece can attack the king's position
    return isSquareAttacked(board, kingPos, opponentColor);
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
    // Find kings positions for check validation
    Vector2f kingsPositions[2];
    findKings(board, kingsPositions);
    
    for(int k=0;k<directions;k++) {
        int newX = x + dx[k];
        int newY = y + dy[k];

        if(inBounds(newX) && inBounds(newY)) {
            if(board[newX][newY] == 0 || opposingColor(board[newX][newY], color)) {
                // Check if this move would leave the king in check
                if (!wouldLeaveKingInCheck(board, y, x, newY, newX, kingsPositions)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
            }
        }
    }
}

//Generate moves on lines, colomns and diagonals
void generateLongMoves(unsigned char board[8][8], int x, int y, int dx[], int dy[], int color, int directions) {
    // Find kings positions for check validation
    Vector2f kingsPositions[2];
    findKings(board, kingsPositions);
    
    for(int k=0;k<directions;k++) { //For each movable line

        //Calculate new positions
        int newX = x + dx[k];
        int newY = y + dy[k];

        while(inBounds(newX) && inBounds(newY)) {
            //Empty square => add possible + continue
            if(board[newX][newY] == 0) {
                // Check if this move would leave the king in check
                if (!wouldLeaveKingInCheck(board, y, x, newY, newX, kingsPositions)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
            }

            //Piece on square => add possible + break 
            else if(opposingColor(board[newX][newY], color)) {
                // Check if this move would leave the king in check
                if (!wouldLeaveKingInCheck(board, y, x, newY, newX, kingsPositions)) {
                    board[newX][newY] |= MOVABLE_MASK;
                }
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
            board[i][j] = board[i][j] & (~MOVABLE_MASK) & (~RISKY_MOVE_MASK);
        }
    }
}

// Helper function to check if a move is legal (doesn't leave the king in check)
bool wouldLeaveKingInCheck(unsigned char board[8][8], int fromX, int fromY, int toX, int toY, Vector2f kingsPositions[]) {
    unsigned char tempBoard[8][8];
    Vector2f tempKingsPositions[2] = {kingsPositions[0], kingsPositions[1]};
    
    // Copy the board to a temporary board
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            tempBoard[i][j] = board[i][j];
        }
    }
    
    // Get the color and type of the moving piece
    unsigned char movingPiece = board[fromY][fromX];
    unsigned int pieceType = movingPiece & TYPE_MASK;
    unsigned int color = (movingPiece & COLOR_MASK) >> 4;
    
    // Make the move on the temporary board
    tempBoard[toY][toX] = tempBoard[fromY][fromX];
    tempBoard[fromY][fromX] = 0;
    
    // Update king position if king is moving
    if(pieceType == KING) {
        tempKingsPositions[color].x = toY;
        tempKingsPositions[color].y = toX;
    }
    
    // Check if the king is in check after the move
    return isCheck(tempBoard, tempKingsPositions[color]);
}

void generatePossibleMoves(unsigned char board[8][8], int x, int y, Vector2f *lastDoublePawn) {
    clearPossibleBoard(board);
    
    unsigned int type = board[x][y] & TYPE_MASK;
    unsigned int color = (board[x][y] & COLOR_MASK) >> 4;
    //color is 1 if piece is black, 0 if its white
    
    // Find kings positions for check validation
    Vector2f kingsPositions[2];
    findKings(board, kingsPositions);

    switch(type) {
        case PAWN:
            //------Single step movement------

            //if dir is 1 -> piece is black => move down
            //if dir is 0 -> piece is white => move up
            int direction = (color == 1) ? 1 : -1;
            int newX = x+direction;
            
            //I.Move forward
            if((board[newX][y] & TYPE_MASK) == 0) {
                board[newX][y] |=  MOVABLE_MASK;
            }

            //II.Capture (diagonally) + En passant
            int dy[2] = {-1, 1};
            int newY;
            for(int i=0;i<2;i++) {
                newY = y + dy[i];
                if(inBounds(newY)) {
                    if(board[newX][newY] != 0 && opposingColor(board[newX][newY], color)) {
                        board[newX][newY] |= MOVABLE_MASK;
                    }
                }
            }


            //------En passant------
            for(int i=0;i<2;i++) {
                newY = y + dy[i];
                if(inBounds(newY)) {
                    // Check if there's a pawn next to us and if it made a double move last turn
                    if((board[x][newY] & TYPE_MASK) == PAWN && 
                       opposingColor(board[x][newY], color) &&
                       (*lastDoublePawn).x == newY && (*lastDoublePawn).y == x) {
                        // Mark the square behind the pawn as a valid capture square
                        board[newX][newY] |= MOVABLE_MASK;
                    }
                }
            }


            //------Double pawn push------
            if((board[x][y] & MODIFIER) == 0) { //pawn hasn't moved
                newX = x + 2*direction;

                if((board[newX][y] & TYPE_MASK) == 0 && (board[newX-direction][y] & TYPE_MASK) == 0) { //no piece on route
                    board[newX][y] |= MOVABLE_MASK;
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
            // Diagonal directions: top-left, top-right, bottom-left, bottom-right
            int diagDX[4] = {-1, -1,  1,  1};
            int diagDY[4] = {-1,  1, -1,  1};
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
            
            // Check for castling possibilities
            // King must not have moved before (MODIFIER flag must be set during initialization and cleared on first move)
            if ((board[x][y] & MODIFIER) != 0) {
                // Get king's color
                unsigned char kingColor = (board[x][y] & COLOR_MASK) >> 4;
                unsigned char enemyColor = 1 - kingColor;
                
                // Check kingside castling (short castle)
                if (y + 3 < 8 && 
                    (board[x][y+3] & TYPE_MASK) == ROOK && 
                    (board[x][y+3] & COLOR_MASK) == (board[x][y] & COLOR_MASK) &&
                    (board[x][y+3] & MODIFIER) != 0 &&  // Rook must not have moved
                    (board[x][y+1] & TYPE_MASK) == 0 && 
                    (board[x][y+2] & TYPE_MASK) == 0) {
                    
                    // Check if king is in check
                    Vector2f kingPos = {x, y};
                    if (!isSquareAttacked(board, kingPos, enemyColor)) {
                        // Check if the square the king passes through is under attack
                        Vector2f passThrough = {x, y+1};
                        if (!isSquareAttacked(board, passThrough, enemyColor)) {
                            // Check if the destination square is under attack
                            Vector2f destination = {x, y+2};
                            if (!isSquareAttacked(board, destination, enemyColor) && 
                                !wouldLeaveKingInCheck(board, y, x, y+2, x, kingsPositions)) {
                                // King can castle kingside
                                board[x][y+2] |= MOVABLE_MASK;
                            }
                        }
                    }
                }
                
                // Check queenside castling (long castle)
                if (y - 4 >= 0 && 
                    (board[x][y-4] & TYPE_MASK) == ROOK && 
                    (board[x][y-4] & COLOR_MASK) == (board[x][y] & COLOR_MASK) &&
                    (board[x][y-4] & MODIFIER) != 0 &&  // Rook must not have moved
                    (board[x][y-1] & TYPE_MASK) == 0 && 
                    (board[x][y-2] & TYPE_MASK) == 0 && 
                    (board[x][y-3] & TYPE_MASK) == 0) {
                    
                    // Check if king is in check
                    Vector2f kingPos = {x, y};
                    if (!isSquareAttacked(board, kingPos, enemyColor)) {
                        // Check if the square the king passes through is under attack
                        Vector2f passThrough = {x, y-1};
                        if (!isSquareAttacked(board, passThrough, enemyColor)) {
                            // Check if the destination square is under attack
                            Vector2f destination = {x, y-2};
                            if (!isSquareAttacked(board, destination, enemyColor) && 
                                !wouldLeaveKingInCheck(board, y, x, y-2, x, kingsPositions)) {
                                // King can castle queenside
                                board[x][y-2] |= MOVABLE_MASK;
                            }
                        }
                    }
                }
            }
            break;
    }
}