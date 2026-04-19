/**
 * Minimal SHA-1 (RFC 3174)
 *
 * Header-only, stdlib-only. Originally extracted from ws_server.cpp for the
 * WebSocket handshake, but usable anywhere a lightweight SHA-1 is needed.
 *
 * Usage:
 *   ftd::SHA1 sha;
 *   sha.update(data, len);
 *   std::array<uint8_t, 20> hash = sha.final_hash();
 */
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

namespace ftd {

struct SHA1 {
    uint32_t state[5];
    uint64_t count;
    uint8_t  buffer[64];

    inline SHA1() { reset(); }

    inline void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        count = 0;
        std::memset(buffer, 0, 64);
    }

    static inline uint32_t rol(uint32_t v, int bits) {
        return (v << bits) | (v >> (32 - bits));
    }

    inline void transform(const uint8_t block[64]) {
        uint32_t w[80];
        for (int i = 0; i < 16; i++) {
            w[i] = (uint32_t(block[i*4]) << 24)
                  | (uint32_t(block[i*4+1]) << 16)
                  | (uint32_t(block[i*4+2]) << 8)
                  |  uint32_t(block[i*4+3]);
        }
        for (int i = 16; i < 80; i++)
            w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        uint32_t a = state[0], b = state[1], c = state[2],
                 d = state[3], e = state[4];

        for (int i = 0; i < 80; i++) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d);       k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else              { f = b ^ c ^ d;                   k = 0xCA62C1D6; }
            uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = t;
        }
        state[0] += a; state[1] += b; state[2] += c;
        state[3] += d; state[4] += e;
    }

    inline void update(const void* data, size_t len) {
        auto p = static_cast<const uint8_t*>(data);
        size_t index = count % 64;
        count += len;
        // Fill partial block
        size_t i = 0;
        if (index) {
            size_t part = 64 - index;
            if (len >= part) {
                std::memcpy(buffer + index, p, part);
                transform(buffer);
                i = part;
            } else {
                std::memcpy(buffer + index, p, len);
                return;
            }
        }
        // Full blocks
        for (; i + 64 <= len; i += 64)
            transform(p + i);
        // Remainder
        if (i < len)
            std::memcpy(buffer, p + i, len - i);
    }

    inline std::array<uint8_t, 20> final_hash() {
        uint64_t bits = count * 8;
        uint8_t pad = 0x80;
        update(&pad, 1);
        pad = 0;
        while (count % 64 != 56)
            update(&pad, 1);
        uint8_t len_be[8];
        for (int i = 7; i >= 0; i--) {
            len_be[i] = uint8_t(bits & 0xFF);
            bits >>= 8;
        }
        update(len_be, 8);

        std::array<uint8_t, 20> hash;
        for (int i = 0; i < 5; i++) {
            hash[i*4]   = uint8_t(state[i] >> 24);
            hash[i*4+1] = uint8_t(state[i] >> 16);
            hash[i*4+2] = uint8_t(state[i] >> 8);
            hash[i*4+3] = uint8_t(state[i]);
        }
        return hash;
    }
};

// Base64 encode helper (used by the WebSocket handshake, kept here so SHA-1
// consumers have a single include for the handshake scenario).
inline std::string base64_encode(const uint8_t* data, size_t len) {
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = uint32_t(data[i]) << 16;
        if (i + 1 < len) n |= uint32_t(data[i+1]) << 8;
        if (i + 2 < len) n |= uint32_t(data[i+2]);
        out.push_back(table[(n >> 18) & 0x3F]);
        out.push_back(table[(n >> 12) & 0x3F]);
        out.push_back((i + 1 < len) ? table[(n >> 6) & 0x3F] : '=');
        out.push_back((i + 2 < len) ? table[n & 0x3F] : '=');
    }
    return out;
}

}  // namespace ftd
