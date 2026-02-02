#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"

#define LED_GPIO 8

void app_main(void)
{
    // 1. Setup the RGB Driver
    led_strip_handle_t led_strip;
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, 
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    printf("Starting Visible Brightness Test...\n");

    while (1) {
        // VERY DIM RED
        printf("Brightness: 2 (Very Dim)\n");
        led_strip_set_pixel(led_strip, 0, 2, 0, 0); // Red = 2
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // MEDIUM RED
        printf("Brightness: 50 (Medium)\n");
        led_strip_set_pixel(led_strip, 0, 50, 0, 0); // Red = 50
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // FULL RED
        printf("Brightness: 255 (Max)\n");
        led_strip_set_pixel(led_strip, 0, 255, 0, 0); // Red = 255
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}