#include "tigr.h"

//16:9 ratio
#define BITMAP_W 48
#define BITMAP_H 27
#define SCREEN_W (BITMAP_W * 16)
#define SCREEN_H (BITMAP_H * 16)

int main() {
    Tigr* screen = tigrWindow(SCREEN_W, SCREEN_H, "CGoL", TIGR_FIXED);

    while(!tigrClosed(screen)) {
        tigrClear(screen, tigrRGB(0, 0, 0));

        tigrPrint(screen, tfont, 0, 0, tigrRGB(255, 255, 255), "Hello, world!");

        tigrUpdate(screen);
    }
    tigrFree(screen);

    return 0;
}
