#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <sys/time.h>     // gettimeofday

namespace dander {
// Initialize SNTP, kick off sync, and block until first sync completes
// (or timeout). Call AFTER WiFi is up.
esp_err_t time_sync_blocking(uint32_t timeout_ms) {
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.sync_cb = nullptr;
    ESP_ERROR_CHECK(esp_netif_sntp_init(&config));

    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(timeout_ms)) != ESP_OK) {
        return ESP_ERR_TIMEOUT;
    }
    return ESP_OK;
}

// Current Unix epoch in ms. Returns 0 (or near-0) before first sync.
uint64_t now_epoch_ms() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return uint64_t(tv.tv_sec) * 1000ull + uint64_t(tv.tv_usec) / 1000ull;
}

} // namespace dander