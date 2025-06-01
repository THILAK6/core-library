#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include <cstdint>

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class MenuItem;

enum DisplayMode {
    VALUE_AND_SHORT_NAME,
    VALUE,
    NAME,
};

class Display {
  public:
    Display() {}
    virtual bool isButtonPressed(int button) = 0;
    virtual void showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) = 0;
    virtual ~Display() {}
  protected:
    virtual void showValueAndShortName(MenuItem &menuItem, bool isEditable) = 0;
    virtual void showValue(bool isEditable, MenuItem &menuItem, MenuItem &menuItemAdditional) = 0;
    virtual void showName(MenuItem &menuItem) = 0;
  };

#endif