#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <math.h>
#include <string.h> // For strlen and strcat
#include <stdio.h> // For snprintf

#include "Events.h"
#include "Piece.h"
#include "GameState.h" // Now necessary
#include "util.h" // For screenWidth, squareSize, etc. from util.c
#include "app_globals.h" // For global enums/macros

const int scrollStep = 20;

// addMoveToHistory is already taking GameState* state, which is good.

void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece) {
    if (state->moveCount >= MAX_MOVES) return; // Use state->moveCount

    const char *pieceChar;
    switch (piece & TYPE_MASK) { // Using TYPE_MASK from Piece.h
        case PAWN:
            pieceChar = ""; // Pawn usually has no character, just notation like e4
            break;
        case KNIGHT:
            pieceChar = "N";
            break;
        case BISHOP:
            pieceChar = "B";
            break;
        case ROOK:
            pieceChar = "R";
            break;
        case QUEEN:
            pieceChar = "Q";
            break;
        case KING:
            pieceChar = "K";
            break;
        default:
            pieceChar = "?";
            break;
    }

    char from[3] = {'a' + startCol, '8' - startRow, '\0'};
    char to[3] = {'a' + endCol, '8' - endRow, '\0'};

    snprintf(state->moveHistory[state->moveCount].notation, sizeof(state->moveHistory[state->moveCount].notation),
             "%s%s%s", pieceChar, from, to);

    state->moveCount++;
}

// In src/Events.c
void getEvents(SDL_Event event, GameState *state, int *scrollOffset) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                state->gameRunning = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                // Global currentScreenState, currentPromptAction, inputFileNameBuffer are still needed
                if (currentScreenState == GAME_STATE_PLAYING) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        state->mouseActions[0] = true;
                    }
                    // Right-click for other actions if implemented
                    // if (event.button.button == SDL_BUTTON_RIGHT) {
                    //     state->mouseActions[1] = true;
                    // }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (currentScreenState == GAME_STATE_PLAYING) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        state->mouseActions[1] = true;
                    }
                }
                break;

            case SDL_MOUSEWHEEL:
                *scrollOffset -= event.wheel.y * scrollStep;
                if (*scrollOffset < 0) *scrollOffset = 0;

                // screenHeight should be visible via util.h
                int maxScroll = (state->moveCount * 25) - screenHeight; // Use state->moveCount and global screenHeight
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
                if (currentScreenState == GAME_STATE_PROMPT_FILENAME) {
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
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        SDL_StopTextInput();
                        textInputActive = SDL_FALSE;
                        currentScreenState = GAME_STATE_PLAYING;
                        currentPromptAction = PROMPT_ACTION_NONE;
                        printf("Operation cancelled.\n");
                    }
                }
                // Keyboard toggles for showEvaluationBar, showMenu are handled in main.c
                // Make sure to add computerPlaysBlack, showAnalysis etc. here if Events.c should handle them.
                break;
        }
    }
}

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth_local, int squareSize_local) { // Using local names to avoid conflict with global util.h
    int boardStartX = 0;
    int boardEndX = 8 * squareSize_local; // boardWidth
    int boardStartY = 0;
    int boardEndY = 8 * squareSize_local; // screenHeight

    return (mouseX >= boardStartX && mouseX < boardEndX &&
            mouseY >= boardStartY && mouseY < boardEndY);
}

void selectAndHold(GameState *state, int squareX, int squareY) {
    state->pieceActions[0] = true;
    state->pieceActions[1] = true;
    state->board[squareY][squareX] |= SELECTED_MASK;
    state->selectedSquare.x = squareX;
    state->selectedSquare.y = squareY;
}

// isMoveLegal now takes GameState* state
bool isMoveLegal(GameState* state, int destX, int destY) {
    unsigned char tempBoard[8][8];
    Vector2f tempKingsPositions[2] = {state->kingsPositions[0], state->kingsPositions[1]}; // Copy king positions

    // Copy the board to a temporary board
    memcpy(tempBoard, state->board, sizeof(unsigned char) * 64);

    // Get the color and type of the moving piece
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;
    unsigned char movingPiece = state->board[oldY][oldX];
    unsigned int pieceType = movingPiece & TYPE_MASK;
    unsigned int color = (movingPiece & COLOR_MASK) >> 4;

    // Make the move on the temporary board
    tempBoard[destY][destX] = tempBoard[oldY][oldX];
    tempBoard[oldY][oldX] = 0;

    // Update king position if king is moving
    if(pieceType == KING) {
        tempKingsPositions[color].x = destY;
        tempKingsPositions[color].y = destX;
    }

    // Check if the king is in check after the move
    return !isCheck(tempBoard, tempKingsPositions[color]);
}

void makeMove(GameState *state, int destX, int destY) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    unsigned char movingPiece = state->board[oldY][oldX];
    unsigned int pieceType = movingPiece & TYPE_MASK;
    unsigned int color = (movingPiece & COLOR_MASK) >> 4;

    bool isPromotion = false;

    // Check for pawn promotion
    if(pieceType == PAWN) {
        if((color == 0 && destY == 0) || (color == 1 && destY == 7)) {
            isPromotion = true;
        }
    }

    // Check if the move is legal (doesn't leave the king in check)
    if (!isMoveLegal(state, destX, destY)) { // Pass state
        printf("Illegal move! King would be in check.\n");
        deselectPiece(state); // Pass state
        clearPossibleBoard(state->board);
        return;
    }

    // Check for capture
    unsigned char capturedPieceOnDest = state->board[destY][destX];
    unsigned char capturedPieceType = (capturedPieceOnDest & TYPE_MASK);
    bool isStandardCapture = (capturedPieceType != NONE);

    if (isStandardCapture) {
        unsigned char capturingColor = (movingPiece & COLOR_MASK); // Color of the piece making the capture
        if ((capturingColor >> 4) == 0) { // White capturing
            state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = capturedPieceType;
        } else { // Black capturing
            state->blackCapturedPieces[state->numBlackCapturedPieces++] = capturedPieceType;
        }
    }

    // Move piece to destination
    state->board[destY][destX] = state->board[oldY][oldX];

    // Deselect piece
    state->board[destY][destX] &= (~SELECTED_MASK);

    // For kings and rooks, clear the MODIFIER flag
    if (pieceType == KING || pieceType == ROOK) {
        // Clear the MODIFIER flag (set it to 0)
        state->board[destY][destX] &= ~MODIFIER;
    }
        // For other pieces that use MODIFIER (like pawns), set it
    else if (pieceType == PAWN) {
        state->board[destY][destX] |= MODIFIER;
    }

    //---Handle special moves---

    // Pawn special moves
    if(pieceType == PAWN) {
        //double pawn push =>  remember for 'En passant'
        if(abs(destY - oldY) == 2) {
            // Store column in x, row in y for consistency with engine.c
            state->lastDoublePushPawn.x = destX;  // Column of the pawn
            state->lastDoublePushPawn.y = destY;  // Row of the pawn

            printf("Double pawn push detected: lastDoublePawn set to (%d, %d)\n",
                   state->lastDoublePushPawn.x, state->lastDoublePushPawn.y);
        }
        else {
            int lastDoubleX = state->lastDoublePushPawn.x;
            int lastDoubleY = state->lastDoublePushPawn.y;

            //En passant => delete captured piece
            // if not a standard capture AND it was a diagonal move
            if(!isStandardCapture && abs(destX - oldX) == 1) { // This implies en passant if no piece at dest
                // Check if the move was actually an en passant capture
                if(lastDoubleX == destX && lastDoubleY == oldY + (color == 0 ? 1 : -1)) { // Pawn moved to square behind double-pushed pawn
                    printf("En passant capture executed: removing pawn at (%d, %d)\n", lastDoubleY, lastDoubleX);
                    state->board[lastDoubleY][lastDoubleX] = NONE;

                    // Add captured pawn to captured pieces list
                    unsigned char capturedPawnType = PAWN; // En passant always captures a pawn
                    if (color == 0) { // White capturing, add black pawn to white's captured list
                        state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = capturedPawnType;
                    } else { // Black capturing, add white pawn to black's captured list
                        state->blackCapturedPieces[state->numBlackCapturedPieces++] = capturedPawnType;
                    }
                }
            }

            // Reset lastDoublePawn
            state->lastDoublePushPawn.x = -1;
            state->lastDoublePushPawn.y = -1;
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
                state->board[destY][destX-1] = (ROOK | (state->board[destY][destX] & COLOR_MASK));
                state->board[destY][7] = 0; // Remove the rook from its original position
            }
                // Queenside castling (long castle)
            else {
                // Move the rook from the queenside to its new position
                // Clear the MODIFIER flag on the rook too
                state->board[destY][destX+1] = (ROOK | (state->board[destY][destX] & COLOR_MASK));
                state->board[destY][0] = 0; // Remove the rook from its original position
            }
        }

        // Update king position
        state->kingsPositions[color].x = destY;
        state->kingsPositions[color].y = destX;

        if(color == 1) {
            printf("Black ");
        }
        else {
            printf("White ");
        }
        printf("king has moved!\n");
    }

    // Handle promotion if applicable
    if (isPromotion) {
        // Assuming promotion menu is handled elsewhere (e.g., in RenderWindow.c)
        // For now, auto-promote to Queen for simplicity
        unsigned char promotedPiece = QUEEN | (color << 4); // Queen of current color
        state->board[destY][destX] = promotedPiece;
        printf("Pawn promoted to Queen!\n"); // For debug
    }


    // Delete from old position
    state->board[oldY][oldX] = NONE;

    // Add move to history
    addMoveToHistory(state, oldY, oldX, destY, destX, movingPiece);

    // Deselect square and piece actions
    state->selectedSquare.x = -1;
    state->selectedSquare.y = -1;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;

    // Change turn
    state->blackTurn = !state->blackTurn;

    // Check for check after move
    unsigned int nextColor = (color == 0) ? 1 : 0;
    if(isCheck(state->board, state->kingsPositions[nextColor])) {
        printf("In check!\n");
    }

    // The timestamp and position analysis are now handled in main.c after this function returns.
}

void deselectPiece(GameState *state) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    state->board[oldY][oldX] &= (~SELECTED_MASK);
    state->selectedSquare.x = -1;
    state->selectedSquare.y = -1;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
}

void handleMouseInput(GameState *state, int mouseX, int mouseY, int squareSize_local) { // Using local squareSize_local
    int boardOffset = 0;
    int squareX = (int) ((mouseX - boardOffset) / squareSize_local);
    int squareY = (int) (mouseY / squareSize_local);

    // Ensure we're within the board bounds
    if (squareX < 0 || squareX > 7 || squareY < 0 || squareY > 7) {
        return;
    }

    // Only process board input if in GAME_STATE_PLAYING
    // currentScreenState is global from app_globals.h
    if (currentScreenState == GAME_STATE_PLAYING) {
        // Only allow moves if it's the current player's turn (PvP or player's turn in PvE)
        // gameMode and computerPlaysBlack are global externs in main.c
        bool isComputerTurn = (gameMode == 2 && state->blackTurn && computerPlaysBlack);
        if (isComputerTurn) {
            return; // Skip player input when it's computer's turn in PvE mode
        }

        if (state->mouseActions[0]) { // MOUSE CLICKED
            if (!state->pieceActions[0]) { // NO SELECTED PIECE => SELECT
                if (state->board[squareY][squareX] != NONE &&
                    !opposingColor(state->board[squareY][squareX], state->blackTurn)) { // Check if it's current player's piece
                    selectAndHold(state, squareX, squareY); // Pass state
                    generatePossibleMoves(state->board, squareY, squareX, &state->lastDoublePushPawn); // Pass board and lastDoublePawn
                }
            } else { // A SELECTED PIECE => TRY TO MOVE
                if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY); // Pass state
                    clearPossibleBoard(state->board);
                } else {
                    deselectPiece(state); // Pass state
                    clearPossibleBoard(state->board);
                }
            }
        } else if (state->mouseActions[1]) { // MOUSE RELEASED
            if (state->pieceActions[1]) { // HOLDING PIECE (drag and drop scenario)
                if (squareX == state->selectedSquare.x && squareY == state->selectedSquare.y) {
                    state->pieceActions[1] = false; // Stop holding, but piece remains selected
                } else {
                    if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                        makeMove(state, squareX, squareY); // Pass state
                        clearPossibleBoard(state->board);
                    } else {
                        deselectPiece(state); // Pass state
                        clearPossibleBoard(state->board);
                    }
                }
            } else { // NOT HOLDING (click-click scenario)
                // If a piece was selected previously and now clicked on a new valid square
                if (state->pieceActions[0] && (state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY); // Pass state
                    clearPossibleBoard(state->board);
                } else { // Clicked on empty/invalid square or same selected square
                    deselectPiece(state); // Pass state
                    clearPossibleBoard(state->board);
                }
            }
        }
    }
}