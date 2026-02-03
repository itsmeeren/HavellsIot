#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MAX_SCAN_RESULTS 10

void app_main(void)
{
    // 1. Initialize NVS (Non-Volatile Storage)
    // Wi-Fi drivers need NVS to store configuration data
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Initialize TCP/IP Stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 3. Initialize Wi-Fi with Default Config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. Set Mode to Station and Start
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Starting Wi-Fi Scan...\n");

    // 5. Configure Scan Parameters
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false
    };

    // 6. Start Scan (Blocking)
    // 'true' means the function waits until the scan is finished
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    // 7. Get the Results
    uint16_t number = MAX_SCAN_RESULTS;
    wifi_ap_record_t ap_info[MAX_SCAN_RESULTS];
    uint16_t ap_count = 0;

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    printf("Scan complete! Found %d networks:\n", ap_count);
    printf("--------------------------------------------------\n");
    printf("%-32s | %-7s | %s\n", "SSID", "Channel", "RSSI");
    
    for (int i = 0; i < number; i++) {
        printf("%-32s | %-7d | %d dBm\n", (char *)ap_info[i].ssid, ap_info[i].primary, ap_info[i].rssi);
    }
    printf("--------------------------------------------------\n");
}