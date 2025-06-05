#ifndef ENGINE_H
#define ENGINE_H

#include "Piece.h"
#include "util.h"
#include <stdbool.h>

// Piece values
#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

// Search parameters
#define MAX_DEPTH 3
#define MAX_PV_LENGTH 64

typedef struct {
    Vector2f from;
    Vector2f to;
    unsigned char capturedPiece;
    bool isPromotion;
    unsigned char promotionPiece;
    bool originalModifier;  // Was the MODIFIER flag set on the original piece?
    int safetyScore;       // Score indicating move safety (negative = risky)
} Move;

typedef struct {
    Move moves[1024];
    int count;
} MoveList;

// Core Functional Modules
void generateMoves(unsigned char board[8][8], unsigned char color, MoveList* moveList, Vector2f* lastDoublePawn);
void generateLegalMoves(unsigned char board[8][8], unsigned char color, MoveList* list, Vector2f* lastDoublePawn, Vector2f kings[]);
void generatePseudoLegalMoves(unsigned char board[8][8], unsigned char color, MoveList* list, Vector2f* lastDoublePawn);
bool isMoveLegal(unsigned char board[8][8], Move move, unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);
void engineMakeMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn, Vector2f kings[]);
void unmakeMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn, Vector2f kings[]);
bool isKingInCheck(unsigned char board[8][8], Vector2f kingPosition, unsigned char color);
bool isSquareAttacked(unsigned char board[8][8], Vector2f position, unsigned char attackerColor);

// Search Functions
int minimax(unsigned char board[8][8], int depth, int alpha, int beta, bool isMaximizingPlayer,
            unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);
Move findBestMoveWithMinimax(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);
Move findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);

// Evaluation System
int evaluatePosition(unsigned char board[8][8], unsigned char color);
int getPhase(unsigned char board[8][8]);
int evaluateMaterial(unsigned char board[8][8]);
int evaluateMobility(unsigned char board[8][8], unsigned char color);
int evaluateKingSafety(unsigned char board[8][8], unsigned char color, Vector2f kings[]);
int evaluatePawnStructure(unsigned char board[8][8], unsigned char color);
int evaluatePieceSquareTables(unsigned char board[8][8], unsigned char color, int phase);
int evaluateMoveSafety(unsigned char board[8][8], Move move, unsigned char color);
void evaluateMovesSafety(unsigned char board[8][8], unsigned char color, MoveList* moveList);

// Move Generation Helpers
void orderMoves(MoveList* moveList);
int quiescenceSearch(unsigned char board[8][8], int alpha, int beta, unsigned char color, 
                    Vector2f* lastDoublePawn, Vector2f kings[]);

// Helper Functions
void copyBoard(unsigned char src[8][8], unsigned char dst[8][8]);
bool isGameOver(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);
void getScoreBar(int score, float* whitePercentage, float* blackPercentage);
void analyzePosition(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]);

// Move function for the AI
void makeEngineMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

// Function to get the relative score based on the current player's perspective
int getRelativeScore(unsigned char board[8][8]);

#endif