#pragma once
#include <vector>
#include "Button.h"
#include <Arduino.h>

class SkipManager {
private:
    std::vector<Button> buttons;
    unsigned long idleLimit;
    

public:
    // Constructor
    SkipManager(const std::vector<Button>& buttons, unsigned long idleLimit);

    // Check if all buttons are idle
    bool areAllButtonsIdle();
};