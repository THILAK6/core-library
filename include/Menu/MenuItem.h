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
    std::function<float()> valueGetter = nullptr;
    std::function<int8_t()> modeGetter = nullptr;
    std::function<int8_t()> decimalPosGetter = nullptr;
    std::function<void()> onReset = nullptr;
public:
    void setDecimalPosGetter(std::function<int8_t()> getter) { this->decimalPosGetter = getter; }
    MenuItem(String mainName, String name, String shortName, float value, int8_t decimalPos);
    MenuItem(String mainName, String name, String shortName, float value, std::function<void()> onReset, int8_t decimalPos = 0);
    MenuItem(String mainName, String name, String shortName, std::function<float()> valueGetter, int8_t decimalPos = 0);
    MenuItem(String mainName, String name, String shortName, std::function<float()> valueGetter, std::function<void()> onReset, int8_t decimalPos = 0);
    MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, int8_t currentMode);
    MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, std::function<int8_t()> modeGetter);
    MenuItem(String mainName, String name, String shortName, std::vector<String> modes, std::vector<String> modesName, std::function<int8_t()> modeGetter, std::function<void()> onReset);
   
    MenuItem(const MenuItem& other);
    MenuItem& operator=(const MenuItem& other);
 
    ~MenuItem();
    void triggerReset();
    void setOnReset(std::function<void()> onReset);
    bool hasReset() const { return onReset != nullptr; }

    void updateValue(float valueToUpdate);

    void updateMode(int8_t modeToUpdate);
    void updateDecimalPos(int8_t decimalPos);
    uint8_t currentMode();
    float value();
    String currentModeValue() const;
    std::vector<String> modes();
    String getMainName();
    String getName() const;
    String getShortName();
    MenuType getDatatype() const;
    void setShortName(String shortName);
    void updateEditingDigit();
    void editValueType(bool upAction, bool moveAction);
    void editModeType(bool upAction, bool moveAction);
    void stopEdit();
    void edit(bool upAction, bool moveAction);
    bool getIsEditing();
    int8_t getEditingDigit();
    ValueData getValueData() const;
    ModeData getModeData() const;
};