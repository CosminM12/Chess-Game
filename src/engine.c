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
    if (position.x + pawnDir >= 0 && position.x + pawnDir < 8) {
        if (position.y - 1 >= 0) {
            unsigned char piece = board[position.x + pawnDir][position.y - 1];
            if ((piece & TYPE_MASK) == PAWN && ((piece & COLOR_MASK) >> 4) == attackerColor) {
                return true;
            }
        }
        
        if (position.y + 1 < 8) {
            unsigned char piece = board[position.x + pawnDir][position.y + 1];
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
    
    // Generate all possible moves for the given color
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j];
            unsigned char pieceType = piece & TYPE_MASK;
            unsigned char pieceColor = (piece & COLOR_MASK) >> 4;
            
            // Skip empty squares and pieces of the wrong color
            if (pieceType == NONE || pieceColor != color) {
                continue;
            }
            
            // Generate moves based on piece type
            switch (pieceType) {
                case PAWN: {
                    // Pawn movement direction depends on color
                    int direction = (color == 0) ? -1 : 1;
                    
                    // Forward move (1 square)
                    int newRow = i + direction;
                    if (newRow >= 0 && newRow < 8 && board[newRow][j] == NONE) {
                        // Get the original piece's MODIFIER state
                        bool hasModifier = (board[i][j] & MODIFIER) != 0;
                        
                        // Check for promotion
                        if (newRow == 0 || newRow == 7) {
                            // Generate a move for each promotion piece type
                            unsigned char promotionPieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
                            for (int p = 0; p < 4; p++) {
                                EngineMove move = {{i, j}, {newRow, j}, NONE, true, promotionPieces[p] | (color << 4), hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                        } else {
                            // Regular move
                            EngineMove move = {{i, j}, {newRow, j}, NONE, false, 0, hasModifier, 0};
                            list->moves[list->count++] = move;
                        }
                    }
                    
                    // Forward move (2 squares) - only from starting position
                    if ((color == 0 && i == 6) || (color == 1 && i == 1)) {
                        int doubleRow = i + 2 * direction;
                        if (board[i + direction][j] == NONE && board[doubleRow][j] == NONE) {
                            bool hasModifier = (board[i][j] & MODIFIER) != 0;
                            EngineMove move = {{i, j}, {doubleRow, j}, NONE, false, 0, hasModifier, 0};
                            list->moves[list->count++] = move;
                        }
                    }
                    
                    // Diagonal captures
                    for (int offset = -1; offset <= 1; offset += 2) {
                        int newCol = j + offset;
                        if (newCol >= 0 && newCol < 8) {
                            int newRow = i + direction;
                            if (newRow >= 0 && newRow < 8) {
                                unsigned char targetPiece = board[newRow][newCol];
                                unsigned char targetType = targetPiece & TYPE_MASK;
                                unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                                
                                // Regular capture
                                if (targetType != NONE && targetColor != color) {
                        bool hasModifier = (board[i][j] & MODIFIER) != 0;
                        
                                    // Check for promotion
                                    if (newRow == 0 || newRow == 7) {
                                        // Generate a move for each promotion piece type
                                        unsigned char promotionPieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
                                        for (int p = 0; p < 4; p++) {
                                            EngineMove move = {{i, j}, {newRow, newCol}, targetPiece, true, promotionPieces[p] | (color << 4), hasModifier, 0};
                                            list->moves[list->count++] = move;
                                        }
                                    } else {
                                        // Regular capture
                                        EngineMove move = {{i, j}, {newRow, newCol}, targetPiece, false, 0, hasModifier, 0};
                                        list->moves[list->count++] = move;
                                    }
                                }
                                
                                // En passant capture
                                else if (targetType == NONE && lastDoublePawn->x == newCol && lastDoublePawn->y == i) {
                                    bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                    EngineMove move = {{i, j}, {newRow, newCol}, board[i][newCol], false, 0, hasModifier, 0};
                                    list->moves[list->count++] = move;
                                }
                            }
                        }
                    }
                    break;
                }
                case KNIGHT: {
                    int knightMoves[8][2] = {
                        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
                        {1, -2}, {1, 2}, {2, -1}, {2, 1}
                    };
                    
                    for (int k = 0; k < 8; k++) {
                        int nx = i + knightMoves[k][0];
                        int ny = j + knightMoves[k][1];
                        
                        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                            unsigned char targetPiece = board[nx][ny];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {nx, ny}, targetPiece, false, 0, hasModifier, 0};
                        list->moves[list->count++] = move;
                    }
                }
            }
                    break;
                }
                case BISHOP: {
                    int diagonalDirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
                    
                    for (int k = 0; k < 4; k++) {
                        int dx = diagonalDirs[k][0];
                        int dy = diagonalDirs[k][1];
                        int x = i + dx;
                        int y = j + dy;
                        
                        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                            unsigned char targetPiece = board[x][y];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {x, y}, targetPiece, false, 0, hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                            
                            if (targetType != NONE) break;
                            x += dx;
                            y += dy;
                        }
                    }
                    break;
                }
                case ROOK: {
                    int straightDirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                    
                    for (int k = 0; k < 4; k++) {
                        int dx = straightDirs[k][0];
                        int dy = straightDirs[k][1];
                        int x = i + dx;
                        int y = j + dy;
                        
                        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                            unsigned char targetPiece = board[x][y];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {x, y}, targetPiece, false, 0, hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                            
                            if (targetType != NONE) break;
                            x += dx;
                            y += dy;
                        }
                    }
                    break;
                }
                case QUEEN: {
                    int diagonalDirs[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
                    int straightDirs[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                    
                    for (int k = 0; k < 4; k++) {
                        int dx = diagonalDirs[k][0];
                        int dy = diagonalDirs[k][1];
                        int x = i + dx;
                        int y = j + dy;
                        
                        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                            unsigned char targetPiece = board[x][y];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {x, y}, targetPiece, false, 0, hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                            
                            if (targetType != NONE) break;
                            x += dx;
                            y += dy;
                        }
                    }
                    
                    for (int k = 0; k < 4; k++) {
                        int dx = straightDirs[k][0];
                        int dy = straightDirs[k][1];
                        int x = i + dx;
                        int y = j + dy;
                        
                        while (x >= 0 && x < 8 && y >= 0 && y < 8) {
                            unsigned char targetPiece = board[x][y];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {x, y}, targetPiece, false, 0, hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                            
                            if (targetType != NONE) break;
                            x += dx;
                            y += dy;
                        }
                    }
                    break;
                }
                case KING: {
                    int kingMoves[8][2] = {
                        {-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                        {0, 1}, {1, -1}, {1, 0}, {1, 1}
                    };
                    
                    for (int k = 0; k < 8; k++) {
                        int nx = i + kingMoves[k][0];
                        int ny = j + kingMoves[k][1];
                        
                        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                            unsigned char targetPiece = board[nx][ny];
                            unsigned char targetType = targetPiece & TYPE_MASK;
                            unsigned char targetColor = (targetPiece & COLOR_MASK) >> 4;
                            
                            if (targetType != NONE && targetColor != color) {
                                bool hasModifier = (board[i][j] & MODIFIER) != 0;
                                
                                EngineMove move = {{i, j}, {nx, ny}, targetPiece, false, 0, hasModifier, 0};
                                list->moves[list->count++] = move;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
}

// Check if a move is legal (doesn't leave king in check)
bool isLegalMove(unsigned char board[8][8], EngineMove move, unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {
    // Make a temporary copy of the board and kings
    unsigned char tempBoard[8][8];
    memcpy(tempBoard, board, sizeof(tempBoard));
    
    Vector2f tempKings[2] = {kings[0], kings[1]};
    Vector2f tempLastDoublePawn = *lastDoublePawn;
    
    // Make the move on the temporary board
    engineMakeMove(tempBoard, move, &tempLastDoublePawn, tempKings, 0);
    
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
        if (isLegalMove(board, pseudoLegalMoves.moves[i], color, lastDoublePawn, kings)) {
            list->moves[list->count++] = pseudoLegalMoves.moves[i];
        }
    }
}

// Make a move on the board and update lastDoublePawn if needed
// isRealMove should be true for actual moves on the board, false for AI calculations
void engineMakeMove(unsigned char board[8][8], EngineMove move, Vector2f* lastDoublePawn, Vector2f kings[], int isRealMove) {
    unsigned char movingPiece = board[(int)move.from.x][(int)move.from.y];
    unsigned char pieceType = movingPiece & TYPE_MASK;
    unsigned char color = (movingPiece & COLOR_MASK) >> 4;

    // For kings and rooks, always clear the MODIFIER flag from source position
    if (pieceType == KING || pieceType == ROOK) {
        // Clear the MODIFIER flag from the original position
        board[move.from.x][move.from.y] &= ~MODIFIER;
    }

    // Handle pawn double push
    if (pieceType == PAWN && abs(move.to.x - move.from.x) == 2) {
        // Only update lastDoublePawn if this is a real move
        if (isRealMove) {
            lastDoublePawn->x = move.to.y; // Column of the pawn
            lastDoublePawn->y = move.to.x; // Row of the pawn
            // Only print debug info when an actual move is made (not during initialization)
            if (board[move.from.x][move.from.y] != NONE) {
                printf("Double pawn push detected: lastDoublePawn set to (%d, %d)\n", (int)lastDoublePawn->x, (int)lastDoublePawn->y);
            }
        }
    } else {
        // Handle en passant capture
        if (pieceType == PAWN && 
            move.to.y != move.from.y && // Diagonal move
            board[move.to.x][move.to.y] == NONE && // No piece at destination
            lastDoublePawn->x == move.to.y && // Column of the captured pawn
            lastDoublePawn->y == move.from.x) { // Row of the capturing pawn
            
            // Remove the captured pawn
            printf("En passant capture executed: removing pawn at (%d, %d)\n", (int)move.from.x, (int)move.to.y);
            board[move.from.x][move.to.y] = NONE;
        }
        
        // Reset lastDoublePawn after any move that's not a double push, but only for real moves
        if (isRealMove) {
        lastDoublePawn->x = -1;
        lastDoublePawn->y = -1;
        }
    }

    // Update king position if the king moves
    if (pieceType == KING) {
        kings[color] = move.to;
        
        // Handle castling
        if (abs(move.to.y - move.from.y) == 2) {
            // Kingside castling (short castle)
            if (move.to.y > move.from.y) {
                // EngineMove the rook from the kingside to its new position
                // Don't set MODIFIER flag on the rook
                board[move.to.x][move.to.y - 1] = (ROOK | (color << 4));
                board[move.to.x][7] = 0; // Remove the rook from its original position
            }
            // Queenside castling (long castle)
            else {
                // EngineMove the rook from the queenside to its new position
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

    // Handle promotion (already handled in newPiece calculation)
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
    
    printf("DEBUG: Generating moves for color %d\n", color);
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            unsigned char piece = board[i][j];
            if (piece == NONE) continue;
            
            unsigned char pieceType = piece & TYPE_MASK;
            unsigned char pieceColor = (piece & COLOR_MASK) >> 4;
            
            if (pieceColor != color) continue;
            
            printf("DEBUG: Found piece type %d at position (%d,%d) with color %d\n", pieceType, i, j, pieceColor);
            
            // Generate possible moves for this piece
            unsigned char tempBoard[8][8];
            copyBoard(board, tempBoard);
            
            // Mark the piece as selected to generate moves
            tempBoard[i][j] |= SELECTED_MASK;
            
            // Clear any previous possible moves
            clearPossibleBoard(tempBoard);
            
            // Generate possible moves for this piece
            generatePossibleMoves(tempBoard, i, j, lastDoublePawn);
            
            int pieceMovesCount = 0;
            
            // Add all possible moves to the move list
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    if (tempBoard[x][y] & MOVABLE_MASK) {
                        // Get the original piece's MODIFIER state
                        bool hasModifier = (board[i][j] & MODIFIER) != 0;
                        
                        // Check for pawn promotion
                        if (pieceType == PAWN && (x == 0 || x == 7)) {
                            // Generate a move for each promotion piece type
                            unsigned char promotionPieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
                            for (int p = 0; p < 4; p++) {
                                EngineMove move = {{i, j}, {x, y}, board[x][y], true, promotionPieces[p] | (color << 4), hasModifier, 0};
                                
                                // Check if the move is legal (doesn't leave king in check)
                                Vector2f kings[2];
                                findKings(board, kings);
                                if (isLegalMove(board, move, color, lastDoublePawn, kings)) {
                                    moveList->moves[moveList->count++] = move;
                                    pieceMovesCount++;
                                }
                            }
                        } else {
                            // Regular move
                            EngineMove move = {{i, j}, {x, y}, board[x][y], false, 0, hasModifier, 0};
                        
                            // Check if the move is legal (doesn't leave king in check)
                            Vector2f kings[2];
                            findKings(board, kings);
                            if (isLegalMove(board, move, color, lastDoublePawn, kings)) {
                                moveList->moves[moveList->count++] = move;
                                pieceMovesCount++;
                            }
                        }
                    }
                }
            }
            
            printf("DEBUG: Piece at (%d,%d) has %d legal moves\n", i, j, pieceMovesCount);
        }
    }
    
    printf("DEBUG: Total legal moves for color %d: %d\n", color, moveList->count);
}

// Function to make a move on the board
void makeEngineMove(unsigned char board[8][8], EngineMove move, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    // Delegate to the main move function
    engineMakeMove(board, move, lastDoublePawn, kingsPositions, 1);
    
    // For AI moves, print promotion information if applicable
    if (move.isPromotion) {
        char pieceChar;
        switch(move.promotionPiece) {
            case QUEEN: pieceChar = 'Q'; break;
            case ROOK: pieceChar = 'R'; break;
            case BISHOP: pieceChar = 'B'; break;
            case KNIGHT: pieceChar = 'N'; break;
            default: pieceChar = '?';
        }
        printf("AI promoted pawn to %c\n", pieceChar);
    }
}

// Helper functions for min and max
int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int minimax(unsigned char board[8][8], int depth, int alpha, int beta, bool maximizing, unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {
    // Base case: if we've reached the maximum depth or the game is over
    if (depth == 0) {
        return evaluatePosition(board, maximizing ? color : color ^ 0x10);
    }

    MoveList moveList;
    generateLegalMoves(board, color, &moveList, lastDoublePawn, kings);

    // Check for checkmate or stalemate
    if (moveList.count == 0) {
        if (isKingInCheck(board, kings[color], color)) {
            return maximizing ? -10000 : 10000; // Checkmate
        } else {
            return 0; // Stalemate
        }
    }
    
    // Make a temporary copy of the board and kings
    unsigned char tempBoard[8][8];
    Vector2f tempKings[2];
    Vector2f tempLastDoublePawn;
    
    if (maximizing) {
        int maxEval = -100000;
        
        for (int i = 0; i < moveList.count; i++) {
            // Copy the board and kings
            memcpy(tempBoard, board, sizeof(tempBoard));
            tempKings[0] = kings[0];
            tempKings[1] = kings[1];
            tempLastDoublePawn = *lastDoublePawn;
            
            // Make the move on the temporary board (pass isRealMove=0 for AI calculations)
            engineMakeMove(tempBoard, moveList.moves[i], &tempLastDoublePawn, tempKings, 0);
            
            // Recursive call
            int eval = minimax(tempBoard, depth - 1, alpha, beta, false, color ^ 0x10, &tempLastDoublePawn, tempKings);
            maxEval = max(maxEval, eval);
            
            // Alpha-beta pruning
            alpha = max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        
        return maxEval;
        } else {
        int minEval = 100000;
        
        for (int i = 0; i < moveList.count; i++) {
            // Copy the board and kings
            memcpy(tempBoard, board, sizeof(tempBoard));
            tempKings[0] = kings[0];
            tempKings[1] = kings[1];
            tempLastDoublePawn = *lastDoublePawn;
            
            // Make the move on the temporary board (pass isRealMove=0 for AI calculations)
            engineMakeMove(tempBoard, moveList.moves[i], &tempLastDoublePawn, tempKings, 0);
            
            // Recursive call
            int eval = minimax(tempBoard, depth - 1, alpha, beta, true, color ^ 0x10, &tempLastDoublePawn, tempKings);
            minEval = min(minEval, eval);
            
            // Alpha-beta pruning
            beta = min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        
        return minEval;
    }
}

// Top-level function to get the best move using the full tree search
EngineMove findBestMoveWithMinimax(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kings[]) {
    MoveList moveList;
    generateLegalMoves(board, color, &moveList, lastDoublePawn, kings);

    if (moveList.count == 0) {
        EngineMove nullMove = {0};
        return nullMove;
    }

    int bestScore = -100000;
    EngineMove bestMove = moveList.moves[0]; // Default to first move

    for (int i = 0; i < moveList.count; i++) {
        // Make a temporary copy of the board and kings
        unsigned char tempBoard[8][8];
        memcpy(tempBoard, board, sizeof(tempBoard));
        
        Vector2f tempKings[2] = {kings[0], kings[1]};
        Vector2f tempLastDoublePawn = *lastDoublePawn;

        // Make the move on the temporary board (pass isRealMove=0 for AI calculations)
        engineMakeMove(tempBoard, moveList.moves[i], &tempLastDoublePawn, tempKings, 0);

        int score = minimax(tempBoard, MAX_DEPTH - 1, -100000, 100000, false, color ^ 0x10, &tempLastDoublePawn, tempKings);

        if (score > bestScore) {
            bestScore = score;
            bestMove = moveList.moves[i];
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
    EngineMove bestMove = findBestMoveWithMinimax(board, color, lastDoublePawn, kingsPositions);
    
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
int evaluateMoveSafety(unsigned char board[8][8], EngineMove move, unsigned char color) {
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
EngineMove findBestMove(unsigned char board[8][8], unsigned char color, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    return findBestMoveWithMinimax(board, color, lastDoublePawn, kingsPositions);
}