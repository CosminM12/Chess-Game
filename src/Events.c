#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

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

void makeMove(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[]) {
    int oldX = (*sourceSquare).x;
    int oldY = (*sourceSquare).y;

    //copy piece to new location (only last 5 bits);
    board[destY][destX] = (board[oldY][oldX] & 0x1F);
    
    //delete from old position
    board[oldY][oldX] = 0;

    //delte selected square
    (*sourceSquare).x = -1.0f;
    (*sourceSquare).y = -1.0f;

    pieceActions[0] = false;
    pieceActions[1] = false;
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


void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], Vector2f* selectedSquare) {
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
            if(board[squareY][squareX] != 0) {
                selectAndHold(board, squareX, squareY, pieceActions, selectedSquare);

                generatePossibleMoves(board, squareY, squareX);
            }
        }
        //A SELECTED PIECE  => TRY TO MOVE
        else {

            //VALID MOVE => MOVE PIECE
            if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(board, squareX, squareY, selectedSquare, pieceActions);
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
                    makeMove(board,squareX, squareY, selectedSquare, pieceActions);
                    clearPossibleBoard(board);
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
            makeMove(board,squareX, squareY, selectedSquare, pieceActions);
            }
            else {
                deselectPiece(board, selectedSquare, pieceActions);
                clearPossibleBoard(board);
            }  
        }
    }
}