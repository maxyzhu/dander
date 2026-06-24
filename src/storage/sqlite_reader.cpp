#include "storage/sqlite_reader.h"
#include <sqlite3.h>
#include <cstdio>
#include <chrono>

namespace {
    constexpr const char* kSelectSql = 
    "SELECT r.*
    FROM readings r
    INNER JOIN (
        SELECT sensor_id, MAX(recv_ms) AS max_ms
        FROM readings
        WHERE recv_ms > ?
        GROUP BY sensor_id
    ) latest
        ON r.sensor_id = latest.sensor_id
        AND r.recv_ms   = latest.max_ms";
} // no name namespace


namespace dander {

SqliteReader::~SqliteReader() {close();}

// Finalize select statement and close database
void SqliteReader::close() {
    if (select_stmt_) {
        sqlite3_finalize(static_cast<sqlite3_stmt*>(select_stmt_));
        select_stmt_=nullptr;
    }
}

SqliteReader(const SqliteReader&) = delete;
SqliteReader& operator=(const SqliteReader&) = delete;

// Open database in read only mode and prepare select statement, return true on success
bool SqliteReader::open(const std::string& path) {
    sqlite3* db=nullptr;
    if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        fprintf(stderr, "SQLite Read open failted: %s\n", sqlite3_errmsg(db));
        return false;
    }
    db_=db;

    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(db, kSelectSql, -1, &stmt, nullptr) != SQLITE_OK) {
        fprintf(stderr, "SQLite Read prepare failed: %s\n", sqlite3_errmsg(db));
        return false;
    }
    select_stmt_=stmt;
    return true;
}

// Get the latest reading for each sensor in the last window_ms
std::vector<LatestReading> latest_per_sensor(uint64_t window_ms) {
    auto* stmt=static_cast<sqlite3_stmt*>(select_stmt_);
    uint64_t cutoff=now_ms()-window_ms;
    sqlite3_bind_int64(stmt, 1, cutoff);

    std::vector<LatestReading> readings;
    while (sqlite3_step(stmt)==SQLITE_ROW) {
        LatestReading reading;
        reading.sensor_id=sqlite3_column_int(stmt, 1);
        reading.epoch_ms=sqlite3_column_int64(stmt, 2);
        reading.recv_ms=sqlite3_column_int64(stmt, 3);
        reading.pm1_0_atm=sqlite3_column_int(stmt, 7);
        reading.pm2_5_atm=sqlite3_column_int(stmt, 8);
        reading.pm10_atm=sqlite3_column_int(stmt, 9);
        reading.cnt_03=sqlite3_column_int(stmt, 10);
        reading.cnt_05=sqlite3_column_int(stmt, 11);
        reading.cnt_10=sqlite3_column_int(stmt, 12);
        reading.cnt_25=sqlite3_column_int(stmt, 13);
        reading.cnt_50=sqlite3_column_int(stmt, 14);
        reading.cnt_100=sqlite3_column_int(stmt, 15);
        readings.push_back(reading);
    }
    sqlite3_reset(stmt);
    return readings;
}

// Get the current time in milliseconds
uint64_t now_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}
} // namespace dander