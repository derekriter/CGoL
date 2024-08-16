#pragma once

#include <string>
#include "tigr.h"

const TPixel buttonCol = tigrRGB(245, 195, 22);
const TPixel buttonTextCol = tigrRGB(255, 255, 255);

class Button {
    public:
        int x, y;
        int w, h;
        const char* label;

        Button(int x, int y, const char* label);

        void render(Tigr* screen) const;
        [[nodiscard]] bool wasClicked(Tigr* screen);

    private:
        bool clickLast;
};
