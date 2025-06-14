# Airzone Tacto V1.5 Replacement - Wiring Diagram

## Hardware Components

### Required Components:
1. **ESP32 DEVKITV1 Development Board**
2. **DHT11 Temperature & Humidity Sensor**
3. **SSD1306 OLED Display** (128x64 I2C)
4. **3x External Push Buttons**
    - **Red**: Increase temperature
    - **Blue**: Decrease temperature
    - **White**: Change mode (hot/cool)
5. **2x Relay Modules** (for Control1 and Control2 outputs)
6. **Breadboard and Jumper Wires**
7. **Power Supply** (5V for relays, 3.3V for ESP32)

## ESP32 DEVKITV1 Pin Layout

### ESP32 DEVKITV1 Features:
- **USB-to-UART**: CP2102 or CH340G chip
- **Power**: 3.3V and 5V outputs available
- **GPIO Pins**: 30+ GPIO pins available

### Pin Connections for ESP32 DEVKITV1:

```
ESP32 DEVKITV1 Pin    →    Component
───────────────────────────────────────────────
GPIO 15 (D15)         →    Red Button (Increase temperature)
GPIO 4  (D4)          →    Blue Button (Decrease temperature)
GPIO 5  (D5)          →    White Button (Change mode hot/cool)
GPIO 18 (D18)         →    Control1 Relay
GPIO 19 (D19)         →    Control2 Relay
GPIO 21 (D21)         →    OLED SDA (I2C)
GPIO 22 (D22)         →    OLED SCL (I2C)
GPIO 2  (D2)          →    DHT11 Data Pin
3.3V                  →    OLED VCC, DHT11 VCC
GND                   →    OLED GND, DHT11 GND, Relay GND, Button GND
5V                    →    Relay VCC
```

## Detailed Wiring for ESP32 DEVKITV1

### 1. DHT11 Sensor
```
DHT11 Pin    →    ESP32 DEVKITV1 Pin
─────────────────────────────────────
VCC          →    3.3V
GND          →    GND
DATA         →    GPIO 2 (D2)
```

### 2. OLED Display (I2C)
```
OLED Pin     →    ESP32 DEVKITV1 Pin
─────────────────────────────────────
VCC          →    3.3V
GND          →    GND
SDA          →    GPIO 21 (D21)
SCL          →    GPIO 22 (D22)
```

### 3. Buttons (External, Color Coded)
```
Button Color    →    ESP32 DEVKITV1 Pin    →    Function
────────────────────────────────────────────────────────────
Red             →    GPIO 15 (D15)         →    Increase temperature
Blue            →    GPIO 4  (D4)          →    Decrease temperature
White           →    GPIO 5  (D5)          →    Change mode (hot/cool)
Other pin       →    GND                    →    (for each button)
```
- Use one pin from each button to the specified GPIO, and the opposite pin to GND.
- The other two pins on each button can be left unconnected or also connected for stability.

### 4. Relay Outputs
```
Relay Module →    ESP32 DEVKITV1 Pin    →    Airzone System
─────────────────────────────────────────────────────────────
Control1     →    GPIO 18 (D18)         →    Control1 Wire
Control2     →    GPIO 19 (D19)         →    Control2 Wire
VCC          →    5V
GND          →    GND
```

## ESP32 DEVKITV1 Physical Layout (with Button Colors)

```
┌────────────────────────────────────────────────────────────┐
│              ESP32 DEVKITV1                                │
│  ┌───────────────────────────────────────────────────────┐ │
│  │ USB Port (CP2102/CH340G)                             │ │
│  │                                                     │ │
│  │ GPIO 2  (D2)  → DHT11 Data                          │ │
│  │ GPIO 4  (D4)  → Blue Button (Decrease Temp)         │ │
│  │ GPIO 5  (D5)  → White Button (Mode Hot/Cool)        │ │
│  │ GPIO 15 (D15) → Red Button (Increase Temp)          │ │
│  │ GPIO 18 (D18) → Control1 Relay                      │ │
│  │ GPIO 19 (D19) → Control2 Relay                      │ │
│  │ GPIO 21 (D21) → OLED SDA                            │ │
│  │ GPIO 22 (D22) → OLED SCL                            │ │
│  │                                                     │ │
│  │ 3.3V → OLED VCC, DHT11 VCC                          │ │
│  │ 5V   → Relay VCC                                    │ │
│  │ GND  → OLED GND, DHT11 GND, Relay GND, Button GND   │ │
│  └───────────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────────┘
```

## Button Wiring Example (Side View)

```
[Red Button]   [Blue Button]   [White Button]
   |   |         |   |           |   |
  GND GPIO15   GND GPIO4      GND GPIO5
```

## Airzone System Connection

### 4-Wire Connection to Airzone:
```
ESP32 DEVKITV1/Relay  →    Airzone Terminal
─────────────────────────────────────────────
5V                    →    VCC (5V)
GND                   →    GND
Control1              →    Control1 (3.3V when active)
Control2              →    Control2 (3.3V when active)
```

## Testing the Setup

1. Connect ESP32 DEVKITV1 via USB
2. Flash the firmware using the correct port
3. Power on the system
4. Check OLED display shows startup message
5. Verify DHT11 readings appear on display
6. Test button functionality:
   - Press **Red button** (GPIO 15) to increase temperature
   - Press **Blue button** (GPIO 4) to decrease temperature
   - Press **White button** (GPIO 5) to change mode

## Configuration for ESP32 DEVKITV1

### GPIO Configuration (already set for DEVKITV1):
```c
#define BUTTON_UP_GPIO GPIO_NUM_15     // Red button (increase temp)
#define BUTTON_DOWN_GPIO GPIO_NUM_4    // Blue button (decrease temp)
#define BUTTON_MODE_GPIO GPIO_NUM_5    // White button (mode hot/cool)
#define DHT11_GPIO GPIO_NUM_2          // DHT11 sensor pin
#define CONTROL1_GPIO GPIO_NUM_18      // Control1 relay output
#define CONTROL2_GPIO GPIO_NUM_19      // Control2 relay output
#define I2C_MASTER_SCL_IO GPIO_NUM_22  // OLED SCL
#define I2C_MASTER_SDA_IO GPIO_NUM_21  // OLED SDA
```

## Safety Notes for ESP32 DEVKITV1

1. **Voltage Levels**: All GPIO pins are 3.3V logic level
2. **Current Limits**: Maximum 12mA per GPIO pin
3. **Power Supply**: Use stable 3.3V or 5V via USB
4. **ESD Protection**: Handle board carefully, avoid static discharge
5. **Heat Management**: ESP32 can get warm during operation

## Next Steps

1. **Connect ESP32 DEVKITV1** via USB
2. **Assemble the hardware** according to the wiring diagram
3. **Flash the firmware** to the ESP32
4. **Test basic functionality** with serial monitor
5. **Connect to Airzone system** and test integration
6. **Fine-tune temperature settings** as needed
7. **Enclose the system** for permanent installation 