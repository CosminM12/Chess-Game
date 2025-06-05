#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <math.h>

#include "Events.h"
#include "Piece.h"


void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[]) {
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                *gameRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    mouseActions[0] = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    mouseActions[1] = true;
                }
                break;
        }
    }
}

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize) {
    int boardOffset = (screenWidth - (8 * squareSize)) / 2;
    return boardOffset < mouseX  && mouseX < (screenWidth - boardOffset);
}

void selectAndHold(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare) {
    //toggle active: hold and select
    pieceActions[0] = true;
    pieceActions[1] = true;

    //change square to selected
    board[squareY][squareX] |= SELECTED_MASK;

    //save selected square coords
    (*selectedSquare).x = squareX;
    (*selectedSquare).y = squareY;
}

void makeMove(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[], Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    int oldX = (*sourceSquare).x;
    int oldY = (*sourceSquare).y;

    //check if any piece is captured
    bool capture = (board[destY][destX] & TYPE_MASK) != 0;

    //copy piece to new location (preserve color and type)
    board[destY][destX] = (board[oldY][oldX] & (TYPE_MASK | COLOR_MASK));
    
    //---Handle pawn special moves---
    if((board[destY][destX] & TYPE_MASK) == PAWN) {

        //Set modifier (pawn can't double push anymore) 
        board[destY][destX] |= MODIFIER;

        
        //double pawn push =>  remember for 'En passant'
        if(abs(destY - oldY) == 2) {
            (*lastDoublePawn).x = destX;
            (*lastDoublePawn).y = destY;
        }
        else {
            int lastDoubleX = (*lastDoublePawn).x;
            int lastDoubleY = (*lastDoublePawn).y;
            
            //En passant => delete captured piece
            if(!capture && abs(destX - oldX) == 1) {
                board[lastDoubleY][lastDoubleX] = 0;
            }

            //remove last double pawn
            (*lastDoublePawn).x = -1.0f;
            (*lastDoublePawn).y = -1.0f;
        }
    }

    //delete from old position
    board[oldY][oldX] = 0;

    //delte selected square
    (*sourceSquare).x = -1.0f;
    (*sourceSquare).y = -1.0f;

    pieceActions[0] = false;
    pieceActions[1] = false;

    unsigned char color = (board[destY][destX] & COLOR_MASK) >> 4;

    // Update king position if a king was moved
    if((board[destY][destX] & TYPE_MASK) == KING) {
        if(color == 1) {
            printf("Black ");
        }
        else {
            printf("White ");
        }
        printf("kings has moved!\n");
        kingsPositions[color].x = destY;
        kingsPositions[color].y = destX;
    }

    unsigned int nextColor = color == 0 ? 1 : 0;

    
    if(isCheck(board, kingsPositions[nextColor])) {
        printf("In check!\n");
    }
}

void deselectPiece(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]) {
    int oldX = (*selectedSquare).x;
    int oldY = (*selectedSquare).y;

    //deselect piece
    board[oldY][oldX] &= (~SELECTED_MASK);

    //delete selected sqaure
    (*selectedSquare).x = -1.0f;;
    (*selectedSquare).y = -1.0f;

    pieceActions[0] = false;
    pieceActions[1] = false;
}

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], bool* blackTurn, Vector2f* selectedSquare, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    //Get square from mouse cursor
    int boardOffset = (screenWidth - (8 * squareSize)) / 2;
    int squareX = (int)((mouseX - boardOffset) / squareSize);
    int squareY = (int)(mouseY / squareSize);

    /*=================
    bool vector arrays
    mouseActions[]:
    0: mousePressed -----> click button is pressed
    1: mouseReleased -----> click button is released

    pieceActions[]:
    0: pieceSelected ----> piece is selected (has been pressed on and (maybe) released)
    1: pieceHolding ----> piece is holded with mouse click and follows the cursor
    =================*/

    if(mouseActions[0]) { //MOUSE CLICKED
        //NO SELECTED PIECE => SELECT
        if(!pieceActions[0]) {
            if(board[squareY][squareX] != 0 && !opposingColor(board[squareY][squareX], *blackTurn)) {
                selectAndHold(board, squareX, squareY, pieceActions, selectedSquare);

                // Generate legal moves that don't put the king in check
                generatePossibleMoves(board, squareY, squareX, lastDoublePawn);
                
                // Filter out moves that would leave the king in check
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        if ((board[i][j] & MOVABLE_MASK) == MOVABLE_MASK) {
                            // Create a move and check if it's legal
                            Move move = {{squareY, squareX}, {i, j}, board[i][j], false, 0};
                            unsigned char color = (board[squareY][squareX] & COLOR_MASK) >> 4;
                            
                            if (!isMoveLegal(board, move, color, lastDoublePawn)) {
                                // Remove the movable flag if the move is illegal
                                board[i][j] &= ~MOVABLE_MASK;
                            }
                        }
                    }
                }
            }
        }
        //A SELECTED PIECE  => TRY TO MOVE
        else {
            //VALID MOVE => MOVE PIECE
            if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions);
                *blackTurn = !(*blackTurn);
                clearPossibleBoard(board);
                
                // Check for checkmate or stalemate after the move
                unsigned char nextColor = *blackTurn ? 1 : 0;
                MoveList moveList;
                generateMoves(board, nextColor, &moveList, lastDoublePawn);
                
                if (moveList.count == 0) {
                    // Check if the king is in check
                    if (isCheck(board, kingsPositions[nextColor])) {
                        printf("Checkmate! %s wins!\n", nextColor == 1 ? "White" : "Black");
                    } else {
                        printf("Stalemate! Game is drawn.\n");
                    }
                }
            }
            //NOT VALID => CANCEL SELECT
            else { 
               // deselectPiece(board, selectedSquare, pieceActions);
                //clearPossibleBoard(board);
            }
        }
    }
    else if(mouseActions[1]) { //MOUSE RELEASED
    
        //HOLDING PIECE
        if(pieceActions[1]) {

            //dest=src => STOP HOLD
            if(squareX == (*selectedSquare).x && squareY == (*selectedSquare).y) {
                pieceActions[1] = false;
            }

            //different dest => MOVE or DESELECT
            else {
                if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions);
                    *blackTurn = !(*blackTurn);
                    clearPossibleBoard(board);
                    
                    // Check for checkmate or stalemate after the move
                    unsigned char nextColor = *blackTurn ? 1 : 0;
                    MoveList moveList;
                    generateMoves(board, nextColor, &moveList, lastDoublePawn);
                    
                    if (moveList.count == 0) {
                        // Check if the king is in check
                        if (isCheck(board, kingsPositions[nextColor])) {
                            printf("Checkmate! %s wins!\n", nextColor == 1 ? "White" : "Black");
                        } else {
                            printf("Stalemate! Game is drawn.\n");
                        }
                    }
                }
                else {
                    deselectPiece(board, selectedSquare, pieceActions);
                    clearPossibleBoard(board);
                }
            }
        }
        //NOT HOLDING => 
        else {
          if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
            makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions);
            *blackTurn = !(*blackTurn);
            clearPossibleBoard(board);
            
            // Check for checkmate or stalemate after the move
            unsigned char nextColor = *blackTurn ? 1 : 0;
            MoveList moveList;
            generateMoves(board, nextColor, &moveList, lastDoublePawn);
            
            if (moveList.count == 0) {
                // Check if the king is in check
                if (isCheck(board, kingsPositions[nextColor])) {
                    printf("Checkmate! %s wins!\n", nextColor == 1 ? "White" : "Black");
                } else {
                    printf("Stalemate! Game is drawn.\n");
                }
            }
            }
            else {
                deselectPiece(board, selectedSquare, pieceActions);
                clearPossibleBoard(board);
            }  
        }
    }
}