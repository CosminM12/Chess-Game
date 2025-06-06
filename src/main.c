#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
// #include <SDL2/SDL_mixer.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "Events.h"
#include "util.h"
#include "engine.h"

/*----------Variable declaration------------*/
bool gameRunning = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};
SDL_Color color_risky = {255, 128, 0, 255}; // Orange color for risky moves

const int screenWidth=1200, screenHeight=800;
const int squareSize=100;

Uint64 currentTick, lastTick;
double deltaTime;

// Engine variables
bool engineEnabled = false;
bool showAnalysis = false;
bool computerPlaysBlack = false;
bool showEvaluationBar = true; // New variable to control evaluation bar visibility
bool showMenu = true; // Variable to control menu visibility
int gameMode = 0; // 0 = not selected, 1 = PvP, 2 = PvE
Uint32 moveTimestamp = 0; // Timestamp of the last player move


/*---------Helper functions-----------*/
bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(TTF_Init() == -1) {
        printf("TTF_Init has failed. Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void printfBoard(unsigned char board[8][8]) {
    for(int i=0;i<8;i++) {
        for(int j=0;j<8;j++) {
            printf("%d ", (int)board[i][j]);
        }
        putchar('\n');
    }
    putchar('\n');
}



// Function to draw the evaluation bar
void drawEvaluationBar(SDL_Renderer* renderer, int score) {
    float whitePercentage, blackPercentage;
    getScoreBar(score, &whitePercentage, &blackPercentage);
    
    // Draw the bar on the right side of the screen
    int barWidth = 30;
    int barHeight = 400;
    int barX = screenWidth - barWidth - 20;
    int barY = (screenHeight - barHeight) / 2;
    
    // Black portion (top)
    SDL_Rect blackRect = {barX, barY, barWidth, (int)(barHeight * blackPercentage)};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &blackRect);
    
    // White portion (bottom)
    SDL_Rect whiteRect = {barX, barY + (int)(barHeight * blackPercentage), 
                         barWidth, (int)(barHeight * whitePercentage)};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &whiteRect);
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect borderRect = {barX, barY, barWidth, barHeight};
    SDL_RenderDrawRect(renderer, &borderRect);
    
    // Draw evaluation value
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20);
    if (font) {
        char scoreText[32];
        snprintf(scoreText, sizeof(scoreText), "%.2f", score / 100.0f);
        
        SDL_Color textColor = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                    barX - 50,  // Position to the left of the bar
                    barY + barHeight + 5,  // Below the bar
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
        TTF_CloseFont(font);
    }
    
    // Reset render color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}



// Function to make the computer move
void makeComputerMove(unsigned char board[8][8], bool* blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    unsigned char color = *blackTurn ? 1 : 0;
    Move bestMove = findBestMove(board, color, lastDoublePawn, kingsPositions);
    
    if (bestMove.from.x != -1) {
        makeEngineMove(board, bestMove, lastDoublePawn, kingsPositions);
        *blackTurn = !*blackTurn;
        
        // Check for checkmate or stalemate after the move
        unsigned char nextColor = *blackTurn ? 1 : 0;
        MoveList moveList;
        generateMoves(board, nextColor, &moveList, lastDoublePawn);
        
        if (moveList.count == 0) {
            // Check if the king is in check
            if (isCheck(board, kingsPositions[nextColor])) {
                printf("Checkmate! %s wins!\n", nextColor == 1 ? "White" : "Black");
            } else {
                printf("Stalemate! Game is drawn.\n");
            }
        }
    }
}

// Function to draw game status text (check, checkmate, stalemate)
void drawGameStatus(SDL_Renderer* renderer, bool isInCheck, bool isGameOver, bool isStalemate, bool blackTurn) {
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (!font) return;
    
    SDL_Color textColor = {255, 0, 0, 255}; // Red for check/checkmate
    if (isStalemate) textColor = (SDL_Color){255, 255, 0, 255}; // Yellow for stalemate
    
    char statusText[32] = "";
    if (isGameOver) {
        if (isStalemate) {
            strcpy(statusText, "Stalemate! Draw");
        } else {
            sprintf(statusText, "Checkmate! %s wins", blackTurn ? "White" : "Black");
        }
    } else if (isInCheck) {
        sprintf(statusText, "%s is in Check", blackTurn ? "Black" : "White");
    }
    
    if (statusText[0] != '\0') {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, statusText, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                    (screenWidth - textSurface->w) / 2,
                    10,  // Top of the screen
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
    
    TTF_CloseFont(font);
}

// Function to analyze the current position for display
void displayPositionAnalysis(unsigned char board[8][8], bool blackTurn, Vector2f* lastDoublePawn, Vector2f kingsPositions[]) {
    // Evaluate the current position
    unsigned char currentColor = blackTurn ? 1 : 0;
    int score = evaluatePosition(board, currentColor);
    
    printf("Current evaluation: %.2f\n", score / 100.0f);
    
    // Generate and evaluate legal moves
    MoveList legalMoves;
    generateMoves(board, currentColor, &legalMoves, lastDoublePawn);
    
    if (legalMoves.count == 0) {
        if (isCheck(board, kingsPositions[currentColor])) {
            printf("Checkmate! %s wins!\n", blackTurn ? "White" : "Black");
        } else {
            printf("Stalemate! Game is drawn.\n");
        }
        return;
    }
    
    // Evaluate move safety
    evaluateMovesSafety(board, currentColor, &legalMoves);
    
    // Find any risky moves
    bool hasRiskyMoves = false;
    for (int i = 0; i < legalMoves.count; i++) {
        if (legalMoves.moves[i].safetyScore < 0) {
            if (!hasRiskyMoves) {
                printf("Risky moves detected:\n");
                hasRiskyMoves = true;
            }
            
            // Convert board coordinates to algebraic notation
            char fromFile = 'a' + legalMoves.moves[i].from.y;
            int fromRank = 8 - legalMoves.moves[i].from.x;
            char toFile = 'a' + legalMoves.moves[i].to.y;
            int toRank = 8 - legalMoves.moves[i].to.x;
            
            unsigned char pieceType = board[legalMoves.moves[i].from.x][legalMoves.moves[i].from.y] & TYPE_MASK;
            const char* pieceName = "Unknown";
            switch (pieceType) {
                case PAWN: pieceName = "Pawn"; break;
                case KNIGHT: pieceName = "Knight"; break;
                case BISHOP: pieceName = "Bishop"; break;
                case ROOK: pieceName = "Rook"; break;
                case QUEEN: pieceName = "Queen"; break;
                case KING: pieceName = "King"; break;
            }
            
            printf("  %s %c%d -> %c%d: Risk score %d\n", 
                   pieceName, fromFile, fromRank, toFile, toRank, 
                   legalMoves.moves[i].safetyScore);
        }
    }
}

// Function to get the renderer for external modules
SDL_Renderer* getMainRenderer() {
    return renderer;
}

// Function to draw the menu overlay
bool drawMenu(SDL_Renderer* renderer, int screenWidth, int screenHeight, int* selectedMode) {
    // Semi-transparent background
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Dark semi-transparent background
    SDL_Rect overlay = {0, 0, screenWidth, screenHeight};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Load font
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }
    
    // Title
    SDL_Color titleColor = {255, 215, 0, 255}; // Gold color
    SDL_Surface* titleSurface = TTF_RenderText_Solid(font, "Chess Game", titleColor);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {
        (screenWidth - titleSurface->w) / 2,
        screenHeight / 4 - titleSurface->h / 2,
        titleSurface->w,
        titleSurface->h
    };
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    
    // Button dimensions
    int buttonWidth = 200;
    int buttonHeight = 60;
    int buttonSpacing = 40;
    
    // PvP Button
    SDL_Color buttonColor = {255, 255, 255, 255}; // White text
    SDL_Surface* pvpSurface = TTF_RenderText_Solid(font, "Player vs Player", buttonColor);
    SDL_Texture* pvpTexture = SDL_CreateTextureFromSurface(renderer, pvpSurface);
    
    SDL_Rect pvpButtonRect = {
        (screenWidth - buttonWidth) / 2,
        screenHeight / 2 - buttonHeight - buttonSpacing / 2,
        buttonWidth,
        buttonHeight
    };
    
    SDL_SetRenderDrawColor(renderer, 50, 100, 150, 255); // Button background color
    SDL_RenderFillRect(renderer, &pvpButtonRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Button border color
    SDL_RenderDrawRect(renderer, &pvpButtonRect);
    
    SDL_Rect pvpTextRect = {
        (screenWidth - pvpSurface->w) / 2,
        pvpButtonRect.y + (buttonHeight - pvpSurface->h) / 2,
        pvpSurface->w,
        pvpSurface->h
    };
    SDL_RenderCopy(renderer, pvpTexture, NULL, &pvpTextRect);
    
    // PvE Button
    SDL_Surface* pveSurface = TTF_RenderText_Solid(font, "Player vs Computer", buttonColor);
    SDL_Texture* pveTexture = SDL_CreateTextureFromSurface(renderer, pveSurface);
    
    SDL_Rect pveButtonRect = {
        (screenWidth - buttonWidth) / 2,
        screenHeight / 2 + buttonSpacing / 2,
        buttonWidth,
        buttonHeight
    };
    
    SDL_SetRenderDrawColor(renderer, 50, 100, 150, 255); // Button background color
    SDL_RenderFillRect(renderer, &pveButtonRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Button border color
    SDL_RenderDrawRect(renderer, &pveButtonRect);
    
    SDL_Rect pveTextRect = {
        (screenWidth - pveSurface->w) / 2,
        pveButtonRect.y + (buttonHeight - pveSurface->h) / 2,
        pveSurface->w,
        pveSurface->h
    };
    SDL_RenderCopy(renderer, pveTexture, NULL, &pveTextRect);
    
    // Clean up
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);
    SDL_FreeSurface(pvpSurface);
    SDL_DestroyTexture(pvpTexture);
    SDL_FreeSurface(pveSurface);
    SDL_DestroyTexture(pveTexture);
    TTF_CloseFont(font);
    
    // Check for button clicks
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
    
    if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        if (mouseX >= pvpButtonRect.x && mouseX <= pvpButtonRect.x + pvpButtonRect.w &&
            mouseY >= pvpButtonRect.y && mouseY <= pvpButtonRect.y + pvpButtonRect.h) {
            // PvP button clicked
            *selectedMode = 1;
            printf("Player vs Player mode selected\n");
            return true;
        }
        
        if (mouseX >= pveButtonRect.x && mouseX <= pveButtonRect.x + pveButtonRect.w &&
            mouseY >= pveButtonRect.y && mouseY <= pveButtonRect.y + pveButtonRect.h) {
            // PvE button clicked
            *selectedMode = 2;
            printf("Player vs Computer mode selected\n");
            return true;
        }
    }
    
    return false;
}

int main(int argc, char* argv[]) {
    //==========Program Initialization==========//
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if(!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    //==========Initialize Variables==========//
    unsigned char board[8][8] = {0};
    //Each square has 1 bite(unsigned char) or 8 bits:
    //Bits 6 to 8 (3 bits) know the type of piece on the square: 1=pawn, 2=bishop, 3=knight, 4=rook, 5=queen, 6=king
    //Bits 4 and 5 (2 bits) know the color of the piece on the square: 1 on bit 5 is white (0x10), 1 on bit 4 is black (0x20)
    //Bit 3 knows if the square is clicked on
    
    bool mouseActions[2] = {false, false};
    /*
    *pos0: mouseButtonDown (click button pressed)
    *pos1: mouseButtonUp (click button released)
    */
    bool pieceActions[2] = {false, false}; 
    /*
    *pos0: isSelected(a piece is selected)
    *pos1: isHoldingPiece (a piece is holded in "hand")
    */

    bool blackTurn = false; // remember who's player is the turn
    SDL_Event event;
    SDL_Texture* pieceTextures[2][7];
    Vector2f selectedSquare = createVector(-1, -1);
    Vector2f kingsPositions[2];
    Vector2f lastDoublePushPawn = createVector(-1, -1);
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;

    // Engine variables
    int currentScore = 0;
    
    //==========Initialize values==========//
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(board, input);
    initCastlingRights(board); // Initialize castling rights (sets MODIFIER flags)
    findKings(board, kingsPositions); //Initialize kings positions
    
    
    // Game state variables for check/checkmate/stalemate
    bool isInCheck = false;
    bool isGameOver = false;
    bool isStalemate = false;
    
    while(gameRunning) {
        //==========Find time variables==========//
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());
        
        //==========Events and movements==========//
        getEvents(event, &gameRunning, mouseActions);
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Handle menu state
        if (showMenu) {
            // Render the chess board in the background
            clear(&renderer);
            drawBoard(renderer, squareSize, screenWidth, color_light, color_dark, color_clicked, color_possible, color_risky, board);
            
            for(int row=0;row<8;row++) {
                for(int col=0;col<8;col++) {
                    renderPiece(pieceTextureCoordinates, 200, squareSize, row, col, getPieceTexture(pieceTextures, board[row][col]), &renderer);
                }
            }
            
            // Draw the menu overlay and check for button clicks
            int selectedMode = 0;
            if (drawMenu(renderer, screenWidth, screenHeight, &selectedMode)) {
                showMenu = false; // Exit menu if a button was clicked
                gameMode = selectedMode;
                
                // If PvE mode is selected, make the computer play black
                if (gameMode == 2) {
                    computerPlaysBlack = true;
                    // If it's already black's turn, make the computer move immediately
                    if (blackTurn) {
                        makeComputerMove(board, &blackTurn, &lastDoublePushPawn, kingsPositions);
                    }
                }
            }
            
            display(&renderer);
            
            // Skip the rest of the game logic while in menu
            continue;
        }
        
        if(mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize)) {
            handleMouseInput(board, mouseX, mouseY, screenWidth, squareSize, mouseActions, pieceActions, &blackTurn, &selectedSquare, &lastDoublePushPawn, kingsPositions);
            mouseActions[0] = false;
            mouseActions[1] = false;
        }
        
        // Make computer move if engine is enabled
        if (engineEnabled) {
            makeComputerMove(board, &blackTurn, &lastDoublePushPawn, kingsPositions);
        }
        
        // Handle computer moves in PvE mode with a 500ms delay
        if (gameMode == 2 && moveTimestamp > 0 && blackTurn) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - moveTimestamp >= 500) {  // 500 ms = 0.5 second
                printf("Computer making move after 0.5 second delay\n");
                makeComputerMove(board, &blackTurn, &lastDoublePushPawn, kingsPositions);
                moveTimestamp = 0;  // Reset the timestamp
            }
        }
        
        // Update the current score
        currentScore = getRelativeScore(board);
        
        // Check for check, checkmate, or stalemate
        unsigned char currentColor = blackTurn ? 1 : 0;
        isInCheck = isCheck(board, kingsPositions[currentColor]);
        
        MoveList moveList;
        generateMoves(board, currentColor, &moveList, &lastDoublePushPawn);
        
        isGameOver = moveList.count == 0;
        isStalemate = isGameOver && !isInCheck;
        
        //==========Render Visuals==========//
        clear(&renderer);
        drawBoard(renderer, squareSize, screenWidth, color_light, color_dark, color_clicked, color_possible, color_risky, board);
        
        // Draw evaluation bar
        if (showEvaluationBar) {
            drawEvaluationBar(renderer, currentScore);
        }
        
        for(int row=0;row<8;row++) {
            for(int col=0;col<8;col++) {
                renderPiece(pieceTextureCoordinates, 200, squareSize, row, col, getPieceTexture(pieceTextures, board[row][col]), &renderer);
            }
        } 
        
        // Draw game status
        drawGameStatus(renderer, isInCheck, isGameOver, isStalemate, blackTurn);
        
        display(&renderer);
        
        // Show analysis if requested
        if (showAnalysis) {
            displayPositionAnalysis(board, blackTurn, &lastDoublePushPawn, kingsPositions);
            showAnalysis = false; // Only show once per request
        }
    }

    printf("Program ended\n");
    return 0;
}