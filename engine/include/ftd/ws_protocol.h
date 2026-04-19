/**
 * WebSocket framing protocol (RFC 6455) + minimal string-search JSON helpers.
 *
 * Extracted from ws_server.cpp. The main file handles command dispatch; this
 * header + ws_protocol.cpp owns the wire protocol.
 *
 * Platform SOCKET type is pulled in from <winsock2.h> (Windows) or defined as
 * int (POSIX) -- callers must have already configured the socket headers
 * before including this file, OR rely on the guarded block below.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Platform SOCKET type.  Kept identical to the definition previously inlined
// at the top of ws_server.cpp so that header consumers don't need to know
// which platform they're on.
// ---------------------------------------------------------------------------
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
   using SOCKET = int;
#  ifndef INVALID_SOCKET
     static constexpr int INVALID_SOCKET = -1;
#  endif
#  ifndef SOCKET_ERROR
     static constexpr int SOCKET_ERROR   = -1;
#  endif
   inline int closesocket(int fd) { return ::close(fd); }
#endif

namespace ftd {

// ---------------------------------------------------------------------------
// WebSocket opcodes
// ---------------------------------------------------------------------------
enum WsOpcode : uint8_t {
    WS_TEXT   = 0x01,
    WS_BINARY = 0x02,
    WS_CLOSE  = 0x08,
    WS_PING   = 0x09,
    WS_PONG   = 0x0A
};

// ---------------------------------------------------------------------------
// Raw socket helpers
// ---------------------------------------------------------------------------

// Read exactly n bytes.  Returns false on disconnect/error.
bool recv_exact(SOCKET sock, void* buf, size_t n);

// Send all bytes.  Returns false on error.
bool send_all(SOCKET sock, const void* buf, size_t n);

// ---------------------------------------------------------------------------
// WebSocket handshake + frames
// ---------------------------------------------------------------------------

// Perform the server-side HTTP Upgrade handshake.
// Reads up to 4 KB of HTTP headers from `client`, computes the Sec-WebSocket-
// Accept value, and sends the 101 Switching Protocols response.
bool ws_handshake(SOCKET client);

// Read one WebSocket frame.  Returns opcode, fills `payload`.
// Returns 0xFF on error/disconnect.
uint8_t ws_read_frame(SOCKET sock, std::vector<uint8_t>& payload);

// Send a WebSocket frame (server frames are NOT masked).
bool ws_send_frame(SOCKET sock, uint8_t opcode, const void* data, size_t len);

// Convenience wrappers.
bool ws_send_text(SOCKET sock, const std::string& msg);
bool ws_send_binary(SOCKET sock, const std::vector<uint8_t>& data);

// ---------------------------------------------------------------------------
// Minimal JSON helpers (string-search based, no external library)
// ---------------------------------------------------------------------------

// Find a string value for `key`.  Returns empty string if not found.
std::string json_string(const std::string& json, const std::string& key);

// Find a numeric (int or float) value for `key`.  Returns 0.0 on failure.
double json_number(const std::string& json, const std::string& key);

// Find a boolean value for `key`.  Returns false on failure or if absent.
bool json_bool(const std::string& json, const std::string& key);

}  // namespace ftd
