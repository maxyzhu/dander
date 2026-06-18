#pragma once
#include <cstdint>
#include <cstddef>
#include "dander/byte_order.h"

namespace dander {
// One packet of data from the PMS7003M sensor is 32 bytes.
struct PMS7003MPacket {
    uint16_t pm1_0_cf1, pm2_5_cf1, pm10_cf1;
    uint16_t pm1_0_atm, pm2_5_atm, pm10_atm;
    uint16_t count_0_3um, count_0_5um, count_1_0um, count_2_5um, count_5_0um, count_10um;
};

enum class ParseResult {
    Ok,
    NeedMoreData, // Not enough data to parse a packet yet.
    NoFrameHeader, // Can't found 0x42, 0x4d in the data.
    ChecksumMismatch, // The checksum is invalid.
};

inline void parse_data(const uint8_t*buf, PMS7003MPacket*out) {
    out->pm1_0_cf1=read_be16(buf);
    out->pm2_5_cf1=read_be16(buf+2);
    out->pm10_cf1=read_be16(buf+4);
    out->pm1_0_atm=read_be16(buf+6);
    out->pm2_5_atm=read_be16(buf+8);
    out->pm10_atm=read_be16(buf+10);
    out->count_0_3um=read_be16(buf+12);
    out->count_0_5um=read_be16(buf+14);
    out->count_1_0um=read_be16(buf+16);
    out->count_2_5um=read_be16(buf+18);
    out->count_5_0um=read_be16(buf+20);
    out->count_10um=read_be16(buf+22);
}

// Main function to parse a packet from PMS7003M sensor.
// @param in: buf (byte buffer), len (length of the buffer)
// @param out: res (result of the parse), consumed (number of bytes consumed)
// @return: ParsedResult
// inline to avoid linker errors
inline ParseResult try_parse_packet(
    const uint8_t* buf, size_t len,
    PMS7003MPacket* out, size_t* consumed) {
    // length check
    if (len < 32) {*consumed=0; return ParseResult::NeedMoreData;}
    // frame header check
    if (buf[0]!=0x42 || buf[1]!=0x4d) {*consumed=1; return ParseResult::NoFrameHeader;}
    // checksum check
    uint16_t checksum=0;
    for (int i=0; i<30; i++) checksum+=buf[i];
    uint16_t expected=read_be16(buf+30);
    if (checksum!=expected) {
        *consumed=1;
        return ParseResult::ChecksumMismatch;
    }
    // parse data
    parse_data(buf+4, out);
    *consumed=32;
    return ParseResult::Ok;
}

} // namespace dander