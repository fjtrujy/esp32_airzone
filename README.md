# Airzone Tacto V1.5 Replacement

A complete ESP32-based replacement for the Airzone Tacto V1.5 thermostat controller. This project implements a smart thermostat with temperature sensing, OLED display, button controls, and relay outputs for HVAC system control.

**Designed for ESP32 DEVKITV1 Development Board**

## Features

- **Temperature Sensing**: DHT11 sensor for accurate temperature and humidity readings
- **OLED Display**: 128x64 I2C display showing current temperature, set temperature, mode, and status
- **Button Controls**: 3 buttons for temperature adjustment and mode switching
- **Smart Control Logic**: Automatic temperature control with configurable margins
- **Dual Mode Support**: COOL and HEAT modes with appropriate control logic
- **Relay Outputs**: 2 relay outputs for Control1 and Control2 signals
- **Real-time Monitoring**: Serial output for debugging and monitoring
- **ESP32 DEVKITV1 Optimized**: Uses built-in buttons and LED for easy setup

## Hardware Requirements

### Components:
1. **ESP32 DEVKITV1 Development Board**
2. **DHT11 Temperature & Humidity Sensor**
3. **SSD1306 OLED Display** (128x64 I2C)
4. **1x External Push Button** (for MODE button)
5. **2x Relay Modules** (for Control1 and Control2 outputs)
6. **Breadboard and Jumper Wires**
7. **Power Supply** (5V for relays, 3.3V for ESP32)

### ESP32 DEVKITV1 Pin Connections:
```
ESP32 DEVKITV1 Pin    →    Component
─────────────────────────────────────────
GPIO 4               →    DHT11 Data Pin
GPIO 0 (BOOT)        →    Button UP (Built-in BOOT button)
GPIO 2 (Built-in LED)→    Button DOWN (Built-in LED)
GPIO 5               →    Button MODE (External button)
GPIO 18              →    Control1 Relay
GPIO 19              →    Control2 Relay
GPIO 21              →    OLED SDA (I2C)
GPIO 22              →    OLED SCL (I2C)
3.3V                 →    OLED VCC, DHT11 VCC
GND                  →    OLED GND, DHT11 GND, Relay GND
5V                   →    Relay VCC
```

### ESP32 DEVKITV1 Built-in Features Used:
- **BOOT Button (GPIO 0)**: Temperature UP button
- **Built-in LED (GPIO 2)**: Temperature DOWN button indicator
- **USB Port**: Programming and power supply
- **3.3V/5V Outputs**: Power for sensors and relays

## Software Features

### Temperature Control Logic:
- **COOL Mode**: Activates when current temperature > set temperature + margin
- **HEAT Mode**: Activates when current temperature < set temperature - margin
- **Hysteresis**: Prevents rapid on/off cycling with configurable margin
- **Configurable Parameters**: Temperature margin, check interval, and adjustment step

### User Interface:
- **OLED Display**: Shows current temperature, set temperature, mode, and status
- **Button Controls**: 
  - BOOT Button: Increase set temperature by 0.5°C
  - Built-in LED area: Decrease set temperature by 0.5°C
  - External MODE Button: Toggle between COOL and HEAT modes
- **Real-time Updates**: Display updates every 2 seconds with new readings

### System Architecture:
- **Multi-tasking**: Separate tasks for temperature reading, button handling, and display updates
- **Interrupt-driven**: Button inputs use GPIO interrupts for responsive control
- **Error Handling**: Robust error handling for sensor failures and communication issues

## Installation and Setup

### 1. Prerequisites
- ESP-IDF v5.4 or later
- Python 3.7 or later
- ESP32 DEVKITV1 development board
- Required hardware components

### 2. Build and Flash
```bash
# Set up ESP-IDF environment
source /path/to/esp-idf/export.sh

# Build the project
idf.py build

# Find the correct port for ESP32 DEVKITV1
ls /dev/tty.*

# Flash to ESP32 (replace with your port)
idf.py -p /dev/tty.usbserial-0001 -b 115200 flash

# Monitor serial output
idf.py -p /dev/tty.usbserial-0001 -b 115200 monitor
```

### 3. Hardware Assembly
1. Connect components according to the wiring diagram in `WIRING_DIAGRAM.md`
2. Use ESP32 DEVKITV1 built-in features:
   - BOOT button for temperature UP
   - Built-in LED area for temperature DOWN
   - Add external button for MODE toggle
3. Power up the system via USB
4. Verify OLED display shows startup message
5. Test button functionality

## Configuration

### Temperature Settings (in `main.c`):
```c
#define TEMP_CHECK_INTERVAL_MS 2000  // Check temperature every 2 seconds
#define TEMP_MARGIN 0.5              // Temperature margin in Celsius
#define TEMP_STEP 0.5                // Temperature adjustment step
```

### GPIO Configuration for ESP32 DEVKITV1:
All GPIO pins are optimized for ESP32 DEVKITV1:
```c
#define DHT11_GPIO GPIO_NUM_4         // DHT11 sensor pin
#define BUTTON_UP_GPIO GPIO_NUM_0     // Built-in BOOT button
#define BUTTON_DOWN_GPIO GPIO_NUM_2   // Built-in LED
#define BUTTON_MODE_GPIO GPIO_NUM_5   // External MODE button
#define CONTROL1_GPIO GPIO_NUM_18     // Control1 relay output
#define CONTROL2_GPIO GPIO_NUM_19     // Control2 relay output
#define I2C_MASTER_SCL_IO GPIO_NUM_22 // OLED SCL
#define I2C_MASTER_SDA_IO GPIO_NUM_21 // OLED SDA
```

## Usage

### Basic Operation:
1. **Power On**: Connect ESP32 DEVKITV1 via USB, system initializes and displays startup message
2. **Temperature Reading**: DHT11 sensor reads temperature every 2 seconds
3. **Display Update**: OLED shows current temperature, set temperature, mode, and status
4. **Button Control**: Use built-in and external buttons to adjust set temperature and change modes
5. **Automatic Control**: System automatically activates/deactivates relays based on temperature

### Button Functions (ESP32 DEVKITV1):
- **BOOT Button (GPIO 0)**: Increase set temperature by 0.5°C
- **Built-in LED Area (GPIO 2)**: Decrease set temperature by 0.5°C
- **External MODE Button (GPIO 5)**: Toggle between COOL and HEAT modes

### Control Logic:
- **COOL Mode**: Activates Control1 when temperature is too high
- **HEAT Mode**: Activates Control1 when temperature is too low
- **Hysteresis**: Prevents rapid cycling with 0.5°C margin

## Airzone Integration

### 4-Wire Connection:
```
ESP32 DEVKITV1/Relay  →    Airzone Terminal
─────────────────────────────────────────────
5V                    →    VCC (5V)
GND                   →    GND
Control1              →    Control1 (3.3V when active)
Control2              →    Control2 (3.3V when active)
```

### Integration Steps:
1. Connect relay outputs to Airzone control terminals
2. Verify signal compatibility (3.3V logic levels)
3. Test complete temperature control cycle
4. Fine-tune temperature settings as needed

## Troubleshooting

### ESP32 DEVKITV1 Specific Issues:
1. **USB Connection Issues**:
   - Install CP2102 or CH340G drivers if needed
   - Try different USB cable
   - Check if port appears in device manager

2. **BOOT Button Not Working**:
   - Ensure GPIO 0 is not pulled low during boot
   - Press BOOT button while uploading firmware
   - Check if button is physically working

3. **Built-in LED Not Responding**:
   - Check if GPIO 2 is configured correctly
   - LED might be inverted (active low)
   - Add external button if needed

4. **Power Issues**:
   - ESP32 DEVKITV1 needs stable 3.3V
   - USB provides 5V, internal regulator converts to 3.3V
   - Don't exceed 3.3V on GPIO pins

### General Issues:
1. **OLED not displaying**: Check I2C connections and address
2. **DHT11 not reading**: Check wiring and power supply
3. **Buttons not responding**: Check GPIO connections and pull-up resistors
4. **Relays not activating**: Check 5V power supply and GPIO connections

### Debug Commands:
```bash
# Find the correct port for ESP32 DEVKITV1
ls /dev/tty.*

# Monitor serial output
idf.py -p /dev/tty.usbserial-0001 -b 115200 monitor

# Flash firmware
idf.py -p /dev/tty.usbserial-0001 -b 115200 flash

# Build and flash
idf.py -p /dev/tty.usbserial-0001 -b 115200 build flash
```

## Project Structure

```
esp32_airzone/
├── main/
│   ├── main.c              # Main application code
│   └── CMakeLists.txt      # Main component configuration
├── components/
│   ├── dht/                # DHT11 sensor component
│   │   ├── dht.h
│   │   ├── dht.c
│   │   └── CMakeLists.txt
│   └── ssd1306/            # OLED display component
│       ├── ssd1306.h
│       ├── ssd1306.c
│       └── CMakeLists.txt
├── .vscode/                # VS Code configuration
│   ├── settings.json
│   └── launch.json
├── WIRING_DIAGRAM.md       # Detailed wiring instructions for ESP32 DEVKITV1
└── README.md               # This file
```

## Development

### Adding Features:
- **WiFi Connectivity**: Add WiFi component for remote monitoring
- **Web Interface**: Create web-based configuration interface
- **Data Logging**: Add SD card or cloud logging capabilities
- **Multiple Sensors**: Support for multiple temperature sensors
- **Advanced Scheduling**: Add time-based temperature scheduling

### Customization:
- Modify GPIO pins in `main.c`
- Adjust temperature parameters
- Change display layout
- Add new control modes
- Implement additional sensors

## Safety and Compliance

1. **Voltage Levels**: All GPIO pins are 3.3V logic level
2. **Current Limits**: Maximum 12mA per GPIO pin
3. **Power Supply**: Use stable 3.3V or 5V via USB
4. **ESD Protection**: Handle board carefully, avoid static discharge
5. **Heat Management**: ESP32 can get warm during operation
6. **Electrical Safety**: Ensure proper grounding and isolation
7. **Relay Protection**: Add flyback diodes if needed
8. **Enclosure**: Use appropriate enclosure for protection

## License

This project is open source and available under the MIT License.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Support

For support and questions:
1. Check the troubleshooting section
2. Review the wiring diagram for ESP32 DEVKITV1
3. Monitor serial output for error messages
4. Open an issue on the project repository
