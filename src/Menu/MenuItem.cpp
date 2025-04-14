#include "Menu/MenuItem.h"

#define MAX_DIGITS 5

MenuItem::MenuItem(String mainName, String name, String shortName, float value, TM1638 *module, int8_t decimalPos, bool isEditable, float additionalValue, uint8_t displayType)
    : name(name), shortName(shortName), module(module), displayType(displayType), isEditable(isEditable), mainName(mainName)
{
    type = MenuItemType::ValueType;
    data.valueData.value = value;
    data.valueData.additionalValue = additionalValue;
    data.valueData.editingDigit = -1;
    data.valueData.decimalPos = decimalPos;
}

MenuItem::MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, int8_t currentMode, TM1638 *module, bool isEditable, uint8_t displayType)
    : name(name), shortName(shortName), module(module), displayType(displayType), isEditable(isEditable), mainName(mainName)
{
    type = MenuItemType::ModeType;
    new (&data.modeData.modes) std::vector<String>(modes);
    new (&data.modeData.modesName) std::vector<String>(modesName);
    data.modeData.currentMode = currentMode;
    data.modeData.isEditing = false;
}

MenuItem::~MenuItem()
{
    data.modeData.modes.~vector<String>();
}

void MenuItem::updateValueWithin()
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

    Serial.println("value: " + String(value));
    Serial.println("editingDigit: " + String(editingDigit));
    Serial.println("increment: " + String(increment));
    Serial.println("val: " + String(val));
    Serial.println("currentDigit: " + String(currentDigit));
    Serial.println("newDigit: " + String(newDigit));
    int newValue = val - (currentDigit * increment) + (newDigit * increment);

    Serial.println("newValue: " + String(newValue));
    float valueToSet = newValue / pow(10, decimalPos);
    Serial.println("valueToSet: " + String(valueToSet));
    Serial.println();
    this->data.valueData.value = valueToSet;
}

void MenuItem::updateValue(float valueToUpdate)
{
    this->data.valueData.value = valueToUpdate;
}

void MenuItem::updateMode(int8_t modeToUpdate)
{
    this->data.modeData.currentMode = modeToUpdate;
}

void MenuItem::updateAdditionalValue(float valueToUpdate)
{
    this->data.valueData.additionalValue = valueToUpdate;
}

void MenuItem::updateDecimalPos(int8_t decimalPos)
{
    this->data.valueData.decimalPos = decimalPos;
}

void MenuItem::displayName()
{
    if (data.modeData.isEditing)
    {
        module->setDisplayToString(data.modeData.modes[data.modeData.currentMode], true);
    }
    else
    {
        module->setDisplayToString(name, false);
    }
    module->setDisplayToString(shortName, false, 0, 5);
}

void MenuItem::displayValueAndShortName()
{
    if (type == MenuItemType::ModeType)
    {
        module->setDisplayToString(data.modeData.modes[data.modeData.currentMode], data.modeData.isEditing);
    }
    else
    {
        int8_t editingDigit = data.valueData.editingDigit;
        bool leadingZeros = isEditable && editingDigit != -1;
        module->setDisplayPart(data.valueData.value, 4, 5, data.valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    }
    module->setDisplayToString(shortName, false, 0, 5);
}

void MenuItem::displayValue()
{
    int8_t editingDigit = data.valueData.editingDigit;
    bool leadingZeros = isEditable && editingDigit != -1;
    module->setDisplayPart(data.valueData.value, 4, 5, data.valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    module->setDisplayPart(data.valueData.additionalValue, 7, 3, data.valueData.decimalPos);
}

void MenuItem::display()
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

void MenuItem::updateEditingDigit()
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

void MenuItem::edit(bool upAction, bool moveAction)
{
    if (isEditable)
    {
        if (type == MenuItemType::ValueType)
        {
            editValueType(upAction, moveAction);
        }
        else if (type == MenuItemType::ModeType)
        {
            editModeType(upAction, moveAction);
        }
    }
}

void MenuItem::stopEdit()
{
    if (type == MenuItemType::ValueType)
    {
        data.valueData.editingDigit = -1;
    }
    else if (type == MenuItemType::ModeType)
    {
        data.modeData.isEditing = false;
    }
}

void MenuItem::editValueType(bool upAction, bool moveAction)
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

void MenuItem::editModeType(bool upAction, bool moveAction)
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

uint8_t MenuItem::currentMode()
{
    return data.modeData.currentMode;
}

String MenuItem::currentModeValue()
{
    return data.modeData.modes[data.modeData.currentMode];
}

float MenuItem::value()
{
    return data.valueData.value;
}

float MenuItem::additionalValue()
{
    return data.valueData.additionalValue;
}

std::vector<String> MenuItem::modes()
{
    return data.modeData.modes;
}

String MenuItem::getMainName()
{
    return mainName;
}

MenuItemType MenuItem::getDatatype()
{
    return type;
}

void MenuItem::setDisplayType(uint8_t displayType)
{
    this->displayType = displayType;
}

void MenuItem::setShortName(String shortName)
{
    this->shortName = shortName;
}