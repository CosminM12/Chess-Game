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


/*---------Helper functions-----------*/
bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }
    if(!(IMG_Init(IMG_INIT_PNG))) {
        printf("IMG_Init has failed. Error: %s\n", SDL_GetError());
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



int main(int argc, char* argv[]) {
    /*==========Program Initialization==========*/
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if(!windowCreation || !SDLInit) {
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
    SDL_Texture* pieceTextures[2][7];
    Vector2f selectedSquare = createVector(-1.0f, -1.0f);
    Vector2f kingsPositions[2];
    Vector2f lastDoublePushPawn = createVector(-1.0f, -1.0f);
    SDL_Rect pieceTextureCoordinates = {0, 0, 60, 60};
    int mouseX, mouseY;

    //==========Initialize values==========//
    loadPieceTextures(pieceTextures, &renderer);
    char input[] = "RNBQKBNR/PPPPPPPP/////pppppppp/rnbqkbnr";
    placePieces(board, input);
    findKings(board, kingsPositions); //Initialize kings positions
    
    while(gameRunning) {
        //==========Find time variables==========//
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());
        // printfBoard(board);

        //==========Events and movements==========//
        getEvents(event, &gameRunning, mouseActions);
        SDL_GetMouseState(&mouseX, &mouseY);
        
        if(mouseInsideBoard(mouseX, mouseY, screenWidth, squareSize)) {
            handleMouseInput(board, mouseX, mouseY, screenWidth, squareSize, mouseActions, pieceActions, &blackTurn, &selectedSquare, &lastDoublePushPawn, kingsPositions);
            mouseActions[0] = false;
            mouseActions[1] = false;
        }
        
        //==========Render Visuals==========//
        clear(&renderer);
        drawBoard(renderer, squareSize, screenWidth, color_light, color_dark, color_clicked, color_possible, board);
        for(int row=0;row<8;row++) {
            for(int col=0;col<8;col++) {
                renderPiece(pieceTextureCoordinates, 200, squareSize, row, col, getPieceTexture(pieceTextures, board[row][col]), &renderer);
            }
        } 
        display(&renderer);

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