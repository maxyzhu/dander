#include "dander/byte_order.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>

using namespace dander;

static void fail(const char* msg) {
    printf("FAIL: %s\n", msg);
    exit(1);
}

static void assert_true(bool cond, const char* msg) {
    if (!cond) fail(msg);
}

static void assert_u64_eq(uint64_t a, uint64_t b, const char* msg) {
    if (a != b) {
        printf("FAIL: %s (expected 0x%llx, got 0x%llx)\n",
               msg, (unsigned long long)b, (unsigned long long)a);
        exit(1);
    }
}

int main() {
    // ---- be16 ----
    {
        // direction: high byte first
        uint8_t buf[2] = {0, 0};
        write_be16(buf, 0x1234);
        assert_true(buf[0] == 0x12 && buf[1] == 0x34, "be16 direction");

        // round-trip across edge values
        uint16_t vals[] = {0, 1, 0xFF, 0x1234, 0xFFFE, 0xFFFF};
        for (uint16_t v : vals) {
            uint8_t b[2];
            write_be16(b, v);
            uint16_t got = read_be16(b);
            if (got != v) fail("be16 round-trip");
        }
    }

    // ---- be32 ----
    {
        uint8_t buf[4] = {0, 0, 0, 0};
        write_be32(buf, 0x12345678);
        assert_true(buf[0]==0x12 && buf[1]==0x34 && buf[2]==0x56 && buf[3]==0x78,
                    "be32 direction");

        uint32_t vals[] = {0u, 1u, 0xFFu, 0x1234u, 0x12345678u, 0xFFFFFFFEu, 0xFFFFFFFFu};
        for (uint32_t v : vals) {
            uint8_t b[4];
            write_be32(b, v);
            uint32_t got = read_be32(b);
            if (got != v) fail("be32 round-trip");
        }
    }

    // ---- be64 ----
    {
        uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        write_be64(buf, 0x0123456789ABCDEFull);
        assert_true(buf[0]==0x01 && buf[1]==0x23 && buf[2]==0x45 && buf[3]==0x67 &&
                    buf[4]==0x89 && buf[5]==0xAB && buf[6]==0xCD && buf[7]==0xEF,
                    "be64 direction");

        uint64_t vals[] = {
            0ull, 1ull, 0xFFull, 0x1234ull,
            0x12345678ull, 0x0123456789ABCDEFull,
            0xFFFFFFFFFFFFFFFEull, 0xFFFFFFFFFFFFFFFFull,
        };
        for (uint64_t v : vals) {
            uint8_t b[8];
            write_be64(b, v);
            uint64_t got = read_be64(b);
            assert_u64_eq(got, v, "be64 round-trip");
        }
    }

    // ---- no buffer overrun ----
    {
        uint8_t buf[4] = {0xAA, 0xBB, 0xCC, 0xDD};
        write_be16(buf, 0x1122);
        assert_true(buf[0]==0x11 && buf[1]==0x22 && buf[2]==0xCC && buf[3]==0xDD,
                    "be16 must not touch bytes past index 1");
    }
    {
        uint8_t buf[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
        write_be32(buf, 0x01020304);
        assert_true(buf[0]==0x01 && buf[1]==0x02 && buf[2]==0x03 && buf[3]==0x04 &&
                    buf[4]==0xEE && buf[5]==0xFF && buf[6]==0x11 && buf[7]==0x22,
                    "be32 must not touch bytes past index 3");
    }

    printf("test_byte_order: OK\n");
    return 0;
}
