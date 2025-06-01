#include "Display/LcdDisplay.h"

LcdDisplay::LcdDisplay(){}

void showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) {}


void showValueAndShortName(MenuItem &menuItem, bool isEditable) {}
void showValue(bool isEditable, MenuItem &menuItem, MenuItem &menuItemAdditional) {}
void showName(MenuItem &menuItem) {}

boolean LcdDisplay::isButtonPressed(int button)
{
    return false;
}