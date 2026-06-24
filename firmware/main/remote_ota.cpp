#include "remote_ota.h"
#include "esp_https_ota.h"
#include "esp_app_desc.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_system.h"           // esp_restart
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>
#include <cerrno>

namespace {
constexpr const char* TAG = "remote_ota";
constexpr const char  kMagic[] = "OTA!";
constexpr size_t      kMagicLen = 4;

// Internal: fetch new firmware from CONFIG_DANDER_OTA_URL, compare versions,
// download + reboot on new version, return without reboot otherwise.
esp_err_t do_ota_check() {
    ESP_LOGI(TAG, "Checking OTA from %s", CONFIG_DANDER_OTA_URL);

    esp_http_client_config_t http_cfg = {};
    http_cfg.url = CONFIG_DANDER_OTA_URL;
    http_cfg.timeout_ms = 30000;

    esp_https_ota_config_t ota_cfg = {};
    ota_cfg.http_config = &http_cfg;

    esp_https_ota_handle_t handle = nullptr;
    if (esp_https_ota_begin(&ota_cfg, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "ota_begin failed (server down? skipping)");
        return ESP_FAIL;
    }

    esp_app_desc_t new_desc;
    if (esp_https_ota_get_img_desc(handle, &new_desc) != ESP_OK) {
        esp_https_ota_abort(handle);
        return ESP_FAIL;
    }

    const esp_app_desc_t* cur_desc = esp_ota_get_description();
    if (strcmp(new_desc.version, cur_desc->version) == 0) {
        ESP_LOGI(TAG, "No new firmware (%s), skipping", new_desc.version);
        esp_https_ota_abort(handle);
        return ESP_OK;
    }

    ESP_LOGI(TAG, "New version: %s (cur: %s), downloading...",
             new_desc.version, cur_desc->version);
    while (esp_https_ota_perform(handle) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {}

    if (esp_https_ota_finish(handle) != ESP_OK) {
        ESP_LOGW(TAG, "ota_finish failed, aborting");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "OTA success, rebooting");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;  // unreachable
}

void trigger_task(void*) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { ESP_LOGE(TAG, "socket failed"); vTaskDelete(nullptr); return; }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(CONFIG_DANDER_OTA_TRIGGER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ESP_LOGE(TAG, "bind failed: %d", errno);
        close(sock); vTaskDelete(nullptr); return;
    }
    ESP_LOGI(TAG, "Listening on udp/%d for OTA triggers", CONFIG_DANDER_OTA_TRIGGER_PORT);

    uint8_t buf[64];
    while (true) {
        sockaddr_in src = {};
        socklen_t srclen = sizeof(src);
        ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&src, &srclen);
        if (n < 0) { ESP_LOGW(TAG, "recvfrom errno=%d", errno); continue; }

        if (n >= 0 && static_cast<size_t>(n) == kMagicLen
            && memcmp(buf, kMagic, kMagicLen) == 0) {
            ESP_LOGI(TAG, "OTA trigger from %s, checking for update...",
                     inet_ntoa(src.sin_addr));
            // On new version: do_ota_check() reboots and never returns.
            // On same-version / failure: returns and we keep listening.
            do_ota_check();
        }
        else {
            ESP_LOGW(TAG, "Bad magic from %s (%zd bytes), ignoring",
                        inet_ntoa(src.sin_addr), n);
        }
    }
    // unreachable
    close(sock);
    vTaskDelete(nullptr);
}
} // anonymous namespace

namespace dander {

esp_err_t remote_ota_start() {
    // Stack 8192: HTTP client + TLS use lots of stack during OTA.
    BaseType_t r = xTaskCreate(trigger_task, "remote_ota",
                               8192, nullptr, 1, nullptr);
    return (r == pdPASS) ? ESP_OK : ESP_FAIL;
}

} // namespace dander
