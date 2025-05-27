#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL_mixer.h>
#include "Events.h"
#include "Piece.h"
#include "GameState.h"

const int scrollStep = 20;

void addMoveToHistory(int startRow, int startCol, int endRow, int endCol, unsigned char piece) {
    if (moveCount >= MAX_MOVES) return;

    const char *pieceChar;
    switch (piece & TYPE_MASK) {
        case PAWN:   pieceChar = "";  break;
        case KNIGHT: pieceChar = "N"; break;
        case BISHOP: pieceChar = "B"; break;
        case ROOK:   pieceChar = "R"; break;
        case QUEEN:  pieceChar = "Q"; break;
        case KING:   pieceChar = "K"; break;
        default:     pieceChar = "?"; break;
    }

    char from[3] = { 'a' + startCol, '8' - startRow, '\0' };
    char to[3] = { 'a' + endCol, '8' - endRow, '\0' };

    snprintf(moveHistory[moveCount].notation, sizeof(moveHistory[moveCount].notation),
             "%s%s%s", pieceChar, from, to);

    moveCount++;
}

void getEvents(SDL_Event event, bool *gameRunning, bool mouseActions[], int *scrollOffset) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *gameRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseActions[0] = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mouseActions[1] = true;
                }
                break;
            case SDL_MOUSEWHEEL:
                *scrollOffset -= event.wheel.y * scrollStep;
                if (*scrollOffset < 0) *scrollOffset = 0;

                int maxScroll = (moveCount * 25) - screenHeight;
                if (maxScroll < 0) maxScroll = 0;
                if (*scrollOffset > maxScroll) *scrollOffset = maxScroll;
                break;
        }
    }
}

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize) {
//    int boardOffset = (screenWidth - (8 * squareSize)) / 2;
    int boardOffset = 0;
    return boardOffset < mouseX && mouseX < (screenWidth - boardOffset);
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

void makeMove(unsigned char board[8][8],
              int destX,
              int destY,
              Vector2f *sourceSquare,
              bool pieceActions[],
              Vector2f *lastDoublePawn,
              Vector2f kingsPositions[],
              unsigned char *capturedByWhite,
              unsigned char *capturedByBlack,
              int *capturedWhiteCount,
              int *capturedBlackCount) {

    Mix_Chunk* moveSound = Mix_LoadWAV("../res/sfx/move-self.mp3");
    Mix_PlayChannel(1, moveSound, 0);

    int oldX = (*sourceSquare).x;
    int oldY = (*sourceSquare).y;

    //check if any piece is captured
    bool capture = (board[destY][destX] & TYPE_MASK) != 0;

    if (capture) {
        unsigned char capturedPiece = board[destY][destX];

        if ((board[oldY][oldX] & COLOR_MASK) == 0x10) {
            // White moved => captured a black piece
            if (*capturedWhiteCount < MAX_CAPTURED) {
                capturedByWhite[(*capturedWhiteCount)++] = capturedPiece;
            }
        } else if ((board[oldY][oldX] & COLOR_MASK) == 0x20) {
            // Black moved => captured a white piece
            if (*capturedBlackCount < MAX_CAPTURED) {
                capturedByBlack[(*capturedBlackCount)++] = capturedPiece;
            }
        }
    }


    // copy piece to new location (only last 5 bits);
    board[destY][destX] = (board[oldY][oldX] & 0x1F);

    addMoveToHistory(oldY, oldX, destY, destX, board[destY][destX]);


    //---Handle pawn special moves---
    if ((board[destY][destX] & TYPE_MASK) == PAWN) {

        //Set modifier (pawn can't double push anymore)
        board[destY][destX] |= MODIFIER;


        //double pawn push =>  remember for 'En passant'
        if (abs(destY - oldY) == 2) {
            (*lastDoublePawn).x = destX;
            (*lastDoublePawn).y = destY;
        } else {
            int lastDoubleX = (*lastDoublePawn).x;
            int lastDoubleY = (*lastDoublePawn).y;

            //En passant => delete captured piece
            if (!capture && abs(destX - oldX) == 1) {
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

    unsigned char color = board[destY][destX] & COLOR_MASK;

    if ((board[destY][destX] & TYPE_MASK) == KING) {
        if (color == COLOR_MASK) {
            printf("Black ");
        } else {
            printf("White ");
        }
        printf("kings has moved!\n");
        kingsPositions[color].x = destY;
        kingsPositions[color].y = destX;
    }

    unsigned int nextColor = (board[destY][destX] & COLOR_MASK) == 0 ? 1 : 0;


    if (isCheck(board, kingsPositions[nextColor])) {
        printf("In check!\n");
    }
}

void deselectPiece(unsigned char board[8][8], Vector2f *selectedSquare, bool pieceActions[]) {
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

void handleMouseInput(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize,
                      bool mouseActions[], bool pieceActions[], bool *blackTurn, Vector2f *selectedSquare,
                      Vector2f *lastDoublePawn, Vector2f kingsPositions[]) {
    //Get square from mouse cursor
//    int boardOffset = (screenWidth - (8 * squareSize)) / 2;
    int boardOffset = 0;
    int squareX = (int) ((mouseX - boardOffset) / squareSize);
    int squareY = (int) (mouseY / squareSize);

    /*=================
    bool vector arrays
    mouseActions[]:
    0: mousePressed -----> click button is pressed
    1: mouseReleased -----> click button is released

    pieceActions[]:
    0: pieceSelected ----> piece is selected (has been pressed on and (maybe) released)
    1: pieceHolding ----> piece is holded with mouse click and follows the cursor
    =================*/

    if (mouseActions[0]) { //MOUSE CLICKED
        //NO SELECTED PIECE => SELECT
        if (!pieceActions[0]) {
            if (board[squareY][squareX] != 0 && !opposingColor(board[squareY][squareX], *blackTurn)) {
                selectAndHold(board, squareX, squareY, pieceActions, selectedSquare);

                generatePossibleMoves(board, squareY, squareX, lastDoublePawn);
            }
        }
            //A SELECTED PIECE  => TRY TO MOVE
        else {

            //VALID MOVE => MOVE PIECE
            if ((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions,
                         capturedByWhite, capturedByBlack, &capturedWhiteCount, &capturedBlackCount);
                *blackTurn = !(*blackTurn);
                clearPossibleBoard(board);
            }

                //NOT VALID => CANCEL SELECT
            else {
                // deselectPiece(board, selectedSquare, pieceActions);
                //clearPossibleBoard(board);
            }
        }
    } else if (mouseActions[1]) { //MOUSE RELEASED

        //HOLDING PIECE
        if (pieceActions[1]) {

            //dest=src => STOP HOLD
            if (squareX == (*selectedSquare).x && squareY == (*selectedSquare).y) {
                pieceActions[1] = false;
            }

                //different dest => MOVE or DESELECT
            else {
                if ((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions,
                             capturedByWhite, capturedByBlack, &capturedWhiteCount, &capturedBlackCount);
                    *blackTurn = !(*blackTurn);
                    clearPossibleBoard(board);
                } else {
                    deselectPiece(board, selectedSquare, pieceActions);
                    clearPossibleBoard(board);
                }
            }
        }
            //NOT HOLDING =>
        else {
            if ((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions,
                         capturedByWhite, capturedByBlack, &capturedWhiteCount, &capturedBlackCount);
                *blackTurn = !(*blackTurn);
            } else {
                deselectPiece(board, selectedSquare, pieceActions);
                clearPossibleBoard(board);
            }
        }
    }
}