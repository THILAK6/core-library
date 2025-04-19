#pragma once
#include <vector>
#include "MenuItem.h"
#include "Preferences.h"

class MenuManager
{
private:
    std::vector<MenuItem *> editMenuItems;
    std::vector<MenuItem *> viewMenuItems;
    TM1638 *module;
    int menuButton;
    int enterButton;
    int selectButton;
    int upButton;
    int resetButton;
    bool isEditMenuActive = false;
    bool isEditing = false;
    uint8_t currentEditMenuItem = 0;
    uint8_t currentViewMenuItem = 0;
    Preferences preferences;
    void viewMenuControl(bool isUpButtonPressed, bool isResetButtonPressed);
    void editMenuControl(bool isMenuButtonPressed, bool isEnterButtonPressed, bool isSelectButtonPressed, bool isUpButtonPressed, bool isResetButtonPressed);
    void nextEditMenuItem(bool isCustomPasswordChange);
    void nextViewMenuItem();
    std::function<void()> save;
    std::function<void(MenuItem*)> reset;

public:
    MenuManager(std::vector<MenuItem *> &viewMenuItems, std::vector<MenuItem *> &editMenuItems, TM1638 *module, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(MenuItem*)> reset, Preferences &preferences);
    void manage();
};