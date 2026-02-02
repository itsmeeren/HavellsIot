#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"

// Define these here since your driver code uses CONFIG variables
#define LED_STRIP_GPIO 8
#define LED_STRIP_LEN  1

static led_strip_handle_t s_led_strip;

// Function to set the color easily
void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(s_led_strip));
}

void app_main(void)
{
    // 1. Setup the LED strip (based on your driver code)
    led_strip_config_t led_strip_conf = {
        .max_leds = LED_STRIP_LEN,
        .strip_gpio_num = LED_STRIP_GPIO,
    };
    led_strip_rmt_config_t rmt_conf = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz timing
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_conf, &rmt_conf, &s_led_strip));
    
    printf("RGB Driver Initialized on GPIO %d\n", LED_STRIP_GPIO);

    while (1) {
        printf("Red\n");
        set_led_color(255, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Green\n");
        set_led_color(0, 255, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Blue\n");
        set_led_color(0, 0, 255);
        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("Yellow (Red + Green)\n");
        set_led_color(255, 255, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}