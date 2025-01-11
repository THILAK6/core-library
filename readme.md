# Menu Manager Library for ESP32 and TM1638 Display

## Overview

This open-source library is designed to help developers manage menus and display information on a 7-segment display module using the ESP32 Arduino framework. It provides an easy-to-use interface to handle menu navigation, editing, and real-time updates on a TM1638-based display.

---

## Features

- **Menu Management**: Create and manage hierarchical menus with editable and non-editable options.
- **7-Segment Display Support**: Works seamlessly with TM1638 modules.
- **Real-Time Interaction**: Update values and modes dynamically.
- **Customizable**: Supports user-defined save and reset behaviors.
- **Multiple Buttons Support**: Handles navigation with dedicated buttons.

---

## Requirements

- **Hardware**: ESP32, TM1638 7-segment display module, push buttons (menu, enter, select, up, reset).
- **Software**: Arduino IDE or PlatformIO with ESP32 support.

---

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/MenuManager.git
   ```

2. Include the library in your Arduino project:
    ```cpp 
    #include "MenuManager.h" 
    ```

## Getting Started
### Setup 
**Menu Items:** Define the items to display in the menu.

**Menu Manager**: Initialize the MenuManager with menu items, display module, buttons, and custom behaviors.

**Loop Management**: Call the manage() method inside your Arduino loop() function.

## Example
1. Define Menu Items
```cpp
 #include "MenuManager.h"

// TM1638 display module instance
TM1638 module( /* parameters specific to your TM1638 configuration */ );

// Menu items for "View" and "Edit" menus
MenuItem item1("Main", "View Temp", "Tmp", 25.5, &module, 1);
MenuItem item2("Main", "Edit Speed", "Spd", 5.0, &module, 0, true);
MenuItem item3("Main", "Select Mode", "Mode", {"Auto", "Manual"}, {"A", "M"}, 0, &module, true);
```
2. Initialize the Menu Manager
```cpp 
// Button GPIO pins
int menuButton = 12, enterButton = 13, selectButton = 14, upButton = 15, resetButton = 16;

// Define "Save" and "Reset" actions
void saveAction() {
    Serial.println("Settings saved!");
}

void resetAction(MenuItem *item) {
    Serial.print("Resetting: ");
    Serial.println(item->name);
}

// Create MenuManager instance
MenuManager manager(
    {&item1, &item3},    // View menu items
    {&item2},            // Edit menu items
    &module,             // TM1638 module instance
    menuButton, enterButton, selectButton, upButton, resetButton,
    item3,               // Custom password menu item (if applicable)
    saveAction,          // Save action
    resetAction          // Reset action
);
```

3. Loop Management
```cpp
void setup() {
    Serial.begin(115200);
    // Initialize other components if required
}

void loop() {
    manager.manage();
}
```

## API Reference

### `MenuManager` Class

#### Constructor
```cpp
MenuManager(
    std::vector<MenuItem *> &viewMenuItems,
    std::vector<MenuItem *> &editMenuItems,
    TM1638 *module,
    int menuButton,
    int enterButton,
    int selectButton,
    int upButton,
    int resetButton,
    MenuItem &customPassword,
    std::function<void()> save,
    std::function<void(MenuItem*)> reset
);
```

#### Parameters

- `viewMenuItems:` Items displayed in view-only mode.
- `editMenuItems:` Items editable by the user.
- `module:` TM1638 display module instance.
- `menuButton, enterButton, selectButton, upButton, resetButton:` GPIO pins for buttons.
- `customPassword:` Menu item for custom password functionality.
- `save:` Function to execute on save.
  
- `reset:` Function to execute on reset.

#### Methods

`void manage():` Call this method in the `loop()` to handle menu interactions.

### `MenuItem` Class

The `MenuItem` class represents individual menu items that can be either numeric or mode-based. These items can be configured to be editable, allowing users to update their values or modes dynamically.

---

#### Constructors

##### 1. Numeric Menu Item

```cpp
MenuItem(
    String mainName,
    String name,
    String shortName,
    float value,
    TM1638 *module,
    int8_t decimalPos = 0,
    bool isEditable = false,
    float additionalValue = 0,
    uint8_t displayType = 0
);
```

#### Parameters:

- `mainName`: The main menu category name.
- `name`: The full name of the menu item.  
- `shortName`: A shorter display name for the item (ideal for 7-segment displays).  
- `value`: The initial numeric value of the item.  
- `module`: Pointer to the TM1638 display module.  
- `decimalPos`: The number of decimal places to display (default: `0`).  
- `isEditable`: Whether the value can be edited by the user (default: `false`).  
- `additionalValue`: An optional additional value for the item (default: `0`).  

- `displayType`: Reserved for future use to define display types.  

#### Methods:

1. `void updateValue(float valueToUpdate)`
Updates the numeric value of the menu item.

2. `void updateMode(int8_t modeToUpdate)`
Updates the current mode of the menu item.
