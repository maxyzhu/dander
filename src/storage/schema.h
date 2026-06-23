#pragma once

namespace dander {
inline constexpr const char* kSqlSchema = R"SQL(
CREATE TABLE IF NOT EXISTS sensors (
    sensor_id    INTEGER PRIMARY KEY,
    name         TEXT,
    install_date TEXT
);

CREATE TABLE IF NOT EXISTS sensor_locations (
    sensor_id   INTEGER NOT NULL,
    x           REAL,
    y           REAL,
    valid_from  INTEGER NOT NULL,
    valid_to    INTEGER,
    PRIMARY KEY (sensor_id, valid_from)
);

CREATE TABLE IF NOT EXISTS readings (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    sensor_id   INTEGER NOT NULL,
    epoch_ms    INTEGER NOT NULL,
    recv_ms     INTEGER NOT NULL,
    pm1_0_cf1   INTEGER, pm2_5_cf1 INTEGER, pm10_cf1  INTEGER,
    pm1_0_atm   INTEGER, pm2_5_atm INTEGER, pm10_atm  INTEGER,
    cnt_03      INTEGER, cnt_05    INTEGER, cnt_10    INTEGER,
    cnt_25      INTEGER, cnt_50    INTEGER, cnt_100   INTEGER,
    flags       INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_readings_sensor_time
    ON readings(sensor_id, epoch_ms);
CREATE INDEX IF NOT EXISTS idx_loc_lookup
    ON sensor_locations(sensor_id, valid_from);
)SQL";
} // namespace dander