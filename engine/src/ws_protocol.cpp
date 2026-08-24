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
#include <cctype>
#include <cstring>
#include <cstdint>
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
        // On Linux/WSL, writing after a WebView reload can otherwise raise
        // SIGPIPE and terminate the entire CUDA server before send() returns an
        // error. Windows has no MSG_NOSIGNAL and does not need it.
#ifdef MSG_NOSIGNAL
        constexpr int send_flags = MSG_NOSIGNAL;
#else
        constexpr int send_flags = 0;
#endif
        int r = ::send(sock, p + sent, static_cast<int>(n - sent), send_flags);
        if (r <= 0) return false;
        sent += r;
    }
    return true;
}

// ============================================================================
//  Handshake
// ============================================================================

static const char* WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

namespace {

bool ascii_iequals(const std::string& lhs, const std::string& rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        const auto a = static_cast<unsigned char>(lhs[i]);
        const auto b = static_cast<unsigned char>(rhs[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return true;
}

std::string http_header_value(const std::string& request,
                              const std::string& wanted_name) {
    std::size_t line_start = 0;
    while (line_start < request.size()) {
        const std::size_t line_end = request.find("\r\n", line_start);
        const std::size_t bounded_end = line_end == std::string::npos
            ? request.size() : line_end;
        const std::size_t colon = request.find(':', line_start);
        if (colon != std::string::npos && colon < bounded_end) {
            std::size_t name_end = colon;
            while (name_end > line_start &&
                   (request[name_end - 1] == ' ' || request[name_end - 1] == '\t'))
                --name_end;
            const std::string name = request.substr(line_start, name_end - line_start);
            if (ascii_iequals(name, wanted_name)) {
                std::size_t value_start = colon + 1;
                while (value_start < bounded_end &&
                       (request[value_start] == ' ' || request[value_start] == '\t'))
                    ++value_start;
                std::size_t value_end = bounded_end;
                while (value_end > value_start &&
                       (request[value_end - 1] == ' ' || request[value_end - 1] == '\t'))
                    --value_end;
                return request.substr(value_start, value_end - value_start);
            }
        }
        if (line_end == std::string::npos) break;
        line_start = line_end + 2;
    }
    return {};
}

}  // namespace

bool ws_peer_is_loopback(SOCKET sock) {
    sockaddr_storage ss{};
#ifdef _WIN32
    int len = static_cast<int>(sizeof(ss));
#else
    socklen_t len = sizeof(ss);
#endif
    if (getpeername(sock, reinterpret_cast<sockaddr*>(&ss), &len) != 0) return false;
    if (ss.ss_family == AF_INET) {
        const auto* a = reinterpret_cast<sockaddr_in*>(&ss);
        const uint32_t addr = ntohl(a->sin_addr.s_addr);
        return (addr >> 24) == 127u;
    }
    if (ss.ss_family == AF_INET6) {
        const auto* a = reinterpret_cast<sockaddr_in6*>(&ss);
        const unsigned char* b = a->sin6_addr.s6_addr;
        static const unsigned char loop6[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1};
        if (std::memcmp(b, loop6, 16) == 0) return true;
        static const unsigned char v4map[12] = {0,0,0,0, 0,0,0,0, 0,0,0xff,0xff};
        if (std::memcmp(b, v4map, 12) == 0 && b[12] == 127) return true;
    }
    return false;
}

bool ws_origin_allowed(const std::string& origin, bool peer_is_loopback) {
    if (origin.empty() || origin == "null" || origin == "NULL") return peer_is_loopback;
    std::string lower;
    lower.reserve(origin.size());
    for (unsigned char c : origin) {
        lower.push_back(static_cast<char>(std::tolower(c)));
    }
    if (lower.rfind("file:", 0) == 0) return peer_is_loopback;
    const auto scheme = lower.find("://");
    if (scheme == std::string::npos) return false;
    const auto host_start = scheme + 3;
    std::string host;
    if (host_start < lower.size() && lower[host_start] == '[') {
        // RFC 3986: IPv6 literals keep their colons inside [...]. Stopping at
        // the first ':' would truncate "[::1]:8080" to "[".
        const auto close = lower.find(']', host_start);
        if (close == std::string::npos) return false;
        host = lower.substr(host_start + 1, close - host_start - 1);
    } else {
        const auto host_end = lower.find_first_of("/:", host_start);
        host = lower.substr(
            host_start,
            host_end == std::string::npos ? std::string::npos : host_end - host_start);
    }
    return host == "localhost" || host == "127.0.0.1" || host == "::1";
}

bool ws_handshake(SOCKET client) {
    // Browser upgrade requests are normally small, but cookies and user-agent
    // metadata can legitimately push them beyond 4 KiB. Keep a strict bound
    // while allowing a conventional 16 KiB header block.
    char buf[16 * 1024];
    int total = 0;
    bool headers_complete = false;
    while (total < (int)sizeof(buf) - 1) {
        int r = ::recv(client, buf + total, 1, 0);
        if (r <= 0) return false;
        total += r;
        buf[total] = '\0';
        // End of HTTP headers
        if (total >= 4 && std::strstr(buf, "\r\n\r\n")) {
            headers_complete = true;
            break;
        }
    }

    if (!headers_complete) {
        std::cerr << "[ws_server] Incomplete or oversized WebSocket handshake\n";
        return false;
    }

    std::string request(buf, total);

    // HTTP field names are case-insensitive (RFC 9110 section 5.1). Chromium,
    // WebView2, and Node are all free to choose different casing here.
    const std::string origin = http_header_value(request, "Origin");
    if (!ws_origin_allowed(origin, ws_peer_is_loopback(client))) {
        std::cerr << "[ws_server] Rejected handshake Origin: " << origin << "\n";
        const char forbid[] =
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 18\r\n"
            "Connection: close\r\n"
            "\r\n"
            "origin not allowed";
        send_all(client, forbid, sizeof(forbid) - 1);
        return false;
    }

    const std::string ws_key = http_header_value(request, "Sec-WebSocket-Key");
    if (ws_key.empty()) {
        std::cerr << "[ws_server] No Sec-WebSocket-Key in handshake\n";
        return false;
    }

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

    constexpr uint64_t kMaxClientFrameBytes = 64ull * 1024ull;

    uint8_t hdr[2];
    if (!recv_exact(sock, hdr, 2)) return 0xFF;

    const bool fin = (hdr[0] & 0x80) != 0;
    const bool has_reserved_bits = (hdr[0] & 0x70) != 0;
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

    // This server deliberately supports only complete, unextended browser
    // command frames. Client-to-server frames must be masked by RFC 6455.
    // Reject before allocating so a bogus 64-bit length cannot exhaust RAM.
    if (!fin || has_reserved_bits || !masked || len > kMaxClientFrameBytes)
        return 0xFF;

    uint8_t mask_key[4] = {0, 0, 0, 0};
    if (!recv_exact(sock, mask_key, 4)) return 0xFF;

    payload.resize(static_cast<size_t>(len));
    if (len > 0) {
        if (!recv_exact(sock, payload.data(), static_cast<size_t>(len)))
            return 0xFF;
        // Unmask
        for (size_t i = 0; i < payload.size(); i++)
            payload[i] ^= mask_key[i % 4];
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

bool json_has_key(const std::string& json, const std::string& key) {
    const std::string search = "\"" + key + "\"";
    return json.find(search) != std::string::npos;
}

}  // namespace ftd
