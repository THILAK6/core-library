#include "Menu/Menu.h"

Menu::Menu(std::vector<MenuItem> menuItems, Display *display, bool isEditable, DisplayMode displayMode)
    : menuItems(menuItems), display(display), displayMode(displayMode), isEditable(isEditable)
{}

Menu::~Menu()
{
    // menuItems.~vector<MenuItem>();
}

void Menu::displayMenu()
{
    display->showMenuItems(menuItems, currentMenuItem, displayMode, isEditable);
}

void Menu::edit(bool upAction, bool moveAction)
{
    if (isEditable)
    {
        menuItems[currentMenuItem].edit(upAction, moveAction);
    }
}

void Menu::stopEdit()
{
    if (isEditable)
    {
        menuItems[currentMenuItem].stopEdit();
    }
}

void Menu::setDisplayType(DisplayMode displayMode)
{
    this->displayMode = displayMode;
}

bool Menu::nextMenuItem()
{
    if (isEditable)
    {
        currentMenuItem++;
        if (currentMenuItem >= menuItems.size())
        {
            currentMenuItem = 0;
            return true;
        }
        return false;
    }
    return true;
}