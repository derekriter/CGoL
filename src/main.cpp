#include <cstdlib>
#include <memory>
#include <iostream>
#include <cstring>
#include "tigr.h"
#include "button.hpp"

#define BITMAP_W 80
#define BITMAP_H 40
#define CELL_SIZE 16
#define PANEL_W 80
#define SCREEN_W (BITMAP_W * CELL_SIZE + PANEL_W)
#define SCREEN_H (BITMAP_H * CELL_SIZE)

const TPixel bgCol = tigrRGB(0, 0, 0);
const TPixel gridCol = tigrRGB(33, 33, 33);
const TPixel fgCol = tigrRGB(255, 255, 255);
const TPixel panelCol = tigrRGB(36, 61, 71);
const TPixel tickCol = tigrRGB(255, 255, 255);
const TPixel radioActiveCol = tigrRGB(245, 195, 22);
const TPixel radioInactiveCol = tigrRGB(33, 33, 33);

bool* bitmap, *newBitmap;
int lastMouseButtons;
bool paintMode;
bool redraw = true;
unsigned long tick = 0;
std::unique_ptr<Button> run, stop, step, reset, speed1, speed5, speed10, speed20, speed50, speedMax;
bool running = false;
int speedSelect = 1;
float updateInterval = 0.2;
float lastUpdateDelta = 0;

static void update(Tigr* screen, float delta);
static void render(Tigr* screen);
static void updateBitmap();
static bool getNeighborFromIndex(int x, int y, int index);

int main() {
    Tigr* screen = tigrWindow(SCREEN_W, SCREEN_H, "CGoL", TIGR_FIXED);

    bitmap = (bool*) calloc(BITMAP_W * BITMAP_H, sizeof(bool)); //calloc is always zero-filled
    newBitmap = (bool*) calloc(BITMAP_W * BITMAP_H, sizeof(bool));

    run = std::make_unique<Button>(0, 10, "Run");
    run->x = SCREEN_W - run->w - (PANEL_W - run->w) / 2;
    stop = std::make_unique<Button>(0, run->y + run->h + 2, "Stop");
    stop->x = SCREEN_W - stop->w - (PANEL_W - stop->w) / 2;
    step = std::make_unique<Button>(0, stop->y + stop->h + 2, "Step");
    step->x = SCREEN_W - step->w - (PANEL_W - step->w) / 2;
    reset = std::make_unique<Button>(0, step->y + step->h + 10, "Reset");
    reset->x = SCREEN_W - reset->w - (PANEL_W - reset->w) / 2;

    speed1 = std::make_unique<Button>(0, reset->y + reset->h + 40, "1 Hz");
    speed1->x = SCREEN_W - speed1->w - (PANEL_W - speed1->w) / 2;
    speed5 = std::make_unique<Button>(0, speed1->y + speed1->h + 2, "5 Hz");
    speed5->x = SCREEN_W - speed5->w - (PANEL_W - speed5->w) / 2;
    speed10 = std::make_unique<Button>(0, speed5->y + speed5->h + 2, "10 Hz");
    speed10->x = SCREEN_W - speed10->w - (PANEL_W - speed10->w) / 2;
    speed20 = std::make_unique<Button>(0, speed10->y + speed10->h + 2, "20 Hz");
    speed20->x = SCREEN_W - speed20->w - (PANEL_W - speed20->w) / 2;
    speed50 = std::make_unique<Button>(0, speed20->y + speed20->h + 2, "50 Hz");
    speed50->x = SCREEN_W - speed50->w - (PANEL_W - speed50->w) / 2;
    speedMax = std::make_unique<Button>(0, speed50->y + speed50->h + 2, "Max");
    speedMax->x = SCREEN_W - speedMax->w - (PANEL_W - speedMax->w) / 2;

    tigrTime();
    while(!tigrClosed(screen)) {
        float delta = tigrTime();

        update(screen, delta);
        if(redraw) {
            render(screen);
            redraw = false;
        }

        tigrUpdate(screen);
    }
    tigrFree(screen);
    free(bitmap);
    free(newBitmap);

    return 0;
}

static void update(Tigr* screen, float delta) {
    int mouseX, mouseY, mouseButtons;
    tigrMouse(screen, &mouseX, &mouseY, &mouseButtons);

    if(mouseButtons & 1 && mouseX >= 0 && mouseX < BITMAP_W * CELL_SIZE && mouseY >= 0 && mouseY < BITMAP_H * CELL_SIZE) {
        int cellX = mouseX / CELL_SIZE;
        int cellY = mouseY / CELL_SIZE;
        int index = cellX + cellY * BITMAP_W;

        if(!(lastMouseButtons & 1))
            paintMode = !bitmap[index];

        if(bitmap[index] != paintMode) {
            bitmap[index] = paintMode;
            redraw = true;
        }
    }

    if(run->wasClicked(screen)) {
        running = true;
        redraw = true;
    }
    else if(stop->wasClicked(screen)) {
        running = false;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(step->wasClicked(screen)) {
        running = false;
        lastUpdateDelta = 0;
        updateBitmap();
        redraw = true;
    }
    else if(reset->wasClicked(screen)) {
        tick = 0;
        running = false;
        lastUpdateDelta = 0;
        memset(bitmap, 0, BITMAP_W * BITMAP_H * sizeof(bool));
        redraw = true;
    }
    else if(speed1->wasClicked(screen)) {
        speedSelect = 0;
        updateInterval = 1;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(speed5->wasClicked(screen)) {
        speedSelect = 1;
        updateInterval = 0.2;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(speed10->wasClicked(screen)) {
        speedSelect = 2;
        updateInterval = 0.1;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(speed20->wasClicked(screen)) {
        speedSelect = 3;
        updateInterval = 0.05;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(speed50->wasClicked(screen)) {
        speedSelect = 4;
        updateInterval = 0.02;
        lastUpdateDelta = 0;
        redraw = true;
    }
    else if(speedMax->wasClicked(screen)) {
        speedSelect = 5;
        updateInterval = 0;
        lastUpdateDelta = 0;
        redraw = true;
    }

    if(running && updateInterval > 0) {
        lastUpdateDelta += delta;
        while(lastUpdateDelta >= updateInterval) {
            updateBitmap();
            lastUpdateDelta -= updateInterval;
        }
    }
    else if(running)
        updateBitmap();

    lastMouseButtons = mouseButtons;
}
static void render(Tigr* screen) {
    tigrClear(screen, bgCol);

    //bitmap
    for(int i = 0; i < BITMAP_W * BITMAP_H; i++) {
        if(!bitmap[i])
            continue;

        int x = i % BITMAP_W * CELL_SIZE;
        int y = i / BITMAP_W * CELL_SIZE;

        tigrRect(screen, x, y, CELL_SIZE, CELL_SIZE, fgCol);
        tigrFillRect(screen, x, y, CELL_SIZE, CELL_SIZE, fgCol);
    }

    //grid
    for(int x = 0; x < BITMAP_W; x++) {
        tigrLine(screen, x * CELL_SIZE, 0, x * CELL_SIZE, SCREEN_H, gridCol);
    }
    for(int y = 0; y < BITMAP_H; y++) {
        tigrLine(screen, 0, y * CELL_SIZE, BITMAP_W * CELL_SIZE, y * CELL_SIZE, gridCol);
    }

    //side panel
    tigrRect(screen, SCREEN_W - PANEL_W, 0, PANEL_W, SCREEN_H, panelCol);
    tigrFillRect(screen, SCREEN_W - PANEL_W, 0, PANEL_W, SCREEN_H, panelCol);

    run->render(screen);
    stop->render(screen);
    step->render(screen);
    reset->render(screen);
    speed1->render(screen);
    speed5->render(screen);
    speed10->render(screen);
    speed20->render(screen);
    speed50->render(screen);
    speedMax->render(screen);

    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, run->y + run->h / 2, 3, running ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, stop->y + stop->h / 2, 3, running ? radioInactiveCol : radioActiveCol);

    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speed1->y + speed1->h / 2, 3, speedSelect == 0 ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speed5->y + speed5->h / 2, 3, speedSelect == 1 ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speed10->y + speed10->h / 2, 3, speedSelect == 2 ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speed20->y + speed20->h / 2, 3, speedSelect == 3 ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speed50->y + speed50->h / 2, 3, speedSelect == 4 ? radioActiveCol : radioInactiveCol);
    tigrFillCircle(screen, SCREEN_W - PANEL_W + 5, speedMax->y + speedMax->h / 2, 3, speedSelect == 5 ? radioActiveCol : radioInactiveCol);

    std::string tickMsgStr = "T: " + std::to_string(tick);
    const char* tickMsg = tickMsgStr.c_str();
    int msgW = tigrTextWidth(tfont, tickMsg);
    int msgH = tigrTextHeight(tfont, tickMsg);
    tigrPrint(screen, tfont, SCREEN_W - msgW - 3, SCREEN_H - msgH, tickCol, tickMsg);
}
static void updateBitmap() {
    memcpy(newBitmap, bitmap, BITMAP_W * BITMAP_H * sizeof(bool));

    for(int y = 0; y < BITMAP_H; y++) {
        for(int x = 0; x < BITMAP_W; x++) {
            int i = x + y * BITMAP_W;

            int nCount = getNeighborFromIndex(x, y, 0) +
                getNeighborFromIndex(x, y, 1) +
                getNeighborFromIndex(x, y, 2) +
                getNeighborFromIndex(x, y, 3) +
                getNeighborFromIndex(x, y, 4) +
                getNeighborFromIndex(x, y, 5) +
                getNeighborFromIndex(x, y, 6) +
                getNeighborFromIndex(x, y, 7);

            if(bitmap[i] && nCount < 2 || nCount > 3)
                newBitmap[i] = false;
            else if(nCount == 3)
                newBitmap[i] = true;
        }
    }

    std::swap(bitmap, newBitmap);
    tick++;
    redraw = true;
}
static bool getNeighborFromIndex(int x, int y, int index) {
    int nX = x;
    int nY = y;

    switch(index) {
        case 0:
            nX--;
            nY--;
            break;
        case 1:
            nY--;
            break;
        case 2:
            nX++;
            nY--;
            break;
        case 3:
            nX++;
            break;
        case 4:
            nX++;
            nY++;
            break;
        case 5:
            nY++;
            break;
        case 6:
            nX--;
            nY++;
            break;
        case 7:
            nX--;
            break;
    }

    if(nX < 0)
        nX += BITMAP_W;
    else if(nX >= BITMAP_W)
        nX -= BITMAP_W;

    if(nY < 0)
        nY += BITMAP_H;
    else if(nY >= BITMAP_H)
        nY -= BITMAP_H;

    return bitmap[nX + nY * BITMAP_W];
}
