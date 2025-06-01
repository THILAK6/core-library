#include "Menu/MenuManager.h"
#define DEFAULT_PASSWORD 98765

MenuItem *accessDecider;
MenuItem *password;
MenuItem *customPassword;

Menu *customPasswordMenu;

MenuManager::MenuManager(std::vector<Menu *> &viewMenus, std::vector<Menu *> &editMenus, Display *display, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(Menu*)> reset, Preferences &preferences)
    : viewMenus(viewMenus), display(display), menuButton(menuButton), enterButton(enterButton), selectButton(selectButton), upButton(upButton), resetButton(resetButton), save(save), reset(reset), preferences(preferences)
{
    int currentAccessMode = preferences.getInt("accessMode", 0);
    float customPasswordValue = preferences.getFloat("CustomPass", 0);

    std::vector<String> accessModes = {"FuLL ", "pASS "};
    std::vector<String> accessModesName = {"Full Access", "Password Access"};
    accessDecider = new MenuItem("Access Mode", "ACCES", "ACC", accessModes, accessModesName, currentAccessMode);
    customPassword = new MenuItem("Custom Password", "cpass", "cps", customPasswordValue, 0);
    password = new MenuItem("Default Password", "PASSW", "PAS", 0, 0);

    Menu* accessDeciderMenu = new Menu({*accessDecider}, display, true, VALUE_AND_SHORT_NAME);
    Menu* passwordMenu = new Menu({*password}, display, true, VALUE_AND_SHORT_NAME);
    customPasswordMenu = new Menu({*customPassword}, display, true, VALUE_AND_SHORT_NAME);

    editMenus.insert(editMenus.begin(), passwordMenu);
    editMenus.insert(editMenus.end(), {customPasswordMenu, accessDeciderMenu});
    this->editMenus = editMenus;
}

MenuManager::~MenuManager() {
    delete accessDecider;
    delete password;
    delete customPassword;
    
    // Don't delete customPasswordMenu here since it's in editMenus
    
    // Delete all menus
    for (Menu* menu : editMenus) {
        delete menu;
    }
}

void MenuManager::manage()
{
    bool isMenuButtonPressed = display->isButtonPressed(menuButton);
    bool isUpButtonPressed = display->isButtonPressed(upButton);
    bool isSelectButtonPressed = display->isButtonPressed(selectButton);
    bool isEnterButtonPressed = display->isButtonPressed(enterButton);
    bool isResetButtonPressed = display->isButtonPressed(resetButton);

    if (isMenuButtonPressed)
    {
        isEditMenuActive = true;
    }

    if (isEditMenuActive)
    {
        editMenuControl(isMenuButtonPressed, isEnterButtonPressed, isSelectButtonPressed, isUpButtonPressed, isResetButtonPressed);
    }
    else
    {
        viewMenuControl(isUpButtonPressed, isResetButtonPressed);
    }
}

void MenuManager::editMenuControl(bool isMenuButtonPressed, bool isEnterButtonPressed, bool isSelectButtonPressed, bool isUpButtonPressed, bool isResetButtonPressed)
{
    if (isResetButtonPressed)
    {
        editMenus[currentEditMenu]->stopEdit();
        isEditMenuActive = false;
        currentEditMenu = 0;
    }
    if (isMenuButtonPressed && !isEditing)
    {
        nextEditMenuItem(false);
    }
    if (isEnterButtonPressed)
    {
        if (isEditing)
        {
            editMenus[currentEditMenu]->stopEdit();
            isEditing = false;
            nextEditMenuItem(editMenus[currentEditMenu] == customPasswordMenu);
            preferences.putFloat("CustomPass", customPassword->value());
            preferences.putInt("accessMode", accessDecider->currentMode());
            save();
        }
        else
        {
            isEditing = true;
            editMenus[currentEditMenu]->edit(isUpButtonPressed, true);
        }
    }
    if (isEditing)
    {
        editMenus[currentEditMenu]->edit(isUpButtonPressed, isSelectButtonPressed);
    }

    editMenus[currentEditMenu]->displayMenu();
}

void MenuManager::viewMenuControl(bool isUpButtonPressed, bool isResetButtonPressed)
{
    if (isUpButtonPressed)
    {
        nextViewMenu();
    }
    if (isResetButtonPressed)
    {
        reset(viewMenus[currentViewMenu]);
    }
    viewMenus[currentViewMenu]->displayMenu();
}

void MenuManager::nextEditMenuItem(bool isCustomPasswordChange)
{
    if (accessDecider->currentMode() == 0)
    {   
        bool reachedEnd = editMenus[currentEditMenu]->nextMenuItem();
        if(reachedEnd)
        {
            currentEditMenu = (currentEditMenu + 1) % editMenus.size();
        }
    }
    if (isCustomPasswordChange || (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (password->value() == customPassword->value()))))
    {
        bool reachedEnd = editMenus[currentEditMenu]->nextMenuItem();
        if(reachedEnd)
        {
            currentEditMenu = (currentEditMenu + 1) % editMenus.size();
        }
    }
}

void MenuManager::nextViewMenu()
{
    currentViewMenu = (currentViewMenu + 1) % viewMenus.size();
}