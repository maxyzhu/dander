#include "udp_sender.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <cstring>

namespace {
    constexpr const char* TAG = "udp sender";
    int s_sock = -1;
    sockaddr_in s_dest{};
}

namespace dander {
// One-time: open socket, resolve dest. 
// Call after WiFi up + SNTP synced.
esp_err_t udp_sender_init() {
    // Open socket
    s_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock == -1) {
        ESP_LOGE(TAG, "socket failed");
        return ESP_FAIL;
    }
    // Resolve destination address
    s_dest.sin_family = AF_INET;
    s_dest.sin_port = htons(CONFIG_DANDER_SERVER_PORT);
    s_dest.sin_addr.s_addr = inet_addr(CONFIG_DANDER_SERVER_IP);
    ESP_LOGI(TAG, "Resolved destination address: %s:%d", CONFIG_DANDER_SERVER_IP, CONFIG_DANDER_SERVER_PORT);
    return ESP_OK;
}

// Send len bytes to the configured dest. Returns ESP_OK or ESP_FAIL.
esp_err_t udp_sender_send(const uint8_t* buf, size_t len) {
    int n = sendto(s_sock, buf, len, 0,
                    reinterpret_cast<sockaddr*>(&s_dest), sizeof(s_dest));
    if (n < 0) {
        ESP_LOGE(TAG, "sendto failed: errno=%d", errno);
        return ESP_FAIL;
    }
    return ESP_OK;
}
}