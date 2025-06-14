#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_0  // Boot button on most ESP32 dev boards
#define LED_GPIO    GPIO_NUM_2  // Built-in LED on most ESP32 dev boards

static const char *TAG = "ESP32_BUTTON";

// Queue for button events
static QueueHandle_t button_queue = NULL;

// Button interrupt handler
static void IRAM_ATTR button_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_queue, &gpio_num, NULL);
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
                ESP_LOGI(TAG, "Button pressed!");
                // Signal main task to increment counter
                xTaskNotifyGive((TaskHandle_t)arg);
            }
        }
    }
}

void app_main(void)
{
    int counter = 0;
    TaskHandle_t main_task_handle = xTaskGetCurrentTaskHandle();
    
    ESP_LOGI(TAG, "ESP32 Button Demo Starting...");
    
    // Initialize GPIO
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,  // Interrupt on falling edge
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 1,  // Enable pull-up resistor
    };
    gpio_config(&io_conf);
    
    // Configure LED GPIO
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_GPIO),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&led_conf);
    
    // Create queue for button events
    button_queue = xQueueCreate(10, sizeof(uint32_t));
    
    // Create button task
    xTaskCreate(button_task, "button_task", 2048, main_task_handle, 10, NULL);
    
    // Install GPIO ISR service
    gpio_install_isr_service(0);
    
    // Add ISR handler for button
    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, (void*) BUTTON_GPIO);
    
    ESP_LOGI(TAG, "Button initialized on GPIO %d", BUTTON_GPIO);
    ESP_LOGI(TAG, "Press the button to increment counter!");
    
    while (1)
    {
        printf("Hello World - Counter: %d\n", counter);
        
        // Wait for button press notification
        if(xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(1000)) == pdTRUE) {
            counter++;
            printf("*** Button pressed! Counter incremented to: %d ***\n", counter);
            
            // Blink LED to indicate button press
            gpio_set_level(LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED_GPIO, 0);
        }
        
        // Add a delay to make debugging easier
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay
        
        // Add a condition for breakpoint testing
        if (counter % 10 == 0 && counter > 0) {
            printf("*** Counter reached multiple of 10: %d ***\n", counter);
        }
    }
}