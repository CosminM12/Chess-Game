#ifndef ENGINE_H
#define ENGINE_H

#include "Piece.h"
#include "util.h"
#include <stdbool.h>

#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE 500
#define QUEEN_VALUE 900
#define KING_VALUE 20000

#define MAX_DEPTH 5

typedef struct {
    Vector2f from;
    Vector2f to;
    unsigned char capturedPiece;
    bool isPromotion;
    unsigned char promotionPiece;
} Move;


typedef struct {
    Move moves[1024];
    int count;
} MoveList;

//Evaluate current position
int evaluatePosition(unsigned char board[8][8]);

//Get relative score (black and white bar)
int getRelativeScore(unsigned char board[8][8]);

//Generate all legal moves
void generateMoves(unsigned char board[8][8], unsigned char color, MoveList* moveList, Vector2f* lastDoublePawn);

//Generate Engine Move 
void makeEngineMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn);

//Restore to previous move
void unmakeMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn);

//Minimax algorithm + alpha-beta pruning
int minimax(unsigned char board[8][8], int depth, int alpha, int beta, bool maximizingPlayer, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

//Find best move
Move findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

//Check for stalemate or checkmate
bool isGameOver(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn);

//Visual representation of relative score
void getScoreBar(int score, float* whitePercentage, float* blackPercentage);

//Analyze the current position
void analyzePosition(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]);

#endif