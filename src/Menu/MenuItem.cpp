#include "Menu/MenuItem.h"

#define MAX_DIGITS 5

MenuItem::MenuItem(const MenuItem& other)
    : name(other.name), shortName(other.shortName), mainName(other.mainName), type(other.type)
{
    if (other.type == MenuType::ValueType) {
        data.valueData.value = other.data.valueData.value;
        data.valueData.editingDigit = other.data.valueData.editingDigit;
        data.valueData.decimalPos = other.data.valueData.decimalPos;
    } else {
        new (&data.modeData.modes) std::vector<String>(other.data.modeData.modes);
        new (&data.modeData.modesName) std::vector<String>(other.data.modeData.modesName);
        data.modeData.currentMode = other.data.modeData.currentMode;
        data.modeData.isEditing = other.data.modeData.isEditing;
    }
}

MenuItem& MenuItem::operator=(const MenuItem& other)
{
    if (this == &other) {
        return *this;
    }
    
    if (type == MenuType::ModeType) {
        data.modeData.modes.~vector<String>();
        data.modeData.modesName.~vector<String>();
    }
    
    name = other.name;
    shortName = other.shortName;
    mainName = other.mainName;
    type = other.type;
    
    if (other.type == MenuType::ValueType) {
        data.valueData.value = other.data.valueData.value;
        data.valueData.editingDigit = other.data.valueData.editingDigit;
        data.valueData.decimalPos = other.data.valueData.decimalPos;
    } else {
        new (&data.modeData.modes) std::vector<String>(other.data.modeData.modes);
        new (&data.modeData.modesName) std::vector<String>(other.data.modeData.modesName);
        data.modeData.currentMode = other.data.modeData.currentMode;
        data.modeData.isEditing = other.data.modeData.isEditing;
    }
    
    return *this;
}

MenuItem::MenuItem(String mainName, String name, String shortName, float value, int8_t decimalPos)
    : name(name), shortName(shortName), mainName(mainName)
{
    type = MenuType::ValueType;
    data.valueData.value = value;
    data.valueData.editingDigit = -1;
    data.valueData.decimalPos = decimalPos;
}

MenuItem::MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, int8_t currentMode)
    : name(name), shortName(shortName), mainName(mainName)
{
    type = MenuType::ModeType;
    new (&data.modeData.modes) std::vector<String>(modes);
    new (&data.modeData.modesName) std::vector<String>(modesName);
    data.modeData.currentMode = currentMode;
    data.modeData.isEditing = false;
}

MenuItem::~MenuItem()
{
    if(type == MenuType::ModeType)
    {
        data.modeData.modes.~vector<String>();
        data.modeData.modesName.~vector<String>();
    }
}

void MenuItem::updateValueWithin()
{
    int8_t editingDigit = data.valueData.editingDigit;
    float value = data.valueData.value;
    float decimalPos = data.valueData.decimalPos;
    if (editingDigit == -1)
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

void MenuItem::updateValue(float valueToUpdate)
{
    this->data.valueData.value = valueToUpdate;
}

void MenuItem::updateMode(int8_t modeToUpdate)
{
    this->data.modeData.currentMode = modeToUpdate;
}

void MenuItem::updateDecimalPos(int8_t decimalPos)
{
    this->data.valueData.decimalPos = decimalPos;
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

std::vector<String> MenuItem::modes()
{
    return data.modeData.modes;
}

String MenuItem::getMainName()
{
    return mainName;
}

String MenuItem::getName()
{
    return name;
}
    
String MenuItem::getShortName()
{
    return shortName;
}

MenuType MenuItem::getDatatype()
{
    return type;
}


void MenuItem::setShortName(String shortName)
{
    this->shortName = shortName;
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

void MenuItem::updateEditingDigit()
{
    data.valueData.editingDigit++;
    if (data.valueData.editingDigit == MAX_DIGITS)
    {
        data.valueData.editingDigit = 0;
    }   
}

void MenuItem::stopEdit()
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

void MenuItem::edit(bool upAction, bool moveAction)
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

bool MenuItem::getIsEditing()
{
    return data.modeData.isEditing;
}

int8_t MenuItem::getEditingDigit()
{
    return data.valueData.editingDigit;
}

MenuItem::ValueData MenuItem::getValueData()
{
    return data.valueData;
}

MenuItem::ModeData MenuItem::getModeData()
{
    return data.modeData;
}

