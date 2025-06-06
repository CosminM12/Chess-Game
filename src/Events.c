#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <math.h>

#include "Events.h"
#include "Piece.h"
#include "engine.h"
#include "main.h"
#include "RenderWindow.h"
#include <stdio.h>

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
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_e) {
                    // Toggle evaluation bar visibility
                    extern bool showEvaluationBar;
                    showEvaluationBar = !showEvaluationBar;
                }
                else if(event.key.keysym.sym == SDLK_ESCAPE) {
                    // Show menu when Escape is pressed
                    showMenu = true;
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

void makeMove(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[], Vector2f* lastDoublePawn, Vector2f kingsPositions[], bool* blackTurn) {
    int oldX = (*sourceSquare).x;
    int oldY = (*sourceSquare).y;

    //check if any piece is captured
    bool capture = (board[destY][destX] & TYPE_MASK) != 0;

    unsigned char pieceType = board[oldY][oldX] & TYPE_MASK;
    unsigned char color = (board[oldY][oldX] & COLOR_MASK) >> 4;
    
    // Check if we are dealing with a king or rook to track castling rights
    bool isKing = (pieceType == KING);
    bool isRook = (pieceType == ROOK);
    
    // For kings and rooks, always clear the MODIFIER flag to mark they've moved
    if (isKing || isRook) { 
        // Clear the MODIFIER flag from the piece before moving it
        board[oldY][oldX] &= ~MODIFIER;
    }
    
    // Check for pawn promotion
    bool isPromotion = (pieceType == PAWN) && (destY == 0 || destY == 7);
    unsigned char promotedPiece = 0;
    
    if (isPromotion) {
        // Get the renderer from the main window
        SDL_Renderer* renderer = getMainRenderer();
        
        // Load piece textures
        SDL_Texture* pieceTextures[2][7];
        loadPieceTextures(pieceTextures, &renderer);
        
        // Show promotion menu and get selected piece
        promotedPiece = showPromotionMenu(renderer, pieceTextures, destX, destY, color, 1200, 800);
        
        // Place the promoted piece
        board[destY][destX] = promotedPiece;
    } else {
        //copy piece to new location (preserve color and type)
        board[destY][destX] = (board[oldY][oldX] & (TYPE_MASK | COLOR_MASK));
        
        // For KING and ROOK, ensure MODIFIER flag is cleared after copying
        // This permanently disallows castling with this piece
        if (pieceType == KING || pieceType == ROOK) {
            // Clear the MODIFIER flag (set it to 0)
            board[destY][destX] &= ~MODIFIER;
        }
        // For other pieces that use MODIFIER (like pawns), set it
        else if (pieceType == PAWN) {
            board[destY][destX] |= MODIFIER;
        }
    }
    
    //---Handle special moves---
    
    // Pawn special moves
    if(pieceType == PAWN && !isPromotion) {
        //double pawn push =>  remember for 'En passant'
        if(abs(destY - oldY) == 2) {
            // Store column in x, row in y for consistency with engine.c
            (*lastDoublePawn).x = destX;  // Column of the pawn
            (*lastDoublePawn).y = destY;  // Row of the pawn
            
            printf("Double pawn push detected: lastDoublePawn set to (%d, %d)\n", 
                   (int)(*lastDoublePawn).x, (int)(*lastDoublePawn).y);
        }
        else {
            int lastDoubleX = (*lastDoublePawn).x;
            int lastDoubleY = (*lastDoublePawn).y;
            
            //En passant => delete captured piece
            if(!capture && abs(destX - oldX) == 1) {
                printf("En passant capture: Removing pawn at (%d, %d)\n", lastDoubleY, lastDoubleX);
                board[lastDoubleY][lastDoubleX] = 0;
            }

            //remove last double pawn
            (*lastDoublePawn).x = -1.0f;
            (*lastDoublePawn).y = -1.0f;
        }
    }
    // King special moves
    else if(pieceType == KING) {
        // Handle castling
        if(abs(destX - oldX) == 2) {
            // Kingside castling (short castle)
            if(destX > oldX) {
                // Move the rook from the kingside to its new position
                // Clear the MODIFIER flag on the rook too
                board[destY][destX-1] = (ROOK | (board[destY][destX] & COLOR_MASK));
                board[destY][7] = 0; // Remove the rook from its original position
            }
            // Queenside castling (long castle)
            else {
                // Move the rook from the queenside to its new position
                // Clear the MODIFIER flag on the rook too
                board[destY][destX+1] = (ROOK | (board[destY][destX] & COLOR_MASK));
                board[destY][0] = 0; // Remove the rook from its original position
            }
        }
        
        // Update king position
        kingsPositions[color].x = destY;
        kingsPositions[color].y = destX;
        
        if(color == 1) {
            printf("Black ");
        }
        else {
            printf("White ");
        }
        printf("king has moved!\n");
    }

    //delete from old position
    board[oldY][oldX] = 0;

    //delete selected square
    (*sourceSquare).x = -1.0f;
    (*sourceSquare).y = -1.0f;

    pieceActions[0] = false;
    pieceActions[1] = false;

    unsigned int nextColor = color == 0 ? 1 : 0;
    
    if(isCheck(board, kingsPositions[nextColor])) {
        printf("In check!\n");
    }
    
    // Record timestamp of the move for PvE mode
    if (gameMode == 2) {
        moveTimestamp = SDL_GetTicks();
        printf("Move timestamp recorded: %u\n", moveTimestamp);
    }
    
    // Analyze the new position for the next player
    displayPositionAnalysis(board, *blackTurn, lastDoublePawn, kingsPositions);
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

    // In PvE mode, player can only move white pieces (when blackTurn is false)
    if (gameMode == 2 && *blackTurn) {
        return; // Skip player input when it's black's turn in PvE mode
    }

    if(mouseActions[0]) { //MOUSE CLICKED
        //NO SELECTED PIECE => SELECT
        if(!pieceActions[0]) {
            if(board[squareY][squareX] != 0 && !opposingColor(board[squareY][squareX], *blackTurn)) {
                selectAndHold(board, squareX, squareY, pieceActions, selectedSquare);

                // Generate legal moves
                generatePossibleMoves(board, squareY, squareX, lastDoublePawn);
            }
        }
        //A SELECTED PIECE  => TRY TO MOVE
        else {
            //VALID MOVE => MOVE PIECE
            if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions, blackTurn);
                *blackTurn = !(*blackTurn);
                clearPossibleBoard(board);
            }
            //NOT VALID => CANCEL SELECT
            else { 
                deselectPiece(board, selectedSquare, pieceActions);
                clearPossibleBoard(board);
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
                    makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions, blackTurn);
                    *blackTurn = !(*blackTurn);
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
            makeMove(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions, blackTurn);
            *blackTurn = !(*blackTurn);
            clearPossibleBoard(board);
          }
          else {
              deselectPiece(board, selectedSquare, pieceActions);
              clearPossibleBoard(board);
          }  
        }
    }
}