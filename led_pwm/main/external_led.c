#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

void app_main(void)
{
    // Initialize all possible LED pins for the C3 SuperMini
    const int pins[] = {2, 8, 9};
    
    for(int i=0; i<3; i++) {
        gpio_reset_pin(pins[i]);
        gpio_set_direction(pins[i], GPIO_MODE_OUTPUT);
    }

    printf("Diagnostic Blink Started...\n");

    while (1) {
        // Toggle all possible pins
        for(int i=0; i<3; i++) gpio_set_level(pins[i], 0); 
        printf("LEDs Forced ON\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        for(int i=0; i<3; i++) gpio_set_level(pins[i], 1);
        printf("LEDs Forced OFF\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}