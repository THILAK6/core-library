#include "Button/ButtonManager.h"

ButtonManager::ButtonManager(const std::vector<std::pair<std::string, int>>& pinMappings) {
    for (const auto& [name, pin] : pinMappings) {
        pinMode(pin, INPUT);  // Set pin mode
        buttons.emplace(name, Button([pin]() { return digitalRead(pin); }));
    }
}

Button& ButtonManager::getButton(const std::string& name) {
    return buttons.at(name);
}

SkipManager ButtonManager::createSkipManager(unsigned long idleLimit) {
    std::vector<Button> buttonList;
    for (auto& [_, button] : buttons) {
        buttonList.push_back(button);
    }
    return SkipManager(buttonList, idleLimit);
}