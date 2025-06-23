#include "Menu/MenuManager.h"
#include "Display/SevenSegmentDisplay.h"
#include <Preferences.h>

Preferences prefs;
MenuManager* manager;

std::vector<Menu*> viewMenus;
std::vector<Menu*> editMenus;


void setup()
{
    manager = new MenuManager(
        viewMenus,
        editMenus,
        new SevenSegmentDisplay(12, 11, 10, 7),
        0,
        1,
        2,
        3,
        4,
        []() {},
        [](MenuItem* menuItem) {},
        prefs
    );
}

void loop()
{
}