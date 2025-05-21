#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "Events.h"
#include "util.h"

/*----------Variable declaration------------*/
bool gameRunning = true;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// TODO: move the colors to a separate file
SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};
SDL_Color color_clicked = {248, 255, 41, 145};
SDL_Color color_possible = {200, 20, 20, 255};

const int squareSize = 100;
const int sidebar1_width = 300;
const int sidebar2_width = 300;

const int boardWidth = squareSize * 8;

const int screenWidth = boardWidth + sidebar1_width + sidebar2_width;
const int screenHeight = squareSize * 8;


Uint64 currentTick, lastTick;
double deltaTime;

int whiteTimeMs = 5 * 60 * 1000;
int blackTimeMs = 5 * 60 * 1000;

char whiteTimerStr[16];
char blackTimerStr[16];


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
        getEvents(event, &gameRunning, mouseActions);
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
        SDL_Rect timerBox = {boardWidth, 0, sidebar1_width, 100};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &timerBox);

        formatTime(whiteTimerStr, whiteTimeMs);
        formatTime(blackTimerStr, blackTimeMs);

        renderText(renderer, "White:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 10);
        renderText(renderer, whiteTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 10);

        renderText(renderer, "Black:", (SDL_Color) {255, 255, 255, 255}, boardWidth + 10, 40);
        renderText(renderer, blackTimerStr, (SDL_Color) {255, 255, 255, 255}, boardWidth + 150, 40);

        // Captured by Black (blue)
        SDL_Rect capturedBlackBox = {boardWidth, 100, sidebar1_width, 150};
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
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &moveHistoryBox);


        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                renderPiece(pieceTextureCoordinates, 0, squareSize, row, col,
                            getPieceTexture(pieceTextures, board[row][col]), &renderer);
            }
        }
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