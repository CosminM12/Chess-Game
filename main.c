#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool gameRunning = true;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int screenWidth=600, screenHeight=600;

Uint64 currentTick, lastTick;
double deltaTime;

bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        printf("SDL_Init has failed. Error: %s\n", SDL_GetError());
    }

    return true;
}


bool createWindow(const char* p_title, SDL_Window* window,SDL_Renderer* renderer, int screenWidth, int screenHeight) {
    SDL_DisplayMode screenSize;
    SDL_GetCurrentDisplayMode(0, &screenSize);
    window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

    if(window == NULL) {
        printf("Window failed to init. Error: %s\n", SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED && SDL_RENDERER_PRESENTVSYNC);

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


void cleanUp(SDL_Window* window) {
    SDL_DestroyWindow(window);
}


int main(int argc, char* argv[]) {
    SDL_Event event;

    currentTick = SDL_GetPerformanceCounter();
    bool SDLInit = init();
    bool windowCreation = createWindow("Chess Game", window, renderer, screenWidth, screenHeight);
    printf("Program started successfully\n");

    while(gameRunning) {
        //find deltaTime
        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000/(double)SDL_GetPerformanceFrequency());

        getEvents(event, &gameRunning);
    }

    cleanUp(window);
    printf("Program ended\n");
    return 0;
}