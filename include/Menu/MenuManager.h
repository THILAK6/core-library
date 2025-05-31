#pragma once
#include <vector>
#include "Menu.h"
#include "Preferences.h"

class MenuManager
{
private:
    std::vector<Menu *> editMenus;
    std::vector<Menu *> viewMenus;
    Display *display;
    int menuButton;
    int enterButton;
    int selectButton;
    int upButton;
    int resetButton;
    bool isEditMenuActive = false;
    bool isEditing = false;
    uint8_t currentEditMenu = 0;
    uint8_t currentViewMenu = 0;
    Preferences preferences;
    void viewMenuControl(bool isUpButtonPressed, bool isResetButtonPressed);
    void editMenuControl(bool isMenuButtonPressed, bool isEnterButtonPressed, bool isSelectButtonPressed, bool isUpButtonPressed, bool isResetButtonPressed);
    void nextEditMenu(bool isCustomPasswordChange);
    void nextViewMenu();
    std::function<void()> save;
    std::function<void(Menu*)> reset;

public:
    MenuManager(std::vector<Menu *> &viewMenus, std::vector<Menu *> &editMenus, Display *display, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(Menu*)> reset, Preferences &preferences);
    void manage();
};