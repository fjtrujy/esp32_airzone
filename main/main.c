#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "dht.h"
#include "ssd1306.h"
#include "translations.h"

static const char *TAG = "ESP32_AIRZONE";

// GPIO Configuration for ESP32 DEVKITV1
#define DHT11_GPIO GPIO_NUM_4
#define BUTTON_WHITE_GPIO GPIO_NUM_5      // White button - MODE (External)
#define BUTTON_BLUE_GPIO GPIO_NUM_18      // Blue button - DECREASE temperature (External)
#define BUTTON_RED_GPIO GPIO_NUM_19       // Red button - INCREASE temperature (External)
#define COOLING_GPIO GPIO_NUM_13          // Cooling relay output
#define HEATING_GPIO GPIO_NUM_14          // Heating relay output

// Temperature control parameters
#define TEMP_CHECK_INTERVAL_MS 2000    // Check temperature every 2 seconds
#define TEMP_MARGIN 1.0                // Temperature margin in Celsius
#define TEMP_STEP 0.5                  // Temperature adjustment step
#define MIN_TEMP 16.0                  // Minimum set temperature
#define MAX_TEMP 35.0                  // Maximum set temperature
#define DEFAULT_TEMP 22.0              // Default set temperature

// Thermostat modes
typedef enum {
    MODE_OFF = 0,
    MODE_COOL = 1,
    MODE_HEAT = 2
} thermostat_mode_t;

// Global variables
static float current_temperature = 0.0f;
static float current_humidity = 0.0f;
static float set_temperature = DEFAULT_TEMP;
static thermostat_mode_t current_mode = MODE_OFF;
static bool cooling_active = false;
static bool heating_active = false;
static QueueHandle_t button_queue = NULL;

// Button debouncing variables
static uint32_t last_button_time[3] = {0, 0, 0}; // Track each button separately
static const uint32_t BUTTON_DEBOUNCE_MS = 300; // 300ms debounce time - prevents multiple rapid button presses

// Button event structure
typedef struct {
    uint32_t gpio_num;
    uint32_t event_type;
} button_event_t;

// Function prototypes
static void button_task(void *pvParameter);
static void temperature_task(void *pvParameter);
static void control_task(void *pvParameter);
static void display_task(void *pvParameter);
static void gpio_isr_handler(void *arg);
static void update_display(void);
static void process_button_event(button_event_t event);
static void update_control_outputs(void);
static int get_button_index(uint32_t gpio_num);

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Airzone TACTO Replacement");

    // Set language (change this to LANG_SPANISH for Spanish)
    set_language(LANG_SPANISH);
    
    // Get translations
    const translations_t* t = get_translations();

    // Initialize SSD1306 display
    init_ssd1306();
    ESP_LOGI(TAG, "SSD1306 display initialized");

    // Show initial welcome message
    ssd1306_print_str(18, 0, t->esp32_airzone, false);
    ssd1306_print_str(28, 17, t->thermostat, false);
    ssd1306_print_str(38, 27, t->starting, false);
    ssd1306_display();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Initialize GPIO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << COOLING_GPIO) | (1ULL << HEATING_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);

    // Configure button GPIOs
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << BUTTON_WHITE_GPIO) | (1ULL << BUTTON_BLUE_GPIO) | (1ULL << BUTTON_RED_GPIO);
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    // Create button queue
    button_queue = xQueueCreate(10, sizeof(button_event_t));

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Add ISR handlers for buttons
    gpio_isr_handler_add(BUTTON_WHITE_GPIO, gpio_isr_handler, (void*)BUTTON_WHITE_GPIO);
    gpio_isr_handler_add(BUTTON_BLUE_GPIO, gpio_isr_handler, (void*)BUTTON_BLUE_GPIO);
    gpio_isr_handler_add(BUTTON_RED_GPIO, gpio_isr_handler, (void*)BUTTON_RED_GPIO);

    ESP_LOGI(TAG, "Starting DHT11 sensor on GPIO %d", DHT11_GPIO);

    // Put relays in OFF state
    gpio_set_level(COOLING_GPIO, 1);
    gpio_set_level(HEATING_GPIO, 1);

    // Create tasks
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(temperature_task, "temp_task", 2048, NULL, 4, NULL);
    xTaskCreate(control_task, "control_task", 2048, NULL, 3, NULL);
    xTaskCreate(display_task, "display_task", 2048, NULL, 2, NULL);

    ESP_LOGI(TAG, "All tasks started successfully");
}

// Button task - handles button events
static void button_task(void *pvParameter)
{
    button_event_t event;
    
    while (1) {
        if (xQueueReceive(button_queue, &event, portMAX_DELAY)) {
            process_button_event(event);
        }
    }
}

// Temperature reading task
static void temperature_task(void *pvParameter)
{
    while (1) {
        float humidity = 0.0f;
        float temperature = 0.0f;
        esp_err_t result = dht_read_float_data(DHT_TYPE_DHT11, DHT11_GPIO, &humidity, &temperature);
        
        if (result == ESP_OK) {
            current_temperature = temperature;
            current_humidity = humidity;
            ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", temperature, humidity);
        } else {
            ESP_LOGE(TAG, "Failed to read DHT11 sensor, error: %s", esp_err_to_name(result));
        }
        
        vTaskDelay(pdMS_TO_TICKS(TEMP_CHECK_INTERVAL_MS));
    }
}

// Control logic task
static void control_task(void *pvParameter)
{
    while (1) {
        update_control_outputs();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check every second
    }
}

// Display update task
static void display_task(void *pvParameter)
{
    while (1) {
        update_display();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Update every second
    }
}

// GPIO ISR handler
static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    button_event_t event = {
        .gpio_num = gpio_num,
        .event_type = GPIO_INTR_NEGEDGE
    };
    
    xQueueSendFromISR(button_queue, &event, NULL);
}

// Process button events
static void process_button_event(button_event_t event)
{
    const translations_t* t = get_translations();
    
    // Get button index and validate
    int button_index = get_button_index(event.gpio_num);
    if (button_index < 0) {
        ESP_LOGE(TAG, "Invalid button GPIO: %lu", event.gpio_num);
        return;
    }
    
    // Get current time for debouncing
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Check if enough time has passed since last button press
    if (current_time - last_button_time[button_index] < BUTTON_DEBOUNCE_MS) {
        ESP_LOGI(TAG, "Button press ignored - debouncing (time since last: %lums)", 
                 current_time - last_button_time[button_index]);
        return;
    }
    
    // Update last button time
    last_button_time[button_index] = current_time;
    
    switch (event.gpio_num) {
        case BUTTON_WHITE_GPIO:
            current_mode = (current_mode + 1) % 3; // Cycle through OFF, COOL, HEAT
            ESP_LOGI(TAG, "Mode changed to: %d", current_mode);
            break;
            
        case BUTTON_BLUE_GPIO:
            set_temperature -= TEMP_STEP;
            if (set_temperature < MIN_TEMP) set_temperature = MIN_TEMP;
            ESP_LOGI(TAG, "Temperature DOWN: %.1f°C", set_temperature);
            break;
            
        case BUTTON_RED_GPIO:
            set_temperature += TEMP_STEP;
            if (set_temperature > MAX_TEMP) set_temperature = MAX_TEMP;
            ESP_LOGI(TAG, "Temperature UP: %.1f°C", set_temperature);
            break;
    }
}

// Update control outputs based on temperature and mode
static void update_control_outputs(void)
{
    bool new_cooling = false;
    bool new_heating = false;
    
    if (current_mode == MODE_COOL) {
        new_cooling = current_temperature > set_temperature - TEMP_MARGIN;
    } else if (current_mode == MODE_HEAT) {
        new_heating = current_temperature < set_temperature + TEMP_MARGIN;
    } else {
        // MODE_OFF - turn off all controls
        new_cooling = false;
        new_heating = false;
    }
    
    // Update control outputs
    if (cooling_active != new_cooling) {
        cooling_active = new_cooling;
        gpio_set_level(COOLING_GPIO, cooling_active ? 0 : 1);
        ESP_LOGI(TAG, "Cooling %s", cooling_active ? "ACTIVATED" : "DEACTIVATED");
    }
    
    if (heating_active != new_heating) {
        heating_active = new_heating;
        gpio_set_level(HEATING_GPIO, heating_active ? 0 : 1);
        ESP_LOGI(TAG, "Heating %s", heating_active ? "ACTIVATED" : "DEACTIVATED");
    }
}

// Update display with current information
static void update_display(void)
{
    const translations_t* t = get_translations();
    
    // Clear the screen
    ssd1306_clear();
    
    // Display temperature based on mode
    char temp_line[32];
    if (current_mode == MODE_OFF) {
        // When OFF, show only current temperature
        snprintf(temp_line, sizeof(temp_line), "%.1f C", current_temperature);
    } else {
        // When COOL or HEAT, show current temperature with desired in parentheses
        snprintf(temp_line, sizeof(temp_line), "%.1f C (%.1f)", current_temperature, set_temperature);
    }
    ssd1306_print_str(0, 0, temp_line, false);
    
    // Display humidity
    char hum_line[32];
    snprintf(hum_line, sizeof(hum_line), "%.1f %%", current_humidity);
    ssd1306_print_str(0, 10, hum_line, false);
    
    // Display mode
    const char* mode_str;
    switch (current_mode) {
        case MODE_COOL:
            mode_str = t->mode_cool;
            break;
        case MODE_HEAT:
            mode_str = t->mode_heat;
            break;
        default:
            mode_str = t->mode_off;
            break;
    }
    ssd1306_print_str(0, 30, t->mode_label, false);
    ssd1306_print_str(0, 40, mode_str, false);
    
    // Update display
    ssd1306_display();
}

// Helper function to get button index from GPIO number
static int get_button_index(uint32_t gpio_num) {
    switch (gpio_num) {
        case BUTTON_WHITE_GPIO: return 0;
        case BUTTON_BLUE_GPIO: return 1;
        case BUTTON_RED_GPIO: return 2;
        default: return -1;
    }
}