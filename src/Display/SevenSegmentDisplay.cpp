#include "Display/SevenSegmentDisplay.h"

SevenSegmentDisplay::SevenSegmentDisplay(TM1638 *module) : module(module) {}

void SevenSegmentDisplay::setDisplayToString(String string, bool shouldBlink, const word dots, const byte pos)
{
}

void SevenSegmentDisplay::setDisplayPart(float part, int startIndex, int length, int decimalPos, int blinkDigitPos, bool leadingZeros)
{
}

boolean SevenSegmentDisplay::isButtonPressed(int button)
{
    return false;
}
