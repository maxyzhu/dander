#include "dander/udp_packet.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <initializer_list>

using namespace dander;

static void fail(const char* msg) {
    printf("FAIL: %s\n", msg);
    exit(1);
}

static void assert_true(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

static void assert_unpack(UnpackResult got, UnpackResult want, const char* msg) {
    if (got != want) {
        printf("FAIL: %s (expected %d, got %d)\n", msg, (int)want, (int)got);
        exit(1);
    }
}

// Build a sample 24-byte sensor payload with recognizable bytes.
static void fill_sample_payload(uint8_t* p) {
    for (size_t i = 0; i < kSensorPayloadSize; ++i) {
        p[i] = static_cast<uint8_t>(0x10 + i);
    }
}

int main() {
    uint8_t payload[kSensorPayloadSize];
    fill_sample_payload(payload);

    // ---- 1. Round-trip ----
    {
        uint8_t buf[kPacketSize];
        const uint8_t sid = 3;
        const uint64_t ts = 0x0123456789ABCDEFull;
        pack_packet(sid, ts, payload, buf);

        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::Ok, "round-trip should be Ok");
        assert_true(out.version == kVersion, "version round-trip");
        assert_true(out.sensor_id == sid, "sensor_id round-trip");
        assert_true(out.epoch_ms == ts, "epoch_ms round-trip");
        assert_true(std::memcmp(out.sensor_payload, payload, kSensorPayloadSize) == 0,
                    "payload round-trip");
    }

    // ---- 2. Byte-order direction on the wire ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 0x0123456789ABCDEFull, payload, buf);
        // magic + version + sensor_id
        assert_true(buf[0] == 0x44 && buf[1] == 0x4E, "magic bytes on wire");
        assert_true(buf[2] == 0x01, "version on wire");
        assert_true(buf[3] == 0x01, "sensor_id on wire");
        // epoch_ms must be big-endian: MSB first
        assert_true(buf[4]  == 0x01, "epoch_ms MSB at offset 4");
        assert_true(buf[11] == 0xEF, "epoch_ms LSB at offset 11");
        // first payload byte right after timestamp
        assert_true(buf[12] == 0x10, "payload starts at offset 12");
        assert_true(buf[35] == static_cast<uint8_t>(0x10 + 23), "payload ends at offset 35");
    }

    // ---- 3. BadSize: too short ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize - 1, &out);
        assert_unpack(r, UnpackResult::BadSize, "len=35 should be BadSize");
    }

    // ---- 4. BadSize: too long ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize + 1, &out);
        assert_unpack(r, UnpackResult::BadSize, "len=37 should be BadSize");
    }

    // ---- 5. BadMagic ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        buf[0] = 0xFF;
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::BadMagic, "BadMagic on buf[0]");
    }
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        buf[1] = 0xFF;
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::BadMagic, "BadMagic on buf[1]");
    }

    // ---- 6. BadVersion ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        buf[2] = 0x02;
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::BadVersion, "BadVersion");
    }

    // ---- 7. BadSensorId: 0 (out of [1,8]) ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        buf[3] = 0;
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::BadSensorId, "sensor_id=0 should be rejected");
    }

    // ---- 8. BadSensorId: 9 (out of [1,8]) ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 1000, payload, buf);
        buf[3] = 9;
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::BadSensorId, "sensor_id=9 should be rejected");
    }

    // ---- 9. Valid sensor_id boundaries: 1 and 8 ----
    {
        for (uint8_t sid : {uint8_t(1), uint8_t(8)}) {
            uint8_t buf[kPacketSize];
            pack_packet(sid, 42, payload, buf);
            UdpPacket out{};
            auto r = unpack_packet(buf, kPacketSize, &out);
            assert_unpack(r, UnpackResult::Ok, "sid=1 or sid=8 should be accepted");
            assert_true(out.sensor_id == sid, "boundary sensor_id round-trip");
        }
    }

    // ---- 10. epoch_ms = 0 (pre-SNTP-sync sentinel) still parses OK ----
    {
        uint8_t buf[kPacketSize];
        pack_packet(1, 0, payload, buf);
        UdpPacket out{};
        auto r = unpack_packet(buf, kPacketSize, &out);
        assert_unpack(r, UnpackResult::Ok, "epoch_ms=0 should still parse");
        assert_true(out.epoch_ms == 0, "epoch_ms=0 round-trip");
    }

    printf("test_udp_packet: OK\n");
    return 0;
}
