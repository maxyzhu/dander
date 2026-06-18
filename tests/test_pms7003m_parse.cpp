#include "dander/pms7003m_parse.h"

#include <cstdio>
#include <cstdlib>

using namespace dander;

void assert_eq(int a, int b, const char* msg) {
    if (a!=b) {
        printf("Assertion failed: %s (expected %d, got %d)\n", msg, b, a);
        exit(1);
    }
}
int main() {
    // Test case 1: Valid packet
    {
        uint8_t frame[32]={
            0x42, 0x4d, 0x00, 0x1c,
            0x00, 0x05, // pm1_0_cf1
            0x00, 0x0a, // pm2_5_cf1
            0x00, 0x03, // pm10_cf1
            0x00, 0x06,
            0x00, 0x01, 
            0x00, 0x02,
            0x00, 0x04, 
            0x00, 0x08,
            0x00, 0x07, 
            0x00, 0x09,
            0x00, 0x0b, 
            0x00, 0x0c,
            0x00, 0x0d
        };
        uint16_t checksum=0;
        for (int i=0; i<30; i++) checksum+=frame[i];
        frame[30]=(checksum>>8) & 0xff;
        frame[31]=checksum & 0xff;

        PMS7003MPacket pkt;
        size_t consumed=0;
        auto r=try_parse_packet(frame, 32, &pkt, &consumed);

        assert_eq((int)r, (int)ParseResult::Ok, "ParseResult should be Ok");
        assert_eq(consumed, 32, "Consumed should be 32 bytes");
        assert_eq(pkt.pm2_5_cf1, 10, "pm2_5_cf1 should be 10");
    }
    // Test case 2: Not enough data
    {
        uint8_t frame[10]={
            0x42, 0x4d, 0x00, 0x1c,
            0x00, 0x05, // pm1_0_cf1
            0x00, 0x0a, // pm2_5_cf1
            0x00, 0x03, // pm10_cf1
        };
        PMS7003MPacket pkt;
        size_t consumed=0;
        auto r=try_parse_packet(frame, 10, &pkt, &consumed);
        assert_eq((int)r, (int)ParseResult::NeedMoreData, "ParseResult should be NeedMoreData");
    }
    // Test case 3: No frame header
    {
        uint8_t frame[32]={
            0xFF, 0xFF, 0x00, 0x1c,
            0x00, 0x05, // pm1_0_cf1
            0x00, 0x0a, // pm2_5_cf1
            0x00, 0x03, // pm10_cf1
            0x00, 0x06,
            0x00, 0x01, 
            0x00, 0x02,
            0x00, 0x04, 
            0x00, 0x08,
            0x00, 0x07, 
            0x00, 0x09,
            0x00, 0x0b, 
            0x00, 0x0c,
            0x00, 0x0d
        };
        PMS7003MPacket pkt;
        size_t consumed=0;
        auto r=try_parse_packet(frame, 32, &pkt, &consumed);
        assert_eq((int)r, (int)ParseResult::NoFrameHeader, "ParseResult should be NoFrameHeader");
        assert_eq(consumed, 1, "Consumed should be 1 byte");
    }
    // Test case 4: Checksum mismatch
    {
        uint8_t frame[32]={
            0x42, 0x4d, 0x00, 0x1c,
            0x00, 0x05, // pm1_0_cf1
            0x00, 0x0a, // pm2_5_cf1
            0x00, 0x03, // pm10_cf1
            0x00, 0x06,
            0x00, 0x01, 
            0x00, 0x02,
            0x00, 0x04, 
            0x00, 0x08,
            0x00, 0x07, 
            0x00, 0x09,
            0x00, 0x0b, 
            0x00, 0x0c,
            0x00, 0x0d
        };
        frame[30]=0x00;
        frame[31]=0x00;
        PMS7003MPacket pkt;
        size_t consumed=0;
        auto r=try_parse_packet(frame, 32, &pkt, &consumed);
        assert_eq((int)r, (int)ParseResult::ChecksumMismatch, 
        "ParseResult should be ChecksumMismatch");
    }
    return 0;
}