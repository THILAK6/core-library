#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "Display.h"
#include "ShiftIO/ReadShiftInput.h"
#include "Button/ButtonManager.h"
#include <vector>
#include <inttypes.h>
#include "Print.h" 
#include <Wire.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100	// Enable bit
#define Rw B00000010	// Read/Write bit
#define Rs B00000001	// Register select bit

class MenuItem;

class LcdDisplay : public Display, public Print {
public:
    LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);
    ~LcdDisplay() override;
    
    // Display interface implementations
    bool isButtonPressed(int button) override;
    void showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) override;
    
    // LCD specific methods
    void init();
    void init(uint8_t serialData, uint8_t serialClock);
    void begin(uint8_t columns, uint8_t lines, uint8_t dotSize = 0);
    void clear();
    void home();
    void setCursor(uint8_t column, uint8_t row);
    void display();
    void backlights(void);
    void setupInputs(uint8_t menuButton, uint8_t enterButton, uint8_t selectButton, uint8_t upButton, uint8_t resetButton);

    virtual size_t write(uint8_t value);

private:
    uint8_t address;
    uint8_t columns;
    uint8_t rows;
    uint8_t backlight;
    uint8_t displayFunction;
    uint8_t displayControl;
    uint8_t displayMode;
    uint8_t numLines;
    ReadShiftInput shiftInput;
    ButtonManager buttonManager;
    
    void initPriv();
    void initPriv(uint8_t serialData, uint8_t serialClock);
    void command(uint8_t value);
    void send(uint8_t value, uint8_t mode);
    void write4bits(uint8_t value);
    void expanderWrite(uint8_t data);
    void pulseEnable(uint8_t data);
};
#endif
