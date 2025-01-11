#include "Button/Button.h"

Button::Button(std::function<int()> readFunc) : flag(true), readFunc(readFunc), lastPressedTime(0) {}

bool Button::isPressed() {
    int state = readFunc();  
    if (state == HIGH && flag) {
        flag = false;
        lastPressedTime = millis();  
        return true;
    }
    if (state == LOW) {
        flag = true;
    }
    return false;
}

unsigned long Button::getLastPressedTime() const {
    return lastPressedTime;
}