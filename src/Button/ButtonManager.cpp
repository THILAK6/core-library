#include "Button/ButtonManager.h"

void ButtonManager::addButton(const std::string& name, std::function<int()> readFunc) {
    buttons.emplace_back(readFunc);
    nameToIndex[name] = buttons.size() - 1;
}

void ButtonManager::addUnnamedButton(std::function<int()> readFunc) {
    buttons.emplace_back(readFunc);
}

Button& ButtonManager::getButton(const std::string& name) {
    return buttons.at(nameToIndex.at(name));
}

Button& ButtonManager::getButton(size_t index) {
    return buttons.at(index);
}

size_t ButtonManager::size() const {
    return buttons.size();
}
