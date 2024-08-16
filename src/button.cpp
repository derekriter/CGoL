#include "button.hpp"

Button::Button(int x, int y, const char* label) {
    this->x = x;
    this->y = y;
    this->label = label;
    this->clickLast = false;

    int lWidth = tigrTextWidth(tfont, this->label);
    int lHeight = tigrTextHeight(tfont, this->label);
    this->w = lWidth + 12;
    this->h = lHeight + 3;
}
void Button::render(Tigr* screen) const {
    tigrPrint(screen, tfont, this->x + 6, this->y + 3, buttonTextCol, this->label);
    tigrRect(screen, this->x, this->y, this->w, this->h, buttonCol);
}
bool Button::wasClicked(Tigr* screen) {
    int mX, mY, mB;
    tigrMouse(screen, &mX, &mY, &mB);

    bool mouseDown = mB & 1 && mX >= this->x && mX < this->x + this->w && mY >= this->y && mY < this->y + this->h;
    bool click = mouseDown && !clickLast;
    this->clickLast = mouseDown;
    return click;
}
