// src/engine.h
#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include "util.h"
#include "Piece.h"

// Piece values for evaluation
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

// Search parameters
#define MAX_DEPTH 3
#define MAX_PV_LENGTH 64
#define MAX_MOVES_PER_POSITION 1024

typedef struct {
    Vector2f from;
    Vector2f to;
    unsigned char capturedPiece;
    bool isPromotion;
    unsigned char promotionPiece;
    bool originalModifier;  // Was the MODIFIER flag set on the original piece?
    int safetyScore;       // Score indicating move safety (negative = risky)
} EngineMove;

typedef struct {
    EngineMove moves[MAX_MOVES_PER_POSITION];
    int count;
} MoveList;

// Function to initialize the engine
void initializeEngine();

// Function to generate all legal moves for the current position
void generateMoves(unsigned char board[8][8], unsigned char color, MoveList* moveList, Vector2f* lastDoublePawn);

// Function to find the best move for the AI
EngineMove findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);

// Corrected prototype for engineMakeMove:
void engineMakeMove(unsigned char board[8][8], EngineMove move, Vector2f* lastDoublePawn, Vector2f kingsPositions[], int isRealMove); // Changed function name and added 'int isRealMove' parameter

// Function to get the relative score based on the current player's perspective
int evaluatePosition(unsigned char board[8][8], unsigned char color);

// Function to convert numerical score to evaluation bar percentages
void getScoreBar(int score, float* whitePercentage, float* blackPercentage);

// Function to evaluate the safety of moves
void evaluateMovesSafety(unsigned char board[8][8], unsigned char color, MoveList* moveList);

#endif