#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"

#define TAG "FREERTOS_DEMO"

QueueHandle_t queue;
int queueSize = 10;

// Semaphore test
int nTasks = 5;
SemaphoreHandle_t barrierSemaphore;

// Function declarations
void taskOne(void *pvParameter);
void taskTwo(void *pvParameter);
void taskPinned(void *pvParameter);
void taskSender(void *pvParameter);
void taskReceiver(void *pvParameter);
void taskSemaphore(void *pvParameter);

void app_main(void)
{
    ESP_LOGI(TAG, "FreeRTOS Task Demo Start");
    ESP_LOGI(TAG, "Setup running on core %d", xPortGetCoreID());
    ESP_LOGI(TAG, "Setup priority = %lu", (unsigned long)uxTaskPriorityGet(NULL));

    // Initialize semaphore
    barrierSemaphore = xSemaphoreCreateCounting(nTasks, 0);
    if (barrierSemaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create counting semaphore");
        return;
    }

    // Launch semaphore test tasks
    for (int i = 0; i < nTasks; i++) {
        xTaskCreatePinnedToCore(
            taskSemaphore,
            "TaskSemaphore",
            2048,
            (void *)(intptr_t)i,
            1,
            NULL,
            0
        );
    }

    // Wait for all semaphore tasks to signal
    for (int i = 0; i < nTasks; i++) {
        xSemaphoreTake(barrierSemaphore, portMAX_DELAY);
    }

    ESP_LOGI(TAG, "All %d tasks launched and semaphore barrier passed.", nTasks);

    // Create main example tasks
    TaskHandle_t taskOneHandler, taskTwoHandler;
    xTaskCreate(taskOne, "TaskOne", 2048, NULL, 10, &taskOneHandler);
    xTaskCreate(taskTwo, "TaskTwo", 2048, NULL, 11, &taskTwoHandler);

    ESP_LOGI(TAG, "taskOne priority = %lu", (unsigned long)uxTaskPriorityGet(taskOneHandler));
    ESP_LOGI(TAG, "taskTwo priority = %lu", (unsigned long)uxTaskPriorityGet(taskTwoHandler));

    // Task pinned to a specific core
    xTaskCreatePinnedToCore(
        taskPinned,
        "TaskPinned",
        2048,
        NULL,
        5,
        NULL,
        0
    );

    // Create queue for inter-task communication
    queue = xQueueCreate(queueSize, sizeof(int));
    if (queue == NULL) {
        ESP_LOGE(TAG, "Error creating queue");
        return;
    }

    // Create sender and receiver tasks
    xTaskCreate(taskSender, "TaskSender", 2048, NULL, 4, NULL);
    xTaskCreate(taskReceiver, "TaskReceiver", 2048, NULL, 4, NULL);

    // app_main can end, scheduler runs tasks
}

/* ------------------- Task Definitions ------------------- */

void taskOne(void *pvParameter)
{
    ESP_LOGI(TAG, "taskOne running on core %d", xPortGetCoreID());
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Hello from taskOne [%d]", i);
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
    ESP_LOGI(TAG, "Ending taskOne");
    vTaskDelete(NULL);
}

void taskTwo(void *pvParameter)
{
    ESP_LOGI(TAG, "taskTwo running on core %d", xPortGetCoreID());
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Hello from taskTwo [%d]", i);
        vTaskDelay(pdMS_TO_TICKS(1300));
    }
    ESP_LOGI(TAG, "Ending taskTwo");
    vTaskDelete(NULL);
}

void taskPinned(void *pvParameter)
{
    ESP_LOGI(TAG, "taskPinned running on core %d", xPortGetCoreID());
    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Hello from taskPinned [%d]", i);
        vTaskDelay(pdMS_TO_TICKS(1700));
    }
    ESP_LOGI(TAG, "Ending taskPinned");
    vTaskDelete(NULL);
}

void taskSender(void *pvParameter)
{
    ESP_LOGI(TAG, "taskSender running");
    for (int i = 0; i < 10; i++) {
        xQueueSend(queue, &i, portMAX_DELAY);
        ESP_LOGI(TAG, "Sent value %d", i);
    }
    ESP_LOGI(TAG, "Ending taskSender");
    vTaskDelete(NULL);
}

void taskReceiver(void *pvParameter)
{
    ESP_LOGI(TAG, "taskReceiver running");
    int element;
    for (int i = 0; i < 10; i++) {
        xQueueReceive(queue, &element, portMAX_DELAY);
        ESP_LOGI(TAG, "Received: %d", element);
    }
    ESP_LOGI(TAG, "Ending taskReceiver");
    vTaskDelete(NULL);
}

void taskSemaphore(void *pvParameter)
{
    int taskNum = (int)(intptr_t)pvParameter;
    ESP_LOGI(TAG, "taskSemaphore executed number: %d", taskNum);
    xSemaphoreGive(barrierSemaphore);
    vTaskDelete(NULL);
}
