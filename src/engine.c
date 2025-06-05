#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "engine.h"
#include "Piece.h"

/*==========
Positional evaluation for white (flipped for black)
==========*/
// Pawn position values
const int pawnTable[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

// Knight position values
const int knightTable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

// Bishop position values
const int bishopTable[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5,  5,  5,  5,  5,-10},
    {-10,  0,  5,  0,  0,  5,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

// Rook position values
const int rookTable[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

// Queen position values
const int queenTable[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

// King middle game position values
const int kingMidGameTable[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

// King end game position values
const int kingEndGameTable[8][8] = {
    {-50,-40,-30,-20,-20,-30,-40,-50},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-50,-30,-30,-30,-30,-30,-30,-50}
};

void copyBoard(unsigned char src[8][8], unsigned char dst[8][8]) {
    memcpy(dst, src, 64 * sizeof(unsigned char));
}

int evaluatePosition(unsigned char board[8][8]) {
    int score = 0;
    int materialCount = 0; //check for endgame

    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            
            if(piece == NONE) {
                continue;
            }

            switch(piece) {
                case PAWN:
                    materialCount += 1;
                    break;
                case KNIGHT:
                case BISHOP:
                    materialCount += 3;
                    break;
                case ROOK:
                    materialCount += 5;
                    break;
                case QUEEN:
                    materialCount += 9;
                    break;
            }
        }
    }

    bool isEndgame = materialCount <= 15;  ///TODO: change in accordance


    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            unsigned char color = (board[i][j] & COLOR_MASK) >> 4;

            if(piece == NONE) {
                continue;
            }

            int value = 0;
            int positionValue = 0;

            //Table coords (mirrored for black)
            int row = (color == 1) ? 7 - i : i;
            int col = (color == 1) ? 7 - j : j;

            switch(piece) {
                case PAWN:
                    value = PAWN_VALUE;
                    positionValue = pawnTable[row][col];
                    break;
                case KNIGHT:
                    value = KNIGHT_VALUE;
                    positionValue = knightTable[row][col];
                    break;
                case BISHOP:
                    value = BISHOP_VALUE;
                    positionValue = bishopTable[row][col];
                    break;
                case ROOK:
                    value = ROOK_VALUE;
                    positionValue = rookTable[row][col];
                    break;
                case QUEEN:
                    value = QUEEN_VALUE;
                    positionValue = queenTable[row][col];
                    break;
                case KING:
                    value = KING_VALUE;
                    positionValue = isEndgame ? kingEndGameTable[row][col] : kingMidGameTable[row][col];
                    break;
            }

            int total = value + positionValue;
            if(color == 1) {
                total = -total;
            }

            score += total;
        }
    }
    return score;
}

// Function to get the relative score (positive for white advantage, negative for black)
int getRelativeScore(unsigned char board[8][8]) {
    return evaluatePosition(board);
}

bool isMoveLegal(unsigned char board[8][8], Move move, unsigned char color, Vector2f* lastDoublePawn) {
    unsigned char tempBoard[8][8];
    copyBoard(board, tempBoard);

    unsigned char movingPiece = tempBoard[move.from.x][move.from.y];
    tempBoard[move.to.x][move.to.y] = movingPiece;
    tempBoard[move.from.x][move.from.y] = NONE;
    
    // Find the king
    Vector2f kingPos;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((tempBoard[i][j] & TYPE_MASK) == KING && ((tempBoard[i][j] & COLOR_MASK) >> 4) == color) {
                kingPos.x = i;
                kingPos.y = j;
                break;
            }
        }
    }
    
    // Check if the king is in check
    return !isCheck(tempBoard, kingPos);
}

// Function to generate all legal moves for a given position and color
void generateMoves(unsigned char board[8][8], unsigned char color, MoveList* moveList, Vector2f* lastDoublePawn) {
    moveList->count = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j];
            if (piece == NONE) continue;
            
            unsigned char pieceType = piece & TYPE_MASK;
            unsigned char pieceColor = (piece & COLOR_MASK) >> 4;
            
            if (pieceColor != color) continue;
            
            // Generate possible moves for this piece
            unsigned char tempBoard[8][8];
            copyBoard(board, tempBoard);
            
            // Mark the piece as selected to generate moves
            tempBoard[i][j] |= SELECTED_MASK;
            
            // Clear any previous possible moves
            clearPossibleBoard(tempBoard);
            
            // Generate possible moves for this piece
            generatePossibleMoves(tempBoard, i, j, lastDoublePawn);
            
            // Add all possible moves to the move list
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    if (tempBoard[x][y] & MOVABLE_MASK) {
                        Move move = {{i, j}, {x, y}, board[x][y], false, 0};
                        
                        // Check for pawn promotion
                        if (pieceType == PAWN && (x == 0 || x == 7)) {
                            move.isPromotion = true;
                            move.promotionPiece = QUEEN | (color << 4); // Default promotion to queen
                        }
                        
                        // Check if the move is legal (doesn't leave king in check)
                        if (isMoveLegal(board, move, color, lastDoublePawn)) {
                            moveList->moves[moveList->count++] = move;
                        }
                    }
                }
            }
        }
    }
}

// Function to make a move on the board
void makeEngineMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn) {
    unsigned char movingPiece = board[move.from.x][move.from.y];
    unsigned char pieceType = movingPiece & TYPE_MASK;
    
    // Handle pawn double push
    if (pieceType == PAWN && abs(move.to.x - move.from.x) == 2) {
        lastDoublePawn->x = move.to.x;
        lastDoublePawn->y = move.to.y;
    } else {
        lastDoublePawn->x = -1;
        lastDoublePawn->y = -1;
    }
    
    // Handle pawn promotion
    if (move.isPromotion) {
        board[move.to.x][move.to.y] = move.promotionPiece;
    } else {
        board[move.to.x][move.to.y] = movingPiece;
    }
    
    board[move.from.x][move.from.y] = NONE;
}

// Function to unmake a move (restore the board to its previous state)
void unmakeMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn) {
    // Restore the moved piece
    board[move.from.x][move.from.y] = board[move.to.x][move.to.y];
    
    // Restore the captured piece
    board[move.to.x][move.to.y] = move.capturedPiece;
    
    // Reset lastDoublePawn to previous state (would need to be tracked in a more complex way for a real engine)
    lastDoublePawn->x = -1;
    lastDoublePawn->y = -1;
}

// Minimax algorithm with alpha-beta pruning
int minimax(unsigned char board[8][8], int depth, int alpha, int beta, bool maximizingPlayer, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    // Base case: reached maximum depth or game over
    if (depth == 0) {
        return evaluatePosition(board);
    }
    
    MoveList moveList;
    generateMoves(board, maximizingPlayer ? 0 : 1, &moveList, lastDoublePawn);
    
    // Check for checkmate or stalemate
    if (moveList.count == 0) {
        // Check if the king is in check
        Vector2f kingPos = kingsPositions[maximizingPlayer ? 0 : 1];
        if (isCheck(board, kingPos)) {
            // Checkmate (worst possible score, adjusted by depth to prefer faster mates)
            return maximizingPlayer ? -100000 + depth : 100000 - depth;
        } else {
            // Stalemate
            return 0;
        }
    }
    
    if (maximizingPlayer) {
        int maxEval = INT_MIN;
        for (int i = 0; i < moveList.count; i++) {
            // Make move
            Move move = moveList.moves[i];
            unsigned char tempBoard[8][8];
            copyBoard(board, tempBoard);
            Vector2f tempLastDoublePawn = *lastDoublePawn;
            makeEngineMove(tempBoard, move, &tempLastDoublePawn);
            
            // Recursive evaluation
            int eval = minimax(tempBoard, depth - 1, alpha, beta, false, &tempLastDoublePawn, kingsPositions);
            maxEval = (eval > maxEval) ? eval : maxEval;
            
            // Alpha-beta pruning
            alpha = (alpha > eval) ? alpha : eval;
            if (beta <= alpha) {
                break;
            }
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (int i = 0; i < moveList.count; i++) {
            // Make move
            Move move = moveList.moves[i];
            unsigned char tempBoard[8][8];
            copyBoard(board, tempBoard);
            Vector2f tempLastDoublePawn = *lastDoublePawn;
            makeEngineMove(tempBoard, move, &tempLastDoublePawn);
            
            // Recursive evaluation
            int eval = minimax(tempBoard, depth - 1, alpha, beta, true, &tempLastDoublePawn, kingsPositions);
            minEval = (eval < minEval) ? eval : minEval;
            
            // Alpha-beta pruning
            beta = (beta < eval) ? beta : eval;
            if (beta <= alpha) {
                break;
            }
        }
        return minEval;
    }
}

// Function to find the best move for the current position
Move findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    MoveList moveList;
    generateMoves(board, color, &moveList, lastDoublePawn);
    
    if (moveList.count == 0) {
        // No legal moves
        Move nullMove = {{-1, -1}, {-1, -1}, 0, false, 0};
        return nullMove;
    }
    
    int bestValue = (color == 0) ? INT_MIN : INT_MAX;
    int bestMoveIndex = 0;
    
    for (int i = 0; i < moveList.count; i++) {
        Move move = moveList.moves[i];
        unsigned char tempBoard[8][8];
        copyBoard(board, tempBoard);
        Vector2f tempLastDoublePawn = *lastDoublePawn;
        
        makeEngineMove(tempBoard, move, &tempLastDoublePawn);
        
        int moveValue = minimax(tempBoard, MAX_DEPTH - 1, INT_MIN, INT_MAX, color == 1, &tempLastDoublePawn, kingsPositions);
        
        if ((color == 0 && moveValue > bestValue) || (color == 1 && moveValue < bestValue)) {
            bestValue = moveValue;
            bestMoveIndex = i;
        }
    }
    
    return moveList.moves[bestMoveIndex];
}

// Function to check if the game is over (checkmate or stalemate)
bool isGameOver(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn) {
    MoveList moveList;
    generateMoves(board, color, &moveList, lastDoublePawn);
    
    // Find king position
    Vector2f kingPos;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((board[i][j] & TYPE_MASK) == KING && ((board[i][j] & COLOR_MASK) >> 4) == color) {
                kingPos.x = i;
                kingPos.y = j;
                break;
            }
        }
    }
    
    // No legal moves and king in check = checkmate
    // No legal moves and king not in check = stalemate
    return moveList.count == 0;
}

// Function to get a visual representation of the relative score (for UI)
void getScoreBar(int score, float* whitePercentage, float* blackPercentage) {
    // Convert score to a percentage (capped at ±2000 centipawns)
    float normalizedScore = score / 2000.0f;
    if (normalizedScore > 1.0f) normalizedScore = 1.0f;
    if (normalizedScore < -1.0f) normalizedScore = -1.0f;
    
    // Convert to percentages (50% each when equal)
    if (normalizedScore >= 0) {
        *whitePercentage = 0.5f + (normalizedScore * 0.5f);
        *blackPercentage = 1.0f - *whitePercentage;
    } else {
        *blackPercentage = 0.5f + (-normalizedScore * 0.5f);
        *whitePercentage = 1.0f - *blackPercentage;
    }
}

// Function to analyze the current position and provide insights
void analyzePosition(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    int score = evaluatePosition(board);
    
    printf("Position analysis:\n");
    printf("Current evaluation: %d centipawns\n", score);
    
    if (score > 0) {
        printf("White has an advantage\n");
    } else if (score < 0) {
        printf("Black has an advantage\n");
    } else {
        printf("Position is equal\n");
    }
    
    // Find best move
    Move bestMove = findBestMove(board, color, lastDoublePawn, kingsPositions);
    
    if (bestMove.from.x != -1) {
        printf("Best move: %c%d to %c%d\n", 
               'a' + bestMove.from.y, 8 - bestMove.from.x,
               'a' + bestMove.to.y, 8 - bestMove.to.x);
    } else {
        printf("No legal moves available\n");
    }
    
    // Check for checks and checkmates
    Vector2f kingPos = kingsPositions[color];
    if (isCheck(board, kingPos)) {
        printf("King is in check!\n");
        
        MoveList moveList;
        generateMoves(board, color, &moveList, lastDoublePawn);
        
        if (moveList.count == 0) {
            printf("Checkmate!\n");
        } else {
            printf("There are %d legal moves to get out of check\n", moveList.count);
        }
    }
}