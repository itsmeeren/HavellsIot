#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

// Define which UART port to use (UART0 is the USB port)
#define UART_PORT UART_NUM_0
#define BUF_SIZE 1024

void app_main(void)
{
    // STEP 1: Configure the UART parameters
    uart_config_t uart_config = {
        .baud_rate = 115200,             // Speed of communication
        .data_bits = UART_DATA_8_BITS,   // 8 bits per character
        .parity    = UART_PARITY_DISABLE,// No error checking bit
        .stop_bits = UART_STOP_BITS_1,   // 1 bit to signal the end
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // STEP 2: Install the driver
    // We allocate a 'Ring Buffer' of 2048 bytes to store incoming data
    uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);
    
    // STEP 3: Apply the config to the hardware
    uart_param_config(UART_PORT, &uart_config);

    // STEP 4: Set pins (UART0 default pins don't need changing on DevKit)
    uart_set_pin(UART_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Buffer to store what we read
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    printf("UART initialized. Type something in your terminal!\n");

   while (1) {
        // We read 1 byte at a time to make it feel instant
        int len = uart_read_bytes(UART_PORT, data, 5, pdMS_TO_TICKS(10));
        
        if (len > 0) {
            char received_char = data[0];

            // Print exactly what you want to see
            // \r makes sure it stays on a clean line if you're typing fast
            printf("Input: %c | ESP heard: %c\n", received_char, received_char);
            
            // Optional: You can also send it back to the UART TX line 
            // so it shows up even in basic serial tools
            uart_write_bytes(UART_PORT, (const char *) &received_char, 1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}