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
      buttonManager(),
      blinkState(true)
{
    backlight = LCD_NOBACKLIGHT;
}

LcdDisplay::~LcdDisplay() {}

void LcdDisplay::setupInputs()
{
    shiftInput.setRegisterCount(4);

    // for (size_t i = 0; i < buttonMap.size(); ++i) {
    //     const std::string& name = buttonMap[i].first;
    //     int bit = buttonMap[i].second;
    //     buttonManager.addButton(name, [this, bit]() {
    //         return this->shiftInput.readBit(bit);
    //     });
    // }
    for (int i = 0; i < 32; ++i)
    {
        buttonManager.addUnnamedButton([this, i]()
                                       { return shiftInput.readBit(i); });
    }
}

void LcdDisplay::showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable)
{
    // Display header
    setCursor(0, 0);
    print("      TK TECH       ");

    // Determine how many items to show
    int maxItems = std::min(3, static_cast<int>(menuItems.size()));

    // Handle blinking state
    updateBlinkState();

    // Display each menu item
    for (int i = 0; i < maxItems; i++)
    {
        displayMenuItem(menuItems[i], i, i == currentMenuItem, isEditable);
    }

    // Clear any unused rows
    clearUnusedRows(maxItems);
}

void LcdDisplay::updateBlinkState()
{
    static unsigned long lastBlinkTime = 0;
    const unsigned long blinkInterval = 500;

    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= blinkInterval)
    {
        blinkState = !blinkState;
        lastBlinkTime = currentTime;
    }
}

bool LcdDisplay::isBlinkOn() const
{
    return blinkState;
}

void LcdDisplay::displayMenuItem(const MenuItem &menuItem, int rowIndex, bool isCurrentItem, bool isEditable)
{
    String name = formatMenuItemName(menuItem, isCurrentItem, isEditable);
    String value = formatMenuItemValue(menuItem, isCurrentItem, isEditable);

    // Format and display the menu item
    if (name.length() > 8)
        name = name.substring(0, 8);
    if (value.length() > 11)
        value = value.substring(0, 11);

    while (name.length() < 8)
        name = " " + name;
    while (value.length() < 11)
        value += " ";

    String displayString = name + ":" + value;

    setCursor(0, rowIndex + 1);
    print(displayString);
}

String LcdDisplay::formatMenuItemName(const MenuItem &menuItem, bool isCurrentItem, bool isEditable)
{
    String name = menuItem.getName();
    bool isActivelyEditing = false;
    
    if (menuItem.getDatatype() == MenuType::ValueType) {
        MenuItem::ValueData valueData = menuItem.getValueData();
        isActivelyEditing = (valueData.editingDigit != -1);
    }
    else if (menuItem.getDatatype() == MenuType::ModeType) {
        MenuItem::ModeData modeData = menuItem.getModeData();
        isActivelyEditing = modeData.isEditing;
    }
    
    bool shouldBlinkName = isCurrentItem && isEditable && !isActivelyEditing;
  
    if (shouldBlinkName && !isBlinkOn())
    {
        name = String("");
        for (int j = 0; j < menuItem.getName().length(); j++)
        {
            name += " ";
        }
    }

    return name;
}

String LcdDisplay::formatMenuItemValue(const MenuItem &menuItem, bool isCurrentItem, bool isEditable)
{
    if (menuItem.getDatatype() == MenuType::ValueType)
    {
        MenuItem::ValueData valueData = menuItem.getValueData();
        bool shouldBlinkDigit = false;
        int8_t editingDigit = -1;

        if (isCurrentItem && isEditable)
        {
            editingDigit = valueData.editingDigit;
            shouldBlinkDigit = (editingDigit >= 0);
        }

        return formatValueWithDigitBlinking(valueData.value, valueData.decimalPos, shouldBlinkDigit, editingDigit);
    }
    else if (menuItem.getDatatype() == MenuType::ModeType)
    {
        MenuItem::ModeData modeData = menuItem.getModeData();
        String modeValue = modeData.modes[modeData.currentMode];

        bool shouldBlinkMode = isCurrentItem && isEditable && modeData.isEditing;
        if (shouldBlinkMode && !isBlinkOn())
        {
            modeValue = String("");
            for (int j = 0; j < modeValue.length(); j++)
            {
                modeValue += " ";
            }
        }
        return modeValue;
    }

    return "";
}

String LcdDisplay::formatValueWithDigitBlinking(float value, int8_t decimalPos, bool shouldBlinkDigit, int8_t editingDigit)
{
    String formattedValue;

    // Format value with decimal places if needed
    if (decimalPos > 0)
    {
        formattedValue = formatDecimalValue(value, decimalPos);
    }
    else
    {
        formattedValue = formatIntegerValue(value);
    }

    // Handle digit blinking if needed
    if (shouldBlinkDigit && !isBlinkOn())
    {
        formattedValue = applyDigitBlinking(formattedValue, editingDigit);
    }

    return formattedValue;
}

String LcdDisplay::formatDecimalValue(float val, int8_t decimalPos)
{
    String formattedValue = String(val, decimalPos);

    int decimalPointPos = formattedValue.indexOf('.');
    if (decimalPointPos != -1)
    {
        String intPart = formattedValue.substring(0, decimalPointPos);
        String decPart = formattedValue.substring(decimalPointPos);

        int padLength = 5 - decimalPos;
        while (intPart.length() < padLength)
        {
            intPart = "0" + intPart;
        }

        formattedValue = intPart + decPart;
    }

    return formattedValue;
}

String LcdDisplay::formatIntegerValue(float val)
{
    String formattedValue = String(int(val));
    while (formattedValue.length() < 5)
    {
        formattedValue = "0" + formattedValue;
    }
    return formattedValue;
}

String LcdDisplay::applyDigitBlinking(const String &formattedValue, int8_t editingDigit)
{
    String result = formattedValue;
    int decimalPointPos = result.indexOf('.');
    int effectivePos;

    if (decimalPointPos == -1)
    {
        effectivePos = editingDigit;
    }
    else
    {
        if (editingDigit < decimalPointPos)
        {
            effectivePos = editingDigit;
        }
        else
        {
            effectivePos = editingDigit + 1;
        }
    }

    if (effectivePos >= 0 && effectivePos < result.length() && result[effectivePos] != '.')
    {
        result[effectivePos] = ' ';
    }

    return result;
}

void LcdDisplay::clearUnusedRows(int maxItems)
{
    for (int i = maxItems; i < 3; i++)
    {
        setCursor(0, i + 1);
        print("                    ");
    }
}

bool LcdDisplay::isButtonPressed(int button)
{
    shiftInput.update();
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