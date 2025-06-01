#include "Menu/MenuManager.h"
#include "Display/LcdDisplay.h"
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
        new LcdDisplay(0, 1, 2),
        0,
        1,
        2,
        3,
        4,
        []() {},
        [](Menu* menu) {},
        prefs
    );
}

void loop()
{
}