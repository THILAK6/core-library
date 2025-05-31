#include "Menu/Menu.h"

#define MAX_DIGITS 5

Menu::Menu(String mainName, String name, String shortName, float value, Display *display, int8_t decimalPos, bool isEditable, float additionalValue, uint8_t displayType)
    : name(name), shortName(shortName), display(display), displayType(displayType), isEditable(isEditable), mainName(mainName)
{
    type = MenuType::ValueType;
    data.valueData.value = value;
    data.valueData.additionalValue = additionalValue;
    data.valueData.editingDigit = -1;
    data.valueData.decimalPos = decimalPos;
}

Menu::Menu(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, int8_t currentMode, Display *display, bool isEditable, uint8_t displayType)
    : name(name), shortName(shortName), display(display), displayType(displayType), isEditable(isEditable), mainName(mainName)
{
    type = MenuType::ModeType;
    new (&data.modeData.modes) std::vector<String>(modes);
    new (&data.modeData.modesName) std::vector<String>(modesName);
    data.modeData.currentMode = currentMode;
    data.modeData.isEditing = false;
}

Menu::~Menu()
{
    data.modeData.modes.~vector<String>();
}

void Menu::updateValueWithin()
{
    int8_t editingDigit = data.valueData.editingDigit;
    float value = data.valueData.value;
    float decimalPos = data.valueData.decimalPos;
    if (!isEditable && editingDigit == -1)
    {
        return;
    }

    if (editingDigit < 0 || editingDigit > 4)
    {
        return;
    }
    int val = round(value * pow(10, decimalPos));

    int increment = pow(10, 4 - editingDigit);
    int currentDigit = (val / increment) % 10;
    int newDigit = (currentDigit + 1) % 10;

    int newValue = val - (currentDigit * increment) + (newDigit * increment);

    float valueToSet = newValue / pow(10, decimalPos);
    this->data.valueData.value = valueToSet;
}

void Menu::updateValue(float valueToUpdate)
{
    this->data.valueData.value = valueToUpdate;
}

void Menu::updateMode(int8_t modeToUpdate)
{
    this->data.modeData.currentMode = modeToUpdate;
}

void Menu::updateAdditionalValue(float valueToUpdate)
{
    this->data.valueData.additionalValue = valueToUpdate;
}

void Menu::updateDecimalPos(int8_t decimalPos)
{
    this->data.valueData.decimalPos = decimalPos;
}

void Menu::displayName()
{
    if (data.modeData.isEditing)
    {
        display->setDisplayToString(data.modeData.modes[data.modeData.currentMode], true);
    }
    else
    {
        display->setDisplayToString(name, false);
    }
    display->setDisplayToString(shortName, false, 0, 5);
}

void Menu::displayValueAndShortName()
{
    if (type == MenuType::ModeType)
    {
        display->setDisplayToString(data.modeData.modes[data.modeData.currentMode], data.modeData.isEditing);
    }
    else
    {
        int8_t editingDigit = data.valueData.editingDigit;
        bool leadingZeros = isEditable && editingDigit != -1;
        display->setDisplayPart(data.valueData.value, 4, 5, data.valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    }
    display->setDisplayToString(shortName, false, 0, 5);
}

void Menu::displayValue()
{
    int8_t editingDigit = data.valueData.editingDigit;
    bool leadingZeros = isEditable && editingDigit != -1;
    display->setDisplayPart(data.valueData.value, 4, 5, data.valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    display->setDisplayPart(data.valueData.additionalValue, 7, 3, data.valueData.decimalPos);
}

void Menu::displayItem()
{
    switch (displayType)
    {
    case 0:
        displayValueAndShortName();
        break;
    case 1:
        displayValue();
        break;
    case 2:
        displayName();
        break;
    default:
        displayValueAndShortName();
        break;
    }
}

void Menu::updateEditingDigit()
{
    if (isEditable)
    {
        data.valueData.editingDigit++;
    }
    if (data.valueData.editingDigit == MAX_DIGITS)
    {
        data.valueData.editingDigit = 0;
    }
}

void Menu::edit(bool upAction, bool moveAction)
{
    if (isEditable)
    {
        if (type == MenuType::ValueType)
        {
            editValueType(upAction, moveAction);
        }
        else if (type == MenuType::ModeType)
        {
            editModeType(upAction, moveAction);
        }
    }
}

void Menu::stopEdit()
{
    if (type == MenuType::ValueType)
    {
        data.valueData.editingDigit = -1;
    }
    else if (type == MenuType::ModeType)
    {
        data.modeData.isEditing = false;
    }
}

void Menu::editValueType(bool upAction, bool moveAction)
{
    if (upAction)
    {
        updateValueWithin();
    }
    if (moveAction)
    {
        updateEditingDigit();
    }
}

void Menu::editModeType(bool upAction, bool moveAction)
{
    if (moveAction)
    {
        data.modeData.isEditing = true;
    }
    if (upAction)
    {
        data.modeData.currentMode = (data.modeData.currentMode + 1) % data.modeData.modes.size();
    }
}

uint8_t Menu::currentMode()
{
    return data.modeData.currentMode;
}

String Menu::currentModeValue()
{
    return data.modeData.modes[data.modeData.currentMode];
}

float Menu::value()
{
    return data.valueData.value;
}

float Menu::additionalValue()
{
    return data.valueData.additionalValue;
}

std::vector<String> Menu::modes()
{
    return data.modeData.modes;
}

String Menu::getMainName()
{
    return mainName;
}

MenuType Menu::getDatatype()
{
    return type;
}

void Menu::setDisplayType(uint8_t displayType)
{
    this->displayType = displayType;
}

void Menu::setShortName(String shortName)
{
    this->shortName = shortName;
}