#include "raylib.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

float framerate = 60;

#define clr_selected_overlay Color{89, 208, 255, 50}
#define clr_selected Color{89, 208, 255, 255}

//some genius on reddit said you could multiply your aspect ratio by the scale you want to get the right resolution!
int screenScale = 70;

int screenWidth = screenScale * 16;
int screenHeight = screenScale * 9;

float defaultWidth = 800;

float cameraX = 0;
float cameraY = 0;
float cameraZ = 4; //1

int camXV = 0;
int camYV = 0;

float gravity = 9.81;

std::vector<std::vector<int>> objects; //0 x, 1 y, 2 w, 3 h, 4 physics, 5 xv, 6 yv, 7 onfloor, texture (0 = box)

std::vector<int> playerHitbox = { 20, 100 }; //w, h

float playerX = 0;
float playerY = 0;

bool freecam = false;
bool editMode = true;

bool onFloor = true;
bool dragSpawning = false;

int mouseX;
int mouseY;
float initialZoom = 2; //.5

int oldWidth = screenWidth;
int oldZoom = initialZoom;

int oldWindowWidth;
int oldWindowHeight;
bool isFullscreen = false;

float zoomVel = 0;

float defaultWidthNumber = initialZoom / defaultWidth;

struct {
    Texture2D ref;
    Texture2D head;
    Texture2D tail;
    Texture2D body;
    Texture2D ok;
} textures;

void initTextures() {
    textures.ref = LoadTexture("assets/player.png");
    textures.head = LoadTexture("assets/head.png");
    textures.tail = LoadTexture("assets/tail.png");
    textures.body = LoadTexture("assets/body.png");
    textures.ok = LoadTexture("assets/ok.png");
}


void drawObject(int x, int y, int w, int h, Color clr) {
    DrawRectangle((x + cameraX) * cameraZ + screenWidth / 2, (y + cameraY) * cameraZ + screenHeight / 2, w*cameraZ, h*cameraZ, clr);
}

float screenToWorldX(int x) { return (x + cameraX) * cameraZ + screenWidth / 2; } float screenToWorldY(int y) { return (y + cameraY) * cameraZ + screenHeight / 2; }
float screenToWorldSize(float s) { return s * cameraZ; }

void drawObjectTexture(int x, int y, int scale, Texture2D texture, Color clr) {
    DrawTextureEx(texture, { (x + cameraX) * cameraZ + screenWidth / 2, (y + cameraY) * cameraZ + screenHeight / 2}, 0, (scale/33)*cameraZ, clr);
}

void renderObjects() {
    for (int i = 0; i < objects.size(); i++) {
        Color clr;
        if (i == 0) continue;
        if (!objects[i][8]) {
            if (objects[i][4]) {
                clr = RED;
            }
            else {
                clr = BLUE;
            }
            drawObject(objects[i][0], objects[i][1], objects[i][2], objects[i][3], clr);
        }
        else {
            drawObjectTexture(objects[i][0], objects[i][1], objects[i][3], textures.ref, WHITE);
        }
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
    //ts will check if a specific object is colliding with anything
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


void defineObject(int x, int y, int w, int h, bool physics, int texture) {
    objects.push_back({ x, y, w, h, physics, 0, 0, 0, texture });
}

void spawnScreenSpace(int x, int y, int w, int h) {

    defineObject((x - screenWidth / 2) / cameraZ - cameraX, (y - screenHeight / 2) / cameraZ - cameraY, w, h, 1, 0);
}

void saveStage() {
    // Create and open a text file
    std::ofstream file("stage.txt");

    // Write to the file
    for (int i = 0; i < objects.size(); i++) {
        for (int j = 0; j < 5; j++) {
            file << objects[i][j];
            if(j<4)file << ',';
            
        }
        file << ';';
    }
    // Close the file
    file.close();
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
                defineObject(obj[0], obj[1], obj[2], obj[3], obj[4], 0);
            }
            if (data[i] == ';') obj.clear();
            continue;
        }
        else {
            output += data[i];
        }

    }
}

void loadStageFromFile(std::string id) {
    std::string text;
    const std::string path = std::string("levels/story/stage[") + id + "].txt";

    std::ifstream file(path);

    while (getline(file, text)) {
        loadStage(text);
    }
    file.close();
}


void handleControls() {
    if (IsKeyDown(KEY_MINUS)) {
        initialZoom /= 1.1;
    }
    if (IsKeyDown(KEY_EQUAL)) {
        initialZoom *= 1.1;
    }
    if (IsKeyPressed(KEY_ONE)) {
        if(editMode)saveStage();
    }
    zoomVel += GetMouseWheelMove()/75;
    initialZoom += zoomVel;
    zoomVel /= 1.3;
    if (initialZoom > 10) initialZoom = 10;
    if (initialZoom < .01) initialZoom = .01;

    if (IsKeyPressed(KEY_F11)) {
        if (isFullscreen) {
            ToggleFullscreen();
            SetWindowSize(oldWindowWidth, oldWindowHeight);
        }
        else {
            oldWindowWidth = GetScreenWidth();
            oldWindowHeight = GetScreenHeight();
            SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
            ToggleFullscreen();
        }
        isFullscreen = !isFullscreen;
    }
    if (IsKeyPressed(KEY_TWO)) {
        if (editMode)loadStageFromFile("");
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

// so proud of this camera system! It's my own design (pretty much like anything here) and works incredibly well.
// Ported it from my game Cattie's World 2
void setCamera(int x, int y, bool s) {
    int smoothness = 10;
    int damping = 100;
    if (s) {
        x = x - cameraX;
        y = y - cameraY;
        camXV += x;
        camYV += y;
        cameraX += (x / smoothness);
        cameraY += (y / smoothness);
        camXV = camXV / damping;
        camYV = camYV / damping;
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





void clearStage() {
    for (int i = 1; i < objects.size() - 1; i++) {
        objects.erase(std::next(objects.begin(), 1), std::next(objects.begin(), objects.size()));
    }
}

void drawBodyPart(Texture2D texture, int x, int y, float scale, int clampSide) {
    //0 = center, 1 = left, 2 = right, 3 = bottom, 4 = top
    float x1;
    float y1;
    float centerX = playerX;
    float centerY = playerY;
    if (clampSide == 0) { //center image
        float w = texture.width * scale;
        float h = texture.height * scale;

        float wx = playerX - w / 2 + x;
        float wy = playerY - h / 2 + y;

        Vector2 pos = {
            screenToWorldX(wx),
            screenToWorldY(wy)
        };

        DrawTextureEx(texture,
            { screenToWorldX((centerX) + x), screenToWorldY((centerY - texture.height / 2) + y) }, 0, screenToWorldSize(scale), RED);
    }



    else if (clampSide == 1) { //texture should clamp to left
        DrawTextureEx(texture, { screenToWorldX((centerX) + x), screenToWorldY((centerY + texture.height / 2) + y) }, 0, screenToWorldSize(scale), RED);
    }
    else if (clampSide == 2) { //texture should clamp to right
        DrawTextureEx(texture, { screenToWorldX((centerX + texture.width) + x), screenToWorldY((centerY + texture.height / 2) + y) }, 0, screenToWorldSize(scale), RED);
    }
    else if (clampSide == 3) { //texture should clamp to right
        DrawTextureEx(texture, { screenToWorldX((centerX - texture.width/2) + x), screenToWorldY((centerY - texture.height) + y) }, 0, screenToWorldSize(scale), RED);
    }
    else if (clampSide == 4) { //texture should clamp to right
        DrawTextureEx(texture, { screenToWorldX((centerX - texture.width / 2) + x), screenToWorldY((centerY + texture.height) + y) }, 0, screenToWorldSize(scale), RED);
    }

    DrawRectangle(screenToWorldX(playerX), screenToWorldY(playerY), 20, 20, GREEN);
}

void renderPlayer() {
    float playerScale = 2.69;
    int offsX = -15;
    int offsY = -5;
    drawBodyPart(textures.ok, 0, 0, playerScale, 0);

    
    
    
    
    
    return;
    //body
    DrawTextureEx(textures.ref, { screenToWorldX((playerX + offsX)), screenToWorldY((playerY + offsY)) }, 0, screenToWorldSize(playerScale), RED);

    //head
    DrawTextureEx(textures.head, { screenToWorldX((playerX + offsX) + 13), screenToWorldY((playerY + offsY))}, 0, screenToWorldSize(playerScale), WHITE);

    //tail
    DrawTextureEx(textures.tail, { screenToWorldX((playerX + offsX) ), screenToWorldY((playerY + offsY) + 59) }, 0, screenToWorldSize(playerScale), WHITE);

    //bpdy
    DrawTextureEx(textures.body, { screenToWorldX((playerX + offsX) + 10), screenToWorldY((playerY + offsY) + 30) }, 0, screenToWorldSize(playerScale), WHITE);


}



int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "cat game");

    SetTargetFPS(framerate);

    initTextures();

    defineObject(100, 0, playerHitbox[0], playerHitbox[1], 1, 0);


    ClearBackground(RAYWHITE);


    loadStageFromFile("1");

    //clearStage();

    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();

        if (oldWidth != screenWidth || oldZoom != initialZoom) {
            defaultWidthNumber = initialZoom / defaultWidth;
            cameraZ = screenWidth * defaultWidthNumber;
            oldWidth = screenWidth;
            oldZoom = initialZoom;
        }

        handleControls();
        int hoveredObj = checkMouseHover();
        bool isSelecting = hoveredObj != -1 && editMode && !IsKeyDown(KEY_LEFT_SHIFT);
        int padding = 3;
        if (isSelecting) {
            drawObject(objects[hoveredObj][0] - padding, objects[hoveredObj][1] - padding, objects[hoveredObj][2] + padding * 2, objects[hoveredObj][3] + padding * 2, clr_selected);
        }
        renderObjects();
        if (isSelecting) {
            drawObject(objects[hoveredObj][0], objects[hoveredObj][1], objects[hoveredObj][2], objects[hoveredObj][3], clr_selected_overlay);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hoveredObj>1) {
                objects.erase(objects.begin() + hoveredObj);
            }
        }
        handlePhysics();

        playerX = objects[0][0]+objects[0][2]/2;
        playerY = objects[0][1]+objects[0][3]/2;

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
            DrawText(std::to_string(initialZoom).c_str(), 20, 100, 20, LIGHTGRAY);
            DrawText("press 1 to save to file", 20, 120, 20, LIGHTGRAY);
        }

        renderPlayer();

        EndDrawing();

    }

    CloseWindow();

    return 0;
}