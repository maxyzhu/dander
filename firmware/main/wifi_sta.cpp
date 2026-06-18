#include "wifi_sta.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <cstring>

/* Use WiFi configuration */

namespace {
// Constants
constexpr const char* TAG = "wifi sta";
constexpr int MAX_RETRY = 5;
/* The event group allows multiple bits for each event, but we only care about two events:
* - we are connected to the AP with an IP
* - we failed to connect after the maximum amount of retries */
constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;
constexpr EventBits_t WIFI_FAIL_BIT = BIT1;
/* FreeRTOS event group to signal when we are connected*/
EventGroupHandle_t s_wifi_event_group = nullptr;
/* Number of retries to connect to the AP */
int s_retry_num = 0;

/* Event handler for WiFi events */
void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}
} // namespace


namespace dander {
// Initialize WiFi station, connect using KConfig credentials, and block
// until an IP is obtained. Returns ESP_OK on success, error otherwise.
// Must be called after nvs_flash_init() succeeds.
esp_err_t wifi_sta_connect_blocking() {
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    // Create event group
    s_wifi_event_group = xEventGroupCreate();
    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    // Create event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // Create default WiFi station
    esp_netif_create_default_wifi_sta();
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // Register event handlers
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    // Set WiFi configuration
    wifi_config_t wifi_config = {};
    std::strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid),     CONFIG_DANDER_WIFI_SSID);
    std::strcpy(reinterpret_cast<char*>(wifi_config.sta.password), CONFIG_DANDER_WIFI_PASSWORD);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    // Set WiFi mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    // Set WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    CONFIG_DANDER_WIFI_SSID, "********");
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                    CONFIG_DANDER_WIFI_SSID, "********");
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    
    return ESP_OK;
}

}