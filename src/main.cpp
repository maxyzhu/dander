#include "dander/udp_packet.h"
#include "dander/pms7003m_parse.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <cerrno>

int main() {
    // Create UDP socket
    int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    // Bind socket to port 0.0.0.0:5005
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5005);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (::bind(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }
    printf("dander-ingest: listening on udp/5005\n");

    // recvfrom loop
    uint8_t buf[1500];
    while (true) {
        sockaddr_in src{};
        socklen_t srclen=sizeof(src);
        ssize_t n = ::recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&src, &srclen);
        if (n < 0) { perror("recvfrom"); continue; }

        // Unpack packet
        dander::UdpPacket pkt;
        auto r = dander::unpack_packet(buf, (size_t)n, &pkt);
        if (r != dander::UnpackResult::Ok) {
            printf("reject: result=%d, len=%zd, from=%s\n", (int)r, n, inet_ntoa(src.sin_addr));
            continue;
        }

        // Parse PMS payload
        dander::PMS7003MPacket pms;
        dander::parse_data(pkt.sensor_payload, &pms); // 24 bytes

        // Print PMS data
        printf("[sensor=%u ts=%llu] PM2.5=%u PM10=%u | counts: "
            "0.3=%u 1.0=%u 2.5=%u\n",
            pkt.sensor_id, (unsigned long long)pkt.epoch_ms,
            pms.pm2_5_atm, pms.pm10_atm,
            pms.count_0_3um, pms.count_1_0um, pms.count_2_5um);
    }
    return 0;
}