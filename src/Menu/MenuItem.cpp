#include "Menu/MenuItem.h"

#define MAX_DIGITS 5

MenuItem::MenuItem(const MenuItem& other)
    : name(other.name), shortName(other.shortName), mainName(other.mainName), type(other.type),
      valueGetter(other.valueGetter), modeGetter(other.modeGetter), onReset(other.onReset)
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
    valueGetter = other.valueGetter;
    modeGetter = other.modeGetter;
    onReset = other.onReset;
    
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

MenuItem::MenuItem(String mainName, String name, String shortName, float value, std::function<void()> onReset, int8_t decimalPos)
    : name(name), shortName(shortName), mainName(mainName), onReset(onReset)
{
    type = MenuType::ValueType;
    data.valueData.value = value;
    data.valueData.editingDigit = -1;
    data.valueData.decimalPos = decimalPos;
}

MenuItem::MenuItem(String mainName, String name, String shortName, std::function<float()> valueGetter, int8_t decimalPos)
    : name(name), shortName(shortName), mainName(mainName), valueGetter(valueGetter)
{
    type = MenuType::ValueType;
    data.valueData.value = valueGetter ? valueGetter() : 0.0f;
    data.valueData.editingDigit = -1;
    data.valueData.decimalPos = decimalPos;
}

MenuItem::MenuItem(String mainName, String name, String shortName, std::function<float()> valueGetter, std::function<void()> onReset, int8_t decimalPos)
    : name(name), shortName(shortName), mainName(mainName), valueGetter(valueGetter), onReset(onReset)
{
    type = MenuType::ValueType;
    data.valueData.value = valueGetter ? valueGetter() : 0.0f;
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

MenuItem::MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, std::function<int8_t()> modeGetter)
    : name(name), shortName(shortName), mainName(mainName), modeGetter(modeGetter)
{
    type = MenuType::ModeType;
    new (&data.modeData.modes) std::vector<String>(modes);
    new (&data.modeData.modesName) std::vector<String>(modesName);
    data.modeData.currentMode = modeGetter ? modeGetter() : 0;
    data.modeData.isEditing = false;
}

MenuItem::MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, std::function<int8_t()> modeGetter, std::function<void()> onReset)
    : name(name), shortName(shortName), mainName(mainName), modeGetter(modeGetter), onReset(onReset)
{
    type = MenuType::ModeType;
    new (&data.modeData.modes) std::vector<String>(modes);
    new (&data.modeData.modesName) std::vector<String>(modesName);
    data.modeData.currentMode = modeGetter ? modeGetter() : 0;
    data.modeData.isEditing = false;
}

void MenuItem::triggerReset()
{
    if (onReset)
    {
        onReset();
    }
}

void MenuItem::setOnReset(std::function<void()> onReset)
{
    this->onReset = onReset;
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
    float decimalPos = (decimalPosGetter != nullptr) ? (float)decimalPosGetter() : (float)data.valueData.decimalPos;
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
    if (type != MenuType::ModeType)
    {
        return 0;
    }
    int8_t m = data.modeData.currentMode;
    if (modeGetter && !data.modeData.isEditing)
    {
        m = modeGetter();
    }
    if (m < 0 || (!data.modeData.modes.empty() && m >= (int8_t)data.modeData.modes.size()))
    {
        return 0;
    }
    return (uint8_t)m;
}

String MenuItem::currentModeValue() const
{
    if (type != MenuType::ModeType || data.modeData.modes.empty())
    {
        return "";
    }
    int8_t m = data.modeData.currentMode;
    if (modeGetter && !data.modeData.isEditing)
    {
        m = modeGetter();
    }
    if (m >= 0 && m < (int8_t)data.modeData.modes.size())
    {
        return data.modeData.modes[m];
    }
    return data.modeData.modes[0];
}

float MenuItem::value()
{
    if (type != MenuType::ValueType)
    {
        return 0.0f;
    }
    if (valueGetter && data.valueData.editingDigit == -1)
    {
        float v = valueGetter();
        if (isnan(v) || isinf(v)) return 0.0f;
        return v;
    }
    float v = data.valueData.value;
    if (isnan(v) || isinf(v)) return 0.0f;
    return v;
}

std::vector<String> MenuItem::modes()
{
    if (type != MenuType::ModeType)
    {
        return std::vector<String>();
    }
    return data.modeData.modes;
}

String MenuItem::getMainName()
{
    return mainName;
}

String MenuItem::getName() const
{
    return name;
}
    
String MenuItem::getShortName()
{
    return shortName;
}

MenuType MenuItem::getDatatype() const
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
    data.modeData.isEditing = true;
    if (upAction || moveAction)
    {
        if (!data.modeData.modes.empty())
        {
            data.modeData.currentMode = (data.modeData.currentMode + 1) % data.modeData.modes.size();
        }
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

MenuItem::ValueData MenuItem::getValueData() const
{
    ValueData vd = data.valueData;
    if (valueGetter && vd.editingDigit == -1)
    {
        vd.value = valueGetter();
    }
    if (decimalPosGetter)
    {
        vd.decimalPos = decimalPosGetter();
    }
    return vd;
}

MenuItem::ModeData MenuItem::getModeData() const
{
    ModeData md = data.modeData;
    if (modeGetter && !md.isEditing)
    {
        md.currentMode = modeGetter();
    }
    return md;
}

