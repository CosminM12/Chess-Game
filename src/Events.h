#ifndef EVENTS_H
#define EVENTS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "util.h"
#include "engine.h"
#include "GameState.h" // Include GameState.h for the GameState struct

// Function declaration from main.c for position analysis (if still needed, ensure main.h includes this)
// void displayPositionAnalysis(unsigned char board[8][8], bool blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]); // Removed if main.c doesn't need it.

// External variables for game mode and computer player status
extern int gameMode;
extern bool computerPlaysBlack;

// Updated prototypes to accept GameState* state
void getEvents(SDL_Event event, GameState *state, int *scrollOffset); // Keep if scrollOffset is still external
bool mouseInsideBoard(int mouseX, int mouseY, int screenWidth, int squareSize);

void selectAndHold(GameState* state, int squareX, int squareY);
bool isMoveLegal(GameState* state, int destX, int destY); // Updated signature
void makeMove(GameState* state, int destX, int destY); // Updated signature
void deselectPiece(GameState* state); // Updated signature
void handleMouseInput(GameState* state, int mouseX, int mouseY, int squareSize); // Updated signature

// UPDATED PROTOYPE: add GameState* state parameter (already there)
void addMoveToHistory(GameState* state, int startRow, int startCol, int endRow, int endCol, unsigned char piece);

#endif