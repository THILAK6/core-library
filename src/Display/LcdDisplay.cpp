#include "Display/LcdDisplay.h"
#include "Menu/MenuItem.h"
#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#define printIIC(args) Wire.write(args)
inline size_t LcdDisplay::write(uint8_t value)
{
    send(value, Rs);
    return 1;
}

#else
#include "WProgram.h"

#define printIIC(args) Wire.send(args)
inline void LcdDisplay::write(uint8_t value)
{
    send(value, Rs);
}

#endif

LcdDisplay::LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows) : address(address), columns(columns), rows(rows)
{
    backlight = LCD_NOBACKLIGHT;
}

LcdDisplay::~LcdDisplay() {}

void LcdDisplay::showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable) {
    print("");
}

bool LcdDisplay::isButtonPressed(int button)
{
    return false;
}

void LcdDisplay::init()
{
    initPriv();
}

void LcdDisplay::init(uint8_t serialData, uint8_t serialClock)
{
    initPriv(serialData, serialClock);
}

void LcdDisplay::initPriv()
{
    Wire.begin();
    displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(columns, rows);
}

void LcdDisplay::initPriv(uint8_t serialData, uint8_t serialClock)
{
    Wire.begin(serialData, serialClock);
    displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(columns, rows);
}

void LcdDisplay::begin(uint8_t columns, uint8_t lines, uint8_t dotSize)
{
    if (lines > 1)
    {
        displayFunction |= LCD_2LINE;
    }
    numLines = lines;

    if ((dotSize != 0) && (lines == 1))
    {
        displayFunction |= LCD_5x10DOTS;
    }

    delay(50);

    expanderWrite(backlight);
    delay(1000);

    write4bits(0x03 << 4);
    delayMicroseconds(4500);

    write4bits(0x03 << 4);
    delayMicroseconds(4500);

    write4bits(0x03 << 4);
    delayMicroseconds(150);

    write4bits(0x02 << 4);

    command(LCD_FUNCTIONSET | displayFunction);

    displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    clear();

    displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    command(LCD_ENTRYMODESET | displayMode);

    home();
}

void LcdDisplay::clear()
{
    command(LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LcdDisplay::home()
{
    command(LCD_RETURNHOME);
    delayMicroseconds(2000);
}

void LcdDisplay::setCursor(uint8_t column, uint8_t row)
{
    int rowOffsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row > numLines)
    {
        row = numLines - 1;
    }
    command(LCD_SETDDRAMADDR | (column + rowOffsets[row]));
}

void LcdDisplay::display() {
	displayControl |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

inline void LcdDisplay::command(uint8_t value) {
	send(value, 0);
}

void LcdDisplay::send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
	write4bits((highnib)|mode);
	write4bits((lownib)|mode); 
}

void LcdDisplay::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void LcdDisplay::expanderWrite(uint8_t data){
	Wire.beginTransmission(address);
	printIIC((int)(data) | backlight);
	Wire.endTransmission();
}

void LcdDisplay::pulseEnable(uint8_t _data){
	expanderWrite(_data | En);
	delayMicroseconds(1);
	
	expanderWrite(_data & ~En);
	delayMicroseconds(50);
} 