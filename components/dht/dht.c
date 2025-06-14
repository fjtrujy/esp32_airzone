#include "dht.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DHT";

#define DHT_TIMEOUT_MS 1000
#define DHT_SAMPLE_PERIOD_MS 2000

esp_err_t dht_read_float_data(dht_type_t type, gpio_num_t pin, float* humidity, float* temperature)
{
    uint8_t data[5] = {0};
    uint8_t i, j;
    uint32_t timeout;
    uint32_t start_time;
    
    // Configure GPIO as output
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << pin),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // Send start signal
    gpio_set_level(pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));  // At least 18ms for DHT11
    gpio_set_level(pin, 1);
    vTaskDelay(pdMS_TO_TICKS(1));   // 20-40us
    
    // Configure GPIO as input with pull-up
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    // Wait for DHT response
    timeout = 0;
    while (gpio_get_level(pin) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout++;
        if (timeout > 100) {
            ESP_LOGE(TAG, "DHT response timeout");
            return ESP_ERR_TIMEOUT;
        }
    }
    
    // Wait for low signal
    timeout = 0;
    while (gpio_get_level(pin) == 0) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout++;
        if (timeout > 100) {
            ESP_LOGE(TAG, "DHT low signal timeout");
            return ESP_ERR_TIMEOUT;
        }
    }
    
    // Wait for high signal
    timeout = 0;
    while (gpio_get_level(pin) == 1) {
        vTaskDelay(pdMS_TO_TICKS(1));
        timeout++;
        if (timeout > 100) {
            ESP_LOGE(TAG, "DHT high signal timeout");
            return ESP_ERR_TIMEOUT;
        }
    }
    
    // Read 40 bits of data
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            // Wait for low signal
            timeout = 0;
            while (gpio_get_level(pin) == 0) {
                vTaskDelay(pdMS_TO_TICKS(1));
                timeout++;
                if (timeout > 100) {
                    ESP_LOGE(TAG, "Data low signal timeout");
                    return ESP_ERR_TIMEOUT;
                }
            }
            
            // Measure high signal duration
            start_time = esp_timer_get_time() / 1000;  // Convert to us
            timeout = 0;
            while (gpio_get_level(pin) == 1) {
                vTaskDelay(pdMS_TO_TICKS(1));
                timeout++;
                if (timeout > 100) {
                    ESP_LOGE(TAG, "Data high signal timeout");
                    return ESP_ERR_TIMEOUT;
                }
            }
            
            uint32_t duration = (esp_timer_get_time() / 1000) - start_time;
            
            // Determine bit value based on duration
            if (duration > 50) {
                data[i] |= (1 << (7 - j));
            }
        }
    }
    
    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGE(TAG, "Checksum error: %d != %d", checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }
    
    // Convert data to temperature and humidity
    if (type == DHT_TYPE_DHT11) {
        *humidity = (float)data[0] + (float)data[1] / 10.0;
        *temperature = (float)data[2] + (float)data[3] / 10.0;
    } else {
        // DHT22
        *humidity = ((data[0] << 8) | data[1]) / 10.0;
        *temperature = ((data[2] & 0x7F) << 8 | data[3]) / 10.0;
        if (data[2] & 0x80) {
            *temperature = -(*temperature);
        }
    }
    
    ESP_LOGI(TAG, "DHT read successful: T=%.1f°C, H=%.1f%%", *temperature, *humidity);
    return ESP_OK;
} 