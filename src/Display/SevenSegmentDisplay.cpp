#include "Display/SevenSegmentDisplay.h"
#include "Menu/MenuItem.h"

int displays = 8; // Number of displays in the TM1638

const byte MINUS = 0b01000000;

// definition for standard hexadecimal numbers
const byte NUMBER_FONT[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A
  0b01111100, // B
  0b00111001, // C
  0b01011110, // D
  0b01111001, // E
  0b01110001  // F
};

// definition for the displayable ASCII chars
const byte FONT_DEFAULT[] = {
    0b00000000, // (32)  <space>
    0b10000110, // (33)	!
    0b00100010, // (34)	"
    0b01111110, // (35)	#
    0b01101101, // (36)	$
    0b00000000, // (37)	%
    0b00000000, // (38)	&
    0b00000010, // (39)	'
    0b00110000, // (40)	(
    0b00000110, // (41)	)
    0b01100011, // (42)	*
    0b00000000, // (43)	+
    0b00000100, // (44)	,
    0b01000000, // (45)	-
    0b10000000, // (46)	.
    0b01010010, // (47)	/
    0b00111111, // (48)	0
    0b00000110, // (49)	1
    0b01011011, // (50)	2
    0b01001111, // (51)	3
    0b01100110, // (52)	4
    0b01101101, // (53)	5
    0b01111101, // (54)	6
    0b00100111, // (55)	7
    0b01111111, // (56)	8
    0b01101111, // (57)	9
    0b00000000, // (58)	:
    0b00000000, // (59)	;
    0b00000000, // (60)	<
    0b01001000, // (61)	=
    0b00000000, // (62)	>
    0b01010011, // (63)	?
    0b01011111, // (64)	@
    0b01110111, // (65)	A
    0b01111111, // (66)	B
    0b00111001, // (67)	C
    0b00111111, // (68)	D
    0b01111001, // (69)	E
    0b01110001, // (70)	F
    0b00111101, // (71)	G
    0b01110110, // (72)	H
    0b00000110, // (73)	I
    0b00011111, // (74)	J
    0b01101001, // (75)	K
    0b00111000, // (76)	L
    0b00010101, // (77)	M
    0b00110111, // (78)	N
    0b00111111, // (79)	O
    0b01110011, // (80)	P
    0b01100111, // (81)	Q
    0b00110001, // (82)	R
    0b01101101, // (83)	S
    0b01111000, // (84)	T
    0b00111110, // (85)	U
    0b00101010, // (86)	V
    0b00011101, // (87)	W
    0b01110110, // (88)	X
    0b01101110, // (89)	Y
    0b01011011, // (90)	Z
    0b00111001, // (91)	[
    0b01100100, // (92)	\ (this can't be the last char on a line, even in comment or it'll concat)
    0b00001111, // (93)	]
    0b00000000, // (94)	^
    0b00001000, // (95)	_
    0b00100000, // (96)	`
    0b01011111, // (97)	a
    0b01111100, // (98)	b
    0b01011000, // (99)	c
    0b01011110, // (100)	d
    0b01111011, // (101)	e
    0b00110001, // (102)	f
    0b01101111, // (103)	g
    0b01110100, // (104)	h
    0b00000100, // (105)	i
    0b00001110, // (106)	j
    0b01110101, // (107)	k
    0b00110000, // (108)	l
    0b01010101, // (109)	m
    0b01010100, // (110)	n
    0b01011100, // (111)	o
    0b01110011, // (112)	p
    0b01100111, // (113)	q
    0b01010000, // (114)	r
    0b01101101, // (115)	s
    0b01111000, // (116)	t
    0b00011100, // (117)	u
    0b00101010, // (118)	v
    0b00011101, // (119)	w
    0b01110110, // (120)	x
    0b01101110, // (121)	y
    0b01000111, // (122)	z
    0b01000110, // (123)	{
    0b00000110, // (124)	|
    0b01110000, // (125)	}
    0b00000001, // (126)	~
};

SevenSegmentDisplay::SevenSegmentDisplay(byte dataPin, byte clockPin, byte strobePin, byte intensity) : dataPin(dataPin), clockPin(clockPin), strobePin(strobePin), intensity(intensity)
{
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(strobePin, OUTPUT);

    digitalWrite(strobePin, HIGH);
    digitalWrite(clockPin, HIGH);

    sendCommand(0x40);
    sendCommand(0x80 | min(7, (int)intensity));

    digitalWrite(strobePin, LOW);
#ifdef ARDUINO_ARCH_ESP32
    delayMicroseconds(2);
#endif
    send(0xC0);
    for (int i = 0; i < 16; i++)
    {
        send(0x00);
    }
    digitalWrite(strobePin, HIGH);
}

SevenSegmentDisplay::~SevenSegmentDisplay()
{
    
}

bool SevenSegmentDisplay::isButtonPressed(int buttonValue)
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

byte SevenSegmentDisplay::getButtons(void)
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

void SevenSegmentDisplay::showMenuItems(std::vector<MenuItem> &menuItems, int8_t currentMenuItem, DisplayMode displayMode, bool isEditable)
{
    switch (displayMode)
    {
    case DisplayMode::VALUE_AND_SHORT_NAME:
        showValueAndShortName(menuItems[0], isEditable);
        break;
    case DisplayMode::VALUE:
        if (menuItems.size() > 1)
        {
            showValue(isEditable, menuItems[0], menuItems[1]);
        }
        else
        {
            showValue(isEditable, menuItems[0], menuItems[0]);
        }
        break;
    case DisplayMode::NAME:
        showName(menuItems[0]);
        break;
    }
}

void SevenSegmentDisplay::showValueAndShortName(MenuItem &menuItem, bool isEditable)
{
    if (menuItem.getDatatype() == MenuType::ModeType)
    {
        MenuItem::ModeData modeData = menuItem.getModeData();
        setDisplayToString(modeData.modes[modeData.currentMode], modeData.isEditing, 0);
    }
    else
    {
        MenuItem::ValueData valueData = menuItem.getValueData();
        int8_t editingDigit = valueData.editingDigit;
        bool leadingZeros = isEditable && editingDigit != -1;

        setDisplayPart(valueData.value, 4, 5, valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    }
    setDisplayToString(menuItem.getShortName(), false, 5);
}

void SevenSegmentDisplay::showValue(bool isEditable, MenuItem &menuItem, MenuItem &menuItemAdditional)
{
    MenuItem::ValueData valueData = menuItem.getValueData();
    MenuItem::ValueData valueDataAdditional = menuItemAdditional.getValueData();
    int8_t editingDigit = valueData.editingDigit;
    bool leadingZeros = isEditable && editingDigit != -1;
    setDisplayPart(valueData.value, 4, 5, valueData.decimalPos, isEditable ? editingDigit : -1, leadingZeros);
    setDisplayPart(valueDataAdditional.value, 7, 3, valueDataAdditional.decimalPos, -1, false);
}

void SevenSegmentDisplay::showName(MenuItem &menuItem)
{
    MenuItem::ModeData modeData = menuItem.getModeData();
    if (modeData.isEditing)
    {
        setDisplayToString(modeData.modes[modeData.currentMode], true);
    }
    else
    {
        setDisplayToString(menuItem.getName(), false);
    }
    setDisplayToString(menuItem.getShortName(), false, 5);
}

void SevenSegmentDisplay::setDisplayPart(float value, int startIndex, int length, int decimalPos, int blinkDigitPos, bool leadingZeros)
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

  if (valueToDisplay < pow(10, decimalPos))
  {
    leadingZeros = true;
  }

  setDisplayNumber(valueToDisplay, startIndex, decimalPos, length, leadingZeros, blinkDigitPos, blinkState);
}

void SevenSegmentDisplay::setDisplayNumber(int32_t value, int8_t startingPos, int8_t decimalPos, int8_t length, bool leadingZeros, int8_t blinkDigitPos, bool blinkState)
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
      setDisplayDigit(value % 10, pos, isDot);
      value /= 10;
    }
    else
    {
      if (leadingZeros || i == 0)
      { 
        setDisplayDigit(0, pos, isDot);
      }
      else
      {
        clearDisplayDigit(pos, 0);
      }
    }
  }
}

void SevenSegmentDisplay::setDisplayToString(const String string, const bool shouldBlink, const byte pos)
{
    static unsigned long lastUpdateTime = 0;
    static bool displayOn = true;
    const unsigned long blinkInterval = 500;

    int stringLength = string.length();

    if (shouldBlink)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastUpdateTime >= blinkInterval)
        {
            lastUpdateTime = currentTime;
            displayOn = !displayOn;
        }
        if (displayOn)
        {
            setDisplayToString(string, pos);
        }
        else
        {
            for (int i = 0; i < displays - pos; i++)
            {
                clearDisplayDigit(i, 0);
            }
        }
    }
    else
    {
        setDisplayToString(string, pos);
    }
}

void SevenSegmentDisplay::setDisplayToString(const String string, const byte pos)
{
    int stringLength = string.length();
    for (int i = 0; i < displays - pos; i++)
    {
        if (i < stringLength)
        {
            sendChar(i + pos, FONT_DEFAULT[string[i] - 32], 0);
        }
        else
        {
            break;
        }
    }
}

void SevenSegmentDisplay::setDisplayDigit(byte digit, byte pos, boolean dot)
{
  sendChar(pos, NUMBER_FONT[digit & 0xF], dot);
}

void SevenSegmentDisplay::clearDisplayDigit(byte pos, boolean dot)
{
    sendChar(pos, 0, dot);
}

void SevenSegmentDisplay::sendChar(byte pos, byte data, boolean dot)
{
    sendData(pos << 1, data | (dot ? 0b10000000 : 0));
}

void SevenSegmentDisplay::sendCommand(byte cmd)
{
    digitalWrite(strobePin, LOW);
#ifdef ARDUINO_ARCH_ESP32
    delayMicroseconds(2);
#endif
    send(cmd);
    digitalWrite(strobePin, HIGH);
}

void SevenSegmentDisplay::sendData(byte address, byte data)
{
    sendCommand(0x44);
    digitalWrite(strobePin, LOW);
    send(0xC0 | address);
    send(data);
    digitalWrite(strobePin, HIGH);
}

void SevenSegmentDisplay::send(byte data)
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(clockPin, LOW);
#ifdef ARDUINO_ARCH_ESP32
        delayMicroseconds(2);
#endif
        digitalWrite(dataPin, data & 1 ? HIGH : LOW);
#ifdef ARDUINO_ARCH_ESP32
        delayMicroseconds(2);
#endif
        data >>= 1;
        digitalWrite(clockPin, HIGH);
#ifdef ARDUINO_ARCH_ESP32
        delayMicroseconds(2);
#endif
    }
}

byte SevenSegmentDisplay::receive()
{
    byte temp = 0;

    // Pull-up on
    pinMode(dataPin, INPUT);
    digitalWrite(dataPin, HIGH);

    for (int i = 0; i < 8; i++)
    {
        temp >>= 1;

        digitalWrite(clockPin, LOW);
#ifdef ARDUINO_ARCH_ESP32
        delayMicroseconds(2);
#endif
        if (digitalRead(dataPin))
        {
            temp |= 0x80;
        }

        digitalWrite(clockPin, HIGH);
    }

    // Pull-up off
    pinMode(dataPin, OUTPUT);
    digitalWrite(dataPin, LOW);
#ifdef ARDUINO_ARCH_ESP32
    delayMicroseconds(2);
#endif

    return temp;
}
