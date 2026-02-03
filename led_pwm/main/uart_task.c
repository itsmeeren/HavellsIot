#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"

#define UART_PORT UART_NUM_0
#define BUF_SIZE 256

QueueHandle_t uart_data_queue;

void sender_task(void *pvParameters) {
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    
    while (1) {
        // Use a simple prompt
        printf("\n>>> Enter Message: ");
        fflush(stdout); 

        // 1. We wait here for data. 
        // Note: We read 1 byte at a time to build the string manually. 
        // This is more reliable in the ESP-IDF monitor.
        char line[BUF_SIZE];
        int idx = 0;

        while (idx < BUF_SIZE - 1) {
            uint8_t byte;
            int len = uart_read_bytes(UART_PORT, &byte, 1, portMAX_DELAY);
            if (len > 0) {
                if (byte == '\n' || byte == '\r') {
                    line[idx] = '\0';
                    break;
                } else {
                    line[idx++] = byte;
                    // Optional: Echo the character back so you can see what you type
                    printf("%c", byte);
                    fflush(stdout);
                }
            }
        }

        if (strlen(line) > 0) {
            printf("\n[SENDER] Sending '%s' to Queue...\n", line);
            xQueueSend(uart_data_queue, line, portMAX_DELAY);
        }
    }
}

void receiver_task(void *pvParameters) {
    char rx_buffer[BUF_SIZE];
    while (1) {
        if (xQueueReceive(uart_data_queue, &rx_buffer, portMAX_DELAY)) {
            printf("[RECEIVER] Task Invoked! I got: **%s**\n", rx_buffer);
        }
    }
}

void app_main(void) {
    // 1. Standard UART Config
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // Install driver with a buffer
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);

    // 2. Create Queue
    uart_data_queue = xQueueCreate(5, BUF_SIZE);

    // 3. Create Tasks
    xTaskCreate(sender_task, "Sender", 4096, NULL, 5, NULL);
    xTaskCreate(receiver_task, "Receiver", 4096, NULL, 5, NULL);

    // This print confirms the setup is done
    printf("\n--- System Ready. Type and hit Enter ---\n");
}