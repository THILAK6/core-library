#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include "Button.h"
#include "SkipManager.h"
#include <Arduino.h>

class ButtonManager {
private:
    std::unordered_map<std::string, Button> buttons;

public:
    ButtonManager(const std::vector<std::pair<std::string, int>>& pinMappings);

    Button& getButton(const std::string& name);

    SkipManager createSkipManager(unsigned long idleLimit);
};