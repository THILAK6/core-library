#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "Display.h"
#include <map>
#include <cstdint>
#include <vector>

class SevenSegmentDisplay : public Display
{

private:
    byte dataPin;
    byte clockPin;
    byte strobePin;
    byte intensity;

    void showValueAndShortName(MenuItem &menuItem, bool isEditable);
    void showValue(bool isEditable, MenuItem &menuItem, MenuItem &menuItemAdditional);
    void showName(MenuItem &menuItem);

    void setDisplayPart(float value, int startIndex, int length, int decimalPos, int blinkDigitPos, bool leadingZeros);
    void setDisplayNumber(int32_t value, int8_t startingPos, int8_t decimalPos, int8_t length, bool leadingZeros, int8_t blinkDigitPos, bool blinkState);
    void setDisplayToString(const String string, const bool shouldBlink, const byte pos);
    void setDisplayToString(const String string, const byte pos);
    void setDisplayDigit(byte digit, byte pos, boolean dot);
    void clearDisplayDigit(byte pos, boolean dot);
    void sendChar(byte pos, byte data, boolean dot);
    void sendCommand(byte cmd);
    void sendData(byte address, byte data);
    void send(byte data);
    byte receive();
    byte getButtons();
    std::map<int, bool> previousButtonStates;
public:
    SevenSegmentDisplay(byte dataPin, byte clockPin, byte strobePin, byte intensity = 7);
    virtual bool isButtonPressed(int button) override;
    virtual void showMenuItems(std::vector<MenuItem*> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) override;
    ~SevenSegmentDisplay() override;
};
#endif