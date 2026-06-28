#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace dander {

struct LatestReading {
    uint8_t  sensor_id;
    uint64_t epoch_ms;
    uint64_t recv_ms;
    uint16_t pm1_0_atm, pm2_5_atm, pm10_atm;
    uint16_t cnt_03, cnt_05, cnt_10, cnt_25, cnt_50, cnt_100;
};

struct SensorLocation {
    uint8_t sensor_id;
    double x_m;
    double y_m;
};

class SqliteReader {
public:
    SqliteReader() = default;
    ~SqliteReader();
    SqliteReader(const SqliteReader&) = delete;
    SqliteReader& operator=(const SqliteReader&) = delete;

    bool open(const std::string& path);
    void close();

    // Get the latest reading for each sensor in the last window_ms
    std::vector<LatestReading> latest_per_sensor(uint64_t window_ms);

    // Get the sensor location
    std::vector<SensorLocation> active_locations();

private:
    void* db_         = nullptr;
    void* select_stmt_ = nullptr;
    void* location_stmt_ = nullptr;
};

} // namespace dander
