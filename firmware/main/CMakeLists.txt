idf_component_register(
    SRCS "node_main.cpp" "wifi_sta.cpp" "time_sync.cpp" "udp_sender.cpp"
    INCLUDE_DIRS "." "../../include"
    REQUIRES esp_driver_uart esp_timer
    PRIV_REQUIRES esp_wifi nvs_flash esp_netif esp_event lwip
)