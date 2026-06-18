#pragma once
#include <cstdint>
#include "esp_err.h"

namespace dander {

// Initialize SNTP, kick off sync, and block until first sync completes
// (or timeout). Call AFTER WiFi is up.
esp_err_t time_sync_blocking(uint32_t timeout_ms = 30000);

// Current Unix epoch in ms. Returns 0 (or near-0) before first sync.
uint64_t now_epoch_ms();

} // namespace dander