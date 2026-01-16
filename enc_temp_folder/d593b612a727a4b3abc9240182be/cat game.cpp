#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>

const int screenWidth = 800;
const int screenHeight = 450;

std::vector<std::vector<int>> stage;

float cameraX = 0;
float cameraY = 0;

std::vector<int> player = {0, 100, 50, 50}; //x, y, w, h

float playerX = 0;
float playerY = 0;
float playerW = 10;
float playerH = 10;

bool freecam = true;

void drawObject(int x, int y, int w, int h) {
    DrawRectangle(x + cameraX, y + cameraY, w, h, RED);
}

bool collide(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    //for debugging, draw the points of your mom first

#if 0
    //BOX 1
    drawObject(x1, y1, 10, 10); //left top corner
    drawObject(w1 + x1, y1, 10, 10); //right top corner
    drawObject(x1, y1 + h1, 10, 10); //left bottom corner 
    drawObject(x1 + w1, y1 + h1, 10, 10); //right bottom corner


    //box 2
    drawObject(x2, y2, 10, 10); //left top corner
    drawObject(w2 + x2, y2, 10, 10); //right top corner
    drawObject(x2, y2 + h2, 10, 10); //left bottom corner 
    drawObject(x2 + w2, y2 + h2, 10, 10); //right bottom corner

#endif

    drawObject(x1, y1, w1, h1);
    drawObject(x2, y2, w2, h2);

    //its so smoll yet so incredibly powerful and painful to do
    if (x1 < w2 + x2 &&
        y1 < y2 + h2 &&
        w1 + x1 > x2 &&
        y1 + h1 > y2
        ) return true;


    return false;
}

bool checkCollisions(int id) {
    for (int i = 0; i < stage.size(); i++) {
        if (collide(stage[i][0], stage[i][1], stage[i][2], stage[i][3], stage[id][0], stage[id][1], stage[id][2], stage[id][3])) {
            return true;
        }
    }
}

void handleControls() {
    if (freecam) {
        if (IsKeyDown(KEY_D)) {
            cameraX -= 10;
        }
        if (IsKeyDown(KEY_A)) {
            cameraX += 10;
        }
        if (IsKeyDown(KEY_W)) {
            cameraY += 10;
        }
        if (IsKeyDown(KEY_S)) {
            cameraY -= 10;
        }
    }
    else {
        if (IsKeyDown(KEY_D)) {
            if (checkCollisions(0)) {
                playerX += 10;

            }
        }
        if (IsKeyDown(KEY_A)) {
            playerX -= 10;
        }
        if (IsKeyDown(KEY_W)) {
            playerY -= 10;
        }
        if (IsKeyDown(KEY_S)) {
            playerY += 10;
        }
    }
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        freecam = !freecam;
    }

}

void handleCollisions() {
    for (int i = 0; i < stage.size(); i++) {
#if 0
        std::cout << "iteration " << i << "\n";

        std::cout << stage[i][0] << "\n";
        std::cout << stage[i][1] << "\n";
        std::cout << stage[i][2] << "\n";
        std::cout << stage[i][3] << "\n";
        std::cout << "-----\n";
#endif

    }
}

void defineObject(int x, int y, int w, int h) {
    stage.push_back({ x, y, w, h });
}

void modObject(int id, int x, int y, int w, int h) {
    stage[id][0] = x;
    stage[id][1] = y;
    stage[id][2] = w;
    stage[id][3] = h;
}

void handleMovement() {

}

int main(void)
{


    InitWindow(screenWidth, screenHeight, "cat game");

    SetTargetFPS(60);

    defineObject(GetMouseX(), GetMouseY(), 50, 50);
    defineObject(100, 100, 200, 200);
    ClearBackground(RAYWHITE);

    while (!WindowShouldClose())
    {
        handleControls();
        handleCollisions();
        if (collide(stage[0][0], stage[0][1], stage[0][2], stage[0][3], stage[1][0], stage[1][1], stage[1][2], stage[1][3])) {
            ClearBackground(GREEN);
            DrawText("overlapping", 0, 0, 20, RED);
        }
        else {
            ClearBackground(YELLOW);

        }

        BeginDrawing();


        //drawObject(0, 0, 20, 20);

        modObject(0, GetMouseX(), GetMouseY(), 67, 67);
        //render player
        DrawRectangle(playerX + cameraX, playerY + cameraY, playerW, playerH, BLUE);


        DrawText(std::to_string(cameraX).c_str(), 190, 200, 20, LIGHTGRAY);
        DrawText(std::to_string(cameraY).c_str(), 190, 220, 20, LIGHTGRAY);
        DrawText(std::to_string(freecam).c_str(), 190, 240, 20, LIGHTGRAY);

        EndDrawing();

    }

    CloseWindow();

    return 0;
}