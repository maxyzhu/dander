#pragma once
#include "esp_err.h"

namespace dander {
// Initialize WiFi station, connect using KConfig credentials, and block
// until an IP is obtained. Returns ESP_OK on success, error otherwise.
// Must be called after nvs_flash_init() succeeds.
esp_err_t wifi_sta_connect_blocking();
}