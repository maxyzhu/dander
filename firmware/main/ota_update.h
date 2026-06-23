#pragma once
#include "esp_err.h"

namespace dander {

// Try to fetch new firmware from CONFIG_DANDER_OTA_URL.
// On success: writes to inactive slot, marks bootable, reboots (function does not return).
// On failure: returns esp_err_t, caller decides whether to continue boot.
esp_err_t ota_check_and_update();

} // namespace dander