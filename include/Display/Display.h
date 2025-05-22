#ifndef DISPLAY_H
#define DISPLAY_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class Display {
  public:
    virtual void setDisplayToString(String string, bool shouldBlink = false, const word dots = 0, const byte pos = 0) = 0;
    virtual void setDisplayPart(float part, int startIndex, int length, int decimalPos = 0,int blinkDigitPos = -1, bool leadingZeros = false) = 0;
    virtual boolean isButtonPressed(int button) = 0;
    virtual ~Display() {}
};

#endif