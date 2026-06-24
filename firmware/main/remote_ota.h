#pragma once
#include "esp_err.h"

namespace dander {
// Start a background task that listens for UDP "OTA!" packets on
// CONFIG_DANDER_OTA_TRIGGER_PORT. On receipt, fetches firmware from
// CONFIG_DANDER_OTA_URL, compares version, and downloads + reboots
// if newer. WiFi is reused (no full reboot before download).
// Returns ESP_OK if task started.
esp_err_t remote_ota_start();

} // namespace dander
