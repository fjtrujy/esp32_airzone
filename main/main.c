#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static const char *TAG = "DHT11_TEST";

#define CONFIG_DHT11_PIN GPIO_NUM_5
#define CONFIG_CONNECTION_TIMEOUT 5

typedef struct
{
    int dht11_pin;
    float temperature;
    float humidity;
} dht11_t;

static int wait_for_state(dht11_t dht11,int state,int timeout)
{
    gpio_set_direction(dht11.dht11_pin, GPIO_MODE_INPUT);
    int count = 0;
    
    while(gpio_get_level(dht11.dht11_pin) != state)
    {
        if(count >= timeout) return -1;
        count += 2;
        ets_delay_us(2);
        
    }

    return  count;
}

static void hold_low(dht11_t dht11,int hold_time_us)
{
    gpio_set_direction(dht11.dht11_pin,GPIO_MODE_OUTPUT);
    gpio_set_level(dht11.dht11_pin,0);
    ets_delay_us(hold_time_us);
    gpio_set_level(dht11.dht11_pin,1);
}

static int dht11_read(dht11_t *dht11,int connection_timeout)
{
    int waited = 0;
    int one_duration = 0;
    int zero_duration = 0;
    int timeout_counter = 0;

    uint8_t received_data[5] =
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };

    while(timeout_counter < connection_timeout)
    {
        timeout_counter++;
        gpio_set_direction(dht11->dht11_pin,GPIO_MODE_INPUT);
        hold_low(*dht11, 18000);
        
        waited = wait_for_state(*dht11,0,40);

        if(waited == -1)
        {
            ESP_LOGE(TAG,"Failed at phase 1");
            ets_delay_us(20000);
            continue;
        } 


        waited = wait_for_state(*dht11,1,90);
        if(waited == -1)
        {
            ESP_LOGE(TAG,"Failed at phase 2");
            ets_delay_us(20000);
            continue;
        } 
        
        waited = wait_for_state(*dht11,0,90);
        if(waited == -1)
        {
            ESP_LOGE(TAG,"Failed at phase 3");
            ets_delay_us(20000);
            continue;
        } 
        break;
        
    }
    
    if(timeout_counter == connection_timeout) return -1;

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            zero_duration = wait_for_state(*dht11,1,58);
            one_duration = wait_for_state(*dht11,0,74);
            received_data[i] |= (one_duration > zero_duration) << (7 - j);
        }
    }
    int crc = received_data[0]+received_data[1]+received_data[2]+received_data[3];
    crc = crc & 0xff;
    if(crc == received_data[4]) {
      dht11->humidity = received_data[0] + received_data[1] / 10.0;
      dht11->temperature = received_data[2] + received_data[3] / 10.0;
      return 0;
    }
    else {
        ESP_LOGE(TAG, "Wrong checksum");
        return -1;
    }
}

void app_main() {
    dht11_t dht11_sensor;
    dht11_sensor.dht11_pin = CONFIG_DHT11_PIN;

    ESP_LOGI(TAG, "Starting DHT11 test on GPIO %d", CONFIG_DHT11_PIN);

    // Read data
    while(1)
    {
      if(!dht11_read(&dht11_sensor, CONFIG_CONNECTION_TIMEOUT))
      {  
        ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", dht11_sensor.temperature, dht11_sensor.humidity);
      }
      else {
        ESP_LOGE(TAG, "Failed to read DHT11 sensor");
      }
      vTaskDelay(pdMS_TO_TICKS(2000));
    } 
}