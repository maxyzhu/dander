#pragma once
#include <cstdint>
#include <cstddef>
#include "esp_err.h"

namespace dander {

// One-time: open socket, resolve dest. 
// Call after WiFi up + SNTP synced.
esp_err_t udp_sender_init();

// Send len bytes to the configured dest. Returns ESP_OK or ESP_FAIL.
esp_err_t udp_sender_send(const uint8_t* buf, size_t len);

} // namespace dander