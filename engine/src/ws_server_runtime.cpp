/**
 * @file ws_server_runtime.cpp
 * @brief Socket readiness, client session ordering, and server lifecycle.
 */

#include "ws_server_internal.h"

#include "ftd/constants.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <sys/ioctl.h>
#endif

namespace ftd::ws_server_detail {

using ftd::WS_BINARY;
using ftd::WS_CLOSE;
using ftd::WS_PING;
using ftd::WS_PONG;
using ftd::WS_TEXT;

// The original server blocked indefinitely in ws_read_frame(), which meant an
// already-completed CUDA event could not be published until the browser sent
// another command.  Keep the WebSocket transport single-writer/ordered, but
// wake periodically to poll the non-blocking native snapshot fence.
enum class ClientPollResult { readable, timeout, error };
enum class ClientFrameReadiness { ready, incomplete, error };

ClientPollResult wait_for_client_activity(SOCKET client, int timeout_ms) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(client, &read_set);

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
#ifdef _WIN32
    const int result = ::select(0, &read_set, nullptr, nullptr, &timeout);
#else
    const int result = ::select(client + 1, &read_set, nullptr, nullptr, &timeout);
#endif
    if (result == 0) return ClientPollResult::timeout;
    if (result == SOCKET_ERROR || result < 0) return ClientPollResult::error;
    return FD_ISSET(client, &read_set)
        ? ClientPollResult::readable
        : ClientPollResult::timeout;
}

bool socket_recv_would_block() {
#ifdef _WIN32
    const int error = WSAGetLastError();
    return error == WSAEWOULDBLOCK;
#else
    return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
}

// ws_read_frame() correctly uses recv_exact() for a complete WebSocket frame,
// but it is intentionally blocking. A TCP socket can become readable after
// only the first few bytes have arrived, so gate the legacy parser on a
// non-consuming, complete-frame check. This keeps telemetry fence polling
// alive during a slow/partial client upload without changing ws_protocol.
ClientFrameReadiness complete_client_frame_available(SOCKET client) {
    std::uint64_t available = 0;
#ifdef _WIN32
    u_long buffered = 0;
    if (::ioctlsocket(client, FIONREAD, &buffered) == SOCKET_ERROR)
        return ClientFrameReadiness::error;
    available = buffered;
#else
    int buffered = 0;
    if (::ioctl(client, FIONREAD, &buffered) < 0)
        return ClientFrameReadiness::error;
    available = buffered > 0 ? static_cast<std::uint64_t>(buffered) : 0u;
#endif

    // select() also reports a clean close as readable. Let ws_read_frame()
    // consume that immediately; it cannot block because recv() returns zero.
    if (available == 0) return ClientFrameReadiness::ready;
    if (available < 2) return ClientFrameReadiness::incomplete;

    std::array<std::uint8_t, 14> header{};
    const int peeked = ::recv(
        client, reinterpret_cast<char*>(header.data()),
        static_cast<int>((std::min)(available,
                                    static_cast<std::uint64_t>(header.size()))),
        MSG_PEEK);
    if (peeked == 0) return ClientFrameReadiness::ready;
    if (peeked < 0) {
        return socket_recv_would_block()
            ? ClientFrameReadiness::incomplete
            : ClientFrameReadiness::error;
    }
    if (peeked < 2) return ClientFrameReadiness::incomplete;

    const bool fin = (header[0] & 0x80u) != 0;
    const bool reserved = (header[0] & 0x70u) != 0;
    const bool masked = (header[1] & 0x80u) != 0;
    const std::uint8_t length_code = header[1] & 0x7fu;
    std::size_t extended_bytes = 0;
    if (length_code == 126u) extended_bytes = 2;
    else if (length_code == 127u) extended_bytes = 8;
    const std::size_t prefix_bytes = 2u + extended_bytes;

    if (available < prefix_bytes
        || static_cast<std::size_t>(peeked) < prefix_bytes) {
        return ClientFrameReadiness::incomplete;
    }

    // Invalid control bits/masking are rejected by ws_read_frame() before it
    // reads a mask/payload, so dispatch it now rather than wait for bytes that
    // well-formed browsers will never send.
    if (!fin || reserved || !masked) return ClientFrameReadiness::ready;

    std::uint64_t payload_bytes = length_code;
    if (length_code == 126u) {
        payload_bytes = (static_cast<std::uint64_t>(header[2]) << 8u)
                      | static_cast<std::uint64_t>(header[3]);
    } else if (length_code == 127u) {
        payload_bytes = 0;
        for (std::size_t i = 0; i < 8; ++i) {
            payload_bytes = (payload_bytes << 8u) | header[2u + i];
        }
    }

    // Match ws_protocol's 64 KiB bound. It rejects this before reading the
    // mask/payload, so the existing parser remains safe to enter.
    constexpr std::uint64_t kMaxClientFrameBytes = 64ull * 1024ull;
    if (payload_bytes > kMaxClientFrameBytes) return ClientFrameReadiness::ready;

    const std::uint64_t frame_bytes = static_cast<std::uint64_t>(prefix_bytes)
                                    + 4u + payload_bytes;
    return available >= frame_bytes
        ? ClientFrameReadiness::ready
        : ClientFrameReadiness::incomplete;
}

// ============================================================================
//  Main
// ============================================================================

int run_server(int argc, char* argv[]) {
    // The desktop host redirects stdout/stderr to its persistent session log.
    // Line-buffer explicitly so startup, allocation, and failure messages are
    // visible immediately instead of appearing only when the process exits.
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    int lattice_size = 32;
    int port = 9100;
    // Revision 1.4 hardening: default to loopback. The protocol has NO
    // authentication and no Origin check, so the previous INADDR_ANY default
    // let any LAN host (or any webpage — same-origin policy does not block
    // cross-origin WebSocket) drive the engine. LAN/remote use is preserved
    // via an explicit opt-in flag: --bind <addr> (e.g. --bind 0.0.0.0).
    std::string bind_addr = "127.0.0.1";
    bool single_client = false;

    std::vector<const char*> positional;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--bind") == 0 && i + 1 < argc) {
            bind_addr = argv[++i];
        } else if (std::strcmp(argv[i], "--once") == 0) {
            single_client = true;
        } else {
            positional.push_back(argv[i]);
        }
    }
    if (positional.size() >= 1) lattice_size = std::atoi(positional[0]);
    if (positional.size() >= 2) port = std::atoi(positional[1]);
    if (lattice_size < 4) lattice_size = 4;
    if (lattice_size > 256) lattice_size = 256;
    if (port < 1 || port > 65535) port = 9100;

    std::cout << "================================================================\n";
    std::cout << "  FTD WebSocket Server\n";
    std::cout << "  G* = " << std::setprecision(10) << ftd::G_STAR
              << "  alpha^-1 = " << ftd::X_PLUS << "\n";
    std::cout << "================================================================\n\n";

    // Create engine through the same conservative budget/error boundary used
    // by runtime replacements. Startup failures now produce a useful log and
    // exit code instead of std::terminate or a CUDA macro exit from deep inside
    // the allocation stack.
    std::cout << "[ws_server] Creating RenderBridge(" << lattice_size << ")...\n";
    const ResourceBudget startup_budget = resource_budget(lattice_size);
    if (!startup_budget.accepted()) {
        std::cerr << "[ws_server] " << budget_error(startup_budget) << "\n";
        return 2;
    }

    std::unique_ptr<ftd::RenderBridge> rb;
    try {
        rb = make_interactive_bridge(lattice_size);
    } catch (const std::exception& ex) {
        std::cerr << "[ws_server] Engine startup failed: " << ex.what() << "\n";
        return 2;
    } catch (...) {
        std::cerr << "[ws_server] Engine startup failed: unknown native exception\n";
        return 2;
    }

    // Report the backend that is actually active, not merely whether this
    // executable was compiled with CUDA support. Desktop and browser clients
    // use the same runtime truth through the `info` command below.
    if (rb->backend_kind() == ftd::Backend::Kind::Gpu) {
        std::cout << "[ws_server] GPU backend active\n";
    } else {
        std::cout << "[ws_server] CPU mode\n";
    }

    // The source exists before any dashboard attaches.  Demand is initially
    // empty, so this only establishes a generation boundary; no telemetry
    // reduction is launched until the client subscribes.
    ftd::NativeTelemetryScheduler telemetry;
    telemetry.on_source_replaced(*rb);

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "[ws_server] WSAStartup failed\n";
        return 1;
    }
#endif

    // socklen_t must match the system ABI; on Windows the socket headers
    // define it as `int`, so mirror that here for the accept() call.
#ifdef _WIN32
    using socklen_t = int;
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

    // Bind (loopback by default — see the --bind flag in main; revision 1.4)
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (bind_addr == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else if (::inet_pton(AF_INET, bind_addr.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "[ws_server] invalid --bind address '" << bind_addr << "'\n";
        closesocket(server_sock);
        return 1;
    }
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(server_sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[ws_server] bind() failed on " << bind_addr << ":" << port << "\n";
        closesocket(server_sock);
        return 1;
    }

    if (::listen(server_sock, 1) == SOCKET_ERROR) {
        std::cerr << "[ws_server] listen() failed\n";
        closesocket(server_sock);
        return 1;
    }

    std::cout << "[ws_server] Listening on " << bind_addr << ":" << port << "\n";
    if (bind_addr != "127.0.0.1") {
        std::cout << "[ws_server] *** WARNING: bound to " << bind_addr << " — this protocol has\n"
                     "[ws_server] *** NO authentication. Origin is enforced when the browser sends\n"
                     "[ws_server] *** it; empty/null Origin is accepted only from loopback peers.\n"
                     "[ws_server] *** Use only on trusted networks.\n";
    }

    // Main accept loop: one client at a time. Keep polling a retired snapshot
    // while no client is attached, so a CUDA event failure is converted into
    // a suspended/recoverable scheduler before a reconnect reaches `info`.
    std::cout << "[ws_server] Waiting for client...\n";
    while (true) {
        const ClientPollResult accept_poll = wait_for_client_activity(
            server_sock, 8);
        if (accept_poll == ClientPollResult::error) {
            std::cerr << "[ws_server] accept readiness poll failed\n";
            continue;
        }
        if (accept_poll == ClientPollResult::timeout) {
            try {
                telemetry.pump(*rb);
                // There is intentionally no observer while disconnected.
                // Demand has been retired, but drain defensively if a future
                // backend changes that behavior.
                while (telemetry.take_publication()) {}
                while (telemetry.take_invalidation()) {}
            } catch (const std::exception& ex) {
                std::cerr << "[ws_server] Retired telemetry publisher failed: "
                          << ex.what() << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
            } catch (...) {
                const std::string message = "unknown retired native telemetry publisher failure";
                std::cerr << "[ws_server] " << message << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
            }
            continue;
        }

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
        if (!ftd::ws_handshake(client)) {
            std::cerr << "[ws_server] Handshake failed\n";
            closesocket(client);
            continue;
        }

        std::cout << "[ws_server] WebSocket handshake complete\n";

        // Message loop.  Snapshot publication is intentionally serviced from
        // this same single transport writer: an unsolicited JSON delta cannot
        // interleave with a binary response frame or be mistaken for a
        // request-correlated response.
        bool connected = true;
        constexpr int kTelemetryPollIntervalMs = 8;
        const auto service_telemetry = [&]() -> bool {
            telemetry.pump(*rb);
            return flush_telemetry_publications(client, telemetry);
        };
        while (connected) {
            try {
                if (!service_telemetry()) {
                    connected = false;
                    break;
                }
            } catch (const std::exception& ex) {
                std::cerr << "[ws_server] Telemetry publisher failed: "
                          << ex.what() << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                send_json_response(
                    client, json_native_recovery_required("telemetry", telemetry), 0);
                connected = false;
                break;
            } catch (...) {
                const std::string message = "unknown native telemetry publisher failure";
                std::cerr << "[ws_server] " << message << "\n";
                if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                send_json_response(
                    client, json_native_recovery_required("telemetry", telemetry), 0);
                connected = false;
                break;
            }

            const ClientPollResult poll = wait_for_client_activity(
                client, kTelemetryPollIntervalMs);
            if (poll == ClientPollResult::error) {
                connected = false;
                break;
            }
            if (poll == ClientPollResult::timeout) continue;

            const ClientFrameReadiness frame =
                complete_client_frame_available(client);
            if (frame == ClientFrameReadiness::error) {
                connected = false;
                break;
            }
            if (frame == ClientFrameReadiness::incomplete) {
                // The socket remains readable while a partial frame sits in
                // its receive buffer. Avoid a tight spin but keep snapshot
                // fence latency bounded to a millisecond in this rare case.
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }

            std::vector<uint8_t> payload;
            uint8_t opcode = ftd::ws_read_frame(client, payload);

            switch (opcode) {
            case WS_TEXT: {
                std::string msg(payload.begin(), payload.end());
                try {
                    if (!handle_command(msg, client, rb, telemetry, lattice_size))
                        connected = false;
                } catch (const std::exception& ex) {
                    // CUDA/kernel/validation failures are surfaced through the
                    // protocol. Allocation commands are transactional, but a
                    // tick/run exception may leave a partially advanced CUDA
                    // state. Send the typed error, then break this client
                    // connection so the dashboard reconnect path rebuilds the
                    // selected scenario instead of immediately retrying the
                    // poisoned bridge in an rAF error loop.
                    std::cerr << "[ws_server] Command failed: " << ex.what() << "\n";
                    const std::string operation = ftd::json_string(msg, "cmd");
                    if (operation == "tick" || operation == "run") {
                        if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                        if (!send_json_response(
                                client,
                                json_native_recovery_required(operation, telemetry),
                                request_id_from(msg))) {
                            connected = false;
                        }
                        connected = false;
                    } else if (!send_json_response(
                                   client, json_error(ex.what(), operation),
                                   request_id_from(msg))) {
                        connected = false;
                    }
                } catch (...) {
                    const std::string message = "unknown native command failure";
                    std::cerr << "[ws_server] " << message << "\n";
                    const std::string operation = ftd::json_string(msg, "cmd");
                    if (operation == "tick" || operation == "run") {
                        if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                        if (!send_json_response(
                                client,
                                json_native_recovery_required(operation, telemetry),
                                request_id_from(msg))) {
                            connected = false;
                        }
                        connected = false;
                    } else if (!send_json_response(
                                   client, json_error(message, operation),
                                   request_id_from(msg))) {
                        connected = false;
                    }
                }
                break;
            }
            case WS_BINARY:
                // Binary frames from client not expected; ignore
                break;
            case WS_PING: {
                // Respond with pong (same payload)
                ftd::ws_send_frame(client, WS_PONG, payload.data(), payload.size());
                break;
            }
            case WS_CLOSE:
                // Send close frame back
                ftd::ws_send_frame(client, WS_CLOSE, nullptr, 0);
                connected = false;
                break;
            default:
                // 0xFF = disconnect/error, anything else = unknown
                connected = false;
                break;
            }

            // A command's normal response (including any binary frame) is
            // completely written before an optional publisher delta.  CPU
            // snapshots may be ready immediately; CUDA snapshots will be
            // picked up by a later idle poll without stalling the command.
            if (connected) {
                try {
                    if (!service_telemetry()) connected = false;
                } catch (const std::exception& ex) {
                    std::cerr << "[ws_server] Telemetry publisher failed: "
                              << ex.what() << "\n";
                    if (!telemetry.suspended()) telemetry.abort_and_suspend(ex.what());
                    send_json_response(
                        client, json_native_recovery_required("telemetry", telemetry), 0);
                    connected = false;
                } catch (...) {
                    const std::string message = "unknown native telemetry publisher failure";
                    std::cerr << "[ws_server] " << message << "\n";
                    if (!telemetry.suspended()) telemetry.abort_and_suspend(message);
                    send_json_response(
                        client, json_native_recovery_required("telemetry", telemetry), 0);
                    connected = false;
                }
            }
        }

        closesocket(client);
        telemetry.on_client_disconnected();
        std::cout << "[ws_server] Client disconnected\n";
        if (single_client) break;
        std::cout << "[ws_server] Waiting for client...\n";
    }

    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

}  // namespace ftd::ws_server_detail
