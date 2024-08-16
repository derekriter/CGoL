#include <cstdlib>
#include "tigr.h"

#define BITMAP_W 40
#define BITMAP_H 20
#define SCREEN_W (BITMAP_W * 32 + 300)
#define SCREEN_H (BITMAP_H * 32)

bool* bitmap;
int lastMouseButtons;
bool paintMode;

static void update(Tigr* screen, float delta);
static void render(Tigr* screen);

int main() {
    Tigr* screen = tigrWindow(SCREEN_W, SCREEN_H, "CGoL", TIGR_FIXED);

    bitmap = (bool*) calloc(BITMAP_W * BITMAP_H, sizeof(bool)); //calloc always zero-filled

    tigrTime();
    while(!tigrClosed(screen)) {
        float delta = tigrTime();

        update(screen, delta);
        render(screen);
    }
    tigrFree(screen);
    free(bitmap);

    return 0;
}

static void update(Tigr* screen, float delta) {
    int mouseX, mouseY, mouseButtons;
    tigrMouse(screen, &mouseX, &mouseY, &mouseButtons);

    if(mouseButtons & 1 && mouseX >= 0 && mouseX < BITMAP_W * 32 && mouseY >= 0 && mouseY < BITMAP_H * 32) {
        int cellX = mouseX / 32;
        int cellY = mouseY / 32;
        int index = cellX + cellY * BITMAP_W;

        if(!(lastMouseButtons & 1))
            paintMode = !bitmap[index];

        bitmap[index] = paintMode;
    }

    lastMouseButtons = mouseButtons;
}
static void render(Tigr* screen) {
    tigrClear(screen, tigrRGB(0, 0, 0));

    //grid
    for(int y = 0; y < BITMAP_H; y++) {
        for(int x = 0; x < BITMAP_W; x++) {
            tigrRect(screen, x * 32, y * 32, 32, 32, tigrRGB(33, 33, 33));
        }
    }

    //bitmap
    for(int i = 0; i < BITMAP_W * BITMAP_H; i++) {
        if(!bitmap[i])
            continue;

        int x = i % BITMAP_W * 32;
        int y = i / BITMAP_W * 32;

        tigrFillRect(screen, x, y, 32, 32, tigrRGB(255, 255, 255));
    }

    tigrUpdate(screen);
}
