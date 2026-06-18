#pragma once
#include <cstdint>
#include <cstddef>

namespace dander {

inline void write_be64(uint8_t* p, uint64_t v) {
    p[0]=(v>>56) & 0xff;
    p[1]=(v>>48) & 0xff;
    p[2]=(v>>40) & 0xff;
    p[3]=(v>>32) & 0xff;
    p[4]=(v>>24) & 0xff;
    p[5]=(v>>16) & 0xff;
    p[6]=(v>>8) & 0xff;
    p[7]=v & 0xff;
}

inline uint64_t read_be64(const uint8_t* p) {
    return (uint64_t(p[0])<<56) 
    | (uint64_t(p[1])<<48) 
    | (uint64_t(p[2])<<40) 
    | (uint64_t(p[3])<<32) 
    | (uint64_t(p[4])<<24) 
    | (uint64_t(p[5])<<16) 
    | (uint64_t(p[6])<<8) 
    | uint64_t(p[7]);
}

inline void write_be32(uint8_t* p, uint32_t v) {
    p[0]=(v>>24) & 0xff;
    p[1]=(v>>16) & 0xff;
    p[2]=(v>>8) & 0xff;
    p[3]=v & 0xff;
}

inline uint32_t read_be32(const uint8_t* p) {
    return (uint32_t(p[0])<<24) 
    | (uint32_t(p[1])<<16) 
    | (uint32_t(p[2])<<8) 
    | uint32_t(p[3]);
}

inline void write_be16(uint8_t* p, uint16_t v) {
    p[0]=(v>>8) & 0xff;
    p[1]=v & 0xff;
}

inline uint16_t read_be16(const uint8_t* p) {
    return (uint16_t(p[0])<<8) | uint16_t(p[1]);
}

} // namespace dander