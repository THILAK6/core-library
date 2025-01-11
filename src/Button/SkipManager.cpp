#include "Button/SkipManager.h"

// Constructor
SkipManager::SkipManager(const std::vector<Button>& buttons, unsigned long idleLimit)
    : buttons(buttons), idleLimit(idleLimit) {}

// Check if all buttons are idle
bool SkipManager::areAllButtonsIdle() {
    for (const Button& button : buttons) {
        if (millis() - button.getLastPressedTime() < idleLimit) {
            return false;
        }
    }
    return true;
}