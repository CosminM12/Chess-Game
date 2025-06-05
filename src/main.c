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

const int screenWidth=1200, screenHeight=800;
const int squareSize=100;

Uint64 currentTick, lastTick;
double deltaTime;

// Engine variables
bool engineEnabled = false;
bool showAnalysis = false;
bool computerPlaysBlack = false;


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
    if ((*blackTurn && computerPlaysBlack) || (!*blackTurn && !computerPlaysBlack)) {
        unsigned char color = *blackTurn ? 1 : 0;
        Move bestMove = findBestMove(board, color, lastDoublePawn, kingsPositions);
        
        if (bestMove.from.x != -1) {
            printf("Computer plays: %c%d to %c%d\n", 
                'a' + bestMove.from.y, 8 - bestMove.from.x,
                'a' + bestMove.to.y, 8 - bestMove.to.x);
            
            makeEngineMove(board, bestMove, lastDoublePawn);
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
    
    // Keyboard state for engine control
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);

    //==========Initialize values==========//
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(board, input);
    findKings(board, kingsPositions); //Initialize kings positions
    
    // Print instructions
    printf("Chess Game with Engine\n");
    printf("Controls:\n");
    printf("E - Toggle engine (computer plays black)\n");
    printf("A - Toggle position analysis\n");
    printf("S - Show evaluation bar\n");
    
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
        
        // Handle keyboard input for engine control
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_E]) {
            if (!keyboardState[SDL_SCANCODE_E]) { // Only toggle once per key press
                engineEnabled = !engineEnabled;
                computerPlaysBlack = engineEnabled;
                printf("Engine %s. Computer plays black: %s\n", 
                       engineEnabled ? "enabled" : "disabled",
                       computerPlaysBlack ? "yes" : "no");
            }
        }
        
        if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_A]) {
            if (!keyboardState[SDL_SCANCODE_A]) {
                showAnalysis = !showAnalysis;
                if (showAnalysis) {
                    analyzePosition(board, blackTurn ? 1 : 0, &lastDoublePushPawn, kingsPositions);
                }
                printf("Analysis %s\n", showAnalysis ? "enabled" : "disabled");
            }
        }
        
        // Update keyboard state for next frame
        keyboardState = SDL_GetKeyboardState(NULL);
        
        if(mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize)) {
            handleMouseInput(board, mouseX, mouseY, screenWidth, squareSize, mouseActions, pieceActions, &blackTurn, &selectedSquare, &lastDoublePushPawn, kingsPositions);
            mouseActions[0] = false;
            mouseActions[1] = false;
        }
        
        // Make computer move if engine is enabled
        if (engineEnabled) {
            makeComputerMove(board, &blackTurn, &lastDoublePushPawn, kingsPositions);
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
        drawBoard(renderer, squareSize, screenWidth, color_light, color_dark, color_clicked, color_possible, board);
        
        // Draw evaluation bar
        drawEvaluationBar(renderer, currentScore);
        
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
            analyzePosition(board, blackTurn ? 1 : 0, &lastDoublePushPawn, kingsPositions);
            showAnalysis = false; // Only show once per request
        }
    }

    char *exportString = NULL;

    // exportPosition(board, &exportString);
    // if(exportString == NULL) {
    //     printf("Error exporting position!\n");
    // }
    // else {
    //     printf("%s\n", exportString);
    // }

    printf("Program ended\n");
    return 0;
}