#include <Arduino.h>
#include "Menu/MenuManager.h"
#define DEFAULT_PASSWORD 98765

MenuItem *accessDecider;
MenuItem *password;
MenuItem *customPassword;

// #define INPUT_CLOCK 27
// #define INPUT_LATCH 14
// #define INPUT_DATA 13

Menu *customPasswordMenu;

MenuManager::MenuManager(std::vector<Menu *> &viewMenus, std::vector<Menu *> &editMenus, Display *display, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, Preferences &preferences)
    : MenuManager(viewMenus, editMenus, display, menuButton, enterButton, selectButton, upButton, resetButton, save, nullptr, preferences)
{
}

MenuManager::MenuManager(std::vector<Menu *> &viewMenus, std::vector<Menu *> &editMenus, Display *display, int menuButton, int enterButton, int selectButton, int upButton, int resetButton, std::function<void()> save, std::function<void(MenuItem*)> reset, Preferences &preferences)
    : viewMenus(viewMenus), display(display), menuButton(menuButton), enterButton(enterButton), selectButton(selectButton), upButton(upButton), resetButton(resetButton), save(save), reset(reset), preferences(preferences)
{
    int currentAccessMode = preferences.getInt("accessMode", 0);
    if (currentAccessMode < 0 || currentAccessMode > 1) {
        currentAccessMode = 0;
    }
    float customPasswordValue = preferences.getFloat("CustomPass", 0);
    if (isnan(customPasswordValue) || isinf(customPasswordValue)) {
        customPasswordValue = 0.0f;
    }

    std::vector<String> accessModes = {"FuLL ", "pASS "};
    std::vector<String> accessModesName = {"Full Access", "Password Access"};
    accessDecider = new MenuItem("Access Mode", "ACCES", "ACC", accessModes, accessModesName, currentAccessMode);
    customPassword = new MenuItem("Custom Password", "cpass", "cps", customPasswordValue, 0);
    password = new MenuItem("Default Password", "PASSW", "PAS", 0, 0);

    Menu* accessDeciderMenu = new Menu({accessDecider}, display, true, VALUE_AND_SHORT_NAME);
    Menu* passwordMenu = new Menu({password}, display, true, VALUE_AND_SHORT_NAME);
    customPasswordMenu = new Menu({customPassword}, display, true, VALUE_AND_SHORT_NAME);

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
    if (waitingForExitButtonsRelease)
    {
        bool isMenuStillDown = display->isButtonDown(menuButton);
        bool isSelectStillDown = display->isButtonDown(selectButton);

        if (isMenuStillDown || isSelectStillDown)
        {
            exitCombinationCooldown = millis() + 400;
            display->clearButtonStates();
            viewMenus[currentViewMenu]->displayMenu();
            return;
        }
        else if (millis() < exitCombinationCooldown)
        {
            display->clearButtonStates();
            viewMenus[currentViewMenu]->displayMenu();
            return;
        }
        else
        {
            waitingForExitButtonsRelease = false;
            display->clearButtonStates();
        }
    }

    bool isMenuButtonPressed = display->isButtonPressed(menuButton);
    bool isUpButtonPressed = display->isButtonPressed(upButton);
    bool isSelectButtonPressed = display->isButtonPressed(selectButton);
    bool isEnterButtonPressed = display->isButtonPressed(enterButton);
    bool isResetButtonPressed = display->isButtonPressed(resetButton);

    bool isExitCombination = false;
    if (isEditMenuActive)
    {
        bool isMenuDown = display->isButtonDown(menuButton);
        bool isSelectDown = display->isButtonDown(selectButton);
        isExitCombination = (isMenuDown && isSelectDown) ||
                            (isMenuDown && isSelectButtonPressed) ||
                            (isSelectDown && isMenuButtonPressed) ||
                            (isMenuButtonPressed && isSelectButtonPressed);

        if (isExitCombination)
        {
            isMenuButtonPressed = false;
            isSelectButtonPressed = false;
            display->clearButtonStates();
        }

        // Keep inactivity timer alive if any button was pressed or is held down
        if (isMenuButtonPressed || isEnterButtonPressed || isSelectButtonPressed ||
            isUpButtonPressed || isResetButtonPressed || isExitCombination ||
            isMenuDown || isSelectDown ||
            display->isButtonDown(enterButton) || display->isButtonDown(upButton) || display->isButtonDown(resetButton))
        {
            lastEditMenuInteractionTime = millis();
        }

        // 10-second inactivity timeout: exit edit menu automatically
        if (millis() - lastEditMenuInteractionTime >= 10000)
        {
            Serial.println("[MenuManager] Edit menu 10s inactivity timeout. Returning to main screen.");
            if (isEditing)
            {
                preferences.putFloat("CustomPass", customPassword->value());
                preferences.putInt("accessMode", accessDecider->currentMode());
                if (save) save();
                editMenus[currentEditMenu]->stopEdit();
                isEditing = false;
            }
            isEditMenuActive = false;
            currentEditMenu = 0;
            password->updateValue(0.0f);
            display->clearButtonStates();
        }
    }

    if (isMenuButtonPressed)
    {
        Serial.println("[Button] MENU clicked");
        if (!isEditMenuActive)
        {
            isEditMenuActive = true;
            currentEditMenu = (accessDecider->currentMode() == 0 && editMenus.size() > 1) ? 1 : 0;
            password->updateValue(0.0f);
            lastEditMenuInteractionTime = millis();
        }
    }
    if (isUpButtonPressed)
    {
        Serial.println("[Button] UP clicked");
    }
    if (isSelectButtonPressed)
    {
        Serial.println("[Button] SELECT clicked");
    }
    if (isEnterButtonPressed)
    {
        Serial.println("[Button] ENTER clicked");
    }
    if (isResetButtonPressed)
    {
        Serial.println("[Button] RESET clicked");
    }

    if (isEditMenuActive)
    {
        editMenuControl(isMenuButtonPressed, isEnterButtonPressed, isSelectButtonPressed, isUpButtonPressed, isResetButtonPressed, isExitCombination);
    }
    else
    {
        viewMenuControl(isUpButtonPressed, isResetButtonPressed);
    }
}

void MenuManager::editMenuControl(bool isMenuButtonPressed, bool isEnterButtonPressed, bool isSelectButtonPressed, bool isUpButtonPressed, bool isResetButtonPressed, bool isExitCombination)
{
    if (isExitCombination)
    {
        if (isEditing)
        {
            preferences.putFloat("CustomPass", customPassword->value());
            preferences.putInt("accessMode", accessDecider->currentMode());
            if (save) save();
        }
        editMenus[currentEditMenu]->stopEdit();
        isEditing = false;
        isEditMenuActive = false;
        currentEditMenu = 0;

        password->updateValue(0.0f);
        display->clearButtonStates();
        waitingForExitButtonsRelease = true;
        exitCombinationCooldown = millis() + 400;
        Serial.println("[MenuManager] Edit menu exited via MENU + DIGIT combination.");
        return;
    }

    // Reset button is deliberately ignored in edit menu to prevent accidental roll count clearing
    if (isResetButtonPressed)
    {
        Serial.println("[MenuManager] Reset pressed in edit menu (ignored for safety).");
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

            if (currentEditMenu == 0 && accessDecider->currentMode() == 1)
            {
                float entered = password->value();
                float custom = customPassword->value();
                if (entered == DEFAULT_PASSWORD)
                {
                    Serial.println("[Auth] Logged in with default password.");
                    nextEditMenuItem(false);
                }
                else if (custom != 0.0f && entered == custom)
                {
                    Serial.println("[Auth] Logged in with custom password.");
                    nextEditMenuItem(false);
                }
                else
                {
                    Serial.println("[Auth] Invalid password entered!");
                    password->updateValue(0.0f);
                }
            }
            else
            {
                bool isCustomPassMenu = (editMenus[currentEditMenu] == customPasswordMenu);
                nextEditMenuItem(isCustomPassMenu);
            }

            preferences.putFloat("CustomPass", customPassword->value());
            preferences.putInt("accessMode", accessDecider->currentMode());
            if (save) save();
        }
        else
        {
            isEditing = true;
            MenuItem* cur = editMenus[currentEditMenu]->getCurrentMenuItem();
            if (cur && cur->getDatatype() == MenuType::ModeType)
            {
                cur->edit(false, false);
            }
            else
            {
                editMenus[currentEditMenu]->edit(false, true);
            }
        }
    }
    else if (isEditing)
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
        MenuItem* currentMenuItem = viewMenus[currentViewMenu]->getCurrentMenuItem();
        if (currentMenuItem != nullptr)
        {
            currentMenuItem->triggerReset();
        }
        if (reset != nullptr)
        {
            reset(currentMenuItem);
        }
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
            currentEditMenu = currentEditMenu + 1;
            if (currentEditMenu >= editMenus.size())
            {
                // In Full Access mode, loop continuously through settings (skipping password menu)
                currentEditMenu = (editMenus.size() > 1) ? 1 : 0;
            }
        }
    }
    else if (isCustomPasswordChange || (accessDecider->currentMode() == 1 && ((password->value() == DEFAULT_PASSWORD) || (customPassword->value() != 0.0f && password->value() == customPassword->value()))))
    {
        bool reachedEnd = editMenus[currentEditMenu]->nextMenuItem();
        if(reachedEnd)
        {
            currentEditMenu = currentEditMenu + 1;
            if (currentEditMenu >= editMenus.size())
            {
                // In Password Access mode, do not loop: exit edit menu and relock
                currentEditMenu = 0;
                isEditMenuActive = false;
                password->updateValue(0.0f);
            }
        }
    }
}

void MenuManager::nextViewMenu()
{
    bool reachedEnd = viewMenus[currentViewMenu]->nextMenuItem();
    if (reachedEnd)
    {
        currentViewMenu = (currentViewMenu + 1) % viewMenus.size();
    }
}