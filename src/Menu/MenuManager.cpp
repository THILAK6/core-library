#include "Menu/MenuManager.h"
#define DEFAULT_PASSWORD 66666

MenuItem *accessDecider;
MenuItem *password;

MenuManager::MenuManager(std::vector<MenuItem *> &viewMenuItems, std::vector<MenuItem *> &editMenuItems, TM1638 *module, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, MenuItem &customPassword, std::function<void()> save, std::function<void(MenuItem*)> reset)
    : viewMenuItems(viewMenuItems), module(module), menuButton(menuButton), enterButton(enterButton), selectButton(selectButton), upButton(upButton), resetButton(resetButton), customPassword(customPassword), save(save), reset(reset)
{
    std::vector<String> accessModes = {"FuLL ", "pASS "};
    std::vector<String> accessModesName = {"Full Access", "Password Access"};
    accessDecider = new MenuItem("Access", "ACCES", "ACC", accessModes, accessModesName, 0, module, true, 0);
    password = new MenuItem("Default Password", "PASSW", "PAS", 0, module, true, 0, 0);
    editMenuItems.insert(editMenuItems.begin(), password);
    editMenuItems.insert(editMenuItems.end(), {&customPassword, accessDecider});
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
        if (accessDecider->currentMode() == 0)
        {
            nextEditMenuItem();
        }
        if (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (password->value() == customPassword.value())))
        {
            nextEditMenuItem();
        }
    }
    if (isEnterButtonPressed)
    {
        if (isEditing)
        {
            editMenuItems[currentEditMenuItem]->stopEdit();
            isEditing = false;
            nextEditMenuItem();
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

void MenuManager::nextEditMenuItem()
{
    if (accessDecider->currentMode() == 0)
    {
        currentEditMenuItem = (currentEditMenuItem + 1) % editMenuItems.size();
    }
    if (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (password->value() == customPassword.value())))
    {
        currentEditMenuItem = (currentEditMenuItem + 1) % editMenuItems.size();
    }
}

void MenuManager::nextViewMenuItem()
{
    currentViewMenuItem = (currentViewMenuItem + 1) % viewMenuItems.size();
}