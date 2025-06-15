#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "dht.h"
#include "ssd1306.h"
#include "translations.h"

static const char *TAG = "ESP32_AIRZONE";

// GPIO Configuration for ESP32 DEVKITV1
#define DHT11_GPIO GPIO_NUM_4

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Airzone with SSD1306 display");

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

    ESP_LOGI(TAG, "Starting DHT11 sensor on GPIO %d", DHT11_GPIO);

    // Main loop: display DHT11 sensor data
    while (1)
    {
        // Clear the screen at the beginning of each frame
        ssd1306_clear();
        
        float humidity = 0.0f;
        float temperature = 0.0f;
        esp_err_t result = dht_read_float_data(DHT_TYPE_DHT11, DHT11_GPIO, &humidity, &temperature);
        
        if (result == ESP_OK)
        {
            ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", temperature, humidity);
            
            // Show sensor data on display
            ssd1306_print_str(0, 0, t->temperature, false);
            ssd1306_print_str(0, 20, t->humidity, false);
            
            // Convert float to string for display
            char temp_str[16];
            char hum_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);
            snprintf(hum_str, sizeof(hum_str), "%.1f %%", humidity);
            
            ssd1306_print_str(0, 10, temp_str, false);
            ssd1306_print_str(0, 30, hum_str, false);
            
            // Show status
            ssd1306_print_str(0, 50, t->status_ok, false);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to read DHT11 sensor, error: %s", esp_err_to_name(result));
            
            // Show error on display
            ssd1306_print_str(0, 0, t->sensor_error, false);
            ssd1306_print_str(0, 17, t->check_wiring, false);
            ssd1306_print_str(0, 27, "GPIO 4", false);
            ssd1306_print_str(0, 37, "DHT11", false);
            ssd1306_print_str(0, 47, t->status_error, false);
        }
        
        ssd1306_display();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}