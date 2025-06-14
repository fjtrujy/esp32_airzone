#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    int counter = 0;
    printf("ESP32 Debug Demo Starting...\n");
    
    while (1)
    {
        printf("Hello World - Counter: %d\n", counter);
        counter++;
        
        // Add a delay to make debugging easier
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 second delay
        
        // Add a condition for breakpoint testing
        if (counter % 10 == 0) {
            printf("*** Counter reached multiple of 10: %d ***\n", counter);
        }
    }
}