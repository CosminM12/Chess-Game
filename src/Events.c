#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <math.h>
#include <string.h> // <--- ADDED: Required for strlen and strcat

#include "Events.h"
#include "Piece.h"
#include "GameState.h"
#include "util.h"        // <--- ADDED: Was missing based on common functions like screenHeight
#include "app_globals.h"

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

void getEvents(SDL_Event event, GameState *state, int *scrollOffset) {
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                state->gameRunning = false;
                break;

            case SDL_MOUSEBUTTONDOWN:
                // <--- MODIFIED: Only process mouse clicks if not in a prompt state
                if (currentScreenState == GAME_STATE_PLAYING) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        state->mouseActions[0] = true;
                    }
                    if (event.button.button == SDL_BUTTON_RIGHT) {
                        state->mouseActions[1] = true;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                // <--- MODIFIED: Only process mouse releases if not in a prompt state
                if (currentScreenState == GAME_STATE_PLAYING) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        state->mouseActions[1] = true;
                    }
                }
                break;

            case SDL_MOUSEWHEEL:
                *scrollOffset -= event.wheel.y * scrollStep;
                if (*scrollOffset < 0) *scrollOffset = 0;

                // Make sure screenHeight and moveCount are accessible globals, which they are via app_globals.h
                int maxScroll = (moveCount * 25) - screenHeight;
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
                break;
        }
    }
}

bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize) {
    // boardOffset is 0 as per your util.c, board starts at X=0
    int boardStartX = 0;
    int boardEndX = boardWidth; // boardWidth is defined in util.c
    int boardStartY = 0;
    int boardEndY = screenHeight; // screenHeight is defined in util.c

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

void makeMove(GameState *state, int destX, int destY) {
    int oldX = state->selectedSquare.x;
    int oldY = state->selectedSquare.y;

    unsigned char capturedPieceOnDest = state->board[destY][destX];
    unsigned char capturedPieceType = (capturedPieceOnDest & TYPE_MASK);
    unsigned char capturingPiece = state->board[oldY][oldX];

    bool isStandardCapture = (capturedPieceType != NONE);

    if (isStandardCapture) {
        unsigned char capturingColor = (capturingPiece & COLOR_MASK);
        if (capturingColor == 0) {
            state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = capturedPieceType;
        } else {
            state->blackCapturedPieces[state->numBlackCapturedPieces++] = capturedPieceType;
        }
    }

    state->board[destY][destX] = (capturingPiece & 0x1F);

    addMoveToHistory(oldY, oldX, destY, destX, capturingPiece);

    state->board[oldY][oldX] = NONE;

    if ((state->board[destY][destX] & TYPE_MASK) == PAWN) {
        state->board[destY][destX] |= MODIFIER;
        if (abs(destY - oldY) == 2) {
            state->lastDoublePushPawn.x = destX;
            state->lastDoublePushPawn.y = destY;
        } else {
            if (!isStandardCapture && abs(destX - oldX) == 1) {
                int pawnColor = (state->board[destY][destX] & COLOR_MASK);
                int capturedPawnRow = destY + (pawnColor == 0 ? 1 : -1);

                if (pawnColor == 0) {
                    state->whiteCapturedPieces[state->numWhiteCapturedPieces++] = PAWN;
                } else {
                    state->blackCapturedPieces[state->numBlackCapturedPieces++] = PAWN;
                }
                state->board[capturedPawnRow][destX] = NONE;
            }
            state->lastDoublePushPawn.x = -1.0f;
            state->lastDoublePushPawn.y = -1.0f;
        }
    }

    state->board[oldY][oldX] = NONE;

    state->selectedSquare.x = -1.0f;
    state->selectedSquare.y = -1.0f;
    state->pieceActions[0] = false;
    state->pieceActions[1] = false;

    unsigned char movedPieceColor = (state->board[destY][destX] & COLOR_MASK);
    if ((state->board[destY][destX] & TYPE_MASK) == KING) {
        unsigned int colorIndex = (movedPieceColor == 0) ? 0 : 1;
        state->kingsPositions[colorIndex].x = destY;
        state->kingsPositions[colorIndex].y = destX;
    }

    state->blackTurn = !state->blackTurn;

    unsigned int nextPlayerColorIndex = (movedPieceColor == 0) ? 1 : 0;
    if (isCheck(state->board, state->kingsPositions[nextPlayerColorIndex])) {
        printf("In check!\n");
    }

    clearPossibleBoard(state->board);
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
    int boardOffset = 0;
    int squareX = (int) ((mouseX - boardOffset) / squareSize);
    int squareY = (int) (mouseY / squareSize);

    // <--- MODIFIED: Only process board input if in GAME_STATE_PLAYING
    if (currentScreenState == GAME_STATE_PLAYING) {
        if (state->mouseActions[0]) { // MOUSE CLICKED
            if (!state->pieceActions[0]) { // NO SELECTED PIECE => SELECT
                if (state->board[squareY][squareX] != 0 &&
                    !opposingColor(state->board[squareY][squareX], state->blackTurn)) {
                    selectAndHold(state, squareX, squareY);
                    generatePossibleMoves(state->board, squareY, squareX, &state->lastDoublePushPawn);
                }
            } else { // A SELECTED PIECE => TRY TO MOVE
                if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY);
                    clearPossibleBoard(state->board);
                } else {
                    deselectPiece(state);
                    clearPossibleBoard(state->board);
                }
            }
        } else if (state->mouseActions[1]) { // MOUSE RELEASED
            if (state->pieceActions[1]) { // HOLDING PIECE
                if (squareX == state->selectedSquare.x && squareY == state->selectedSquare.y) {
                    state->pieceActions[1] = false;
                } else {
                    if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                        makeMove(state, squareX, squareY);
                        clearPossibleBoard(state->board);
                    } else {
                        deselectPiece(state);
                        clearPossibleBoard(state->board);
                    }
                }
            } else {
                if ((state->board[squareY][squareX] & MOVABLE_MASK) == MOVABLE_MASK) {
                    makeMove(state, squareX, squareY);
                } else {
                    deselectPiece(state);
                    clearPossibleBoard(state->board);
                }
            }
        }
    }
    // <--- REMOVED: Save/Load button handling (it's now in main.c)
    // if (mouseX >= (boardWidth + 10) && mouseX <= (boardWidth + 10 + 130) &&
    //     mouseY >= (screenHeight - 140) && mouseY <= (screenHeight - 140 + 40)) {
    //     printf("Save Game button clicked!\n");
    //     saveGameToFile(state, inputFileNameBuffer);
    // } else if (mouseX >= (boardWidth + 160) && mouseX <= (boardWidth + 160 + 130) &&
    //            mouseY >= (screenHeight - 140) && mouseY <= (screenHeight - 140 + 40)) {
    //     printf("Load Game button clicked!\n");
    //     loadGameFromFile(state, inputFileNameBuffer);
    // }
}