#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "Display.h"
#include <vector>

class MenuItem;
class LcdDisplay : public Display {
public:
    LcdDisplay() ;
    virtual bool isButtonPressed(int button) override;
    virtual void showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) override;
    ~LcdDisplay() override;
  private:
    virtual void showValueAndShortName(MenuItem &menuItem, bool isEditable) override;
    virtual void showValue(bool isEditable, MenuItem &menuItem, MenuItem &menuItemAdditional) override;
    virtual void showName(MenuItem &menuItem) override;
};
#endif
