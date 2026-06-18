#pragma once

#include <cstdint>

namespace dander::sensor {

struct PMS7003MPacket {
    /**
     * @brief Particulate matter concentration in CF1 mode
     */
    uint16_t pm1_0_cf1;
    uint16_t pm2_5_cf1;
    uint16_t pm10_cf1;

    /**
     * @brief Particulate matter concentration in atmospheric mode
     */
    uint16_t pm1_0_atm;
    uint16_t pm2_5_atm;
    uint16_t pm10_atm;

    /**
     * @brief Particle count in different size ranges
     */
    uint16_t count_0_3um;
    uint16_t count_0_5um;
    uint16_t count_1_0um;
    uint16_t count_2_5um;
    uint16_t count_5_0um;
    uint16_t count_10um;
};

bool read_packet_stub();

} // namespace dander::sensor