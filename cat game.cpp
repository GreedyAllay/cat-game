#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>

const int screenWidth = 800;
const int screenHeight = 450;

float cameraX = 0;
float cameraY = 0;

float gravity = 9.81;

std::vector<std::vector<int>> objects; //0 x, 1 y, 2 w, 3 h, 4 physics, 5 xv, 6 yv, 7 onfloor

std::vector<int> playerHitbox = {50, 50}; //w, h

float playerX = 0;
float playerY = 0;

bool freecam = true;

void drawObject(int x, int y, int w, int h) {
    DrawRectangle(x + cameraX, y + cameraY, w, h, RED);
}

void handleObjects() {
    for (int i = 0; i < objects.size(); i++) {
        drawObject(objects[i][0], objects[i][1], objects[i][2], objects[i][3]);
    }

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

    drawObject(x1, y1, w1, h1);
    drawObject(x2, y2, w2, h2);
#endif



    //its so smoll yet so incredibly powerful and painful to do holy crap this took forever
    if (x1 < w2 + x2 &&
        y1 < y2 + h2 &&
        w1 + x1 > x2 &&
        y1 + h1 > y2
        ) return true;


    return false;
}

bool checkCollisions(int id) {
    for (int i = 0; i < objects.size(); i++) {
        if (id == i) continue;
        if (collide(objects[id][0], objects[id][1], objects[id][2], objects[id][3], objects[i][0], objects[i][1], objects[i][2], objects[i][3])) {
            return true;
        }
    }
    return false;
}

void moveObject(int id, int dx, int dy) {
    float stepX = (dx < 0 ? -1 : 1);
    float stepY = (dy < 0 ? -1 : 1);
    for (int i = 0; i < abs(dx); i++) {
        objects[id][0] += stepX;

        if (checkCollisions(id)) {
            objects[id][0] -= stepX;
            objects[id][5] = 0;
            break;
        }
    }
    for (int i = 0; i < abs(dy); i++) {
        objects[id][1] += stepY;
        if (checkCollisions(id)) {
            objects[id][1] -= stepY;
            objects[id][6] = 0;
            break;
        }
    }
}

void setXvel(int id, int xv) { objects[id][5] = xv; }
void setYvel(int id, int yv) { objects[id][6] = yv; }

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
            setXvel(0, 10);
        }
        if (IsKeyDown(KEY_A)) {
            setXvel(0, -10);
        }
        if (IsKeyDown(KEY_W)) {
            setYvel(0, -10);
        }
        if (IsKeyDown(KEY_S)) {
            moveObject(0, 0, 10);
        }
    }
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        freecam = !freecam;
    }

}

void defineObject(int x, int y, int w, int h, bool physics) {
    objects.push_back({ x, y, w, h, physics, 0, 0, 0 });
}

void modObject(int id, int x, int y, int w, int h) {
    objects[id][0] = x;
    objects[id][1] = y;
    objects[id][2] = w;
    objects[id][3] = h;
}

void handlePhysics() {
    for (int i = 0; i < objects.size(); i++) {
        if (!objects[i][4]) continue;
        moveObject(i, objects[i][5], objects[i][6]);
        int onfloor = 0;
        objects[i][5] /= 1.5;
        objects[i][6] += 1;
    }
}



int main(void)
{
    InitWindow(screenWidth, screenHeight, "cat game");

    SetTargetFPS(60);

    defineObject(100, 50, playerHitbox[0], playerHitbox[1], 1);
    defineObject(100, 100, 200, 200, 0);
    defineObject(200, 150, 200, 200, 0);
    ClearBackground(RAYWHITE);

    while (!WindowShouldClose())
    {
        handleControls();
        handleObjects();
        handlePhysics();

        if (checkCollisions(0)) {
            ClearBackground(GREEN);
            DrawText("overlapping", 0, 0, 20, RED);
        }
        else {
            ClearBackground(YELLOW);

        }

        BeginDrawing();


        //drawObject(0, 0, 20, 20);

        //render player
        //modObject(0, playerX, playerY, playerHitbox[0], playerHitbox[1]);



        DrawText(std::to_string(cameraX).c_str(), 190, 200, 20, LIGHTGRAY);
        DrawText(std::to_string(cameraY).c_str(), 190, 220, 20, LIGHTGRAY);
        DrawText(std::to_string(freecam).c_str(), 190, 240, 20, LIGHTGRAY);

        EndDrawing();

    }

    CloseWindow();

    return 0;
}