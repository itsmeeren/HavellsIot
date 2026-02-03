#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Task A: The "Left Hand"
void task_one(void *pvParameters) {
    while(1) {
        printf("Task 1 is running on the CPU...\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
    }
}

// Task B: The "Right Hand"
void task_two(void *pvParameters) {
    while(1) {
        printf("Task 2 is running on the CPU...\n");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Sleep for 1 second
    }
}

void app_main(void) {       
    printf("Starting the Scheduler...\n");

    // Create Task 1
    xTaskCreate(
        task_one,     // Function name
        "Task_One",   // Name for debugging
        2048,         // Stack size (RAM)
        NULL,         // Parameter (not used here)
        5,            // Priority
        NULL          // Task handle (not used here)
    );

    // Create Task 2
    xTaskCreate(
        task_two, 
        "Task_Two", 
        2048, 
        NULL, 
        5, 
        NULL
    );

    printf("app_main is finished. The tasks are now on their own!\n");
}