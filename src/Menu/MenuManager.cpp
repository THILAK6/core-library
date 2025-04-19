#include "Menu/MenuManager.h"
#define DEFAULT_PASSWORD 98765

MenuItem *accessDecider;
MenuItem *password;
MenuItem *customPassword;

MenuManager::MenuManager(std::vector<MenuItem *> &viewMenuItems, std::vector<MenuItem *> &editMenuItems, TM1638 *module, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(MenuItem*)> reset, Preferences &preferences)
    : viewMenuItems(viewMenuItems), module(module), menuButton(menuButton), enterButton(enterButton), selectButton(selectButton), upButton(upButton), resetButton(resetButton), save(save), reset(reset), preferences(preferences)
{
    int currentAccessMode = preferences.getInt("accessMode", 0);
    float customPasswordValue = preferences.getFloat("CustomPass", 0);

    std::vector<String> accessModes = {"FuLL ", "pASS "};
    std::vector<String> accessModesName = {"Full Access", "Password Access"};
    accessDecider = new MenuItem("Access", "ACCES", "ACC", accessModes, accessModesName, currentAccessMode, module, true, VALUE_AND_SHORT_NAME);
    password = new MenuItem("Default Password", "PASSW", "PAS", 0, module, 0, true, 0, VALUE_AND_SHORT_NAME);
    customPassword = new MenuItem("Custom Password", "cpass", "cps", customPasswordValue, module, 0, true, 0, VALUE_AND_SHORT_NAME);
    editMenuItems.insert(editMenuItems.begin(), password);
    editMenuItems.insert(editMenuItems.end(), {customPassword, accessDecider});
    this->editMenuItems = editMenuItems;
}

void MenuManager::manage()
{
    bool isMenuButtonPressed = module->isButtonPressed(menuButton);
    bool isUpButtonPressed = module->isButtonPressed(upButton);
    bool isSelectButtonPressed = module->isButtonPressed(selectButton);
    bool isEnterButtonPressed = module->isButtonPressed(enterButton);
    bool isResetButtonPressed = module->isButtonPressed(resetButton);

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
        editMenuItems[currentEditMenuItem]->stopEdit();
        isEditMenuActive = false;
        currentEditMenuItem = 0;
    }
    if (isMenuButtonPressed && !isEditing)
    {
        nextEditMenuItem(false);
    }
    if (isEnterButtonPressed)
    {
        if (isEditing)
        {
            editMenuItems[currentEditMenuItem]->stopEdit();
            isEditing = false;
            nextEditMenuItem(editMenuItems[currentEditMenuItem]->getMainName() == "Custom Password");
            preferences.putFloat("CustomPass", customPassword->value());
            preferences.putInt("accessMode", accessDecider->currentMode());
            save();
        }
        else
        {
            isEditing = true;
            editMenuItems[currentEditMenuItem]->edit(isUpButtonPressed, true);
        }
    }
    if (isEditing)
    {
        editMenuItems[currentEditMenuItem]->edit(isUpButtonPressed, isSelectButtonPressed);
    }

    editMenuItems[currentEditMenuItem]->display();
}

void MenuManager::viewMenuControl(bool isUpButtonPressed, bool isResetButtonPressed)
{
    if (isUpButtonPressed)
    {
        nextViewMenuItem();
    }
    if (isResetButtonPressed)
    {
        reset(viewMenuItems[currentViewMenuItem]);
    }
    viewMenuItems[currentViewMenuItem]->display();
}

void MenuManager:: nextEditMenuItem(bool isCustomPasswordChange)
{
    if (accessDecider->currentMode() == 0)
    {
        currentEditMenuItem = (currentEditMenuItem + 1) % editMenuItems.size();
    }
    if (isCustomPasswordChange || (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (password->value() == customPassword->value()))))
    {
        currentEditMenuItem = (currentEditMenuItem + 1) % editMenuItems.size();
    }
}

void MenuManager::nextViewMenuItem()
{
    currentViewMenuItem = (currentViewMenuItem + 1) % viewMenuItems.size();
}