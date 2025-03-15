#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

bool createWindow(const char* p_title, SDL_Window** window,SDL_Renderer** renderer, int screenWidth, int screenHeight);

void drawBoard(SDL_Renderer* renderer, int squareSize, int screenWidth, SDL_Color color1, SDL_Color color2, SDL_Color colorClicked, SDL_Color colorPossible, unsigned char board[8][8]);

SDL_Texture* loadTexture(const char* p_filePath, SDL_Renderer** renderer);

void render(SDL_Rect textureAtlas, int posx, int posy, SDL_Texture* tex, SDL_Renderer** renderer);

void renderPiece(SDL_Rect pieceAtlas, int boardOffset, int squareSize, int line, int col, SDL_Texture* tex, SDL_Renderer** renderer);

void display(SDL_Renderer** renderer);

void clear(SDL_Renderer** renderer);

void cleanUp(SDL_Window* window);

#endif