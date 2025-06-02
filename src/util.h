#ifndef VECTOR_H
#define VECTOR_H

typedef struct Point {
    int x, y;
} Vector2f;


Vector2f createVector(float x, float y);

// Add screen dimension constants here
const int squareSize = 100;
const int sidebar1_width = 300;
const int sidebar2_width = 300;
const int boardWidth = squareSize * 8;
const int screenWidth = boardWidth + sidebar1_width + sidebar2_width;
const int screenHeight = squareSize * 8;

#endif