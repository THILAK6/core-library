#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include "Button.h"

class ButtonManager {
public:
    void addButton(const std::string& name, std::function<int()> readFunc);
    void addUnnamedButton(std::function<int()> readFunc);

    Button& getButton(const std::string& name);
    Button& getButton(size_t index);

    size_t size() const;

private:
    std::vector<Button> buttons;
    std::unordered_map<std::string, size_t> nameToIndex;
};

#endif
