#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "RenderWindow.h"

bool gameRunning = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Color color_light = {240, 240, 240, 255};
SDL_Color color_dark = {119, 149, 86, 255};

int screenWidth=1200, screenHeight=800;

Uint64 currentTick, lastTick;
double deltaTime;

bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }

    return true;
}


void getEvents(SDL_Event event, bool *gameRunning) {
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                *gameRunning = false;
                break;
        }
    }
}



int main(int argc, char* argv[]) {
    SDL_Event event;

    currentTick = SDL_GetPerformanceCounter();
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", &window, &renderer, screenWidth, screenHeight);

    if(!windowCreation || !SDLInit) {
        return -1;
    }

    printf("Program started successfully\n");

    while(gameRunning) {
        clear(&renderer);
        drawBoard(renderer, 100, screenWidth, color_light, color_dark);

        //find deltaTime
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());

        getEvents(event, &gameRunning);
        display(&renderer);

    }

    cleanUp(window);
    printf("Program ended\n");
    return 0;
}