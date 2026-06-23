#include "storage/sqlite_store.h"
#include "storage/schema.h"
#include <sqlite3.h>
#include <cstdio>

namespace {
constexpr const char* kInsertSql = 
    "INSERT INTO readings ("
    "sensor_id, epoch_ms, recv_ms, "
    "pm1_0_cf1, pm2_5_cf1, pm10_cf1, "
    "pm1_0_atm, pm2_5_atm, pm10_atm, "
    "cnt_03, cnt_05, cnt_10, cnt_25, cnt_50, cnt_100) "
    "VALUES (?,?,?, ?,?,?, ?,?,?, ?,?,?,?,?,?)";
} // no name namespace

namespace dander {

SqliteStore::~SqliteStore() {close();}

void SqliteStore::close() {
    if (insert_stmt_) {
        sqlite3_finalize(static_cast<sqlite3_stmt*>(insert_stmt_));
        insert_stmt_=nullptr;
    }
    if (db_) {
        sqlite3_close(static_cast<sqlite3*>(db_));
        db_=nullptr;
    }
}

bool SqliteStore::open(const std::string& path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        fprintf(stderr, "open failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);   // sqlite 文档要求：即使 open 失败也要 close
        return false;
    }
    db_ = db;

    char* errmsg = nullptr;
    if (sqlite3_exec(db, kSqlSchema, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "schema failed: %s\n", errmsg);
        sqlite3_free(errmsg);
        close();
        return false;
    }

    sqlite3_exec(db, "PRAGMA journal_mode=WAL", nullptr, nullptr, nullptr);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, kInsertSql, -1, &stmt, nullptr) != SQLITE_OK) {
        fprintf(stderr, "prepare failed: %s\n", sqlite3_errmsg(db));
        close();
        return false;
    }
    insert_stmt_ = stmt;
    return true;
}

bool SqliteStore::insert_reading(
    uint8_t sensor_id,
    uint64_t epoch_ms,
    uint64_t recv_ms,
    const PMS7003MPacket& pms
) {
    auto* stmt = static_cast<sqlite3_stmt*>(insert_stmt_);

    sqlite3_bind_int  (stmt,  1, sensor_id);
    sqlite3_bind_int64(stmt,  2, static_cast<sqlite3_int64>(epoch_ms));
    sqlite3_bind_int64(stmt,  3, static_cast<sqlite3_int64>(recv_ms));
    sqlite3_bind_int  (stmt,  4, pms.pm1_0_cf1);
    sqlite3_bind_int  (stmt,  5, pms.pm2_5_cf1);
    sqlite3_bind_int  (stmt,  6, pms.pm10_cf1);
    sqlite3_bind_int  (stmt,  7, pms.pm1_0_atm);
    sqlite3_bind_int  (stmt,  8, pms.pm2_5_atm);
    sqlite3_bind_int  (stmt,  9, pms.pm10_atm);
    sqlite3_bind_int  (stmt, 10, pms.count_0_3um);
    sqlite3_bind_int  (stmt, 11, pms.count_0_5um);
    sqlite3_bind_int  (stmt, 12, pms.count_1_0um);
    sqlite3_bind_int  (stmt, 13, pms.count_2_5um);
    sqlite3_bind_int  (stmt, 14, pms.count_5_0um);
    sqlite3_bind_int  (stmt, 15, pms.count_10um);

    int rc = sqlite3_step(stmt);
    sqlite3_reset(stmt);   //  must reset, otherwise next step will fail

    return rc == SQLITE_DONE;
}
} // namespace dander