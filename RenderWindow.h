#include <SDL2/SDL.h>

#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

bool createWindow(const char* p_title, SDL_Window** window,SDL_Renderer** renderer, int screenWidth, int screenHeight);

void drawBoard(SDL_Renderer* renderer, int squareSize, int screenWidth, SDL_Color color1, SDL_Color color2);

void display(SDL_Renderer** renderer);

void clear(SDL_Renderer** renderer);

void cleanUp(SDL_Window* window);

#endif