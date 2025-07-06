#pragma once
#include <cstdint>
#include <vector>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Display/Display.h"
#include "MenuItem.h"

class Menu
{
private:
    Display *display;
    DisplayMode displayMode;
    bool isEditable;
    int8_t currentMenuItem = 0;
    std::vector<MenuItem*> menuItems;
    void editValueType(bool upAction, bool moveAction);
    void editModeType(bool upAction, bool moveAction);

public:
    Menu(const std::vector<MenuItem*> &menuItems, Display *display, bool isEditable, DisplayMode displayMode);

    ~Menu();

    void displayMenu();

    void stopEdit();
    void edit(bool upAction, bool moveAction);
    void setDisplayType(DisplayMode displayMode);
    bool nextMenuItem();
    MenuItem* getCurrentMenuItem();

};