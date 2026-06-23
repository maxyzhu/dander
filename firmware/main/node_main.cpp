#include "dander/pms7003m_parse.h"
#include "dander/udp_packet.h"
#include "ota_update.h"
#include "wifi_sta.h"
#include "time_sync.h"
#include "udp_sender.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include <cstring>

 static const char* TAG = "dander";

 // Visualized stats for parsing performance
 struct Stats {
    uint32_t ok=0;
    uint32_t no_header=0;
    uint32_t checksum_mismatch=0;
};
 
 extern "C" void app_main(void)
 {
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Connect to WiFi
    ESP_ERROR_CHECK(dander::wifi_sta_connect_blocking());
    ESP_LOGI(TAG, "WiFi connected");
    // Check for OTA update, LOGI is included.
    dander::ota_check_and_update();
    // Synchronize time
    ESP_ERROR_CHECK(dander::time_sync_blocking());
    ESP_LOGI(TAG, "Time synced, epoch_ms=%llu", (unsigned long long)dander::now_epoch_ms());
    // Initialize UDP sender
    ESP_ERROR_CHECK(dander::udp_sender_init());
    ESP_LOGI(TAG, "UDP sender initialized");
    // Initialize UART
    uart_config_t cfg = {}; // default init
    cfg.baud_rate  = 9600;
    cfg.data_bits  = UART_DATA_8_BITS;
    cfg.parity     = UART_PARITY_DISABLE;
    cfg.stop_bits  = UART_STOP_BITS_1;
    cfg.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;
    uart_param_config(UART_NUM_2, &cfg); // UART2 is the default serial port for the ESP32
    uart_set_pin(
        UART_NUM_2,
        17, // TX
        16, // RX
        UART_PIN_NO_CHANGE, // RTS
        UART_PIN_NO_CHANGE // CTS
    );
    uart_driver_install(
        UART_NUM_2,
        1024, // RX buffer size
        0, // TX buffer size
        0, // event queue size
        NULL, // queue handle
        0 // intr_alloc_flags
    );
    // Application Buffer: Read from UART
    uint8_t buf[256];
    size_t buf_len=0;
    // Visualized stats for parsing performance
    Stats stats;
    int64_t last_stats=0;
    // Main loop: Read from UART, parse packets, and visualize stats
    while (true) {
        // Application Buffer: Read from UART
        int n=uart_read_bytes(
            UART_NUM_2, 
            buf+buf_len, 
            sizeof(buf)-buf_len, 
            pdMS_TO_TICKS(1000)
        );
        if (n>0) buf_len+=n;
        // Parse packets from the application buffer
        while (true) {
            dander::PMS7003MPacket pkt;
            size_t consumed=0;
            auto r=dander::try_parse_packet(buf, buf_len, &pkt, &consumed);
            if (r==dander::ParseResult::NeedMoreData) break;
            // Create a raw packet for socket send before memmove
            uint8_t raw_payload[dander::kSensorPayloadSize];
            bool ok_packet = (r == dander::ParseResult::Ok);
            if (ok_packet) std::memcpy(raw_payload, buf+4, dander::kSensorPayloadSize);
            // Move the remaining data to the beginning of the buffer
            memmove(buf, buf+consumed, buf_len-consumed);
            buf_len-=consumed;
            // Update stats
            switch (r) {
                case dander::ParseResult::Ok:
                    stats.ok++;
                    // Send packet to the server per frame
                    uint8_t udp_buf[dander::kPacketSize];
                    dander::pack_packet(
                        CONFIG_DANDER_SENSOR_ID,
                        dander::now_epoch_ms(),
                        raw_payload,
                        udp_buf
                    );
                    dander::udp_sender_send(udp_buf, dander::kPacketSize);
                    // Visualize valid packets
                    ESP_LOGI(TAG, "PM2.5=%u PM10=%u | counts: 0.3=%u 1.0=%u 2.5=%u",
                        pkt.pm2_5_atm, pkt.pm10_atm,
                        pkt.count_0_3um, pkt.count_1_0um, pkt.count_2_5um);
                    break;
                case dander::ParseResult::NoFrameHeader:
                    stats.no_header++;
                    break;
                case dander::ParseResult::ChecksumMismatch:
                    stats.checksum_mismatch++;
                    break;
                default:
                    break;
            }   
        }
        // Visualize stats periodically
        int64_t now=esp_timer_get_time();
        if (now-last_stats>10*1000*1000) {
            ESP_LOGI(TAG, "Stats: ok=%u no_header=%u bad_cs=%u",
                    stats.ok, stats.no_header, stats.checksum_mismatch);
            last_stats=now;
        }
    }
 }
 