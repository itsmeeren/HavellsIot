#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "esp_netif.h"
#include "esp_sntp.h"

/* ================= USER CONFIG ================= */
#define WIFI_SSID "Igris"
#define WIFI_PASS "karthikeren"
/* =============================================== */

static const char *TAG = "HTTPS_FLOW";

/* Wi-Fi sync */
static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

/* ============================================================
   1️⃣ WIFI EVENT HANDLER
   ============================================================ */
static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, retrying...");
        esp_wifi_connect();
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ============================================================
   2️⃣ WIFI INIT (STA MODE)
   ============================================================ */
static void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_default_netif(netif);   // Force IPv4 routing

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT,
                                             ESP_EVENT_ANY_ID,
                                             &wifi_event_handler,
                                             NULL,
                                             NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT,
                                             IP_EVENT_STA_GOT_IP,
                                             &wifi_event_handler,
                                             NULL,
                                             NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    esp_wifi_set_ps(WIFI_PS_NONE); // VERY important on ESP32-C3

    ESP_ERROR_CHECK(esp_wifi_start());

    /* Block until connected */
    xEventGroupWaitBits(wifi_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);
}

/* ============================================================
   3️⃣ TIME SYNC (SNTP)
   ============================================================ */
static void obtain_time_blocking(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now;
    struct tm timeinfo = {0};
    int retry = 0;

    while (timeinfo.tm_year < (2016 - 1900) && retry < 20) {
        ESP_LOGI(TAG, "Waiting for time sync...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
        retry++;
    }

    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGE(TAG, "Time sync failed");
    } else {
        ESP_LOGI(TAG, "Time synced successfully");
    }
}

/* ============================================================
   4️⃣ HTTPS EVENT HANDLER
   ============================================================ */
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        printf("%.*s", evt->data_len, (char *)evt->data);
    }
    return ESP_OK;
}

/* ============================================================
   5️⃣ HTTPS TASK
   ============================================================ */
static void https_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting HTTPS request");

    esp_http_client_config_t config = {
        .url = "https://google.com",
        .event_handler = http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .timeout_ms = 20000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS SUCCESS");
    } else {
        ESP_LOGE(TAG, "HTTPS FAILED: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

/* ============================================================
   6️⃣ MAIN ENTRY
   ============================================================ */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    wifi_init_sta();          // Step 1
    obtain_time_blocking();   // Step 2
    xTaskCreate(https_task, "https_task", 8192, NULL, 5, NULL); // Step 3
}
