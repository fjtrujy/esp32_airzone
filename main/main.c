#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

static const char *TAG = "DHT11_TEST";

#define CONFIG_DHT11_PIN GPIO_NUM_5
#define CONFIG_CONNECTION_TIMEOUT 5

void app_main() {
    ESP_LOGI(TAG, "Starting DHT11 test on GPIO %d", CONFIG_DHT11_PIN);

    // Read data using the espidflib DHT library
    while(1)
    {
        float humidity = 0.0f;
        float temperature = 0.0f;
        
        esp_err_t result = dht_read_float_data(DHT_TYPE_DHT11, CONFIG_DHT11_PIN, &humidity, &temperature);
        
        if(result == ESP_OK)
        {  
            ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", temperature, humidity);
        }
        else {
            ESP_LOGE(TAG, "Failed to read DHT11 sensor, error: %s", esp_err_to_name(result));
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    } 
}