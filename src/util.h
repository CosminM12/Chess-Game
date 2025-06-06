#ifndef VECTOR_H
#define VECTOR_H

typedef struct Point {
    int x, y;
} Vector2f;


Vector2f createVector(int x, int y); // Note: Original was float, I will keep current int signature for consistency.

// Add screen dimension constants here
extern const int squareSize;
extern const int sidebar1_width;
extern const int sidebar2_width;
extern const int boardWidth;
extern const int screenWidth;
extern const int screenHeight;

#endif