#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

#include "Events.h"
// #include "util.h"

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

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], Vector2f* selectedSquare) {
    //Get square from mouse cursor
    int boardOffset = (screenWidth - (8 * squareSize)) / 2;
    int squareX = (int)((mouseX - boardOffset) / squareSize);
    int squareY = (int)(mouseY / squareSize);

    /*=================
    bool vector maps
    mouseActions[]:
    0: mousePressed -----> click button is pressed
    1: mouseReleased -----> click button is released

    pieceActions[]:
    0: pieceSelected ----> piece is selected (has been pressed on and (maybe) released)
    1: pieceHolding ----> piece is holded with mouse click and follows the cursor
    =================*/

    if(mouseActions[0]) { //if mouse clicked
        if(!pieceActions[0]) { //if there is no selected piece
            if(board[squareY][squareX] != 0) {
                pieceActions[0] = true;
                pieceActions[1] = true;

                //change square to be selected
                board[squareY][squareX] = board[squareY][squareX] | (0x1 << 5);
                (*selectedSquare).x = squareX;
                (*selectedSquare).y = squareY;
            }
        }
        else {
            //if valid move, then move
            if(board[squareY][squareX] == 0) {
                int oldX = (*selectedSquare).x;
                int oldY = (*selectedSquare).y;
                board[squareY][squareX] = board[oldY][oldX]  & (~(0x1 << 5)); //move piece to new pos + deselect
                board[oldY][oldX] = 0;

                (*selectedSquare).x = -1.0f;
                (*selectedSquare).y = -1.0f;
                pieceActions[0] = false;
                pieceActions[1] = false;
            }
            else { //either capture or cancel select 
                /*if capture to be inserted*/
                //else:
                int oldX = (*selectedSquare).x;
                int oldY = (*selectedSquare).y;
                board[oldY][oldX] = board[oldY][oldX] & (~(0x1 << 5));
                (*selectedSquare).x = -1.0f;
                (*selectedSquare).y = -1.0f;
                pieceActions[0] = false;
                pieceActions[1] = false;
            }
        }
    }
    else if(mouseActions[1]) { //mouse released
        if(pieceActions[1]) { //piece holding
            if(squareX == (*selectedSquare).x && squareY == (*selectedSquare).y) {
                pieceActions[1] = false;
            }
            else {
                if(board[squareY][squareX] == 0) {
                    int oldX = (*selectedSquare).x;
                    int oldY = (*selectedSquare).y;
                    board[squareY][squareX] = board[oldY][oldX] & (~(0x1 << 5)); //move piece to new pos + deselect
                    board[oldY][oldX] = 0;
                }
                (*selectedSquare).x = -1.0f;
                (*selectedSquare).y = -1.0f;
                pieceActions[0] = false;
                pieceActions[1] = false;
            }
        }
    }
}
    // bool pressed = mouseActions[0], released = mouseActions[1], dragging = mouseActions[2];
    // // if((int)board[squareY][squareX] != 0) {
    // //     board[squareY][squareX] = board[squareY][squareX] | (0x1 << 5);
    // // }

    // if(pressed) { //is mouse pressed
    //     if((int)(board[squareY][squareX] != 0)) { //exists a piece where we pressed the mouse
    //         if((*selectedSquare).x == -1 && (*selectedSquare).y == -1) { //no piece selected
    //             (*selectedSquare).x = squareX;
    //             (*selectedSquare).y = squareY;

    //             board[squareY][squareX] = board[squareY][squareX] | (0x1 << 5); //select square
    //         }
    //         else if((*selectedSquare).x == squareX && (*selectedSquare).y == squareY) { //deselect piece
    //             (*selectedSquare).x = -1.0f;
    //             (*selectedSquare).y = -1.0f;

    //             board[squareY][squareX] = board[squareY][squareX] & (~(0x1 << 5)); //deselect square
    //         }
    //         else {
    //             //piece capturing

    //         }
    //     }
    //     else {
    //         if((*selectedSquare).x != -1 && (*selectedSquare).y != -1) { //move piece here
    //             board[squareY][squareX] = board[(int)(*selectedSquare).y][(int)(*selectedSquare).x];
    //             board[(int)(*selectedSquare).y][(int)(*selectedSquare).x] = board[(int)(*selectedSquare).y][(int)(*selectedSquare).x] & (~(0x1 << 5));
    //         }
    //     }
    // }
    // else if(released) {
    //     if((*selectedSquare).x != -1 && (*selectedSquare).y != -1) {
            
    //     }
    // }
