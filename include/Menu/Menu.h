#pragma once
#include <cstdint>
#include <vector>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "Display/Display.h"

enum DisplayType {
    VALUE_AND_SHORT_NAME,
    VALUE,
    NAME,
};

enum class MenuType
{
    ValueType,
    ModeType
};

class Menu
{
private:
    String mainName;
    String name;
    String shortName;
    Display *display;
    uint8_t displayType;
    bool isEditable;
    MenuType type;
    void updateValueWithin();
    void editValueType(bool upAction, bool moveAction);
    void editModeType(bool upAction, bool moveAction);
    union Data
    {
        struct
        {
            float value;
            float additionalValue;
            int8_t editingDigit = -1;
            int8_t decimalPos = 0;
        } valueData;
        struct
        {
            std::vector<String> modes;
            std::vector<String> modesName;
            uint8_t currentMode = 0;
            bool isEditing = false;
        } modeData;

        Data() {}
        ~Data() {}
    } data;

public:
    Menu(String mainName, String name, String shortName, float value, Display *display, int8_t decimalPos = 0, bool isEditable = false, float additionalValue = 0, uint8_t displayType = 0);

    Menu(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName,int8_t currentMode, Display *display, bool isEditable = false, uint8_t displayType = 0);

    ~Menu();

    void updateValue(float valueToUpdate);
    void updateMode(int8_t modeToUpdate);
    void updateAdditionalValue(float valueToUpdate);
    void updateDecimalPos(int8_t decimalPos);

    void displayItem();

    void displayName();

    void displayValue();

    void displayValueAndShortName();

    void updateEditingDigit();
    void stopEdit();
    void edit(bool upAction, bool moveAction);
    uint8_t currentMode();
    float value();
    float additionalValue();
    std::vector<String> modes();
    String currentModeValue();
    String getMainName();
    MenuType getDatatype();
    void setDisplayType(uint8_t displayType);
    void setShortName(String shortName);
};