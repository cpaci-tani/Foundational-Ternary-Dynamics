/**
 * WebSocket framing protocol implementation.
 *
 * See ws_protocol.h for API contract.  Extracted from ws_server.cpp so the
 * main server file becomes a pure command-dispatch loop.
 */
#include "ftd/ws_protocol.h"
#include "ftd/ws_sha1.h"

#ifdef _WIN32
#  pragma comment(lib, "ws2_32.lib")
#endif

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace ftd {

// ============================================================================
//  Socket helpers
// ============================================================================

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
//  Handshake
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
//  Frame read / write
// ============================================================================

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
//  Minimal JSON helpers
// ============================================================================

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

bool json_bool(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return false;
    return json.find("true", pos) < json.find("false", pos);
}

}  // namespace ftd
