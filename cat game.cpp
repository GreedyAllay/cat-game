#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

float framerate = 60;

#define clr_selected_overlay Color{89, 208, 255, 50}
#define clr_selected Color{89, 208, 255, 255}


const int screenWidth = 800;
const int screenHeight = 450;

float cameraX = 0;
float cameraY = 0;
float cameraZ = 1;

int camXV = 0;
int camYV = 0;

float gravity = 9.81;

std::vector<std::vector<int>> objects; //0 x, 1 y, 2 w, 3 h, 4 physics, 5 xv, 6 yv, 7 onfloor

std::vector<int> playerHitbox = { 50, 50 }; //w, h

float playerX = 0;
float playerY = 0;

bool freecam = false;
bool editMode = true;

bool onFloor = true;
bool dragSpawning = false;

int mouseX;
int mouseY;

void drawObject(int x, int y, int w, int h, Color clr) {
    DrawRectangle((x + cameraX) * cameraZ + screenWidth / 2, (y + cameraY) * cameraZ + screenHeight / 2, w*cameraZ, h*cameraZ, clr);
}

void handleObjects() {
    for (int i = 0; i < objects.size(); i++) {
        drawObject(objects[i][0], objects[i][1], objects[i][2], objects[i][3], RED);
    }

}

bool collide(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {

    //its so smoll yet so incredibly powerful and painful to do holy crap this took forever check first commits
    if (x1 < w2 + x2 &&
        y1 < y2 + h2 &&
        w1 + x1 > x2 &&
        y1 + h1 > y2
        ) return true;


    return false;
}

int checkMouseHover() {
    for (int i = 0; i < objects.size(); i++) {
        if (collide(objects[i][0], objects[i][1], objects[i][2], objects[i][3],

            (GetMouseX() - screenWidth / 2) / cameraZ - cameraX, //this line is ai generated cuz I was lazy and it works really well
            (GetMouseY() - screenHeight / 2) / cameraZ - cameraY, 1, 1)) { //this line is also ai generated cuz I was lazy and it also works really well

            return i;
        }
    }
    return -1;
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
    if (id == 0) onFloor = false;
    for (int i = 0; i < abs(dy); i++) {
        objects[id][1] += stepY;
        if (checkCollisions(id)) {
            objects[id][1] -= stepY;
            objects[id][6] = 0;
            if (!onFloor && id == 0) onFloor = dy > 0;
            break;
        }
    }
}

void setXvel(int id, int xv) { objects[id][5] = xv; }
void setYvel(int id, int yv) { objects[id][6] = yv; }


void defineObject(int x, int y, int w, int h, bool physics) {
    objects.push_back({ x, y, w, h, physics, 0, 0, 0 });
}

void spawnScreenSpace(int x, int y, int w, int h) {
    defineObject((x - screenWidth / 2) / cameraZ - cameraX, (y - screenHeight / 2) / cameraZ - cameraY, w, h, 1);
}


void handleControls() {
    if (IsKeyDown(KEY_MINUS)) {
        cameraZ /= 1.1;
        if (cameraZ < .01) cameraZ = .01;
    }
    if (IsKeyDown(KEY_EQUAL)) {
        cameraZ *= 1.1;
        if (cameraZ > 10) cameraZ = 10;
    }
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
            if (onFloor)setYvel(0, -15);

        }
        if (IsKeyDown(KEY_S)) {
        }
    }
    if (IsKeyPressed(KEY_LEFT_CONTROL)) {
        freecam = !freecam;
    }

    //everything for edit mode down here
    if (!editMode) return;
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        dragSpawning = true;
        mouseX = GetMouseX();
        mouseY = GetMouseY();
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        int dx = GetMouseX() - mouseX;
        int dy = GetMouseY() - mouseY;
        if (dx < 15 || dy < 15) {
            DrawRectangle(mouseX, mouseY, dx, dy, BROWN);
        }
        else {
            DrawRectangle(mouseX, mouseY, dx, dy, GREEN);
        }
        DrawText(std::to_string(dx).c_str(), GetMouseX(), mouseY - 15, 15, LIGHTGRAY);
        DrawText(std::to_string(dy).c_str(), mouseX - 15, GetMouseY(), 15, LIGHTGRAY);
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && dragSpawning) {
        dragSpawning = false;
        int dx = GetMouseX() - mouseX;
        int dy = GetMouseY() - mouseY;
        if (dx < 15 || dy < 15) return;
        spawnScreenSpace(mouseX, mouseY, dx/cameraZ, dy/cameraZ);
    }

}

void setCamera(int x, int y, bool s) {
    if (s) {
        x = x - cameraX;
        y = y - cameraY;
        camXV += x;
        camYV += y;
        cameraX += (x / 5);
        cameraY += (y / 5);
        camXV = camXV / 10;
        camYV = camYV / 10;
    }
    else {
        cameraX = x;
        cameraY = y;
    }
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
        int tVel = 35;
        if (abs(objects[i][6]) > tVel) {
            objects[i][6] = (objects[i][6] < 0) ? -tVel : tVel;
        }
    }
}

void unstuckObjects() {
    //basically going thru every object and checking how much steps it takes in each direction to get out of colliding area, then choosing the closest one
    //im kind of a genius, you gotta give me that

    //it doesnt yet work tho
    std::vector<int> stepCounts;
    int steps = 0;
    for (int i = 0; i < objects.size(); i++) {
        //left
        if (checkCollisions(i))
        for (int j = 0; j < 100; j++) {
            if (!checkCollisions(j)) break;
            objects[i][0] -= 1;
            steps++;
        }
        stepCounts.push_back(steps);
        steps = 0;

        //right
        if (checkCollisions(i))
            for (int j = 0; j < 100; j++) {
                if (!checkCollisions(j)) break;
                objects[i][0] += 1;
                steps++;
            }
        stepCounts.push_back(steps);
        steps = 0;

        //up or down idk, you never know it with c++
        if (checkCollisions(i))
            for (int j = 0; j < 100; j++) {
                if (!checkCollisions(j)) break;
                objects[i][1] += 1;
                steps++;
            }
        stepCounts.push_back(steps);
        steps = 0;

        //the opposite direction of that
        if (checkCollisions(i))
            for (int j = 0; j < 100; j++) {
                if (!checkCollisions(j)) break;
                objects[i][1] -= 1;
                steps++;
            }
        stepCounts.push_back(steps);
        steps = 0;

        for (int i = 0; i > stepCounts.size(); i++) {

        }
    }
}

void loadStage(std::string data) {
    std::vector<int> obj;
    std::string output;
    for (int i = 0; i < data.length(); i++) {
        if (data[i] == ' ') continue;
        if (data[i] == ',' || data[i] == ';') {
            obj.push_back(stoi(output));
            std::cout << output << "\n";
            output = "";
            if (obj.size() == 5) {
                defineObject(obj[0], obj[1], obj[2], obj[3], obj[4]);
            }
            if (data[i] == ';') obj.clear();
            continue;
        }
        else {
            output += data[i];
        }

    }
}

void loadStageFromFile(int id) {
    std::string text;
    const std::string path = "stage" + std::to_string(id) + ".txt";

    std::ifstream file(path);

    while (getline(file, text)) {
        loadStage(text);
    }
    file.close();
}

void clearStage() {
    for (int i = 1; i < objects.size() - 1; i++) {
        objects.erase(std::next(objects.begin(), 1), std::next(objects.begin(), objects.size()));
    }
}



int main(void)
{
    InitWindow(screenWidth, screenHeight, "cat game");

    SetTargetFPS(framerate);


    defineObject(100, 50, playerHitbox[0], playerHitbox[1], 1);


    ClearBackground(RAYWHITE);


    loadStageFromFile(1);

    //clearStage();

    while (!WindowShouldClose())
    {

        handleControls();
        int hoveredObj = checkMouseHover();
        int padding = 3;
        if (hoveredObj != -1 && editMode) {
            drawObject(objects[hoveredObj][0] - padding, objects[hoveredObj][1] - padding, objects[hoveredObj][2] + padding * 2, objects[hoveredObj][3] + padding * 2, clr_selected);
        }
        handleObjects();
        if (hoveredObj != -1 && editMode && !IsKeyDown(KEY_LEFT_SHIFT)) {
            drawObject(objects[hoveredObj][0], objects[hoveredObj][1], objects[hoveredObj][2], objects[hoveredObj][3], clr_selected_overlay);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                objects.erase(objects.begin() + hoveredObj);
            }
        }
        handlePhysics();

        playerX = objects[0][0];
        playerY = objects[0][1];

        //ALL THIS CODE ONLY TO MOVE THE CAMERA 😭
        if (!freecam) {
            setCamera(((0 - playerX) - objects[0][2] / 2), ((0 - playerY) - objects[0][3] / 2), true);
        }

        ClearBackground(GRAY);

        if (checkCollisions(0)) {
            DrawText("overlapping", 0, 0, 20, RED);
        }
        else {
        }


        BeginDrawing();


        //drawObject(0, 0, 20, 20);

        //render player
        //modObject(0, playerX, playerY, playerHitbox[0], playerHitbox[1]);


        if (editMode) {
            DrawText(std::to_string(cameraX).c_str(), 20, 20, 20, LIGHTGRAY);
            DrawText(std::to_string(cameraY).c_str(), 20, 40, 20, LIGHTGRAY);
            DrawText(std::to_string(freecam).c_str(), 20, 60, 20, LIGHTGRAY);
            DrawText(std::to_string(onFloor).c_str(), 20, 80, 20, LIGHTGRAY);
            DrawText(std::to_string(cameraZ).c_str(), 20, 100, 20, LIGHTGRAY);
            DrawText("s to save to file", 20, 120, 20, LIGHTGRAY);
        }


        EndDrawing();

    }

    CloseWindow();

    return 0;
}