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
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <limits>
#include <system_error>
#include <exception>
#ifndef _WIN32
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#endif

namespace ftd {

// ============================================================================
//  Socket helpers
// ============================================================================

namespace {
using IoClock = std::chrono::steady_clock;
thread_local WsIoScope* io_scope = nullptr;
thread_local WsIoStatus last_io_status = WsIoStatus::ok;
int socket_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}
bool would_block(int error) {
#ifdef _WIN32
    return error == WSAEWOULDBLOCK;
#else
    return error == EAGAIN || error == EWOULDBLOCK;
#endif
}
bool interrupted(int error) {
#ifdef _WIN32
    return error == WSAEINTR;
#else
    return error == EINTR;
#endif
}
bool make_nonblocking(SOCKET socket) {
#ifdef _WIN32
    u_long enabled = 1;
    return ioctlsocket(socket, FIONBIO, &enabled) == 0;
#else
    const int flags = fcntl(socket, F_GETFL, 0);
    return flags >= 0 && fcntl(socket, F_SETFL, flags | O_NONBLOCK) == 0;
#endif
}
}
struct WsIoAccess {
    static WsIoScope* matching(SOCKET socket) {
        return io_scope && io_scope->socket_ == socket ? io_scope : nullptr;
    }
    static const WsIoPolicy& policy(WsIoScope* scope) {
        static const WsIoPolicy defaults{};
        return scope ? scope->policy_ : defaults;
    }
    static bool poisoned(WsIoScope* scope) { return scope && scope->poisoned_; }
    static void poison(WsIoScope* scope) { if (scope) scope->poisoned_ = true; }
    static void pump(WsIoScope* scope) { if (scope && scope->wait_pump_) scope->wait_pump_(); }
};
WsIoScope::WsIoScope(SOCKET socket, WsIoPolicy policy, std::function<void()> wait_pump)
    : socket_(socket), policy_(policy), wait_pump_(std::move(wait_pump)) {
    if (io_scope) throw std::logic_error("nested WebSocket I/O scope");
    // Bound duration arithmetic as well as the operational default. Public
    // runtime policy has a narrower [100,60000] ms command-line range.
    for (const auto timeout : {policy.handshake_timeout, policy.read_timeout,
                               policy.write_timeout, policy.poll_interval}) {
        if (timeout.count() <= 0 || timeout > std::chrono::hours(24))
            throw std::invalid_argument("WebSocket I/O policy must be in (0,24h]");
    }
    if (!make_nonblocking(socket))
        throw std::system_error(socket_error(), std::system_category(), "nonblocking WebSocket setup");
    io_scope = this;
}
WsIoScope::~WsIoScope() { if (io_scope == this) io_scope = nullptr; }
WsIoStatus ws_last_io_status() { return last_io_status; }

namespace {
enum class IoKind { read, write, handshake };
struct IoContext {
    SOCKET socket = INVALID_SOCKET;
    WsIoScope* scope = nullptr;
    IoClock::time_point deadline;
    WsIoStatus status = WsIoStatus::ok;
    bool in_callback = false;
    bool wrote = false;
};
thread_local IoContext* active_io = nullptr;
void fail_io(WsIoStatus status) {
    if (active_io && active_io->status == WsIoStatus::ok) active_io->status = status;
}
class IoOperation {
    IoContext local;
    bool owner = false;
    bool admitted = true;
    int exceptions = std::uncaught_exceptions();
public:
    IoOperation(SOCKET socket, IoKind kind) {
        if (active_io) {
            if (active_io->in_callback || active_io->socket != socket) {
                fail_io(WsIoStatus::reentrant_operation); admitted = false;
            }
            return;
        }
        owner = true; local.socket = socket; local.scope = WsIoAccess::matching(socket);
        const auto& policy = WsIoAccess::policy(local.scope);
        const auto timeout = kind == IoKind::handshake ? policy.handshake_timeout
            : kind == IoKind::read ? policy.read_timeout : policy.write_timeout;
        local.deadline = IoClock::now() + timeout;
        active_io = &local;
        if (WsIoAccess::poisoned(local.scope)) fail_io(WsIoStatus::poisoned);
        else if (!make_nonblocking(socket)) fail_io(WsIoStatus::system_error);
    }
    ~IoOperation() {
        if (!owner) return;
        if (std::uncaught_exceptions() > exceptions && local.status == WsIoStatus::ok)
            local.status = WsIoStatus::system_error;
        if (local.wrote && local.status != WsIoStatus::ok) WsIoAccess::poison(local.scope);
        last_io_status = local.status;
        active_io = nullptr;
    }
    bool good() const { return admitted && active_io && active_io->status == WsIoStatus::ok; }
};
bool before_deadline() {
    if (!active_io || active_io->status != WsIoStatus::ok) return false;
    if (IoClock::now() >= active_io->deadline) { fail_io(WsIoStatus::timeout); return false; }
    return true;
}
bool wait_ready(bool writing) {
    if (!before_deadline()) return false;
    // Never flush a frame while another frame/header is partially transmitted.
    // Any callback socket helper call marks this operation rejected instead.
    struct CallbackGuard {
        CallbackGuard() { active_io->in_callback = true; }
        ~CallbackGuard() { active_io->in_callback = false; }
    };
    { CallbackGuard guard; WsIoAccess::pump(active_io->scope); }
    if (!before_deadline()) return false;
    const auto remaining = active_io->deadline - IoClock::now();
    if (remaining <= IoClock::duration::zero()) { fail_io(WsIoStatus::timeout); return false; }
    const auto interval = std::min(remaining,
        std::chrono::duration_cast<IoClock::duration>(WsIoAccess::policy(active_io->scope).poll_interval));
#ifdef _WIN32
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(interval).count();
    timeval timeout{}; timeout.tv_sec = static_cast<long>(us / 1000000);
    timeout.tv_usec = static_cast<long>(us % 1000000);
    fd_set events; FD_ZERO(&events); FD_SET(active_io->socket, &events);
    const int result = ::select(0, writing ? nullptr : &events, writing ? &events : nullptr, nullptr, &timeout);
#else
    const auto us = std::chrono::duration_cast<std::chrono::microseconds>(interval).count();
    const int ms = static_cast<int>(std::max<std::int64_t>(0, (us + 999) / 1000));
    pollfd event{active_io->socket, static_cast<short>(writing ? POLLOUT : POLLIN), 0};
    const int result = ::poll(&event, 1, ms);
#endif
    if (result < 0 && !interrupted(socket_error())) { fail_io(WsIoStatus::system_error); return false; }
    // EOF and errors are consumed by the next nonblocking recv/send so status
    // reflects the actual syscall, not an interpretation of readiness bits.
    return before_deadline();
}
}

bool recv_exact(SOCKET sock, void* buf, size_t n) {
    IoOperation operation(sock, IoKind::read);
    if (!operation.good()) return false;
    if (n && !buf) { fail_io(WsIoStatus::system_error); return false; }
    auto p = static_cast<char*>(buf);
    size_t got = 0;
    while (got < n) {
        if (!before_deadline()) return false;
        const int chunk = static_cast<int>(std::min(n - got, static_cast<size_t>(std::numeric_limits<int>::max())));
        const int result = ::recv(sock, p + got, chunk, 0);
        if (result > 0) { got += static_cast<size_t>(result); continue; }
        if (result == 0) { fail_io(WsIoStatus::disconnected); return false; }
        const int error = socket_error();
        if (interrupted(error)) continue;
        if (would_block(error)) { if (!wait_ready(false)) return false; continue; }
        fail_io(WsIoStatus::system_error); return false;
    }
    return before_deadline();
}
bool send_all(SOCKET sock, const void* buf, size_t n) {
    IoOperation operation(sock, IoKind::write);
    if (!operation.good()) return false;
    if (n && !buf) { fail_io(WsIoStatus::system_error); return false; }
    auto p = static_cast<const char*>(buf);
    size_t sent = 0;
    while (sent < n) {
        if (!before_deadline()) return false;
#ifdef MSG_NOSIGNAL
        constexpr int flags = MSG_NOSIGNAL;
#else
        constexpr int flags = 0;
#endif
        const int chunk = static_cast<int>(std::min(n - sent, static_cast<size_t>(std::numeric_limits<int>::max())));
        const int result = ::send(sock, p + sent, chunk, flags);
        if (result > 0) { sent += static_cast<size_t>(result); active_io->wrote = true; continue; }
        if (result == 0) { fail_io(WsIoStatus::disconnected); return false; }
        const int error = socket_error();
        if (interrupted(error)) continue;
        if (would_block(error)) { if (!wait_ready(true)) return false; continue; }
        fail_io(WsIoStatus::system_error); return false;
    }
    return before_deadline();
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
    IoOperation operation(client, IoKind::handshake);
    if (!operation.good()) return false;
    // Browser upgrade requests are normally small, but cookies and user-agent
    // metadata can legitimately push them beyond 4 KiB. Keep a strict bound
    // while allowing a conventional 16 KiB header block.
    char buf[16 * 1024];
    int total = 0;
    bool headers_complete = false;
    while (total < (int)sizeof(buf) - 1) {
        if (!recv_exact(client, buf + total, 1)) return false;
        ++total;
        buf[total] = '\0';
        // End of HTTP headers
        if (total >= 4 && std::strstr(buf, "\r\n\r\n")) {
            headers_complete = true;
            break;
        }
    }

    if (!headers_complete) {
        std::cerr << "[ws_server] Incomplete or oversized WebSocket handshake\n";
        fail_io(WsIoStatus::protocol_error);
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
        fail_io(WsIoStatus::protocol_error);
        return false;
    }

    const std::string ws_key = http_header_value(request, "Sec-WebSocket-Key");
    if (ws_key.empty()) {
        std::cerr << "[ws_server] No Sec-WebSocket-Key in handshake\n";
        fail_io(WsIoStatus::protocol_error);
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

WsFramePrefix inspect_ws_client_frame_prefix(const std::uint8_t* bytes, std::size_t available) {
    WsFramePrefix out;
    if (available && !bytes) { out.status = WsFramePrefixStatus::invalid; return out; }
    if (available < 2) return out;
    out.opcode = bytes[0] & 0x0f;
    const bool known = out.opcode == WS_TEXT || out.opcode == WS_BINARY
        || out.opcode == WS_CLOSE || out.opcode == WS_PING || out.opcode == WS_PONG;
    const auto code = bytes[1] & 0x7f;
    const bool control = (out.opcode & 8) != 0;
    if (!known || (bytes[0] & 0x80) == 0 || (bytes[0] & 0x70) != 0
        || (bytes[1] & 0x80) == 0 || (control && code > 125)
        || (out.opcode == WS_CLOSE && code == 1)) {
        out.status = WsFramePrefixStatus::invalid; return out;
    }
    out.prefix_bytes = code == 126 ? 4 : code == 127 ? 10 : 2;
    if (available < out.prefix_bytes) return out;
    std::uint64_t length = static_cast<std::uint64_t>(code);
    if (code == 126) length = (std::uint64_t(bytes[2]) << 8) | bytes[3];
    else if (code == 127) {
        if (bytes[2] & 0x80) { out.status = WsFramePrefixStatus::invalid; return out; }
        length = 0;
        for (unsigned i = 2; i < 10; ++i) length = (length << 8) | bytes[i];
    }
    if ((code == 126 && length < 126) || (code == 127 && length < 65536)
        || length > 65536) {
        out.status = WsFramePrefixStatus::invalid; return out;
    }
    out.header_bytes = out.prefix_bytes + 4;
    out.payload_bytes = static_cast<std::size_t>(length);
    out.total_bytes = out.header_bytes + out.payload_bytes;
    out.status = WsFramePrefixStatus::valid;
    return out;
}
uint8_t ws_read_frame(SOCKET sock, std::vector<uint8_t>& payload) {
    IoOperation operation(sock, IoKind::read);
    if (!operation.good()) return 0xff;
    payload.clear();
    std::uint8_t header[14]{};
    if (!recv_exact(sock, header, 2)) return 0xff;
    auto prefix = inspect_ws_client_frame_prefix(header, 2);
    if (prefix.status == WsFramePrefixStatus::incomplete) {
        if (!recv_exact(sock, header + 2, prefix.prefix_bytes - 2)) return 0xff;
        prefix = inspect_ws_client_frame_prefix(header, prefix.prefix_bytes);
    }
    if (prefix.status != WsFramePrefixStatus::valid) {
        fail_io(WsIoStatus::protocol_error); return 0xff;
    }
    auto* mask = header + prefix.prefix_bytes;
    if (!recv_exact(sock, mask, 4)) return 0xff;
    payload.resize(prefix.payload_bytes);
    if (!recv_exact(sock, payload.data(), payload.size())) { payload.clear(); return 0xff; }
    for (std::size_t i = 0; i < payload.size(); ++i) payload[i] ^= mask[i % 4];
    if (!before_deadline()) { payload.clear(); return 0xff; }
    return prefix.opcode;
}

bool ws_send_frame(SOCKET sock, uint8_t opcode, const void* data, size_t len) {
    IoOperation operation(sock, IoKind::write);
    if (!operation.good()) return false;
    const bool known = opcode == WS_TEXT || opcode == WS_BINARY || opcode == WS_CLOSE
        || opcode == WS_PING || opcode == WS_PONG;
    if (!known || ((opcode & 8) && len > 125) || (opcode == WS_CLOSE && len == 1)
        || (len && !data) || len > (std::numeric_limits<std::uint64_t>::max() >> 1)) {
        fail_io(WsIoStatus::protocol_error); return false;
    }
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
//  Bounded typed JSON (command records only; not an ontic state encoding)
// ============================================================================
namespace {
[[noreturn]] void json_fail(const std::string& message) {
    throw std::invalid_argument("invalid command JSON: " + message);
}
class JsonParser {
    const std::string& s;
    std::size_t pos = 0;
    void space() { while (pos < s.size() && (s[pos]==' ' || s[pos]=='\t' || s[pos]=='\r' || s[pos]=='\n')) ++pos; }
    bool take(char c) { if (pos < s.size() && s[pos] == c) { ++pos; return true; } return false; }
    void need(char c) { if (!take(c)) json_fail("expected punctuation at byte " + std::to_string(pos)); }
    static bool digit(char c) { return c >= '0' && c <= '9'; }
    unsigned hex4() {
        unsigned value = 0;
        for (int i=0;i<4;++i) {
            if (pos == s.size()) json_fail("truncated Unicode escape");
            char c=s[pos++]; unsigned d;
            if (c>='0' && c<='9') d=c-'0';
            else if (c>='a' && c<='f') d=c-'a'+10;
            else if (c>='A' && c<='F') d=c-'A'+10;
            else json_fail("invalid Unicode escape");
            value=value*16+d;
        }
        return value;
    }
    static void utf8(std::string& out, unsigned cp) {
        if (cp<0x80) out.push_back(static_cast<char>(cp));
        else if (cp<0x800) { out.push_back(static_cast<char>(0xc0|(cp>>6))); out.push_back(static_cast<char>(0x80|(cp&63))); }
        else if (cp<0x10000) { out.push_back(static_cast<char>(0xe0|(cp>>12))); out.push_back(static_cast<char>(0x80|((cp>>6)&63))); out.push_back(static_cast<char>(0x80|(cp&63))); }
        else { out.push_back(static_cast<char>(0xf0|(cp>>18))); out.push_back(static_cast<char>(0x80|((cp>>12)&63))); out.push_back(static_cast<char>(0x80|((cp>>6)&63))); out.push_back(static_cast<char>(0x80|(cp&63))); }
    }
    std::string string_token() {
        need('"'); std::string out;
        while (pos<s.size()) {
            unsigned char c=static_cast<unsigned char>(s[pos++]);
            if (c=='"') return out;
            if (c<32) json_fail("unescaped control character");
            if (c=='\\') {
                if (pos==s.size()) json_fail("truncated escape");
                switch (s[pos++]) {
                case '"': out+='"'; break; case '\\': out+='\\'; break; case '/': out+='/'; break;
                case 'b': out+='\b'; break; case 'f': out+='\f'; break; case 'n': out+='\n'; break;
                case 'r': out+='\r'; break; case 't': out+='\t'; break;
                case 'u': {
                    unsigned cp=hex4();
                    if (cp>=0xd800 && cp<=0xdbff) {
                        need('\\'); need('u'); unsigned low=hex4();
                        if (low<0xdc00 || low>0xdfff) json_fail("unpaired surrogate");
                        cp=0x10000+((cp-0xd800)<<10)+(low-0xdc00);
                    } else if (cp>=0xdc00 && cp<=0xdfff) json_fail("unpaired surrogate");
                    utf8(out,cp); break;
                }
                default: json_fail("invalid escape");
                }
            } else if (c<0x80) out.push_back(static_cast<char>(c));
            else {
                unsigned cp; int remaining; unsigned minimum;
                if (c>=0xc2 && c<=0xdf) { cp=c&31; remaining=1; minimum=0x80; }
                else if (c>=0xe0 && c<=0xef) { cp=c&15; remaining=2; minimum=0x800; }
                else if (c>=0xf0 && c<=0xf4) { cp=c&7; remaining=3; minimum=0x10000; }
                else json_fail("invalid UTF-8 lead byte");
                for (int i=0;i<remaining;++i) {
                    if (pos==s.size()) json_fail("truncated UTF-8");
                    unsigned char next=static_cast<unsigned char>(s[pos++]);
                    if ((next&0xc0)!=0x80) json_fail("invalid UTF-8 continuation");
                    cp=(cp<<6)|(next&63);
                }
                if (cp<minimum || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff)) json_fail("invalid UTF-8 scalar");
                utf8(out,cp);
            }
        }
        json_fail("unterminated string");
    }
    JsonValue value(unsigned depth) {
        space(); if (pos==s.size()) json_fail("missing value");
        JsonValue v; const char c=s[pos];
        if (c=='{' || c=='[') {
            if (depth>=64) json_fail("container depth exceeds 64");
            ++pos; space();
            if (c=='{') {
                v.kind=JsonValue::Kind::Object;
                if (take('}')) return v;
                do {
                    space(); std::string key=string_token(); space(); need(':');
                    auto child=value(depth+1);
                    if (!v.members.emplace(std::move(key),std::move(child)).second) json_fail("duplicate decoded object key");
                    space(); if (take('}')) return v;
                    need(',');
                } while (true);
            }
            v.kind=JsonValue::Kind::Array;
            if (take(']')) return v;
            do { v.elements.push_back(value(depth+1)); space(); if (take(']')) return v; need(','); } while (true);
        }
        if (c=='"') { v.kind=JsonValue::Kind::String; v.text=string_token(); return v; }
        for (const auto& token : {std::string("true"),std::string("false"),std::string("null")}) {
            if (s.compare(pos,token.size(),token)==0) {
                pos+=token.size(); v.kind=token=="null" ? JsonValue::Kind::Null : JsonValue::Kind::Boolean;
                v.flag=token=="true"; return v;
            }
        }
        const auto begin=pos;
        take('-');
        if (!take('0')) {
            if (pos==s.size() || s[pos]<'1' || s[pos]>'9') json_fail("expected JSON value");
            while (pos<s.size() && digit(s[pos])) ++pos;
        }
        if (take('.')) {
            const auto first=pos;
            while (pos<s.size() && digit(s[pos])) ++pos;
            if (pos==first) json_fail("missing fraction digits");
        }
        if (take('e') || take('E')) {
            if (!take('+')) take('-');
            const auto first=pos;
            while (pos<s.size() && digit(s[pos])) ++pos;
            if (pos==first) json_fail("missing exponent digits");
        }
        v.kind=JsonValue::Kind::Number; v.text=s.substr(begin,pos-begin);
        const auto converted=std::from_chars(v.text.data(),v.text.data()+v.text.size(),v.numeric);
        if (converted.ec!=std::errc{} || converted.ptr!=v.text.data()+v.text.size() || !std::isfinite(v.numeric))
            json_fail("number outside finite binary64 range");
        return v;
    }
public:
    explicit JsonParser(const std::string& input):s(input) {}
    JsonValue parse() {
        if (s.size()>65536) json_fail("input exceeds 65536 bytes");
        auto v=value(0); space();
        if (pos!=s.size()) json_fail("trailing input");
        if (v.kind!=JsonValue::Kind::Object) json_fail("top-level value must be object");
        return v;
    }
};
} // namespace
JsonValue parse_json_object(const std::string& input) { return JsonParser(input).parse(); }
const std::map<std::string,JsonValue>& JsonValue::object() const {
    if (kind!=Kind::Object) json_fail("expected object"); return members;
}
bool JsonValue::has(const std::string& key) const { return object().count(key)!=0; }
const JsonValue& JsonValue::at(const std::string& key) const {
    const auto& map=object(); auto it=map.find(key);
    if (it==map.end()) json_fail("missing field: "+key); return it->second;
}
const std::string& JsonValue::string() const { if (kind!=Kind::String) json_fail("expected string"); return text; }
double JsonValue::number() const { if (kind!=Kind::Number) json_fail("expected number"); return numeric; }
bool JsonValue::boolean() const { if (kind!=Kind::Boolean) json_fail("expected boolean"); return flag; }
std::int64_t JsonValue::integer(std::int64_t lo,std::int64_t hi) const {
    number();
    if (lo < -kJsonSafeInteger || hi > kJsonSafeInteger || lo>hi) json_fail("invalid integer accessor bounds");
    std::size_t p=0; const bool negative=text[p]=='-'; if (negative) ++p;
    std::string digits; int fraction=0; bool after_dot=false;
    for (;p<text.size() && text[p]!='e' && text[p]!='E';++p) {
        if (text[p]=='.') { after_dot=true; continue; }
        digits+=text[p]; if (after_dot) ++fraction;
    }
    int exponent=0; bool exp_negative=false;
    if (p<text.size()) {
        ++p; if (text[p]=='-' || text[p]=='+') { exp_negative=text[p]=='-'; ++p; }
        for (;p<text.size();++p) exponent=std::min(100000,exponent*10+(text[p]-'0'));
        if (exp_negative) exponent=-exponent;
    }
    const auto first=digits.find_first_not_of('0');
    if (first==std::string::npos) { if (lo>0 || hi<0) json_fail("integer outside field bounds"); return 0; }
    digits.erase(0,first);
    const int shift=exponent-fraction;
    if (shift<0) {
        const auto count=static_cast<std::size_t>(-shift);
        if (count>=digits.size() || digits.find_first_not_of('0',digits.size()-count)!=std::string::npos)
            json_fail("expected exact integer");
        digits.resize(digits.size()-count);
    } else {
        if (digits.size()+static_cast<std::size_t>(shift)>16) json_fail("integer outside exact range");
        digits.append(static_cast<std::size_t>(shift),'0');
    }
    if (digits.size()>16) json_fail("integer outside exact range");
    std::int64_t result=0;
    for (char d:digits) result=result*10+(d-'0');
    if (negative) result=-result;
    if (result<lo || result>hi) json_fail("integer outside field bounds");
    return result;
}
std::string JsonValue::string_or(const std::string& key,std::string fallback) const { return has(key)?string(key):fallback; }
double JsonValue::number_or(const std::string& key,double fallback) const { return has(key)?number(key):fallback; }
bool JsonValue::boolean_or(const std::string& key,bool fallback) const { return has(key)?boolean(key):fallback; }
std::int64_t JsonValue::integer_or(const std::string& key,std::int64_t fallback,std::int64_t lo,std::int64_t hi) const { return has(key)?integer(key,lo,hi):fallback; }

// Legacy convenience signatures preserve absent defaults only. Provided values
// must have the named type and the entire input must be a valid object.
std::string json_string(const std::string& json,const std::string& key) { return parse_json_object(json).string_or(key); }
double json_number(const std::string& json,const std::string& key) { return parse_json_object(json).number_or(key); }
bool json_bool(const std::string& json,const std::string& key) { return parse_json_object(json).boolean_or(key); }
bool json_has_key(const std::string& json,const std::string& key) { return parse_json_object(json).has(key); }
} // namespace ftd
