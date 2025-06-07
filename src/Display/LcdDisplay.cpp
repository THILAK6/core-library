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

LcdDisplay::LcdDisplay(uint8_t address, uint8_t columns, uint8_t rows, uint8_t dataPin, uint8_t clockPin, uint8_t latchPin)
    : address(address), columns(columns), rows(rows),
      shiftInput(ReadShiftInput(dataPin, clockPin, latchPin)),
      buttonManager()
{
    backlight = LCD_NOBACKLIGHT;
}

LcdDisplay::~LcdDisplay() {}

void LcdDisplay::setupInputs(uint8_t menuButton, uint8_t enterButton, uint8_t selectButton, uint8_t upButton, uint8_t resetButton)
{
    std::vector<std::pair<std::string, int>> buttonMap = {
        {"menu", menuButton},
        {"enter", enterButton},
        {"select", selectButton},
        {"up", upButton},
        {"reset", resetButton}
    };
    shiftInput.setRegisterCount(4);

    // for (size_t i = 0; i < buttonMap.size(); ++i) {
    //     const std::string& name = buttonMap[i].first;
    //     int bit = buttonMap[i].second;
    //     buttonManager.addButton(name, [this, bit]() {
    //         return this->shiftInput.readBit(bit);
    //     });
    // }
    for (int i = 0; i < 32; ++i) {
        buttonManager.addUnnamedButton([this, i]() { return shiftInput.readBit(i); });
    }
}

void LcdDisplay::showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable)
{
    setCursor(0, 0);
    print("      TK TECH       ");
    int maxItems = 3;
    if (menuItems.size() < maxItems)
    {
        maxItems = menuItems.size();
    }
    for (int i = 0; i < maxItems; i++)
    {
        String name, value;

        if (menuItems[i].getDatatype() == MenuType::ValueType)
        {
            value = String(menuItems[i].value(), 2);
        }
        else if (menuItems[i].getDatatype() == MenuType::ModeType)
        {
            value = menuItems[i].currentModeValue();
        }
        else
        {
            continue;
        }

        name = menuItems[i].getName();

        if (name.length() > 8)
            name = name.substring(0, 8);
        if (value.length() > 11)
            value = value.substring(0, 11);

        while (name.length() < 8)
            name = " " + name;
        while (value.length() < 11)
            value += " ";

        String displayString = name + ":" + value;

        setCursor(0, i + 1);
        print(displayString);
    }
}

bool LcdDisplay::isButtonPressed(int button)
{
    return buttonManager.getButton(button).isPressed();
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

void LcdDisplay::backlights(void)
{
    backlight = LCD_BACKLIGHT;
    expanderWrite(0);
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

void LcdDisplay::display()
{
    displayControl |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | displayControl);
}

inline void LcdDisplay::command(uint8_t value)
{
    send(value, 0);
}

void LcdDisplay::send(uint8_t value, uint8_t mode)
{
    uint8_t highnib = value & 0xf0;
    uint8_t lownib = (value << 4) & 0xf0;
    write4bits((highnib) | mode);
    write4bits((lownib) | mode);
}

void LcdDisplay::write4bits(uint8_t value)
{
    expanderWrite(value);
    pulseEnable(value);
}

void LcdDisplay::expanderWrite(uint8_t data)
{

    uint8_t value = data | backlight;

    Wire.beginTransmission(address);
    printIIC((int)(data) | backlight);
    Wire.endTransmission();
}

void LcdDisplay::pulseEnable(uint8_t _data)
{
    expanderWrite(_data | En);
    delayMicroseconds(1);

    expanderWrite(_data & ~En);
    delayMicroseconds(50);
}