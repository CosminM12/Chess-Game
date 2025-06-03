#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <math.h>

#include "Events.h"
#include "Piece.h"
#include "GameState.h"

const int scrollStep = 20;

void addMoveToHistory(int startRow, int startCol, int endRow, int endCol, unsigned char piece) {
    if (moveCount >= MAX_MOVES) return;

    const char *pieceChar;
    switch (piece & TYPE_MASK) {
        case PAWN:
            pieceChar = "";
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

    snprintf(moveHistory[moveCount].notation, sizeof(moveHistory[moveCount].notation),
             "%s%s%s", pieceChar, from, to);

    moveCount++;
}

// In src/Events.c
void getEvents(SDL_Event event, GameState *state, int *scrollOffset) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                state->gameRunning = false; // Directly update gameRunning in state
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state->mouseActions[0] = true; // Update mouseActions in state
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    state->mouseActions[1] = true; // Update mouseActions in state
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

void selectAndHold(GameState *state, int squareX, int squareY) {
    state->pieceActions[0] = true;
    state->pieceActions[1] = true;
    state->board[squareY][squareX] |= SELECTED_MASK;
    state->selectedSquare.x = squareX;
    state->selectedSquare.y = squareY;
}

void makeMove(GameState *state, int destX, int destY) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    unsigned char capturedPieceOnDest = state->board[destY][destX]; // Get the piece that was on the destination square
    unsigned char capturedPieceType = (capturedPieceOnDest & TYPE_MASK); // Extract its type
    unsigned char capturingPiece = state->board[oldY][oldX]; // Get the piece making the move

    // Check for a standard capture (a piece on the destination square before move)
    bool isStandardCapture = (capturedPieceType != NONE);

    // If a standard capture occurred, add the captured piece to the correct array
    if (isStandardCapture) {
        unsigned char capturingColor = (capturingPiece & COLOR_MASK);

        // Assuming COLOR_MASK = 0x10 for black, 0 for white (default if COLOR_MASK is not present)
        // Adjust the condition based on your actual COLOR_MASK setup:
        // If capturingColor is 0 (white), then a Black piece was captured.
        // If capturingColor is 0x10 (black), then a White piece was captured.
        if (capturingColor == 0) { // White is capturing, so a Black piece is captured
            state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = capturedPieceType;
        } else { // Black is capturing, so a White piece is captured
            state->blackCapturedPieces[state->numBlackCapturedPieces++] = capturedPieceType;
        }
    }

    // Copy piece to new location (only last 5 bits, preserving modifier bits if any)
    state->board[destY][destX] = (capturingPiece & 0x1F); // Use 0x1F mask to copy piece type and color correctly

    addMoveToHistory(oldY, oldX, destY, destX, capturingPiece); // Pass the original piece information

    state->board[oldY][oldX] = NONE;

    // ---Handle pawn special moves---
    if ((state->board[destY][destX] & TYPE_MASK) == PAWN) {
        // Set modifier (pawn can't double push anymore)
        state->board[destY][destX] |= MODIFIER;

        // Double pawn push => remember for 'En passant'
        if (abs(destY - oldY) == 2) {
            state->lastDoublePushPawn.x = destX;
            state->lastDoublePushPawn.y = destY;
        } else {
            // En passant capture: If it was a diagonal pawn move to an empty square,
            // it's an en passant capture. Remove the captured pawn.
            if (!isStandardCapture && abs(destX - oldX) == 1) {
                // Determine the row of the captured pawn (one square behind the destination in pawn's direction)
                int pawnColor = (state->board[destY][destX] & COLOR_MASK); // Color of the pawn that just moved
                int capturedPawnRow = destY + (pawnColor == 0 ? 1
                                                              : -1); // If white pawn moved down, captured pawn is above; if black pawn moved up, captured pawn is below

                // Add the captured pawn type to the correct array (it's always a PAWN)
                if (pawnColor == 0) { // White pawn captured Black's pawn via en passant
                    state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = PAWN;
                } else { // Black pawn captured White's pawn via en passant
                    state->blackCapturedPieces[state->numBlackCapturedPieces++] = PAWN;
                }
                state->board[capturedPawnRow][destX] = NONE; // Remove the captured pawn from the board
            }
            state->lastDoublePushPawn.x = -1.0f; // Reset lastDoublePushPawn if it wasn't a double push
            state->lastDoublePushPawn.y = -1.0f;
        }
    }

    // Delete piece from old position
    state->board[oldY][oldX] = NONE; // Use NONE for clarity instead of 0

    // Deselect piece and clear related states
    state->selectedSquare.x = -1.0f;
    state->selectedSquare.y = -1.0f;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;

    unsigned char movedPieceColor = (state->board[destY][destX] & COLOR_MASK);
    if ((state->board[destY][destX] & TYPE_MASK) == KING) {
        // Update king's position in kingsPositions array
        unsigned int colorIndex = (movedPieceColor == 0) ? 0 : 1; // 0 for white king, 1 for black king
        state->kingsPositions[colorIndex].x = destY;
        state->kingsPositions[colorIndex].y = destX;
    }

    // Toggle turn for the next player
    state->blackTurn = !state->blackTurn;

    // Check if the opponent's king is in check after the move
    unsigned int nextPlayerColorIndex = (movedPieceColor == 0) ? 1
                                                               : 0; // If white moved, check black king; if black moved, check white king
    if (isCheck(state->board, state->kingsPositions[nextPlayerColorIndex])) {
        printf("In check!\n");
    }

    clearPossibleBoard(state->board); // Clear possible moves after a move has been made
}

void deselectPiece(GameState *state) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    state->board[oldY][oldX] &= (~SELECTED_MASK);
    state->selectedSquare.x = -1.0f;
    state->selectedSquare.y = -1.0f;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;
}

void handleMouseInput(GameState *state, int mouseX, int mouseY) {
    int boardOffset = 0; // Assuming this is also a global constant or defined locally
    int squareX = (int) ((mouseX - boardOffset) / squareSize);
    int squareY = (int) (mouseY / squareSize);

    if (state->mouseActions[0]) { // MOUSE CLICKED
        if (!state->pieceActions[0]) { // NO SELECTED PIECE => SELECT
            if (state->board[squareY][squareX] != 0 &&
                !opposingColor(state->board[squareY][squareX], state->blackTurn)) {
                selectAndHold(state, squareX, squareY);
                generatePossibleMoves(state->board, squareY, squareX, &state->lastDoublePushPawn);
            }
        } else { // A SELECTED PIECE => TRY TO MOVE
            if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(state, squareX, squareY); // Pass state
                clearPossibleBoard(state->board);
            } else {
//                deselectPiece(state); // If not valid, deselect
//                clearPossibleBoard(state->board);
            }
        }
    } else if (state->mouseActions[1]) { // MOUSE RELEASED
        if (state->pieceActions[1]) { // HOLDING PIECE
            if (squareX == state->selectedSquare.x && squareY == state->selectedSquare.y) {
                state->pieceActions[1] = false;
            } else {
                if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY); // Pass state
                    clearPossibleBoard(state->board);
                } else {
                    deselectPiece(state);
                    clearPossibleBoard(state->board);
                }
            }
        } else {
            if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                makeMove(state, squareX, squareY); // Pass state
            } else {
                deselectPiece(state);
                clearPossibleBoard(state->board);
            }
        }
    }
}