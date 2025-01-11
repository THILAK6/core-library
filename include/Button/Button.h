#ifndef BUTTON_H
#define BUTTON_H

#include <functional>
#include <Arduino.h>

class Button {
private:
    bool flag;                      
    std::function<int()> readFunc;  
    unsigned long lastPressedTime;  

public:
    Button(std::function<int()> readFunc);

    bool isPressed();

    unsigned long getLastPressedTime() const;
};

#endif // BUTTON_H