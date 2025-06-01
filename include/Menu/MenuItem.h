#pragma once
#include <cstdint>
#include <vector>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

enum class MenuType
{
    ValueType,
    ModeType
};

class MenuItem
{
public:
    struct ValueData
    {
        float value;
        int8_t editingDigit;
        int8_t decimalPos;
    };

    struct ModeData
    {
        std::vector<String> modes;
        std::vector<String> modesName;
        int8_t currentMode;
        bool isEditing;
    };

    union Data
    {
        ValueData valueData;
        ModeData modeData;

        Data() {}
        ~Data() {}
    };
private:
    String name;
    String shortName;
    String mainName;
    MenuType type;
    void updateValueWithin();
    Data data;
public:
    MenuItem(String mainName, String name, String shortName, float value, int8_t decimalPos);
    MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, int8_t currentMode);
   
    MenuItem(const MenuItem& other);
    MenuItem& operator=(const MenuItem& other);
 
    ~MenuItem();

    void updateValue(float valueToUpdate);

    void updateMode(int8_t modeToUpdate);
    void updateDecimalPos(int8_t decimalPos);
    uint8_t currentMode();
    float value();
    String currentModeValue();
    std::vector<String> modes();
    String getMainName();
    String getName();
    String getShortName();
    MenuType getDatatype();
    void setShortName(String shortName);
    void updateEditingDigit();
    void editValueType(bool upAction, bool moveAction);
    void editModeType(bool upAction, bool moveAction);
    void stopEdit();
    void edit(bool upAction, bool moveAction);
    bool getIsEditing();
    int8_t getEditingDigit();
    ValueData getValueData();
    ModeData getModeData();
};