/**
 * FTD WebSocket Server
 *
 * Standalone executable that bridges the FTD engine to the web dashboard
 * via WebSocket on port 9100.  Uses a minimal embedded WebSocket
 * implementation over raw winsock2 sockets -- no external dependencies.
 *
 * Build: link against ftd_core (and ftd_cuda when available).
 * Usage: ws_server.exe [lattice_size] [port]
 *        Defaults: lattice_size=32, port=9100
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
   using socklen_t = int;
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
   using SOCKET = int;
   static constexpr int INVALID_SOCKET = -1;
   static constexpr int SOCKET_ERROR   = -1;
   static inline int closesocket(int fd) { return ::close(fd); }
#endif

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <array>
#include <memory>

// ============================================================================
//  Minimal SHA-1 (RFC 3174) -- only used once per WebSocket handshake
// ============================================================================

namespace {

struct SHA1 {
    uint32_t state[5];
    uint64_t count;
    uint8_t  buffer[64];

    SHA1() { reset(); }

    void reset() {
        state[0] = 0x67452301;
        state[1] = 0xEFCDAB89;
        state[2] = 0x98BADCFE;
        state[3] = 0x10325476;
        state[4] = 0xC3D2E1F0;
        count = 0;
        std::memset(buffer, 0, 64);
    }

    static uint32_t rol(uint32_t v, int bits) {
        return (v << bits) | (v >> (32 - bits));
    }

    void transform(const uint8_t block[64]) {
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

    void update(const void* data, size_t len) {
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

    std::array<uint8_t, 20> final_hash() {
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

// ============================================================================
//  Base64 encode
// ============================================================================

std::string base64_encode(const uint8_t* data, size_t len) {
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

// ============================================================================
//  Socket helpers
// ============================================================================

// Read exactly n bytes. Returns false on disconnect/error.
bool recv_exact(SOCKET sock, void* buf, size_t n) {
    auto p = static_cast<char*>(buf);
    size_t got = 0;
    while (got < n) {
        int r = ::recv(sock, p + got, static_cast<int>(n - got), 0);
        if (r <= 0) return false;
        got += r;
    }
    return true;
}

// Send all bytes.
bool send_all(SOCKET sock, const void* buf, size_t n) {
    auto p = static_cast<const char*>(buf);
    size_t sent = 0;
    while (sent < n) {
        int r = ::send(sock, p + sent, static_cast<int>(n - sent), 0);
        if (r <= 0) return false;
        sent += r;
    }
    return true;
}

// ============================================================================
//  Minimal JSON helpers (string-search based, no library)
// ============================================================================

// Find a string value for a key in a JSON string.
// Returns empty string if not found.
std::string json_string(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

// Find a numeric value (int or float) for a key. Returns 0 on failure.
double json_number(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return 0.0;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return 0.0;
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
        pos++;
    return std::atof(json.c_str() + pos);
}

// Find a boolean value for a key. Returns false on failure.
bool json_bool(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return false;
    return json.find("true", pos) < json.find("false", pos);
}

// ============================================================================
//  WebSocket handshake
// ============================================================================

static const char* WS_GUID = "258EAFA5-E914-47DA-95CA-5AB9DC799073";

bool ws_handshake(SOCKET client) {
    // Read the HTTP upgrade request (up to 4KB is plenty)
    char buf[4096];
    int total = 0;
    while (total < (int)sizeof(buf) - 1) {
        int r = ::recv(client, buf + total, 1, 0);
        if (r <= 0) return false;
        total += r;
        buf[total] = '\0';
        // End of HTTP headers
        if (total >= 4 && std::strstr(buf, "\r\n\r\n"))
            break;
    }

    std::string request(buf, total);

    // Extract Sec-WebSocket-Key
    std::string key_header = "Sec-WebSocket-Key: ";
    auto pos = request.find(key_header);
    if (pos == std::string::npos) {
        std::cerr << "[ws_server] No Sec-WebSocket-Key in handshake\n";
        return false;
    }
    pos += key_header.size();
    auto end = request.find("\r\n", pos);
    std::string ws_key = request.substr(pos, end - pos);
    // Trim whitespace
    while (!ws_key.empty() && ws_key.back() == ' ') ws_key.pop_back();

    // Compute accept: SHA1(key + GUID), base64
    std::string concat = ws_key + WS_GUID;
    SHA1 sha;
    sha.update(concat.data(), concat.size());
    auto hash = sha.final_hash();
    std::string accept = base64_encode(hash.data(), hash.size());

    // Send HTTP 101 Switching Protocols
    std::string response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept + "\r\n"
        "\r\n";

    return send_all(client, response.data(), response.size());
}

// ============================================================================
//  WebSocket frame reading / writing
// ============================================================================

enum WsOpcode : uint8_t {
    WS_TEXT   = 0x01,
    WS_BINARY = 0x02,
    WS_CLOSE  = 0x08,
    WS_PING   = 0x09,
    WS_PONG   = 0x0A
};

// Read one WebSocket frame.  Returns opcode, fills payload.
// Returns 0xFF on error/disconnect.
uint8_t ws_read_frame(SOCKET sock, std::vector<uint8_t>& payload) {
    payload.clear();

    uint8_t hdr[2];
    if (!recv_exact(sock, hdr, 2)) return 0xFF;

    // bool fin  = (hdr[0] & 0x80) != 0;
    uint8_t opcode = hdr[0] & 0x0F;
    bool masked = (hdr[1] & 0x80) != 0;
    uint64_t len = hdr[1] & 0x7F;

    if (len == 126) {
        uint8_t ext[2];
        if (!recv_exact(sock, ext, 2)) return 0xFF;
        len = (uint64_t(ext[0]) << 8) | ext[1];
    } else if (len == 127) {
        uint8_t ext[8];
        if (!recv_exact(sock, ext, 8)) return 0xFF;
        len = 0;
        for (int i = 0; i < 8; i++)
            len = (len << 8) | ext[i];
    }

    uint8_t mask_key[4] = {0, 0, 0, 0};
    if (masked) {
        if (!recv_exact(sock, mask_key, 4)) return 0xFF;
    }

    payload.resize(static_cast<size_t>(len));
    if (len > 0) {
        if (!recv_exact(sock, payload.data(), static_cast<size_t>(len)))
            return 0xFF;
        // Unmask
        if (masked) {
            for (size_t i = 0; i < payload.size(); i++)
                payload[i] ^= mask_key[i % 4];
        }
    }

    return opcode;
}

// Send a WebSocket frame (server frames are NOT masked).
bool ws_send_frame(SOCKET sock, uint8_t opcode, const void* data, size_t len) {
    std::vector<uint8_t> frame;
    frame.push_back(0x80 | opcode);  // FIN + opcode

    if (len < 126) {
        frame.push_back(static_cast<uint8_t>(len));
    } else if (len <= 0xFFFF) {
        frame.push_back(126);
        frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--)
            frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
    }

    if (!send_all(sock, frame.data(), frame.size())) return false;
    if (len > 0 && !send_all(sock, data, len)) return false;
    return true;
}

bool ws_send_text(SOCKET sock, const std::string& msg) {
    return ws_send_frame(sock, WS_TEXT, msg.data(), msg.size());
}

bool ws_send_binary(SOCKET sock, const std::vector<uint8_t>& data) {
    return ws_send_frame(sock, WS_BINARY, data.data(), data.size());
}

// ============================================================================
//  Particle data extraction (matches WASM get_particle_data)
// ============================================================================

std::vector<uint8_t> pack_particle_data(ftd::RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const int N = rb.lattice().size();
    const int total = N * N * N;

    // Count visible voxels
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (voxels[i].state != 0 || voxels[i].density() > ftd::K_B * 0.05)
            count++;
    }

    // Layout: [uint32 count][float32 pos[count*3]][float32 col[count*3]][float32 size[count]]
    size_t header_bytes = 4;
    size_t pos_bytes    = count * 3 * sizeof(float);
    size_t col_bytes    = count * 3 * sizeof(float);
    size_t size_bytes   = count * sizeof(float);
    size_t total_bytes  = header_bytes + pos_bytes + col_bytes + size_bytes;

    std::vector<uint8_t> buf(total_bytes);
    auto* ptr = buf.data();

    // Header: particle count as uint32 LE
    uint32_t cnt = static_cast<uint32_t>(count);
    std::memcpy(ptr, &cnt, 4);
    ptr += 4;

    float* positions = reinterpret_cast<float*>(ptr);
    float* colors    = reinterpret_cast<float*>(ptr + pos_bytes);
    float* sizes     = reinterpret_cast<float*>(ptr + pos_bytes + col_bytes);

    int idx = 0;
    for (int i = 0; i < total; i++) {
        const auto& v = voxels[i];
        if (v.state == 0 && v.density() <= ftd::K_B * 0.05) continue;

        auto c = rb.lattice().coord(i);

        // Position
        positions[idx * 3]     = static_cast<float>(c.x);
        positions[idx * 3 + 1] = static_cast<float>(c.y);
        positions[idx * 3 + 2] = static_cast<float>(c.z);

        // Color by state
        if (v.state == 1) {
            // Green (positive)
            colors[idx * 3]     = 0.29f;
            colors[idx * 3 + 1] = 0.87f;
            colors[idx * 3 + 2] = 0.50f;
        } else if (v.state == -1) {
            // Red (negative)
            colors[idx * 3]     = 0.97f;
            colors[idx * 3 + 1] = 0.44f;
            colors[idx * 3 + 2] = 0.44f;
        } else {
            // Void with flux: blue-gray, brightness proportional to density
            float brightness = static_cast<float>(v.density() / (ftd::K_B * 2.0));
            if (brightness > 1.0f) brightness = 1.0f;
            colors[idx * 3]     = 0.37f + brightness * 0.1f;
            colors[idx * 3 + 1] = 0.45f + brightness * 0.1f;
            colors[idx * 3 + 2] = 0.58f + brightness * 0.2f;
        }

        // Size: manifested particles larger
        if (v.state != 0) {
            sizes[idx] = 12.0f;
        } else {
            float s = 4.0f + static_cast<float>(v.density() / ftd::K_B) * 8.0f;
            if (s > 12.0f) s = 12.0f;
            sizes[idx] = s;
        }

        idx++;
    }

    return buf;
}

// ============================================================================
//  JSON response builders
// ============================================================================

std::string json_ok(int tick) {
    std::ostringstream ss;
    ss << "{\"ok\":true,\"tick\":" << tick << "}";
    return ss.str();
}

std::string json_error(const std::string& msg) {
    // Escape quotes in message
    std::string escaped;
    for (char c : msg) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else escaped += c;
    }
    return "{\"error\":\"" + escaped + "\"}";
}

std::string json_diagnostics(ftd::RenderBridge& rb) {
    auto d = rb.diagnostics();
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    ss << "\"tick\":"          << d.tick;
    ss << ",\"physicalTime\":" << rb.physical_time();
    ss << ",\"dt\":"           << rb.dt();
    ss << ",\"manifested\":"   << d.manifested_count;
    ss << ",\"positive\":"     << d.positive_count;
    ss << ",\"negative\":"     << d.negative_count;
    ss << ",\"totalFlux\":"    << d.total_flux;
    ss << ",\"totalEnergy\":"  << d.total_energy;
    ss << ",\"maxBandwidth\":" << d.max_bandwidth;
    ss << ",\"avgDrag\":"      << d.avg_drag;
    ss << ",\"entropy\":"      << d.total_entropy;
    ss << ",\"chargeBalance\":" << (d.positive_count - d.negative_count);
    ss << ",\"spinUp\":"       << d.spin_up_count;
    ss << ",\"spinDown\":"     << d.spin_down_count;
    ss << ",\"colorless\":"    << d.color_count[0];
    ss << ",\"colorRed\":"     << d.color_count[1];
    ss << ",\"colorGreen\":"   << d.color_count[2];
    ss << ",\"colorBlue\":"    << d.color_count[3];
    ss << ",\"angMomX\":"      << d.total_angular_momentum.x;
    ss << ",\"angMomY\":"      << d.total_angular_momentum.y;
    ss << ",\"angMomZ\":"      << d.total_angular_momentum.z;
    ss << "}";
    return ss.str();
}

std::string json_energy_audit(ftd::RenderBridge& rb) {
    auto ea = rb.energy_audit();
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    ss << "\"fieldEnergy\":"        << ea.field_energy;
    ss << ",\"waveEnergy\":"        << ea.wave_energy;
    ss << ",\"particleKE\":"        << ea.particle_ke;
    ss << ",\"totalEnergy\":"       << ea.total_energy;
    ss << ",\"gaussViolation\":"    << ea.gauss_violation;
    ss << ",\"maxGaussError\":"     << ea.max_gauss_error;
    ss << ",\"selfFieldInjection\":" << ea.self_field_injection;
    ss << ",\"coulombPE\":"         << ea.coulomb_pe;
    ss << ",\"EFieldEnergy\":"      << ea.E_field_energy;
    ss << ",\"BFieldEnergy\":"      << ea.B_field_energy;
    ss << ",\"chargeTotal\":"       << ea.charge_total;
    ss << ",\"manifested\":"        << ea.manifested_count;
    ss << ",\"poyntingX\":"         << ea.total_poynting.x;
    ss << ",\"poyntingY\":"         << ea.total_poynting.y;
    ss << ",\"poyntingZ\":"         << ea.total_poynting.z;
    ss << "}";
    return ss.str();
}

std::string json_info(ftd::RenderBridge& rb, bool gpu_active) {
    std::ostringstream ss;
    ss << "{";
    ss << "\"latticeSize\":" << rb.lattice().size();
    ss << ",\"tick\":"       << rb.current_tick();
    ss << ",\"gpu\":"        << (gpu_active ? "true" : "false");
    ss << ",\"version\":\"2.11\"";
    ss << "}";
    return ss.str();
}

// ============================================================================
//  Toggle name -> pointer mapping
// ============================================================================

bool* find_toggle(ftd::TermToggles& t, const std::string& name) {
    if (name == "wave_propagation") return &t.wave_propagation;
    if (name == "coupling")         return &t.coupling;
    if (name == "damping")          return &t.damping;
    if (name == "genesis")          return &t.genesis;
    if (name == "gauss_projection") return &t.gauss_projection;
    if (name == "forces")           return &t.forces;
    if (name == "gravity")          return &t.gravity;
    if (name == "poisson_coulomb")  return &t.poisson_coulomb;
    if (name == "movement")         return &t.movement;
    if (name == "lorentz_force")    return &t.lorentz_force;
    if (name == "selective_damping") return &t.selective_damping;
    if (name == "larmor_radiation") return &t.larmor_radiation;
    if (name == "dual_substrate")   return &t.dual_substrate;
    if (name == "color_forces")     return &t.color_forces;
    if (name == "weak_transmutation") return &t.weak_transmutation;
    if (name == "strong_force")     return &t.strong_force;
    if (name == "triad_binding")    return &t.triad_binding;
    if (name == "pair_production")  return &t.pair_production;
    if (name == "exchange_force")   return &t.exchange_force;
    if (name == "latency_field")    return &t.latency_field;
    return nullptr;
}

// ============================================================================
//  Command dispatch
// ============================================================================

// Returns false if the client should be disconnected.
bool handle_command(const std::string& json, SOCKET client,
                    std::unique_ptr<ftd::RenderBridge>& rb,
                    int& lattice_size, bool gpu_active)
{
    std::string cmd = json_string(json, "cmd");

    if (cmd == "tick") {
        rb->tick();
        return true;  // No response — fire-and-forget for speed
    }
    else if (cmd == "run") {
        int n = static_cast<int>(json_number(json, "n"));
        if (n < 1) n = 1;
        if (n > 100000) n = 100000;
        rb->run(n);
        return true;  // No response — fire-and-forget for speed
    }
    else if (cmd == "get_particles") {
        auto data = pack_particle_data(*rb);
        return ws_send_binary(client, data);
    }
    else if (cmd == "get_diagnostics") {
        return ws_send_text(client, json_diagnostics(*rb));
    }
    else if (cmd == "get_energy_audit") {
        return ws_send_text(client, json_energy_audit(*rb));
    }
    else if (cmd == "set_toggle") {
        std::string name = json_string(json, "name");
        bool value = json_bool(json, "value");
        bool* ptr = find_toggle(rb->toggles, name);
        if (ptr) {
            *ptr = value;
            std::string validErr;
            if (!rb->toggles.validate(&validErr))
                std::cerr << "[TermToggles] Invalid combination: " << validErr;
        }
        return true;  // Fire-and-forget
    }
    else if (cmd == "set_param") {
        std::string name = json_string(json, "name");
        double value = json_number(json, "value");
        if (name == "dt") rb->set_dt(value);
        return true;  // Fire-and-forget
    }
    else if (cmd == "inject_flux") {
        int x = static_cast<int>(json_number(json, "x"));
        int y = static_cast<int>(json_number(json, "y"));
        int z = static_cast<int>(json_number(json, "z"));
        double fx = json_number(json, "fx");
        double fy = json_number(json, "fy");
        double fz = json_number(json, "fz");
        rb->inject_flux(x, y, z, {fx, fy, fz});
        return true;
    }
    else if (cmd == "inject_particle") {
        int x = static_cast<int>(json_number(json, "x"));
        int y = static_cast<int>(json_number(json, "y"));
        int z = static_cast<int>(json_number(json, "z"));
        int8_t state = static_cast<int8_t>(json_number(json, "state"));
        double fx = json_number(json, "fx");
        double fy = json_number(json, "fy");
        double fz = json_number(json, "fz");
        rb->inject_particle(x, y, z, state, {fx, fy, fz});
        return true;
    }
    else if (cmd == "inject_wavepacket") {
        int x = static_cast<int>(json_number(json, "x"));
        int y = static_cast<int>(json_number(json, "y"));
        int z = static_cast<int>(json_number(json, "z"));
        int8_t state = static_cast<int8_t>(json_number(json, "state"));
        rb->inject_wavepacket(x, y, z, state);
        return true;
    }
    else if (cmd == "create_pair") {
        int x = static_cast<int>(json_number(json, "x"));
        int y = static_cast<int>(json_number(json, "y"));
        int z = static_cast<int>(json_number(json, "z"));
        double fx = json_number(json, "fx");
        double fy = json_number(json, "fy");
        double fz = json_number(json, "fz");
        rb->create_entangled_pair(x, y, z, {fx, fy, fz});
        return true;
    }
    else if (cmd == "resize") {
        int new_size = static_cast<int>(json_number(json, "size"));
        if (new_size < 4) new_size = 4;
        if (new_size > 256) new_size = 256;
        lattice_size = new_size;
        rb = std::make_unique<ftd::RenderBridge>(lattice_size);
        std::cout << "[ws_server] Resized lattice to " << lattice_size << "^3\n";
        return ws_send_text(client, json_ok(rb->current_tick()));
    }
    else if (cmd == "reset") {
        rb = std::make_unique<ftd::RenderBridge>(lattice_size);
        std::cout << "[ws_server] Reset engine (" << lattice_size << "^3)\n";
        return ws_send_text(client, json_ok(rb->current_tick()));
    }
    else if (cmd == "info") {
        return ws_send_text(client, json_info(*rb, gpu_active));
    }
    else {
        return ws_send_text(client, json_error("unknown command: " + cmd));
    }
}

}  // anonymous namespace

// ============================================================================
//  Main
// ============================================================================

int main(int argc, char* argv[]) {
    int lattice_size = 32;
    int port = 9100;

    if (argc >= 2) lattice_size = std::atoi(argv[1]);
    if (argc >= 3) port = std::atoi(argv[2]);
    if (lattice_size < 4) lattice_size = 4;
    if (lattice_size > 256) lattice_size = 256;
    if (port < 1 || port > 65535) port = 9100;

    std::cout << "================================================================\n";
    std::cout << "  FTD WebSocket Server\n";
    std::cout << "  G* = " << std::setprecision(10) << ftd::G_STAR
              << "  alpha^-1 = " << ftd::X_PLUS << "\n";
    std::cout << "================================================================\n\n";

    // Create engine
    std::cout << "[ws_server] Creating RenderBridge(" << lattice_size << ")...\n";
    auto rb = std::make_unique<ftd::RenderBridge>(lattice_size);

    // Detect GPU status: RenderBridge prints a banner when CUDA is active,
    // but we can check at compile time whether CUDA support is built in.
    bool gpu_active = false;
#ifdef FTD_ENABLE_CUDA
    gpu_active = true;
    std::cout << "[ws_server] GPU backend active\n";
#else
    std::cout << "[ws_server] CPU mode\n";
#endif

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[ws_server] WSAStartup failed\n";
        return 1;
    }
#endif

    // Create server socket
    SOCKET server_sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "[ws_server] socket() failed\n";
        return 1;
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    // Bind
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(server_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ws_server] bind() failed on port " << port << "\n";
        closesocket(server_sock);
        return 1;
    }

    if (::listen(server_sock, 1) == SOCKET_ERROR) {
        std::cerr << "[ws_server] listen() failed\n";
        closesocket(server_sock);
        return 1;
    }

    std::cout << "[ws_server] Listening on port " << port << "\n";

    // Main accept loop: one client at a time
    while (true) {
        std::cout << "[ws_server] Waiting for client...\n";
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        SOCKET client = ::accept(server_sock,
                                 reinterpret_cast<sockaddr*>(&client_addr),
                                 &client_len);
        if (client == INVALID_SOCKET) {
            std::cerr << "[ws_server] accept() failed\n";
            continue;
        }

        std::cout << "[ws_server] Client connected\n";

        // WebSocket handshake
        if (!ws_handshake(client)) {
            std::cerr << "[ws_server] Handshake failed\n";
            closesocket(client);
            continue;
        }

        std::cout << "[ws_server] WebSocket handshake complete\n";

        // Message loop
        bool connected = true;
        while (connected) {
            std::vector<uint8_t> payload;
            uint8_t opcode = ws_read_frame(client, payload);

            switch (opcode) {
            case WS_TEXT: {
                std::string msg(payload.begin(), payload.end());
                if (!handle_command(msg, client, rb, lattice_size, gpu_active))
                    connected = false;
                break;
            }
            case WS_BINARY:
                // Binary frames from client not expected; ignore
                break;
            case WS_PING: {
                // Respond with pong (same payload)
                ws_send_frame(client, WS_PONG, payload.data(), payload.size());
                break;
            }
            case WS_CLOSE:
                // Send close frame back
                ws_send_frame(client, WS_CLOSE, nullptr, 0);
                connected = false;
                break;
            default:
                // 0xFF = disconnect/error, anything else = unknown
                connected = false;
                break;
            }
        }

        closesocket(client);
        std::cout << "[ws_server] Client disconnected\n";
    }

    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
