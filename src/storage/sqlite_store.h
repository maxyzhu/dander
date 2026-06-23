#pragma once
#include "dander/pms7003m_parse.h"
#include <cstdint>
#include <string>

namespace dander {

class SqliteStore {
public:
    SqliteStore() = default;
    ~SqliteStore(); // RAII: Resource Acquisition Is Initialization

    // Disable copy (sqlite handle cannot be copied)
    SqliteStore(const SqliteStore&) = delete;
    SqliteStore& operator=(const SqliteStore&) = delete;

    // Open / create db file. Returns true on success.
    bool open(const std::string& path);
    void close();
    
    // Insert one reading. Returns true on success.
    bool insert_reading(
        uint8_t sensor_id,
        uint64_t epoch_ms,
        uint64_t recv_ms,
        const PMS7003MPacket& pms);

private:
    void* db_ = nullptr;          // sqlite3*，header does not expose sqlite3.h
    void* insert_stmt_ = nullptr; // sqlite3_stmt*，prepared statement
};

} // namespace dander