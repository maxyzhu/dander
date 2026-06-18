#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "dander/byte_order.h"

namespace dander {

inline constexpr uint8_t kMagic0=0x44, kMagic1=0x4E; // "DN"
inline constexpr uint8_t kVersion=0x01;
inline constexpr size_t kPacketSize=36;
inline constexpr size_t kSensorPayloadSize=24;

struct UdpPacket {
    uint8_t version; // version of the packet format
    uint8_t sensor_id; // id of the sensor
    uint64_t epoch_ms; // milliseconds since epoch
    uint8_t sensor_payload[kSensorPayloadSize]; // payload from the sensor
};

enum class UnpackResult {
    Ok,
    BadMagic,
    BadVersion,
    BadSensorId,
    BadSize,
};

inline void pack_packet(
    uint8_t sensor_id,
    uint64_t epoch_ms,
    const uint8_t* sensor_payload, // 24 bytes
    uint8_t* out // 36 bytes
) {
    out[0]=kMagic0; out[1]=kMagic1;
    out[2]=kVersion;
    out[3]=sensor_id;
    write_be64(out+4, epoch_ms);
    std::memcpy(out+12, sensor_payload, kSensorPayloadSize);
}

inline UnpackResult unpack_packet(
    const uint8_t* buf, // 36 bytes
    size_t len,
    UdpPacket* out
) {
    if (len != kPacketSize) return UnpackResult::BadSize;
    if (buf[0]!=kMagic0 || buf[1]!=kMagic1) return UnpackResult::BadMagic;
    if (buf[2]!=kVersion) return UnpackResult::BadVersion;
    if (buf[3]<1 || buf[3]>8) return UnpackResult::BadSensorId;
    out->version=buf[2];
    out->sensor_id=buf[3];
    out->epoch_ms=read_be64(buf+4);
    std::memcpy(out->sensor_payload, buf+12, kSensorPayloadSize);
    return UnpackResult::Ok;
}
} // namespace dander