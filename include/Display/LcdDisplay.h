#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H
#endif

#include "Display.h"

class LcdDisplay : public Display {
private:
public:

    LcdDisplay();

    void setDisplayToString(String string, bool shouldBlink = false, const word dots = 0, const byte pos = 0) override;

    void setDisplayPart(float part, int startIndex, int length, int decimalPos = 0,int blinkDigitPos = -1, bool leadingZeros = false) override;

    boolean isButtonPressed(int button) override;
};