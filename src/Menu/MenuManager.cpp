#include "Menu/MenuManager.h"
#define DEFAULT_PASSWORD 98765

Menu *accessDecider;
Menu *password;
Menu *customPassword;

MenuManager::MenuManager(std::vector<Menu *> &viewMenus, std::vector<Menu *> &editMenus, Display *display, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(Menu*)> reset, Preferences &preferences)
    : viewMenus(viewMenus), display(display), menuButton(menuButton), enterButton(enterButton), selectButton(selectButton), upButton(upButton), resetButton(resetButton), save(save), reset(reset), preferences(preferences)
{
    int currentAccessMode = preferences.getInt("accessMode", 0);
    float customPasswordValue = preferences.getFloat("CustomPass", 0);

    std::vector<String> accessModes = {"FuLL ", "pASS "};
    std::vector<String> accessModesName = {"Full Access", "Password Access"};
    accessDecider = new Menu("Access", "ACCES", "ACC", accessModes, accessModesName, currentAccessMode, display, true, VALUE_AND_SHORT_NAME);
    password = new Menu("Default Password", "PASSW", "PAS", 0, display, 0, true, 0, VALUE_AND_SHORT_NAME);
    customPassword = new Menu("Custom Password", "cpass", "cps", customPasswordValue, display, 0, true, 0, VALUE_AND_SHORT_NAME);
    editMenus.insert(editMenus.begin(), password);
    editMenus.insert(editMenus.end(), {customPassword, accessDecider});
    this->editMenus = editMenus;
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
        nextEditMenu(false);
    }
    if (isEnterButtonPressed)
    {
        if (isEditing)
        {
            editMenus[currentEditMenu]->stopEdit();
            isEditing = false;
            nextEditMenu(editMenus[currentEditMenu]->getMainName() == "Custom Password");
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

    editMenus[currentEditMenu]->displayItem();
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
    viewMenus[currentViewMenu]->displayItem();
}

void MenuManager:: nextEditMenu(bool isCustomPasswordChange)
{
    if (accessDecider->currentMode() == 0)
    {
        currentEditMenu = (currentEditMenu + 1) % editMenus.size();
    }
    if (isCustomPasswordChange || (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (password->value() == customPassword->value()))))
    {
        currentEditMenu = (currentEditMenu + 1) % editMenus.size();
    }
}

void MenuManager::nextViewMenu()
{
    currentViewMenu = (currentViewMenu + 1) % viewMenus.size();
}