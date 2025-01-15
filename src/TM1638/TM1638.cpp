/*
TM1638.cpp - Library implementation for TM1638.

Copyright (C) 2011 Ricardo Batista (rjbatista <at> gmail <dot> com)

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "TM1638/TM1638.h"
#include "string.h"

TM1638::TM1638(byte dataPin, byte clockPin, byte strobePin, boolean activateDisplay, byte intensity)
    : TM16XX(dataPin, clockPin, strobePin, 8, activateDisplay, intensity)
{
  // nothing else to do - calling super is enough
}

void TM1638::setDisplayToHexNumber(unsigned long number, byte dots, boolean leadingZeros,
                                   const byte numberFont[])
{
  for (int i = 0; i < displays; i++)
  {
    if (!leadingZeros && number == 0)
    {
      clearDisplayDigit(displays - i - 1, (dots & (1 << i)) != 0);
    }
    else
    {
      setDisplayDigit(number & 0xF, displays - i - 1, (dots & (1 << i)) != 0, numberFont);
      number >>= 4;
    }
  }
}

void TM1638::setDisplayToDecNumberAt(unsigned long number, byte dots, byte startingPos, boolean leadingZeros,
                                     const byte numberFont[])
{
  if (number > 99999999L)
  {
    setDisplayToError();
  }
  else
  {
    for (int i = 0; i < displays - startingPos; i++)
    {
      if (number != 0)
      {
        setDisplayDigit(number % 10, displays - i - 1, (dots & (1 << i)) != 0, numberFont);
        number /= 10;
      }
      else
      {
        if (leadingZeros)
        {
          setDisplayDigit(0, displays - i - 1, (dots & (1 << i)) != 0, numberFont);
        }
        else
        {
          clearDisplayDigit(displays - i - 1, (dots & (1 << i)) != 0);
        }
      }
    }
  }
}

void TM1638::setDisplayAsFiveAndThreeParts(unsigned long fivePartValue, unsigned long threePartValue, byte dots, const byte numberFont[], bool leadingZerosForThreePart, bool leadingZerosForFivePart, int blinkDigitPos)
{
  if (fivePartValue > 99999 || threePartValue > 999)
  {
    setDisplayToError();
  }
  else
  {
    setDisplayPart(fivePartValue, 4, 5, dots, -1, leadingZerosForFivePart, numberFont);
    setDisplayPart(threePartValue, 7, 3, dots >> 5, -1, leadingZerosForThreePart, numberFont);
  }
}

void TM1638::setDisplayPart(float value, int startIndex, int length, int decimalPos, int blinkDigitPos, bool leadingZeros, const byte numberFont[])
{
  unsigned long currentTime = millis();
  static unsigned long lastBlinkTime = 0;
  static bool blinkState = true;
  const unsigned long blinkInterval = 500;

  if (currentTime - lastBlinkTime >= blinkInterval)
  {
    blinkState = !blinkState;
    lastBlinkTime = currentTime;
  }

  bool isNegative = false;

  if (value < 0)
  {
    sendChar(0, MINUS, 0);
    value = -value;
    isNegative = true;
  }

  float scaledValue = value * pow(10, decimalPos);

  int valueToDisplay = (int)scaledValue + 0.5;

  if (isNegative && valueToDisplay >= 10000)
  {
    valueToDisplay = valueToDisplay % 10000;
    length--;
  }

  if(valueToDisplay < pow(10, decimalPos))
  {
    leadingZeros = true;
  }

  setDisplayNumber(valueToDisplay, startIndex, decimalPos, length, leadingZeros, numberFont, blinkDigitPos, blinkState);
}

void TM1638::setDisplayNumber(int32_t value, int8_t startingPos, int8_t decimalPos, int8_t length, bool leadingZeros, const byte numberFont[], int8_t blinkDigitPos, bool blinkState)
{
  for (int i = 0; i < length; i++)
  {
    int pos = startingPos - i;
    bool isBlinkingDigit = (pos == blinkDigitPos);
    bool isDot = (i == decimalPos);

    if (isBlinkingDigit && !blinkState)
    {
      clearDisplayDigit(pos, 0);
      value /= 10;
      continue;
    }

    if (value != 0)
    {
      setDisplayDigit(value % 10, pos, isDot, numberFont);
      value /= 10;
    }
    else
    {
      if (leadingZeros || i == 0)
      {
        setDisplayDigit(0, pos, isDot, numberFont);
      }
      else
      {
        clearDisplayDigit(pos, 0);
      }
    }
  }
}

void TM1638::setDisplayToDecNumber(unsigned long number, byte dots, boolean leadingZeros,
                                   const byte numberFont[])
{
  setDisplayToDecNumberAt(number, dots, 0, leadingZeros, numberFont);
}

void TM1638::setDisplayToSignedDecNumber(signed long number, byte dots, boolean leadingZeros,
                                         const byte numberFont[])
{
  if (number >= 0)
  {
    setDisplayToDecNumberAt(number, dots, 0, leadingZeros, numberFont);
  }
  else
  {
    if (-number > 9999999L)
    {
      setDisplayToError();
    }
    else
    {
      setDisplayToDecNumberAt(-number, dots, 1, leadingZeros, numberFont);
      sendChar(0, MINUS, (dots & (0x80)) != 0);
    }
  }
}

void TM1638::setDisplayToBinNumber(byte number, byte dots, const byte numberFont[])
{
  for (int i = 0; i < displays; i++)
  {
    setDisplayDigit((number & (1 << i)) == 0 ? 0 : 1, displays - i - 1, (dots & (1 << i)) != 0, numberFont);
  }
}

void TM1638::setLED(byte color, byte pos)
{
  sendData((pos << 1) + 1, color);
}

void TM1638::setLEDs(word leds)
{
  for (int i = 0; i < displays; i++)
  {
    byte color = 0;

    if ((leds & (1 << i)) != 0)
    {
      color |= TM1638_COLOR_RED;
    }

    if ((leds & (1 << (i + 8))) != 0)
    {
      color |= TM1638_COLOR_GREEN;
    }

    setLED(color, i);
  }
}

byte TM1638::getButtons(void)
{
  byte keys = 0;

  digitalWrite(strobePin, LOW);
  send(0x42);
  for (int i = 0; i < 4; i++)
  {
    keys |= receive() << i;
  }
  digitalWrite(strobePin, HIGH);

  return keys;
}

bool TM1638::isButtonPressed(byte buttonValue)
{
  int buttons = getButtons();
  bool currentButtonState = (buttons == buttonValue);
  bool wasPressed = false;

  if (currentButtonState && !previousButtonStates[buttonValue])
  {
    wasPressed = true;
  }

  previousButtonStates[buttonValue] = currentButtonState; // Update the previous button state
  return wasPressed;
}

void TM1638::sendChar(byte pos, byte data, boolean dot)
{
  sendData(pos << 1, data | (dot ? 0b10000000 : 0));
}
