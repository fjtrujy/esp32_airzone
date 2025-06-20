# Airzone Tacto V1.5 Replacement

A complete ESP32-based replacement for the Airzone Tacto V1.5 thermostat controller. This project implements a smart thermostat with temperature sensing, OLED display, button controls, and relay outputs for HVAC system control.

**Designed for ESP32 DEVKITV1 Development Board**

## Features

- **Temperature Sensing**: DHT11 sensor for accurate temperature and humidity readings
- **OLED Display**: 128x64 I2C display showing current temperature, set temperature, mode, and status
- **Button Controls**: 3 buttons for temperature adjustment and mode switching
- **Smart Control Logic**: Automatic temperature control with configurable margins
- **Dual Mode Support**: COOL and HEAT modes with appropriate control logic
- **Relay Outputs**: 2 relay outputs for Cooling and Heating signals
- **Real-time Monitoring**: Serial output for debugging and monitoring
- **ESP32 DEVKITV1 Optimized**: Uses built-in buttons and LED for easy setup

## Hardware Requirements

### Components:
1. **ESP32 DEVKITV1 Development Board**
2. **DHT11 Temperature & Humidity Sensor**
3. **SSD1306 OLED Display** (128x64 I2C)
4. **3x External Push Buttons** (White, Blue, Red)
5. **2x Relay Modules** (for Cooling and Heating outputs)
6. **Breadboard and Jumper Wires**
7. **Power Supply** (5V for relays, 3.3V for ESP32)

### ESP32 DEVKITV1 Pin Connections:
```
ESP32 DEVKITV1 Pin    →    Component
─────────────────────────────────────────
GPIO 4               →    DHT11 Data Pin
GPIO 5               →    White Button (MODE) - External
GPIO 13              →    Cooling Relay
GPIO 14              →    Heating Relay
GPIO 18              →    Blue Button (DECREASE) - External
GPIO 19              →    Red Button (INCREASE) - External
GPIO 21              →    OLED SDA (I2C)
GPIO 22              →    OLED SCL (I2C)
3.3V                 →    OLED VCC, DHT11 VCC, Button VCC
GND                  →    OLED GND, DHT11 GND, Relay GND, Button GND
5V                   →    Relay VCC
```

### ESP32 DEVKITV1 Built-in Features Used:
- **USB Port**: Programming and power supply
- **3.3V/5V Outputs**: Power for sensors, relays, and external buttons

## Software Features

### Temperature Control Logic:
- **COOL Mode**: Activates Cooling when temperature > set temperature + margin, stops when temperature ≤ set temperature
- **HEAT Mode**: Activates Heating when temperature < set temperature - margin, stops when temperature ≥ set temperature
- **Hysteresis**: Prevents rapid cycling with configurable margin (1.0°C)
- **Configurable Parameters**: Temperature margin, check interval, and adjustment step

### User Interface:
- **OLED Display**: Shows current temperature, set temperature, mode, and status
- **Button Controls**: 
  - White Button: Change thermostat mode (OFF → COOL → HEAT → OFF)
  - Blue Button: Decrease set temperature by 0.5°C
  - Red Button: Increase set temperature by 0.5°C
- **Button Debouncing**: 300ms debounce time prevents multiple rapid button presses
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
2. Connect external buttons:
   - White button to GPIO 5 for mode control
   - Blue button to GPIO 18 for temperature decrease
   - Red button to GPIO 19 for temperature increase
3. Power up the system via USB
4. Verify OLED display shows startup message
5. Test button functionality

## CI/CD Pipeline

This project uses GitHub Actions for continuous integration and deployment.

### Workflows

#### 1. Development Build (`esp32-dev.yml`)
**Triggers:** Push to `develop`, `feature/*`, `bugfix/*` branches, and PRs to main branches
**Purpose:** Quick feedback during development

**Features:**
- Fast build and test
- Build size reporting
- Artifact upload (7-day retention)
- Memory usage analysis

#### 2. Production Build (`esp32-build.yml`)
**Triggers:** Push to `main`/`master` branches, and PRs to main branches
**Purpose:** Full production pipeline

**Jobs:**
- **Build:** Compiles the project and uploads artifacts
- **Test:** Runs tests and code formatting checks
- **Security:** Performs CodeQL security analysis
- **Release:** Creates GitHub releases with firmware files

### Artifacts

#### Development Builds
- `esp32_airzone.bin` - Main application firmware
- Build size reports
- Memory usage analysis

#### Production Releases
- `esp32_airzone.bin` - Main application firmware
- `bootloader.bin` - ESP32 bootloader
- `partition-table.bin` - Partition table
- GitHub release with version tags

### CI/CD Integration
1. **Push to feature branch** → Triggers development build
2. **Create PR to main** → Triggers full pipeline
3. **Merge to main** → Creates release automatically

### Configuration

#### ESP-IDF Version
- **Version:** v5.4.1
- **Target:** ESP32
- **Python:** 3.11

#### Dependencies
The pipeline automatically handles:
- ESP-IDF installation
- Component dependencies (via `idf-component-manager`)
- Build environment setup

### Security

#### CodeQL Analysis
- Automatic security scanning
- C/C++ code analysis
- Vulnerability detection

#### Code Quality
- Format checking with `idf.py check-format`
- Component size monitoring
- Memory usage tracking

### Local Development
```bash
# Build locally
idf.py build

# Check formatting
idf.py check-format

# Run tests (when available)
idf.py test

# Test the build locally
idf.py build
idf.py size-components
idf.py check-format
```

### Troubleshooting

#### Common Issues
1. **Build failures:** Check ESP-IDF version compatibility
2. **Dependency issues:** Verify `idf_component.yml` configuration
3. **Memory issues:** Monitor build size reports

### Future Enhancements

- [ ] Add unit tests
- [ ] Add hardware-in-the-loop testing
- [ ] Add firmware signing
- [ ] Add OTA update testing
- [ ] Add performance benchmarking

## Configuration

### Temperature Settings (in `main.c`):
```c
#define TEMP_CHECK_INTERVAL_MS 2000  // Check temperature every 2 seconds
#define TEMP_MARGIN 1.0              // Temperature margin in Celsius
#define TEMP_STEP 0.5                // Temperature adjustment step
#define BUTTON_DEBOUNCE_MS 300       // Button debounce time in milliseconds
```

### GPIO Configuration for ESP32 DEVKITV1:
All GPIO pins are optimized for ESP32 DEVKITV1:
```c
#define DHT11_GPIO GPIO_NUM_4         // DHT11 sensor pin
#define BUTTON_WHITE_GPIO GPIO_NUM_5  // White button - MODE
#define BUTTON_BLUE_GPIO GPIO_NUM_18  // Blue button - DECREASE temperature
#define BUTTON_RED_GPIO GPIO_NUM_19   // Red button - INCREASE temperature
#define COOLING_GPIO GPIO_NUM_13      // Cooling relay output
#define HEATING_GPIO GPIO_NUM_14      // Heating relay output
#define I2C_MASTER_SCL_IO GPIO_NUM_22 // OLED SCL
#define I2C_MASTER_SDA_IO GPIO_NUM_21 // OLED SDA
```

## Usage

### Basic Operation:
1. **Power On**: Connect ESP32 DEVKITV1 via USB, system initializes and displays startup message
2. **Temperature Reading**: DHT11 sensor reads temperature every 2 seconds
3. **Display Update**: OLED shows current temperature, set temperature, mode, and status
4. **Button Control**: Use white, blue, and red buttons to adjust set temperature and change modes
5. **Automatic Control**: System automatically activates/deactivates relays based on temperature

### Button Functions (External Buttons):
- **White Button (GPIO 5)**: Change thermostat mode (OFF → COOL → HEAT → OFF)
- **Blue Button (GPIO 18)**: Decrease set temperature by 0.5°C
- **Red Button (GPIO 19)**: Increase set temperature by 0.5°C

### Control Logic:
- **COOL Mode**: Activates Cooling when temperature > set temperature + margin, stops when temperature ≤ set temperature
- **HEAT Mode**: Activates Heating when temperature < set temperature - margin, stops when temperature ≥ set temperature
- **Hysteresis**: Prevents rapid cycling with configurable margin (1.0°C)

## Airzone Integration

### 4-Wire Connection:
```
ESP32 DEVKITV1/Relay  →    Airzone Terminal
─────────────────────────────────────────────
5V                    →    VCC (5V)
GND                   →    GND
Cooling               →    Cooling (3.3V when active)
Heating               →    Heating (3.3V when active)
```

### Integration Steps:
1. Connect cooling relay output to Airzone cooling control terminal
2. Connect heating relay output to Airzone heating control terminal
3. Verify signal compatibility (3.3V logic levels)
4. Test complete temperature control cycle
5. Fine-tune temperature settings as needed

## Troubleshooting

### ESP32 DEVKITV1 Specific Issues:
1. **USB Connection Issues**:
   - Install CP2102 or CH340G drivers if needed
   - Try different USB cable
   - Check if port appears in device manager

2. **White Button Not Working**:
   - Check if GPIO 5 is connected correctly
   - Verify button wiring (3.3V to one side, GPIO 5 to other side)
   - Check if button is physically working
   - Ensure proper pull-up resistor configuration

3. **Blue Button Not Working**:
   - Check if GPIO 18 is connected correctly
   - Verify button wiring (3.3V to one side, GPIO 18 to other side)
   - Button might be inverted (active low)
   - Check pull-up resistor configuration

4. **Red Button Not Working**:
   - Check if GPIO 19 is connected correctly
   - Verify button wiring (3.3V to one side, GPIO 19 to other side)
   - Button might be inverted (active low)
   - Check pull-up resistor configuration

5. **Power Issues**:
   - ESP32 DEVKITV1 needs stable 3.3V
   - USB provides 5V, internal regulator converts to 3.3V
   - Don't exceed 3.3V on GPIO pins
   - Ensure buttons have proper 3.3V power supply

### General Issues:
1. **OLED not displaying**: Check I2C connections and address
2. **DHT11 not reading**: Check wiring and power supply
3. **Buttons not responding**: Check GPIO connections and pull-up resistors
4. **Relays not activating**: Check 5V power supply and GPIO connections

### Debug Commands:
```bash
# Find the correct port for ESP32 DEVKITV1
ls /dev/tty.*
```

## Project Structure

```
esp32_airzone/
├── main/
│   ├── main.c              # Main application code
│   ├── CMakeLists.txt      # Main component configuration
│   └── idf_component.yml   # Component dependencies
├── .github/workflows/      # CI/CD pipelines
│   ├── esp32-build.yml     # Production build workflow
│   └── esp32-dev.yml       # Development build workflow
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
