/**
 * WebSocket framing protocol (RFC 6455) + bounded typed JSON helpers.
 *
 * Extracted from ws_server.cpp. The main file handles command dispatch; this
 * header + ws_protocol.cpp owns the wire protocol.
 *
 * Platform SOCKET type is pulled in from <winsock2.h> (Windows) or defined as
 * int (POSIX) -- callers must have already configured the socket headers
 * before including this file, OR rely on the guarded block below.
 */
#pragma once

#include "ftd/ws_json.h"

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <type_traits>
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
#  ifndef NOMINMAX
#    define NOMINMAX   // windows.h's min/max macros break std::/Clock:: min()/max() calls
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

// Store/load trivially-copyable protocol values without creating a potentially
// misaligned typed pointer into a byte vector. The WebSocket binary formats are
// native little-endian by contract on every supported engine target.
template <typename T>
inline void write_binary_value(std::vector<std::uint8_t>& buffer,
                               std::size_t offset,
                               const T& value) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "binary protocol values must be trivially copyable");
    if (offset > buffer.size() || sizeof(T) > buffer.size() - offset) {
        throw std::out_of_range("binary protocol write exceeds frame buffer");
    }
    std::memcpy(buffer.data() + offset, &value, sizeof(T));
}

template <typename T>
inline T read_binary_value(const std::vector<std::uint8_t>& buffer,
                           std::size_t offset) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "binary protocol values must be trivially copyable");
    if (offset > buffer.size() || sizeof(T) > buffer.size() - offset) {
        throw std::out_of_range("binary protocol read exceeds frame buffer");
    }
    T value{};
    std::memcpy(&value, buffer.data() + offset, sizeof(T));
    return value;
}

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

struct WsIoPolicy {
    std::chrono::milliseconds handshake_timeout{5000};
    std::chrono::milliseconds read_timeout{5000};
    std::chrono::milliseconds write_timeout{5000};
    std::chrono::milliseconds poll_interval{8};
};
enum class WsIoStatus {
    ok, timeout, disconnected, system_error, protocol_error,
    reentrant_operation, poisoned
};
// One client on one thread. Socket remains nonblocking after scope destruction;
// runtime owns closure. Throws invalid_argument for invalid policy, logic_error
// for nested scopes, system_error if nonblocking setup fails. wait_pump may only
// perform bounded non-socket observation work; socket reentry rejects. Exceptions
// from the callback propagate to its owner, never become transport timeouts.
class WsIoScope {
public:
    WsIoScope(SOCKET socket, WsIoPolicy policy = {},
              std::function<void()> wait_pump = {});
    ~WsIoScope();
    WsIoScope(const WsIoScope&) = delete;
    WsIoScope& operator=(const WsIoScope&) = delete;
private:
    friend struct WsIoAccess;
    SOCKET socket_;
    WsIoPolicy policy_;
    std::function<void()> wait_pump_;
    bool poisoned_ = false;
};
// Status of the most recent top-level operation on this thread. Nested helper
// failures cannot be reset by a later segment of the same operation.
WsIoStatus ws_last_io_status();

// Complete operations use an absolute deadline, including all partial progress.
// Helpers without a scope use finite defaults and also leave sockets nonblocking.
// Failed partial writes poison a scoped connection; close it before reuse.
bool recv_exact(SOCKET sock, void* buf, size_t n);

// Send all bytes. Returns false on deadline, disconnect or socket error.
bool send_all(SOCKET sock, const void* buf, size_t n);

// ---------------------------------------------------------------------------
// WebSocket handshake + frames
// ---------------------------------------------------------------------------

// True when Origin names a loopback / file origin, or when Origin is
// absent/"null" AND the TCP peer is loopback. Foreign https origins are
// rejected. Empty Origin from a non-loopback peer is rejected.
bool ws_origin_allowed(const std::string& origin, bool peer_is_loopback = true);

// True when getpeername() reports 127.0.0.0/8, ::1, or v4-mapped ::ffff:127.x.
bool ws_peer_is_loopback(SOCKET sock);

// Perform the server-side HTTP Upgrade handshake.
// Reads one complete HTTP header block (bounded to 16 KiB) from `client`,
// computes the Sec-WebSocket-Accept value, and sends the 101 Switching
// Protocols response. Header names are matched case-insensitively.
// Origin must pass ws_origin_allowed(origin, ws_peer_is_loopback(client)).
bool ws_handshake(SOCKET client);

enum class WsFramePrefixStatus { incomplete, valid, invalid };
struct WsFramePrefix {
    WsFramePrefixStatus status = WsFramePrefixStatus::incomplete;
    std::size_t prefix_bytes = 2; // base + extended length, excludes mask
    std::size_t header_bytes = 0; // includes the four-byte client mask
    std::size_t payload_bytes = 0;
    std::size_t total_bytes = 0;
    std::uint8_t opcode = 0;
};
// Pure validation of up to fourteen available prefix bytes. `valid` means the
// length/control prefix is complete, not that the mask/payload has arrived.
// Reject unsupported opcodes, RSV/fragmentation/unmasked clients, noncanonical
// lengths, controls >125 bytes, close length one and payloads >65536 bytes.
WsFramePrefix inspect_ws_client_frame_prefix(const std::uint8_t* bytes,
                                             std::size_t available);

// Read one WebSocket frame.  Returns opcode, fills `payload`.
// Client frames must be final, unfragmented, masked, and no larger than 64 KiB.
// Returns 0xFF on protocol error or disconnect.
uint8_t ws_read_frame(SOCKET sock, std::vector<uint8_t>& payload);

// Send a WebSocket frame (server frames are NOT masked).
bool ws_send_frame(SOCKET sock, uint8_t opcode, const void* data, size_t len);

// Convenience wrappers.
bool ws_send_text(SOCKET sock, const std::string& msg);
bool ws_send_binary(SOCKET sock, const std::vector<uint8_t>& data);

// ---------------------------------------------------------------------------
// Typed JSON compatibility helpers (parse the complete top-level object)
// ---------------------------------------------------------------------------

// Missing fields retain defaults. Malformed JSON or a provided wrong type
// throws invalid_argument. New dispatch code should parse once with ws_json.h.
// Find a top-level string value for `key`; empty if absent.
std::string json_string(const std::string& json, const std::string& key);

// Find a numeric (int or float) value for `key`.  Returns 0.0 if absent.
double json_number(const std::string& json, const std::string& key);

// Find a boolean value for `key`.  Returns false if absent.
bool json_bool(const std::string& json, const std::string& key);

// True when a decoded top-level key is present. This disambiguates an explicit
// JSON `false` from an absent field for atomic toggle-profile updates.
bool json_has_key(const std::string& json, const std::string& key);

}  // namespace ftd
