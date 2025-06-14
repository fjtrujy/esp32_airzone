#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "dht.h"
#include "ssd1306.h"

#define DEBUG true // Set to false for final Airzone relay control

// Configuration
#define TEMP_CHECK_INTERVAL_MS 2000  // Check temperature every 2 seconds
#define TEMP_MARGIN 0.5              // Temperature margin in Celsius
#define TEMP_STEP 0.5                // Temperature adjustment step

// GPIO Definitions
#define DHT11_GPIO GPIO_NUM_12        // DHT11 sensor pin (now D12)
#define BUTTON_UP_GPIO GPIO_NUM_15    // Red button (increase temp)
#define BUTTON_DOWN_GPIO GPIO_NUM_4   // Blue button (decrease temp)
#define BUTTON_MODE_GPIO GPIO_NUM_5   // White button (mode hot/cool)
#define CONTROL1_GPIO GPIO_NUM_18     // Control1 relay output or Red LED
#define CONTROL2_GPIO GPIO_NUM_19     // Control2 relay output or Blue LED
#define I2C_MASTER_SCL_IO GPIO_NUM_22 // OLED SCL
#define I2C_MASTER_SDA_IO GPIO_NUM_21 // OLED SDA
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000

// System states
typedef enum {
    MODE_COOL,
    MODE_HEAT
} system_mode_t;

typedef struct {
    float current_temp;
    float set_temp;
    system_mode_t mode;
    bool control1_active;
    bool control2_active;
} system_state_t;

static const char *TAG = "AIRZONE_THERMOSTAT";

// Global variables
static system_state_t system_state = {
    .current_temp = 20.0,
    .set_temp = 22.0,
    .mode = MODE_COOL,
    .control1_active = false,
    .control2_active = false
};

static QueueHandle_t button_queue = NULL;
static ssd1306_t dev;

// Button interrupt handler
static void IRAM_ATTR button_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_queue, &gpio_num, NULL);
}

// Initialize I2C for OLED display
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// Initialize GPIO
static void gpio_init(void)
{
    // Configure buttons
    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_UP_GPIO) | (1ULL << BUTTON_DOWN_GPIO) | (1ULL << BUTTON_MODE_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&button_conf);
    
    // Configure relay outputs or LEDs
    gpio_config_t relay_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << CONTROL1_GPIO) | (1ULL << CONTROL2_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&relay_conf);
    
    // Initialize outputs to OFF
    gpio_set_level(CONTROL1_GPIO, 0);
    gpio_set_level(CONTROL2_GPIO, 0);
}

// Update OLED display
static void update_display(void)
{
    ssd1306_clear_screen(&dev, false);
    
    // Display current temperature
    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "Current: %.1fC", system_state.current_temp);
    ssd1306_display_text(&dev, 0, temp_str, 16, false);
    
    // Display set temperature
    snprintf(temp_str, sizeof(temp_str), "Set: %.1fC", system_state.set_temp);
    ssd1306_display_text(&dev, 1, temp_str, 16, false);
    
    // Display mode
    const char* mode_str = (system_state.mode == MODE_COOL) ? "Mode: COOL" : "Mode: HEAT";
    ssd1306_display_text(&dev, 2, mode_str, 16, false);
    
    // Display status
    const char* status_str = system_state.control1_active ? "Status: ON" : "Status: OFF";
    ssd1306_display_text(&dev, 3, status_str, 16, false);
    
    ssd1306_refresh_gram(&dev);
}

// Temperature control logic
static void temperature_control(void)
{
    bool should_activate = false;
    
    if (system_state.mode == MODE_COOL) {
        // COOL mode: activate when current temp > set temp + margin
        should_activate = (system_state.current_temp > (system_state.set_temp + TEMP_MARGIN));
    } else {
        // HEAT mode: activate when current temp < set temp - margin
        should_activate = (system_state.current_temp < (system_state.set_temp - TEMP_MARGIN));
    }
    
    // Update relay state or LEDs
    if (should_activate && !system_state.control1_active) {
        system_state.control1_active = true;
        gpio_set_level(CONTROL1_GPIO, 1); // Red LED ON or relay ON
        if (DEBUG) {
            gpio_set_level(CONTROL2_GPIO, 0); // Blue LED OFF
        }
        ESP_LOGI(TAG, "Control1 activated - Mode: %s, Current: %.1f, Set: %.1f", 
                 (system_state.mode == MODE_COOL) ? "COOL" : "HEAT",
                 system_state.current_temp, system_state.set_temp);
    } else if (!should_activate && system_state.control1_active) {
        system_state.control1_active = false;
        gpio_set_level(CONTROL1_GPIO, 0); // Red LED OFF or relay OFF
        if (DEBUG) {
            gpio_set_level(CONTROL2_GPIO, 1); // Blue LED ON (simulate Control2)
        }
        ESP_LOGI(TAG, "Control1 deactivated - Mode: %s, Current: %.1f, Set: %.1f", 
                 (system_state.mode == MODE_COOL) ? "COOL" : "HEAT",
                 system_state.current_temp, system_state.set_temp);
    }
    // In production, CONTROL2_GPIO can be used for a second relay if needed
}

// Button task
static void button_task(void* arg)
{
    uint32_t gpio_num;
    while(1) {
        if(xQueueReceive(button_queue, &gpio_num, portMAX_DELAY)) {
            // Debounce delay
            vTaskDelay(pdMS_TO_TICKS(50));
            
            // Check if button is still pressed (active low)
            if(gpio_get_level(gpio_num) == 0) {
                if (gpio_num == BUTTON_UP_GPIO) {
                    system_state.set_temp += TEMP_STEP;
                    ESP_LOGI(TAG, "Temperature UP: %.1fC", system_state.set_temp);
                } else if (gpio_num == BUTTON_DOWN_GPIO) {
                    system_state.set_temp -= TEMP_STEP;
                    ESP_LOGI(TAG, "Temperature DOWN: %.1fC", system_state.set_temp);
                } else if (gpio_num == BUTTON_MODE_GPIO) {
                    system_state.mode = (system_state.mode == MODE_COOL) ? MODE_HEAT : MODE_COOL;
                    ESP_LOGI(TAG, "Mode changed to: %s", (system_state.mode == MODE_COOL) ? "COOL" : "HEAT");
                }
                
                // Update display immediately after button press
                update_display();
            }
        }
    }
}

// Temperature reading task
static void temperature_task(void* arg)
{
    while(1) {
        // Read temperature from DHT11
        float temperature = 0.0;
        float humidity = 0.0;
        
        esp_err_t result = dht_read_float_data(DHT_TYPE_DHT11, DHT11_GPIO, &humidity, &temperature);
        
        if (result == ESP_OK) {
            system_state.current_temp = temperature;
            ESP_LOGI(TAG, "Temperature: %.1f°C, Humidity: %.1f%%", temperature, humidity);
            
            // Update temperature control
            temperature_control();
            
            // Update display
            update_display();
        } else {
            ESP_LOGE(TAG, "Failed to read DHT11 sensor");
        }
        
        vTaskDelay(pdMS_TO_TICKS(TEMP_CHECK_INTERVAL_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Airzone Tacto V1.5 Replacement Starting...");
    
    // Initialize I2C for OLED
    ESP_ERROR_CHECK(i2c_master_init());
    
    // Initialize OLED display
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
    ssd1306_display_text(&dev, 0, "Airzone Tacto", 16, false);
    ssd1306_display_text(&dev, 1, "Replacement", 16, false);
    ssd1306_display_text(&dev, 2, "Starting...", 16, false);
    ssd1306_refresh_gram(&dev);
    
    // Initialize GPIO
    gpio_init();
    
    // Create button queue
    button_queue = xQueueCreate(10, sizeof(uint32_t));
    
    // Create tasks
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
    xTaskCreate(temperature_task, "temp_task", 4096, NULL, 5, NULL);
    
    // Install GPIO ISR service
    gpio_install_isr_service(0);
    
    // Add ISR handlers for buttons
    gpio_isr_handler_add(BUTTON_UP_GPIO, button_isr_handler, (void*) BUTTON_UP_GPIO);
    gpio_isr_handler_add(BUTTON_DOWN_GPIO, button_isr_handler, (void*) BUTTON_DOWN_GPIO);
    gpio_isr_handler_add(BUTTON_MODE_GPIO, button_isr_handler, (void*) BUTTON_MODE_GPIO);
    
    ESP_LOGI(TAG, "System initialized successfully!");
    ESP_LOGI(TAG, "Initial settings: Set temp=%.1f°C, Mode=%s", 
             system_state.set_temp, (system_state.mode == MODE_COOL) ? "COOL" : "HEAT");
    
    // Initial display update
    update_display();
    
    // Main loop (mostly handled by tasks)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}