#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "GameState.h"
#include "Events.h"
#include "util.h"

#define MAX_CAPTURED 16


Move moveHistory[MAX_MOVES];
int moveCount = 0;


int moveHistoryScrollOffset = 0;


const char* pieceToChar(unsigned char piece) {
    switch (piece & 0b11100000) {
        case 0x10: // white
        case 0x20: // black
            break;
    }

    switch (piece & 0b00111000) {
        case 0x08: return "";  // pawn
        case 0x10: return "B"; // bishop
        case 0x18: return "N"; // knight
        case 0x20: return "R"; // rook
        case 0x28: return "Q"; // queen
        case 0x30: return "K"; // king
        default:   return "?";
    }
}


/*----------Variable declaration------------*/
bool gameRunning = true;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// TODO: move the colors to a separate file
SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};

// ----- windows -----
const int squareSize = 100;
const int sidebar1_width = 300;
const int sidebar2_width = 300;

const int boardWidth = squareSize * 8;

const int screenWidth = boardWidth + sidebar1_width + sidebar2_width;
int screenHeight = squareSize * 8;

const int timer_height = 100;
const int captured_height = 150;


// ----- time variables -----
Uint64 currentTick, lastTick;
double deltaTime;

int whiteTimeMs = 5 * 60 * 1000;
int blackTimeMs = 5 * 60 * 1000;

char whiteTimerStr[16];
char blackTimerStr[16];

// ----- captured variables -----
#include "GameState.h"
unsigned char capturedByWhite[MAX_CAPTURED] = {0};
unsigned char capturedByBlack[MAX_CAPTURED] = {0};
int capturedWhiteCount = 0;
int capturedBlackCount = 0;


void formatTime(char *buffer, int timeMs) {
    int totalSeconds = timeMs / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    snprintf(buffer, 16, "%02d:%02d", minutes, seconds);
}


/*---------Helper functions-----------*/
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if (!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", SDL_GetError());
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer failed: %s\n", Mix_GetError());
        return false;
    }

    if (!initFont("../res/fonts/JetBrainsMono/JetBrainsMono-Bold.ttf", 24)) {
        fprintf(stderr, "Font failed to load, check the path and font file.\n");
        return -1;
    }

    return true;
}

void printfBoard(unsigned char board[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%d ", (int) board[i][j]);
        }
        putchar('\n');
    }
    putchar('\n');
}


int main() {
    /*==========Program Initialization==========*/
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if (!windowCreation || !SDLInit) {
        return -1;
    }
    printf("Program started successfully\n");

    /*==========Initialize Variables==========*/
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

    bool blackTurn = false; //remember who's player is the turn
    SDL_Event event;
    SDL_Texture *pieceTextures[2][7];
    Vector2f selectedSquare = createVector(-1.0f, -1.0f);
    Vector2f kingsPositions[2];
    Vector2f lastDoublePushPawn = createVector(-1.0f, -1.0f);
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;

    //========== Initialize values ==========//
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(board, input);
    findKings(board, kingsPositions); //Initialize kings positions

    bool inMenu = true;

    while (inMenu) {
        SDL_Event menuEvent;
        while (SDL_PollEvent(&menuEvent)) {
            if (menuEvent.type == SDL_QUIT) {
                gameRunning = false;
                inMenu = false;
            } else if (menuEvent.type == SDL_MOUSEBUTTONDOWN) {
                int x = menuEvent.button.x;
                int y = menuEvent.button.y;

                // Example button position and size (adjust as needed)
                if (x >= 400 && x <= 700 && y >= 300 && y <= 370) {
                    inMenu = false; // Start game
                }
            }
        }

        // Render Menu
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // dark background
        SDL_RenderClear(renderer);

        renderText(renderer, "Chess Game", (SDL_Color){255, 255, 255, 255}, 500, 150);
        SDL_Rect startButton = {450, 300, 200, 70};
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
        SDL_RenderFillRect(renderer, &startButton);
        renderText(renderer, "Start Game", (SDL_Color){255, 255, 255, 255}, 480, 320);
        SDL_RenderPresent(renderer);
    }

    lastTick = SDL_GetPerformanceCounter();
    currentTick = lastTick;

    while (gameRunning) {
        //========== Find time variables ==========//
        lastTick = currentTick;
        // printfBoard(board);

        if (blackTurn) {
            blackTimeMs -= deltaTime;
            if (blackTimeMs < 0) {
                blackTimeMs = 0;
            }
        } else {
            whiteTimeMs -= deltaTime;
            if (whiteTimeMs < 0) {
                whiteTimeMs = 0;
            }
        }

        //========== Events and movements ==========//
        getEvents(event, &gameRunning, mouseActions, &moveHistoryScrollOffset);
        SDL_GetMouseState(&mouseX, &mouseY);

        if (mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize)) {
            handleMouseInput(board, mouseX, mouseY, screenWidth, squareSize, mouseActions, pieceActions, &blackTurn,
                             &selectedSquare, &lastDoublePushPawn, kingsPositions);
            mouseActions[0] = false;
            mouseActions[1] = false;
        }

        //========== Rendering Visuals ==========//
        clear(&renderer);
        drawBoard(renderer, squareSize, 0, screenWidth, color_light, color_dark, color_clicked, color_possible, board);

        // ----- setting the background color -----
        SDL_Rect sidebar_background = {boardWidth, 0, sidebar1_width + sidebar2_width, screenHeight};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &sidebar_background);

        // Timer section (red) on right sidebar
        SDL_Rect timerBox = {boardWidth, 0, sidebar1_width, timer_height};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &timerBox);

        formatTime(whiteTimerStr, whiteTimeMs);
        formatTime(blackTimerStr, blackTimeMs);

        renderText(renderer, "White:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 10);
        renderText(renderer, whiteTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 10);

        renderText(renderer, "Black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 40);
        renderText(renderer, blackTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 40);

        // Captured by Black (blue)
        SDL_Rect capturedBlackBox = {boardWidth, timer_height, sidebar1_width, captured_height * 2};
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        SDL_RenderFillRect(renderer, &capturedBlackBox);

        // Captured by White (purple)
        SDL_Rect capturedWhiteBox = {boardWidth + sidebar1_width, 250, sidebar2_width, 150};
        SDL_SetRenderDrawColor(renderer, 200, 160, 255, 255);
        SDL_RenderFillRect(renderer, &capturedWhiteBox);

        // Buttons (pink)
        SDL_Rect buttonBox = {boardWidth, screenHeight - 150, sidebar1_width, 150};
        SDL_SetRenderDrawColor(renderer, 255, 100, 180, 255);
        SDL_RenderFillRect(renderer, &buttonBox);

        // Move History (green)
        SDL_Rect moveHistoryBox = {boardWidth + sidebar1_width, 0, sidebar2_width, screenHeight};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &moveHistoryBox);


        renderText(renderer, "Captured by white:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + 10);
        renderText(renderer, "Captured by black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, timer_height + captured_height + 10);

        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                renderPiece(pieceTextureCoordinates, 0, squareSize, row, col,
                            getPieceTexture(pieceTextures, board[row][col]), &renderer);
            }
        }



        // Render captured by Black (white pieces)
        renderCapturedPieces(renderer, pieceTextures, capturedByBlack, capturedBlackCount, boardWidth + 10, 110, 65);

        // Render captured by White (black pieces)
        renderCapturedPieces(renderer, pieceTextures, capturedByWhite, capturedWhiteCount,
                             boardWidth + 10, 260, 65);


        renderText(renderer, "MOVE HISTORY:", (SDL_Color) {255, 255, 255, 255}, boardWidth + sidebar1_width + 10, 10);

        // Use scroll offset
        const int moveHeight = 25;
        int visibleStart = moveHistoryScrollOffset / moveHeight;
        int visibleEnd = visibleStart + (screenHeight / moveHeight) + 1;

        for (int i = visibleStart; i < visibleEnd && i < moveCount; ++i) {
            char buffer[64];
            sprintf(buffer, "%d. %s", (i + 1), moveHistory[i].notation);

            int y = 40 + (i * moveHeight) - moveHistoryScrollOffset;

            if (y >= 40 && y < screenHeight - 10) {
                renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255},
                           boardWidth + sidebar1_width + 15, y);
            }
        }


//        for (int i = 0; i < moveCount; ++i) {
//            char buffer[64];
//            sprintf(buffer, "%d. %s", (i + 1), moveHistory[i].notation);
//            int yOffset = 40 + (i * 25);
//            renderText(renderer, buffer, (SDL_Color){255, 255, 255, 255}, boardWidth + sidebar1_width + 15, yOffset);
//
//        }


//        int startY = 30 + moveHistoryScrollOffset;
//        int moveHeight = 20;
//        int visibleStart = moveHistoryScrollOffset / moveHeight;
//        int visibleEnd = visibleStart + (screenHeight / moveHeight) + 2;
//
//        if (visibleEnd > moveCount) visibleEnd = moveCount;
//
//        for (int i = visibleStart; i < visibleEnd; ++i) {
//            char buffer[64];
//            sprintf(buffer, "%d. %s", (i / 2) + 1, moveHistory[i].notation);
//            int y = startY + i * moveHeight;
////            if (y > 0)
//            printf("%d\n", y);
//            renderText(renderer, buffer, (SDL_Color){0, 0, 0, 255}, boardWidth + sidebar1_width + 10, y);
//        }




        display(&renderer);

        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double) ((currentTick - lastTick) * 1000 / (double) SDL_GetPerformanceFrequency());

//        if (whiteTimeMs <= 0 || blackTimeMs <= 0) {
//            gameRunning = false;
//            printf("Time's up!\n");
//        }
    }

    char *exportString = NULL;

    // exportPosition(board, &exportString);
    // if(exportString == NULL) {
    //     printf("Error exporting position!\n");
    // }
    // else {
    //     printf("%s\n", exportString);
    // }

    cleanUp(window);
    printf("Program ended\n");
    return 0;
}