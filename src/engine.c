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
// Pawn position values - middlegame
const int pawnTableMG[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

// Pawn position values - endgame
const int pawnTableEG[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {80, 80, 80, 80, 80, 80, 80, 80},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {30, 30, 30, 30, 30, 30, 30, 30},
    {20, 20, 20, 20, 20, 20, 20, 20},
    {10, 10, 10, 10, 10, 10, 10, 10},
    {10, 10, 10, 10, 10, 10, 10, 10},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

// Knight position values - middlegame
const int knightTableMG[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

// Knight position values - endgame
const int knightTableEG[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

// Bishop position values - middlegame
const int bishopTableMG[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5,  5,  5,  5,  5,-10},
    {-10,  0,  5,  0,  0,  5,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

// Bishop position values - endgame
const int bishopTableEG[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5,  5,  5,  5,  5,-10},
    {-10,  0,  5,  0,  0,  5,  0,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

// Rook position values - middlegame
const int rookTableMG[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

// Rook position values - endgame
const int rookTableEG[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

// Queen position values - middlegame
const int queenTableMG[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

// Queen position values - endgame
const int queenTableEG[8][8] = {
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
const int kingTableMG[8][8] = {
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
const int kingTableEG[8][8] = {
    {-50,-40,-30,-20,-20,-30,-40,-50},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-50,-30,-30,-30,-30,-30,-30,-50}
};

// Helper Functions
void copyBoard(unsigned char src[8][8], unsigned char dst[8][8]) {
    memcpy(dst, src, 64 * sizeof(unsigned char));
}

// Determine game phase (0-256, where 0 is endgame and 256 is opening)
int getPhase(unsigned char board[8][8]) {
    int phase = 256; // Start with maximum (opening)
    
    // Count all pieces except pawns and kings
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            if (piece == NONE || piece == PAWN || piece == KING) {
                continue;
            }

            // Reduce phase based on piece type
            switch (piece) {
                case KNIGHT:
                case BISHOP:
                    phase -= 8; // 32 points total for all knights and bishops
                    break;
                case ROOK:
                    phase -= 13; // 26 points total for all rooks
                    break;
                case QUEEN:
                    phase -= 24; // 48 points total for all queens
                    break;
            }
        }
    }

    // Ensure phase is in valid range
    if (phase < 0) phase = 0;
    if (phase > 256) phase = 256;
    
    return phase;
}

// Check if a square is attacked by a piece of the given color
bool isSquareAttacked(unsigned char board[8][8], Vector2f position, unsigned char attackerColor) {
    // Check for pawn attacks
    int pawnDir = (attackerColor == 0) ? 1 : -1;
    
    // Check diagonal pawn captures
    if (position.x - pawnDir >= 0 && position.x - pawnDir < 8) {
        if (position.y - 1 >= 0) {
            unsigned char piece = board[position.x - pawnDir][position.y - 1];
            if ((piece & TYPE_MASK) == PAWN && ((piece & COLOR_MASK) >> 4) == attackerColor) {
                return true;
            }
        }
        
        if (position.y + 1 < 8) {
            unsigned char piece = board[position.x - pawnDir][position.y + 1];
            if ((piece & TYPE_MASK) == PAWN && ((piece & COLOR_MASK) >> 4) == attackerColor) {
                return true;
            }
        }
    }
    
    // Check knight attacks
    int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (int i = 0; i < 8; i++) {
        int nx = position.x + knightMoves[i][0];
        int ny = position.y + knightMoves[i][1];
        
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            unsigned char piece = board[nx][ny];
            if ((piece & TYPE_MASK) == KNIGHT && ((piece & COLOR_MASK) >> 4) == attackerColor) {
                return true;
            }
        }
    }
    
    // Check diagonal attacks (bishop, queen)
    int diagonalDirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = diagonalDirs[i][0];
        int dy = diagonalDirs[i][1];
        int x = position.x + dx;
        int y = position.y + dy;
        
        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
            unsigned char piece = board[x][y];
            if (piece != NONE) {
                if (((piece & TYPE_MASK) == BISHOP || (piece & TYPE_MASK) == QUEEN) && 
                    ((piece & COLOR_MASK) >> 4) == attackerColor) {
                    return true;
                }
                break;
            }
            x += dx;
            y += dy;
        }
    }
    
    // Check horizontal/vertical attacks (rook, queen)
    int straightDirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (int i = 0; i < 4; i++) {
        int dx = straightDirs[i][0];
        int dy = straightDirs[i][1];
        int x = position.x + dx;
        int y = position.y + dy;
        
        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
            unsigned char piece = board[x][y];
            if (piece != NONE) {
                if (((piece & TYPE_MASK) == ROOK || (piece & TYPE_MASK) == QUEEN) && 
                    ((piece & COLOR_MASK) >> 4) == attackerColor) {
                    return true;
                }
                break;
            }
            x += dx;
            y += dy;
        }
    }
    
    // Check king attacks
    int kingMoves[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
        {0, 1}, {1, -1}, {1, 0}, {1, 1}
    };
    
    for (int i = 0; i < 8; i++) {
        int nx = position.x + kingMoves[i][0];
        int ny = position.y + kingMoves[i][1];
        
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            unsigned char piece = board[nx][ny];
            if ((piece & TYPE_MASK) == KING && ((piece & COLOR_MASK) >> 4) == attackerColor) {
                return true;
            }
        }
    }
    
    return false;
}

// Check if the king is in check
bool isKingInCheck(unsigned char board[8][8], Vector2f kingPosition, unsigned char color) {
    return isCheck(board, kingPosition); // Use the isCheck function from Piece.c
}

// Generate pseudo-legal moves (without checking if they leave king in check)
void generatePseudoLegalMoves(unsigned char board[8][8], unsigned char color, MoveList* list, Vector2f* lastDoublePawn) {
    list->count = 0;
    
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
                        // Get the original piece's MODIFIER state
                        bool hasModifier = (board[i][j] & MODIFIER) != 0;
                        
                        Move move = {{i, j}, {x, y}, board[x][y], false, 0, hasModifier, 0};
                        
                        // Check for pawn promotion
                        if (pieceType == PAWN && (x == 0 || x == 7)) {
                            move.isPromotion = true;
                            move.promotionPiece = QUEEN | (color << 4); // Default promotion to queen
                        }
                        
                        list->moves[list->count++] = move;
                    }
                }
            }
        }
    }
}

// Check if a move is legal (doesn't leave king in check)
bool isMoveLegal(unsigned char board[8][8], Move move, unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {
    unsigned char tempBoard[8][8];
    copyBoard(board, tempBoard);
    Vector2f tempKings[2] = {kings[0], kings[1]};
    
    // Make the move on the temporary board
    engineMakeMove(tempBoard, move, lastDoublePawn, tempKings);
    
    // Check if the king is in check after the move
    return !isKingInCheck(tempBoard, tempKings[color], color);
}

// Generate all legal moves
void generateLegalMoves(unsigned char board[8][8], unsigned char color, MoveList* list, Vector2f* lastDoublePawn, Vector2f kings[]) {
    MoveList pseudoLegalMoves;
    generatePseudoLegalMoves(board, color, &pseudoLegalMoves, lastDoublePawn);
    
    // Filter out moves that leave the king in check
    list->count = 0;
    for (int i = 0; i < pseudoLegalMoves.count; i++) {
        if (isMoveLegal(board, pseudoLegalMoves.moves[i], color, lastDoublePawn, kings)) {
            list->moves[list->count++] = pseudoLegalMoves.moves[i];
        }
    }
}

// Make a move on the board
void engineMakeMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn, Vector2f kings[]) {
    unsigned char movingPiece = board[move.from.x][move.from.y];
    unsigned char pieceType = movingPiece & TYPE_MASK;
    unsigned char color = (movingPiece & COLOR_MASK) >> 4;

    // For kings and rooks, always clear the MODIFIER flag from source position
    if (pieceType == KING || pieceType == ROOK) {
        // Clear the MODIFIER flag from the original position
        board[move.from.x][move.from.y] &= ~MODIFIER;
    }

    // Handle pawn double push
    if (pieceType == PAWN && abs(move.to.x - move.from.x) == 2) {
        lastDoublePawn->x = move.to.x;
        lastDoublePawn->y = move.to.y;
    } else {
        lastDoublePawn->x = -1;
        lastDoublePawn->y = -1;
    }

    // Update king position if the king moves
    if (pieceType == KING) {
        kings[color] = move.to;
        
        // Handle castling
        if (abs(move.to.y - move.from.y) == 2) {
            // Kingside castling (short castle)
            if (move.to.y > move.from.y) {
                // Move the rook from the kingside to its new position
                // Don't set MODIFIER flag on the rook
                board[move.to.x][move.to.y - 1] = (ROOK | (color << 4));
                board[move.to.x][7] = 0; // Remove the rook from its original position
            }
            // Queenside castling (long castle)
            else {
                // Move the rook from the queenside to its new position
                // Don't set MODIFIER flag on the rook
                board[move.to.x][move.to.y + 1] = (ROOK | (color << 4));
                board[move.to.x][0] = 0; // Remove the rook from its original position
            }
        }
    }

    // Prepare the new piece state
    unsigned char newPiece = move.isPromotion ? move.promotionPiece : movingPiece;
    
    // For KING and ROOK, clear the MODIFIER flag to indicate they have moved
    // This permanently disallows castling with this piece
    if (pieceType == KING || pieceType == ROOK) {
        // Clear the MODIFIER flag
        newPiece &= ~MODIFIER;
    }
    // For PAWN, set the MODIFIER flag
    else if (pieceType == PAWN) {
        newPiece |= MODIFIER;
    }
    
    // Place the piece on the board
    board[move.to.x][move.to.y] = newPiece;
    board[move.from.x][move.from.y] = NONE;
}

// Evaluation Functions

// Material evaluation
int evaluateMaterial(unsigned char board[8][8]) {
    int score = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            unsigned char color = (board[i][j] & COLOR_MASK) >> 4;

            if (piece == NONE) continue;

            int value = 0;
            switch (piece) {
                case PAWN:
                    value = PAWN_VALUE;
                    break;
                case KNIGHT:
                    value = KNIGHT_VALUE;
                    break;
                case BISHOP:
                    value = BISHOP_VALUE;
                    break;
                case ROOK:
                    value = ROOK_VALUE;
                    break;
                case QUEEN:
                    value = QUEEN_VALUE;
                    break;
                case KING:
                    value = KING_VALUE;
                    break;
            }

            if (color == 1) {
                value = -value;
            }

            score += value;
        }
    }
    
    return score;
}

// Mobility evaluation (count legal moves)
int evaluateMobility(unsigned char board[8][8], unsigned char color) {
    MoveList moveList;
    Vector2f lastDoublePawn = {-1, -1};
    Vector2f kings[2];
    findKings(board, kings);
    
    // Generate moves for white
    generateLegalMoves(board, 0, &moveList, &lastDoublePawn, kings);
    int whiteMobility = moveList.count;
    
    // Generate moves for black
    generateLegalMoves(board, 1, &moveList, &lastDoublePawn, kings);
    int blackMobility = moveList.count;
    
    // Return mobility difference (positive for white advantage)
    return (whiteMobility - blackMobility) * 5; // 5 centipawns per move advantage
}

// King safety evaluation
int evaluateKingSafety(unsigned char board[8][8], unsigned char color, Vector2f kings[]) {
    int whiteKingSafety = 0;
    int blackKingSafety = 0;
    
    // Check if kings are castled or in the center
    Vector2f whiteKing = kings[0];
    Vector2f blackKing = kings[1];
    
    // Penalize kings in the center
    if (whiteKing.y >= 2 && whiteKing.y <= 5 && whiteKing.x < 7) {
        whiteKingSafety -= 30;
    }
    
    if (blackKing.y >= 2 && blackKing.y <= 5 && blackKing.x > 0) {
        blackKingSafety -= 30;
    }
    
    // Check pawn shield for white king
    if (whiteKing.x == 7) {
        for (int j = whiteKing.y - 1; j <= whiteKing.y + 1; j++) {
            if (j >= 0 && j < 8) {
                if (board[6][j] != NONE && (board[6][j] & TYPE_MASK) == PAWN && 
                    ((board[6][j] & COLOR_MASK) >> 4) == 0) {
                    whiteKingSafety += 15; // Pawn shield bonus
                }
            }
        }
    }
    
    // Check pawn shield for black king
    if (blackKing.x == 0) {
        for (int j = blackKing.y - 1; j <= blackKing.y + 1; j++) {
            if (j >= 0 && j < 8) {
                if (board[1][j] != NONE && (board[1][j] & TYPE_MASK) == PAWN && 
                    ((board[1][j] & COLOR_MASK) >> 4) == 1) {
                    blackKingSafety += 15; // Pawn shield bonus
                }
            }
        }
    }
    
    // Check if king is in check
    if (isKingInCheck(board, kings[0], 0)) {
        whiteKingSafety -= 50;
    }
    
    if (isKingInCheck(board, kings[1], 1)) {
        blackKingSafety -= 50;
    }
    
    return whiteKingSafety - blackKingSafety;
}

// Pawn structure evaluation
int evaluatePawnStructure(unsigned char board[8][8], unsigned char color) {
    int whiteScore = 0;
    int blackScore = 0;
    
    // Count pawns in each file for doubled pawns detection
    int whitePawnsInFile[8] = {0};
    int blackPawnsInFile[8] = {0};
    
    // Check for passed pawns, isolated pawns, etc.
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((board[i][j] & TYPE_MASK) == PAWN) {
                unsigned char pawnColor = (board[i][j] & COLOR_MASK) >> 4;
                
                if (pawnColor == 0) { // White pawn
                    whitePawnsInFile[j]++;
                    
                    // Check for passed pawns
                    bool passed = true;
                    for (int r = i - 1; r >= 0; r--) {
                        for (int c = j - 1; c <= j + 1; c++) {
                            if (c >= 0 && c < 8) {
                                if ((board[r][c] & TYPE_MASK) == PAWN && 
                                    ((board[r][c] & COLOR_MASK) >> 4) == 1) {
                                    passed = false;
                                    break;
                                }
                            }
                        }
                        if (!passed) break;
                    }
                    
                    if (passed) {
                        whiteScore += 30 + (7 - i) * 10; // More bonus as pawn advances
                    }
                    
                } else { // Black pawn
                    blackPawnsInFile[j]++;
                    
                    // Check for passed pawns
                    bool passed = true;
                    for (int r = i + 1; r < 8; r++) {
                        for (int c = j - 1; c <= j + 1; c++) {
                            if (c >= 0 && c < 8) {
                                if ((board[r][c] & TYPE_MASK) == PAWN && 
                                    ((board[r][c] & COLOR_MASK) >> 4) == 0) {
                                    passed = false;
                break;
                                }
                            }
                        }
                        if (!passed) break;
                    }
                    
                    if (passed) {
                        blackScore += 30 + i * 10; // More bonus as pawn advances
                    }
                }
            }
        }
    }
    
    // Penalize doubled pawns
    for (int j = 0; j < 8; j++) {
        if (whitePawnsInFile[j] > 1) {
            whiteScore -= 15 * (whitePawnsInFile[j] - 1);
        }
        if (blackPawnsInFile[j] > 1) {
            blackScore -= 15 * (blackPawnsInFile[j] - 1);
        }
    }
    
    // Penalize isolated pawns
    for (int j = 0; j < 8; j++) {
        if (whitePawnsInFile[j] > 0) {
            bool isolated = true;
            if (j > 0 && whitePawnsInFile[j - 1] > 0) isolated = false;
            if (j < 7 && whitePawnsInFile[j + 1] > 0) isolated = false;
            
            if (isolated) {
                whiteScore -= 20;
            }
        }
        
        if (blackPawnsInFile[j] > 0) {
            bool isolated = true;
            if (j > 0 && blackPawnsInFile[j - 1] > 0) isolated = false;
            if (j < 7 && blackPawnsInFile[j + 1] > 0) isolated = false;
            
            if (isolated) {
                blackScore -= 20;
            }
        }
    }
    
    return whiteScore - blackScore;
}

// Piece-square table evaluation
int evaluatePieceSquareTables(unsigned char board[8][8], unsigned char color, int phase) {
    int score = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j] & TYPE_MASK;
            unsigned char pieceColor = (board[i][j] & COLOR_MASK) >> 4;
            
            if (piece == NONE) continue;
            
            // Table coords (mirrored for black)
            int row = (pieceColor == 1) ? i : 7 - i;
            int col = (pieceColor == 1) ? j : 7 - j;
            
            int mgScore = 0;
            int egScore = 0;
            
            switch (piece) {
                case PAWN:
                    mgScore = pawnTableMG[row][col];
                    egScore = pawnTableEG[row][col];
                    break;
                case KNIGHT:
                    mgScore = knightTableMG[row][col];
                    egScore = knightTableEG[row][col];
                    break;
                case BISHOP:
                    mgScore = bishopTableMG[row][col];
                    egScore = bishopTableEG[row][col];
                    break;
                case ROOK:
                    mgScore = rookTableMG[row][col];
                    egScore = rookTableEG[row][col];
                    break;
                case QUEEN:
                    mgScore = queenTableMG[row][col];
                    egScore = queenTableEG[row][col];
                    break;
                case KING:
                    mgScore = kingTableMG[row][col];
                    egScore = kingTableEG[row][col];
                    break;
            }
            
            // Interpolate between middlegame and endgame scores based on phase
            int positionScore = (mgScore * phase + egScore * (256 - phase)) / 256;
            
            // Adjust score based on piece color
            if (pieceColor == 1) {
                positionScore = -positionScore;
            }
            
            score += positionScore;
        }
    }
    
    return score;
}

// Main evaluation function
int evaluatePosition(unsigned char board[8][8], unsigned char color) {
    Vector2f kings[2];
    findKings(board, kings);
    
    int phase = getPhase(board);
    
    // Material evaluation (most important)
    int materialScore = evaluateMaterial(board);
    
    // Piece-square tables
    int pstScore = evaluatePieceSquareTables(board, color, phase);
    
    // Mobility evaluation
    int mobilityScore = evaluateMobility(board, color);
    
    // King safety
    int kingSafetyScore = evaluateKingSafety(board, color, kings);
    
    // Pawn structure
    int pawnStructureScore = evaluatePawnStructure(board, color);
    
    // Combine all evaluation terms
    int totalScore = materialScore + 
                    pstScore + 
                    mobilityScore + 
                    kingSafetyScore +
                    pawnStructureScore;
    
    return totalScore;
}

// Function to get the relative score (positive for white advantage, negative for black)
int getRelativeScore(unsigned char board[8][8]) {
    // Evaluate from white's perspective
    int whiteScore = evaluatePosition(board, 0);
    return whiteScore;
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
                        // Get the original piece's MODIFIER state
                        bool hasModifier = (board[i][j] & MODIFIER) != 0;
                        
                        Move move = {{i, j}, {x, y}, board[x][y], false, 0, hasModifier, 0};
                        
                        // Check for pawn promotion
                        if (pieceType == PAWN && (x == 0 || x == 7)) {
                            move.isPromotion = true;
                            move.promotionPiece = QUEEN | (color << 4); // Default promotion to queen
                        }
                        
                        // Check if the move is legal (doesn't leave king in check)
                        Vector2f kings[2]; // We need to find kings for this check
                        findKings(board, kings);
                        if (isMoveLegal(board, move, color, lastDoublePawn, kings)) {
                            moveList->moves[moveList->count++] = move;
                        }
                    }
                }
            }
        }
    }
}

// Function to make a move on the board
void makeEngineMove(unsigned char board[8][8], Move move, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    // Delegate to the main move function
    engineMakeMove(board, move, lastDoublePawn, kingsPositions);
}

// Minimax with alpha-beta pruning, evaluating future outcomes
int minimax(unsigned char board[8][8], int depth, int alpha, int beta, bool isMaximizingPlayer,
            unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {

    // Terminal condition
    if (depth == 0 || isGameOver(board, color, lastDoublePawn, kings)) {
        return evaluatePosition(board, color); // Dynamic evaluation based on the position
    }

    MoveList moveList;
    generateLegalMoves(board, color, &moveList, lastDoublePawn, kings);

    if (moveList.count == 0) {
        return evaluatePosition(board, color); // Handles checkmate/stalemate as part of eval
    }

    int bestScore = isMaximizingPlayer ? -100000 : 100000;

    for (int i = 0; i < moveList.count; ++i) {
        Move move = moveList.moves[i];

        // Backup state
        unsigned char tempBoard[8][8];
        copyBoard(board, tempBoard);
        Vector2f tempLastDoublePawn = *lastDoublePawn;
        Vector2f tempKings[2] = {kings[0], kings[1]};

        engineMakeMove(board, move, lastDoublePawn, kings);

        // Alternate maximizing/minimizing and switch sides (XOR 0x10 flips color)
        int score = minimax(board, depth - 1, alpha, beta, !isMaximizingPlayer, color ^ 0x10, lastDoublePawn, kings);

        // Restore
        copyBoard(tempBoard, board);
        *lastDoublePawn = tempLastDoublePawn;
        kings[0] = tempKings[0];
        kings[1] = tempKings[1];

        if (isMaximizingPlayer) {
            if (score > bestScore) bestScore = score;
            if (score > alpha) alpha = score;
        } else {
            if (score < bestScore) bestScore = score;
            if (score < beta) beta = score;
        }

        if (beta <= alpha)
            break; // Prune
    }

    return bestScore;
}

// Top-level function to get the best move using the full tree search
Move findBestMoveWithMinimax(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {
    MoveList moveList;
    generateLegalMoves(board, color, &moveList, lastDoublePawn, kings);

    if (moveList.count == 0) {
        // No legal moves available
        Move dummyMove = {{-1, -1}, {-1, -1}, 0, false, 0, false, 0};
        return dummyMove;
    }

    int bestScore = -100000;
    Move bestMove = moveList.moves[0];

    for (int i = 0; i < moveList.count; ++i) {
        Move move = moveList.moves[i];

        // Backup
        unsigned char tempBoard[8][8];
        copyBoard(board, tempBoard);
        Vector2f tempLastDoublePawn = *lastDoublePawn;
        Vector2f tempKings[2] = {kings[0], kings[1]};

        engineMakeMove(board, move, lastDoublePawn, kings);

        int score = minimax(board, MAX_DEPTH - 1, -100000, 100000, false, color ^ 0x10, lastDoublePawn, kings);

        // Undo
        copyBoard(tempBoard, board);
        *lastDoublePawn = tempLastDoublePawn;
        kings[0] = tempKings[0];
        kings[1] = tempKings[1];

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

// Function to check if the game is over (checkmate or stalemate)
bool isGameOver(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    MoveList moveList;
    generateMoves(board, color, &moveList, lastDoublePawn);
    
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
    int score = evaluatePosition(board, color);
    
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
    Move bestMove = findBestMoveWithMinimax(board, color, lastDoublePawn, kingsPositions);
    
    if (bestMove.from.x != -1) {
        printf("Best move: %c%d to %c%d\n", 
               'a' + bestMove.from.y, 8 - bestMove.from.x,
               'a' + bestMove.to.y, 8 - bestMove.to.x);
    } else {
        printf("No legal moves available\n");
    }
    
    // Check for checks and checkmates
    Vector2f kingPos = kingsPositions[color];
    if (isKingInCheck(board, kingPos, color)) {
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

// Function to evaluate if a piece would be captured after moving to a square
// Returns a score based on the net material change from the exchange
int evaluateMoveSafety(unsigned char board[8][8], Move move, unsigned char color) {
    // Get the value of the piece being moved
    int movingPieceValue = 0;
    unsigned char pieceType = board[move.from.x][move.from.y] & TYPE_MASK;
    
    switch (pieceType) {
        case PAWN:   movingPieceValue = PAWN_VALUE; break;
        case KNIGHT: movingPieceValue = KNIGHT_VALUE; break;
        case BISHOP: movingPieceValue = BISHOP_VALUE; break;
        case ROOK:   movingPieceValue = ROOK_VALUE; break;
        case QUEEN:  movingPieceValue = QUEEN_VALUE; break;
        case KING:   movingPieceValue = KING_VALUE; break;
        default:     return 0; // No piece or unknown piece
    }
    
    // Check if the destination square is attacked by opponent
    Vector2f destination = {move.to.x, move.to.y};
    unsigned char enemyColor = 1 - color;
    
    if (isSquareAttacked(board, destination, enemyColor)) {
        // Piece would be hanging - estimate the material loss
        // For simplicity, we assume the worst case: total loss of piece value
        // A more sophisticated SEE (Static Exchange Evaluation) would be better
        return -movingPieceValue;
    }
    
    return 0; // No immediate threat detected
}

// Function to evaluate all legal moves and determine their safety
void evaluateMovesSafety(unsigned char board[8][8], unsigned char color, MoveList* moveList) {
    // Reset safety scores for all moves
    for (int i = 0; i < moveList->count; i++) {
        moveList->moves[i].safetyScore = 0;
    }
    
    // For each move, check if it puts the piece at risk
    for (int i = 0; i < moveList->count; i++) {
        // Make a temporary copy of the board
        unsigned char tempBoard[8][8];
        copyBoard(board, tempBoard);
        
        // Remove the piece from its original position
        unsigned char movingPiece = tempBoard[moveList->moves[i].from.x][moveList->moves[i].from.y];
        tempBoard[moveList->moves[i].from.x][moveList->moves[i].from.y] = 0;
        
        // Place it at the destination
        tempBoard[moveList->moves[i].to.x][moveList->moves[i].to.y] = movingPiece;
        
        // Evaluate safety on the new position
        int safetyScore = evaluateMoveSafety(tempBoard, moveList->moves[i], color);
        moveList->moves[i].safetyScore = safetyScore;
    }
}

// Legacy wrapper for findBestMove to maintain compatibility
Move findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    return findBestMoveWithMinimax(board, color, lastDoublePawn, kingsPositions);
}