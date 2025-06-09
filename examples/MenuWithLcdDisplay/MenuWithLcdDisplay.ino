#include "Menu/MenuManager.h"
#include "Display/LcdDisplay.h"
#include <Preferences.h>

LcdDisplay display(0x27, 20, 4, 13, 12, 11);
Preferences prefs;
MenuManager* manager;

MenuItem* item1 = new MenuItem("Item 1", "Value 1", "V", 1, 0);

volatile MenuItem* volatileItem1 = item1;

Menu viewMenu({*item1}, &display, false, DisplayMode::VALUE);

std::vector<Menu*> viewMenus = {&viewMenu};
std::vector<Menu*> editMenus;


void setup()
{
    manager = new MenuManager(
        viewMenus,
        editMenus,
        &display,
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