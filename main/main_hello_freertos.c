#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

void hello_task(void *pvParameter)
{
    while (1)
    {
        printf("Hello World from FreeRTOS task!\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // delay 1 second
    }
}

void app_main(void)
{
    printf("Starting FreeRTOS task demo...\n");

    // Create a new FreeRTOS task
    xTaskCreate(
        hello_task,     // Task function
        "hello_task",   // Name of the task
        2048,           // Stack size in words
        NULL,           // Task input parameter
        5,              // Priority
        NULL            // Task handle
    );
}
