#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

#include "RenderWindow.h"
#include "Piece.h"
#include "app_globals.h"

// Store the renderer as a static variable for access from other modules
static SDL_Renderer* globalRenderer = NULL;
static TTF_Font* globalFont = NULL;

bool createWindow(const char* p_title, SDL_Window** window, SDL_Renderer** renderer, int screenWidth, int screenHeight) {
    //Create new window
    SDL_DisplayMode screenSize;
    SDL_GetCurrentDisplayMode(0, &screenSize);

    *window = SDL_CreateWindow(p_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);

    if(*window == NULL) {
        printf("Window failed to init. Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(*renderer == NULL) {
        printf("Renderer failed to init. Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Store the renderer globally
    globalRenderer = *renderer;
    
    return true;
}

// Function to get the renderer for external use
SDL_Renderer* getRenderer() {
    return globalRenderer;
}

void drawBoard(SDL_Renderer* renderer, int squareSize, int boardOffset, int screenWidth, SDL_Color color1, SDL_Color color2, SDL_Color colorClicked, SDL_Color colorPossible, SDL_Color colorRisky, unsigned char board[8][8]) {
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {

            // printf("0x%X ", board[row][col]);
            SDL_Color currentColor = ((row + col) % 2 == 0) ? color1 : color2;
            int isClicked = board[row][col] & (0x1 << 5);
            int isPossible = board[row][col] & 0x40;
            int isRisky = board[row][col] & 0x80;

            if(isClicked) {
                currentColor = colorClicked;
            }
            else if(isPossible) {
                if(isRisky) {
                    currentColor = colorRisky; // Use risky color for moves that are possible but risky
                } else {
                    currentColor = colorPossible;
                }
            }

            SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);

            SDL_Rect square = {boardOffset + col * squareSize, row * squareSize, squareSize, squareSize};
            SDL_RenderFillRect(renderer, &square);

        }
        // putchar('\n');
    }
    // putchar('\n');

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect boardBorder = {boardOffset-1, -1, squareSize*8+2, squareSize*8+2};
    SDL_RenderDrawRect(renderer, &boardBorder);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

SDL_Texture* loadTexture(const char* p_filePath, SDL_Renderer** renderer) {
    SDL_Texture* texture = NULL;
    texture = IMG_LoadTexture(*renderer, p_filePath);
    
    if(texture == NULL) {
        printf("Texture failed to load. Error: %s\n", SDL_GetError());
    }

    return texture;
}


void render(SDL_Rect textureAtlas, int posx, int posy, SDL_Texture* tex, SDL_Renderer** renderer) {
    SDL_Rect src = textureAtlas;

    SDL_Rect dst;
    dst.x = posx;
    dst.y = posy;
    dst.w = 100;
    dst.h = 100;

    SDL_RenderCopy(*renderer, tex, &src, &dst);

}

void renderPiece(SDL_Rect pieceAtlas, int boardOffset, int squareSize, int line, int col, SDL_Texture* tex, SDL_Renderer** renderer) {
    SDL_Rect src = pieceAtlas;

    SDL_Rect dst;
    dst.x = boardOffset+col*squareSize;
    dst.y = line*squareSize;
    dst.w = squareSize;
    dst.h = squareSize;

    SDL_RenderCopy(*renderer, tex, &src, &dst);
}

unsigned char showPromotionMenu(SDL_Renderer* renderer, SDL_Texture* pieceTextures[2][7], int x, int y, unsigned char color, int screenWidth, int screenHeight) {
    // Create a semi-transparent overlay
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180); // Semi-transparent black
    SDL_Rect overlay = {0, 0, screenWidth, screenHeight};
    SDL_RenderFillRect(renderer, &overlay);
    
    // Create a menu box
    int menuWidth = 300;
    int menuHeight = 100;
    int menuX = (screenWidth - menuWidth) / 2;
    int menuY = (screenHeight - menuHeight) / 2;
    
    // Draw menu background
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255); // Light gray
    SDL_Rect menuRect = {menuX, menuY, menuWidth, menuHeight};
    SDL_RenderFillRect(renderer, &menuRect);
    
    // Draw menu border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
    SDL_RenderDrawRect(renderer, &menuRect);
    
    // Draw menu title
    if (globalFont) {
        SDL_Color textColor = {0, 0, 0, 255}; // Black
        SDL_Surface* textSurface = TTF_RenderText_Solid(globalFont, "Promote pawn to:", textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (textTexture) {
                SDL_Rect textRect = {
                    menuX + (menuWidth - textSurface->w) / 2,
                    menuY + 10,
                    textSurface->w,
                    textSurface->h
                };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
            SDL_FreeSurface(textSurface);
        }
    }
    
    // Draw promotion options
    int pieceSize = 60;
    int spacing = 10;
    int startX = menuX + (menuWidth - (4 * pieceSize + 3 * spacing)) / 2;
    int pieceY = menuY + menuHeight - pieceSize - 10;
    
    // Define option rectangles
    SDL_Rect queenRect = {startX, pieceY, pieceSize, pieceSize};
    SDL_Rect rookRect = {startX + pieceSize + spacing, pieceY, pieceSize, pieceSize};
    SDL_Rect bishopRect = {startX + 2 * (pieceSize + spacing), pieceY, pieceSize, pieceSize};
    SDL_Rect knightRect = {startX + 3 * (pieceSize + spacing), pieceY, pieceSize, pieceSize};
    
    // Draw pieces
    SDL_RenderCopy(renderer, pieceTextures[color >> 4][QUEEN], NULL, &queenRect);
    SDL_RenderCopy(renderer, pieceTextures[color >> 4][ROOK], NULL, &rookRect);
    SDL_RenderCopy(renderer, pieceTextures[color >> 4][BISHOP], NULL, &bishopRect);
    SDL_RenderCopy(renderer, pieceTextures[color >> 4][KNIGHT], NULL, &knightRect);
    
    // Draw borders around options
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &queenRect);
    SDL_RenderDrawRect(renderer, &rookRect);
    SDL_RenderDrawRect(renderer, &bishopRect);
    SDL_RenderDrawRect(renderer, &knightRect);
    
    // Present the menu
    SDL_RenderPresent(renderer);
    
    // Wait for user selection
    SDL_Event event;
    bool selected = false;
    unsigned char selectedPiece = QUEEN | color;
    
    while (!selected) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                
                if (mouseX >= queenRect.x && mouseX < queenRect.x + queenRect.w &&
                    mouseY >= queenRect.y && mouseY < queenRect.y + queenRect.h) {
                    selectedPiece = QUEEN | color;
                    selected = true;
                }
                else if (mouseX >= rookRect.x && mouseX < rookRect.x + rookRect.w &&
                         mouseY >= rookRect.y && mouseY < rookRect.y + rookRect.h) {
                    selectedPiece = ROOK | color;
                    selected = true;
                }
                else if (mouseX >= bishopRect.x && mouseX < bishopRect.x + bishopRect.w &&
                         mouseY >= bishopRect.y && mouseY < bishopRect.y + bishopRect.h) {
                    selectedPiece = BISHOP | color;
                    selected = true;
                }
                else if (mouseX >= knightRect.x && mouseX < knightRect.x + knightRect.w &&
                         mouseY >= knightRect.y && mouseY < knightRect.y + knightRect.h) {
                    selectedPiece = KNIGHT | color;
                    selected = true;
                }
            }
            else if (event.type == SDL_QUIT) {
                // Handle quit event
                selected = true;
                //gameRunning = false;
            }
        }
    }
    
    return selectedPiece;
}

// Render captured pieces in the sidebar
void renderCapturedPieces(SDL_Renderer* renderer, GameState* state) {
    // Constants for rendering captured pieces
    const int capturedPieceSize = 30; // Smaller size for captured pieces
    const int padding = 5;
    const int piecesPerRow = 5; // Number of pieces to display per row in sidebar
    
    // Load piece textures
    SDL_Texture* pieceTextures[2][7];
    loadPieceTextures(pieceTextures, &renderer);
    
    SDL_Rect srcRect = {0, 0, 60, 60}; // Assuming piece textures are 60x60 within a larger atlas

    // Render White's Captured Pieces (Black's pieces)
    int startX_whiteCaptured = boardWidth + padding;
    int startY_whiteCaptured = 150; // Below the timer and move history title

    for (int i = 0; i < state->numWhiteCapturedPieces; i++) {
        unsigned char pieceType = state->whiteCapturedPieces[i];
        // Captured pieces by White are Black's pieces (COLOR_MASK represents black if set)
        unsigned char pieceByte = pieceType | COLOR_MASK; // Combine type with black color mask

        SDL_Texture *tex = getPieceTexture(pieceTextures, pieceByte);
        if (tex) {
            int currentX = startX_whiteCaptured + (i % piecesPerRow) * (capturedPieceSize + padding);
            int currentY = startY_whiteCaptured + (i / piecesPerRow) * (capturedPieceSize + padding);

            SDL_Rect destRect = {currentX, currentY, capturedPieceSize, capturedPieceSize};
            SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
        }
    }

    // Render Black's Captured Pieces (White's pieces)
    int startX_blackCaptured = boardWidth + padding;
    int startY_blackCaptured = 400; // Further down for black's captured pieces

    for (int i = 0; i < state->numBlackCapturedPieces; i++) {
        unsigned char pieceType = state->blackCapturedPieces[i];
        // Captured pieces by Black are White's pieces (no COLOR_MASK for white)
        unsigned char pieceByte = pieceType; // White pieces have no color mask

        SDL_Texture *tex = getPieceTexture(pieceTextures, pieceByte);
        if (tex) {
            int currentX = startX_blackCaptured + (i % piecesPerRow) * (capturedPieceSize + padding);
            int currentY = startY_blackCaptured + (i / piecesPerRow) * (capturedPieceSize + padding);

            SDL_Rect destRect = {currentX, currentY, capturedPieceSize, capturedPieceSize};
            SDL_RenderCopy(renderer, tex, &srcRect, &destRect);
        }
    }
}

void display(SDL_Renderer** renderer) {
    SDL_RenderPresent(*renderer);
}

void clear(SDL_Renderer** renderer) {
    SDL_RenderClear(*renderer);
}

void cleanUp(SDL_Window* window) {
    destroyFont();
    TTF_Quit();
    SDL_DestroyWindow(window);
}

// Font handling functions
bool initFont(const char *fontPath, int fontSize) {
    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return false;
    }

    // Try multiple font paths in case the first one doesn't exist
    const char* fontPaths[] = {
        fontPath,
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf"
    };
    
    for (int i = 0; i < sizeof(fontPaths) / sizeof(fontPaths[0]); i++) {
        globalFont = TTF_OpenFont(fontPaths[i], fontSize);
        if (globalFont) {
            printf("Font loaded successfully: %s\n", fontPaths[i]);
            return true;
        }
    }
    
    printf("Failed to load any font\n");
    return false;
}

void renderText(SDL_Renderer *renderer, const char *text, SDL_Color color, int x, int y) {
    if (!globalFont) {
        printf("Font not initialized!\n");
        return;
    }

    SDL_Surface *surface = TTF_RenderText_Blended(globalFont, text, color);
    if (!surface) {
        printf("Text render error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect dstRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void destroyFont() {
    if (globalFont) {
        TTF_CloseFont(globalFont);
        globalFont = NULL;
    }
}