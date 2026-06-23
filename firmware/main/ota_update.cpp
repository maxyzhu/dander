#include "ota_update.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"   // esp_restart
#include "freertos/FreeRTOS.h"  // vTaskDelay
#include "freertos/task.h"      // vTaskDelay

namespace dander {
static const char* TAG = "ota_update";

esp_err_t ota_check_and_update() {
    ESP_LOGI(TAG, "Checking OTA from %s", CONFIG_DANDER_OTA_URL);
    // Create HTTP client, no auth for now
    esp_http_client_config_t http_cfg={};
    http_cfg.url=CONFIG_DANDER_OTA_URL;
    http_cfg.timeout_ms=30000;
    // OTA config
    esp_https_ota_config_t ota_cfg={};
    ota_cfg.http_config=&http_cfg;
    // Start OTA and error handling
    // esp_https_ota() is a blocking function, including the following steps:
    // 1. downloads new firmware to flash
    // 2. writes ota_data to nvs
    // 3. verifies ota_data is valid
    esp_err_t err=esp_https_ota(&ota_cfg); 
    if (err==ESP_OK) {
        ESP_LOGI(TAG, "OTA success, rebooting in 1s...");
        // Wait nvs flush before restarting
        vTaskDelay(pdMS_TO_TICKS(1000));
        // Manually restart to boot into new firmware
        esp_restart();
    }
    ESP_LOGW(TAG, "OTA finished without restart: %s", esp_err_to_name(err));
    return err;
}
} // namespace dander