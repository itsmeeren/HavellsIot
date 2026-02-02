#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#define BOOT_BUTTON_PIN 9
#define UART_PORT UART_NUM_0
#define MY_STRING "hello"

// Handle for our communication pipe
QueueHandle_t uart_queue;

// --- TASK 1: THE UART "WORKER" ---
// This task is INVOKED only when the queue receives data
void uart_worker_task(void *pvParameters) {
    char char_to_print;
    while (1) {
        // Wait indefinitely (portMAX_DELAY) for a character to arrive in the queue
        if (xQueueReceive(uart_queue, &char_to_print, portMAX_DELAY)) {
            // This code only runs when the button is pressed
            printf("\n[UART TASK] Button pressed! Sending: %c\n", char_to_print);
            
            // Send it physically via UART
            uart_write_bytes(UART_PORT, &char_to_print, 1);
        }
    }
}

// --- TASK 2: THE BUTTON "MONITOR" ---
void button_monitor_task(void *pvParameters) {
    int char_index = 0;
    int string_len = strlen(MY_STRING);
    int last_state = 1;

    while (1) {
        int current_state = gpio_get_level(BOOT_BUTTON_PIN);

        // Detect the moment the button is pushed down (Falling Edge)
        if (last_state == 1 && current_state == 0) {
            char data_to_send = MY_STRING[char_index];

            // Pushing data to the queue triggers/invokes the UART task
            xQueueSend(uart_queue, &data_to_send, portMAX_DELAY);

            // Increment index for the next press
            char_index = (char_index + 1) % string_len;

            // Simple debounce delay
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10)); // Tiny sleep to allow other tasks to run
    }
}

void app_main(void) {
    // 1. Hardware Init: UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART_PORT, 2048, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);

    // 2. Hardware Init: BOOT Button
    gpio_reset_pin(BOOT_BUTTON_PIN);
    gpio_set_direction(BOOT_BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOOT_BUTTON_PIN, GPIO_PULLUP_ONLY);

    // 3. Create the Communication Queue
    uart_queue = xQueueCreate(5, sizeof(char));

    // 4. Create the Tasks
    // Task 1: The worker (Higher priority so it responds instantly)
    xTaskCreate(uart_worker_task, "UART_Worker", 2048, NULL, 10, NULL);
    
    // Task 2: The monitor
    xTaskCreate(button_monitor_task, "Button_Monitor", 2048, NULL, 5, NULL);

    printf("Tasks initialized. Press the BOOT button to invoke the UART task!\n");
}