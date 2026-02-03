#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define LED_PIN 2

// HTML code for the webpage
const char* html_page = 
    "<html>"
    "<head><title>  ESP32 LED Control</title></head>"
    "<body>"
    "<h1> Karthik's personalized led </h1>"
    "<p><a href=\"/led/on\"><button style=\"height:50px;width:100px;background-color:green;color:white;\">TURN ON</button></a></p>"
    "<p><a href=\"/led/off\"><button style=\"height:50px;width:100px;background-color:red;color:white;\">TURN OFF</button></a></p>"
    "</body>"
    "</html>";

// --- HTTP HANDLER: When you just open the page ---
esp_err_t get_handler(httpd_req_t *req) {
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// --- HTTP HANDLER: When you click TURN ON ---
esp_err_t on_handler(httpd_req_t *req) {
    gpio_set_level(LED_PIN, 1);
    printf("LED Turned ON!\n");
    // Send the page back so the user stays on the interface
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// --- HTTP HANDLER: When you click TURN OFF ---
esp_err_t off_handler(httpd_req_t *req) {
    gpio_set_level(LED_PIN, 0);
    printf("LED Turned OFF!\n");
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Registering the URIs (The "Address" on the web server)
httpd_uri_t uri_get = { .uri = "/", .method = HTTP_GET, .handler = get_handler };
httpd_uri_t uri_on  = { .uri = "/led/on", .method = HTTP_GET, .handler = on_handler };
httpd_uri_t uri_off = { .uri = "/led/off", .method = HTTP_GET, .handler = off_handler };

void start_webserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_on);
        httpd_register_uri_handler(server, &uri_off);
    }
}

// --- Standard AP Setup Logic (Simplified for brevity) ---
void app_main(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    wifi_config_t wifi_config = {
        .ap = { .ssid = "karthik_esp", .password = "12345678", .max_connection = 4, .authmode = WIFI_AUTH_WPA2_PSK },
    };
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    // Setup GPIO
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // Start Server
    start_webserver();
    printf("Web Server Started! Connect to 'My_ESP32_Control' and visit 192.168.4.1\n");
}