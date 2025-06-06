#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <math.h>

#include "Events.h"
#include "Piece.h"
#include "engine.h"
#include "main.h"
#include "RenderWindow.h"
#include "app_globals.h"
#include "GameState.h"
#include <stdio.h>

// Original functions - keep for backward compatibility
void getEvents_legacy(SDL_Event event, bool *gameRunning, bool mouseActions[]) {
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

bool mouseInsideBoard(int mouseX, int mouseY) {
    int boardWidth = squareSize * 8;
    return mouseX >= 0 && mouseX < boardWidth && mouseY >= 0 && mouseY < boardWidth;
}

void selectAndHold_legacy(unsigned char board[8][8], int squareX, int squareY, bool pieceActions[], Vector2f *selectedSquare) {
    //toggle active: hold and select
    pieceActions[0] = true;
    pieceActions[1] = true;

    //change square to selected
    board[squareY][squareX] |= SELECTED_MASK;

    //save selected square coords
    (*selectedSquare).x = squareX;
    (*selectedSquare).y = squareY;
}

void makeMove_legacy(unsigned char board[8][8], int destX, int destY, Vector2f* sourceSquare, bool pieceActions[], Vector2f* lastDoublePawn, Vector2f kingsPositions[], bool* blackTurn) {
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
        promotedPiece = showPromotionMenu(renderer, pieceTextures, destX, destY, color, screenWidth, screenHeight);
        
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

void deselectPiece_legacy(unsigned char board[8][8], Vector2f* selectedSquare, bool pieceActions[]) {
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

void handleMouseInput_legacy(unsigned char board[8][8], int mouseX, int mouseY, int screenWidth, int squareSize, bool mouseActions[], bool pieceActions[], bool* blackTurn, Vector2f* selectedSquare, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    //Get square from mouse cursor - board is now on the left side, not centered
    int squareX = (int)(mouseX / squareSize);
    int squareY = (int)(mouseY / squareSize);

    // Ensure we're within the board bounds
    if (squareX < 0 || squareX > 7 || squareY < 0 || squareY > 7) {
        return;
    }

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
                selectAndHold_legacy(board, squareX, squareY, pieceActions, selectedSquare);

                // Generate legal moves
                generatePossibleMoves(board, squareY, squareX, lastDoublePawn);
            }
        }
        //A SELECTED PIECE  => TRY TO MOVE
        else {
            //VALID MOVE => MOVE PIECE
            if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove_legacy(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions, blackTurn);
                *blackTurn = !(*blackTurn);
                clearPossibleBoard(board);
            }
            //INVALID MOVE => DESELECT
            else {
                deselectPiece_legacy(board, selectedSquare, pieceActions);
                clearPossibleBoard(board);
            }
        }
    }
    else if(mouseActions[1]) { //MOUSE RELEASED
        //HOLDING PIECE => RELEASE
        if(pieceActions[1]) {
            //Same square => just release
            if(squareX == (*selectedSquare).x && squareY == (*selectedSquare).y) {
                pieceActions[1] = false;
            }
            //Different square => try to move
            else {
                //VALID MOVE => MOVE PIECE
                if((board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove_legacy(board, squareX, squareY, selectedSquare, pieceActions, lastDoublePawn, kingsPositions, blackTurn);
                    *blackTurn = !(*blackTurn);
                    clearPossibleBoard(board);
                }
                //INVALID MOVE => DESELECT
                else {
                    deselectPiece_legacy(board, selectedSquare, pieceActions);
                    clearPossibleBoard(board);
                }
            }
        }
    }
}

// New GameState-based functions

// Handle SDL events with GameState
void getEvents(SDL_Event event, GameState *state, int *scrollOffset) {
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                state->gameRunning = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    state->mouseActions[0] = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    state->mouseActions[1] = true;
                }
                break;
            case SDL_MOUSEWHEEL:
                *scrollOffset -= event.wheel.y * 25; // 25 pixels per scroll step
                if (*scrollOffset < 0) *scrollOffset = 0;
                
                // Calculate maximum scroll offset based on move count
                int maxScroll = (state->moveCount * 25) - screenHeight;
                if (maxScroll < 0) maxScroll = 0;
                if (*scrollOffset > maxScroll) *scrollOffset = maxScroll;
                break;
            case SDL_TEXTINPUT:
                if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                    if (strlen(inputFileNameBuffer) + strlen(event.text.text) < sizeof(inputFileNameBuffer)) {
                        strcat(inputFileNameBuffer, event.text.text);
                    }
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_e) {
                    // Toggle evaluation bar visibility
                    extern bool showEvaluationBar;
                    showEvaluationBar = !showEvaluationBar;
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                        SDL_StopTextInput();
                        textInputActive = SDL_FALSE;
                        currentScreenState = GAME_STATE_PLAYING;
                        currentPromptAction = PROMPT_ACTION_NONE;
                        printf("Operation cancelled.\n");
                    } else {
                        // Show menu when Escape is pressed
                        showMenu = true;
                    }
                }
                else if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
                    if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(inputFileNameBuffer) > 0) {
                        inputFileNameBuffer[strlen(inputFileNameBuffer) - 1] = '\0';
                    } else if (event.key.keysym.sym == SDLK_RETURN) {
                        if (strlen(inputFileNameBuffer) > 0) {
                            if (currentPromptAction == PROMPT_ACTION_SAVE) {
                                saveGameToFile(state, inputFileNameBuffer);
                            } else if (currentPromptAction == PROMPT_ACTION_LOAD) {
                                loadGameFromFile(state, inputFileNameBuffer);
                                // After loading, reset history to the loaded state
                                historyCount = 0; // Clear existing history
                                currentHistoryIdx = -1; // Reset index
                                recordGameState(state); // Record the newly loaded state
                            }
                        } else {
                            printf("Filename cannot be empty. Please enter a name.\n");
                        }
                        SDL_StopTextInput();
                        textInputActive = SDL_FALSE;
                        currentScreenState = GAME_STATE_PLAYING;
                        currentPromptAction = PROMPT_ACTION_NONE;
                    }
                }
                break;
        }
    }
}

// Select and hold a piece with GameState
void selectAndHold(GameState *state, int squareX, int squareY) {
    state->pieceActions[0] = true;
    state->pieceActions[1] = true;
    state->board[squareY][squareX] |= SELECTED_MASK;
    state->selectedSquare.x = squareX;
    state->selectedSquare.y = squareY;
}

// Make a move with GameState
void makeMove(GameState *state, int destX, int destY) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    unsigned char capturedPieceOnDest = state->board[destY][destX];
    unsigned char capturedPieceType = (capturedPieceOnDest & TYPE_MASK);
    unsigned char capturingPiece = state->board[oldY][oldX];
    unsigned char pieceType = capturingPiece & TYPE_MASK;
    unsigned char color = (capturingPiece & COLOR_MASK) >> 4;

    // Handle standard capture
    bool isStandardCapture = (capturedPieceType != NONE);
    if (isStandardCapture) {
        if (color == 0) { // White captured a black piece
            state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = capturedPieceType;
        } else { // Black captured a white piece
            state->blackCapturedPieces[state->numBlackCapturedPieces++] = capturedPieceType;
        }
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
        promotedPiece = showPromotionMenu(renderer, pieceTextures, destX, destY, color << 4, screenWidth, screenHeight);
        
        // Place the promoted piece
        state->board[destY][destX] = promotedPiece;
    } else {
        // Copy piece to new location (preserve color and type)
        state->board[destY][destX] = (capturingPiece & (TYPE_MASK | COLOR_MASK));
        
        // For KING and ROOK, ensure MODIFIER flag is cleared after copying
        if (pieceType == KING || pieceType == ROOK) {
            // Clear the MODIFIER flag (set it to 0)
            state->board[destY][destX] &= ~MODIFIER;
        }
        // For other pieces that use MODIFIER (like pawns), set it
        else if (pieceType == PAWN) {
            state->board[destY][destX] |= MODIFIER;
        }
    }

    // Add move to history
    addMoveToHistory(state, oldY, oldX, destY, destX, capturingPiece);
    
    // Handle special moves
    
    // Pawn special moves
    if (pieceType == PAWN && !isPromotion) {
        // Double pawn push => remember for En passant
        if (abs(destY - oldY) == 2) {
            state->lastDoublePushPawn.x = destX;
            state->lastDoublePushPawn.y = destY;
        } else {
            // En passant capture
            if (!isStandardCapture && abs(destX - oldX) == 1) {
                int capturedPawnRow = oldY;
                int capturedPawnCol = destX;
                
                // Add the captured pawn to the captured pieces list
                if (color == 0) { // White captured a black pawn
                    state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = PAWN;
                } else { // Black captured a white pawn
                    state->blackCapturedPieces[state->numBlackCapturedPieces++] = PAWN;
                }
                
                // Remove the captured pawn
                state->board[capturedPawnRow][capturedPawnCol] = 0;
            }
            
            // Reset last double push pawn
            state->lastDoublePushPawn.x = -1.0f;
            state->lastDoublePushPawn.y = -1.0f;
        }
    }
    // King special moves
    else if (pieceType == KING) {
        // Handle castling
        if (abs(destX - oldX) == 2) {
            // Kingside castling (short castle)
            if (destX > oldX) {
                // Move the rook from the kingside to its new position
                state->board[destY][destX-1] = (ROOK | (state->board[destY][destX] & COLOR_MASK));
                state->board[destY][7] = 0; // Remove the rook from its original position
            }
            // Queenside castling (long castle)
            else {
                // Move the rook from the queenside to its new position
                state->board[destY][destX+1] = (ROOK | (state->board[destY][destX] & COLOR_MASK));
                state->board[destY][0] = 0; // Remove the rook from its original position
            }
        }
        
        // Update king position
        state->kingsPositions[color].x = destY;
        state->kingsPositions[color].y = destX;
    }
    
    // Delete piece from old position
    state->board[oldY][oldX] = 0;
    
    // Reset selected square
    state->selectedSquare.x = -1.0f;
    state->selectedSquare.y = -1.0f;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
    
    // Switch turns
    state->blackTurn = !state->blackTurn;
    
    // Check if the opponent is in check
    unsigned char nextColor = color == 0 ? 1 : 0;
    if (isCheck(state->board, state->kingsPositions[nextColor])) {
        printf("In check!\n");
    }
    
    // Record timestamp of the move for PvE mode
    if (gameMode == 2) {
        moveTimestamp = SDL_GetTicks();
        printf("Move timestamp recorded: %u\n", moveTimestamp);
    }
    
    // Record the game state for undo/redo
    recordGameState(state);
    
    // Clear possible moves
    clearPossibleBoard(state->board);
}

// Deselect a piece with GameState
void deselectPiece(GameState *state) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    // Deselect piece
    state->board[oldY][oldX] &= (~SELECTED_MASK);

    // Reset selected square
    state->selectedSquare.x = -1.0f;
    state->selectedSquare.y = -1.0f;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
}

// Handle mouse input with GameState
void handleMouseInput(GameState *state, int mouseX, int mouseY) {
    // Get square from mouse cursor - board is on the left side
    int squareX = (int)(mouseX / squareSize);
    int squareY = (int)(mouseY / squareSize);

    // Ensure we're within the board bounds
    if (squareX < 0 || squareX > 7 || squareY < 0 || squareY > 7) {
        return;
    }

    // In PvE mode, player can only move white pieces (when blackTurn is false)
    if (gameMode == 2 && state->blackTurn) {
        return; // Skip player input when it's black's turn in PvE mode
    }

    if (state->mouseActions[0]) { // MOUSE CLICKED
        // NO SELECTED PIECE => SELECT
        if (!state->pieceActions[0]) {
            if (state->board[squareY][squareX] != 0 && 
                !opposingColor(state->board[squareY][squareX], state->blackTurn)) {
                selectAndHold(state, squareX, squareY);
                generatePossibleMoves(state->board, squareY, squareX, &state->lastDoublePushPawn);
            }
        }
        // A SELECTED PIECE => TRY TO MOVE
        else {
            // VALID MOVE => MOVE PIECE
            if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(state, squareX, squareY);
            }
            // INVALID MOVE => DESELECT
            else {
                deselectPiece(state);
                clearPossibleBoard(state->board);
            }
        }
    }
    else if (state->mouseActions[1]) { // MOUSE RELEASED
        // HOLDING PIECE => RELEASE
        if (state->pieceActions[1]) {
            // Same square => just release
            if (squareX == state->selectedSquare.x && squareY == state->selectedSquare.y) {
                state->pieceActions[1] = false;
            }
            // Different square => try to move
            else {
                // VALID MOVE => MOVE PIECE
                if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY);
                }
                // INVALID MOVE => DESELECT
                else {
                    deselectPiece(state);
                    clearPossibleBoard(state->board);
                }
            }
        }
    }
}