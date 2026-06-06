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
 *
 * Framing + handshake live in ws_protocol.{h,cpp}.  SHA-1 lives in ws_sha1.h.
 * This file is the command-dispatch loop and main().
 */

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/ws_protocol.h"   // Pulls in SOCKET type + framing API
#include "ftd/scenarios.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <memory>

namespace {

// SOCKET is declared at global scope in ws_protocol.h (platform-aligned)
// so no `using` is needed — just the WS opcode enum values.
using ftd::WS_TEXT;
using ftd::WS_BINARY;
using ftd::WS_CLOSE;
using ftd::WS_PING;
using ftd::WS_PONG;

// ============================================================================
//  Particle data extraction (matches WASM get_particle_data)
// ============================================================================

std::vector<uint8_t> pack_particle_data(ftd::RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const int N = rb.lattice().size();
    const int total = N * N * N;

    // Count visible voxels (strictly manifested particles state != 0)
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (voxels[i].state != 0)
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
        if (v.state == 0) continue;

        auto c = rb.lattice().coord(i);

        // Position with +0.5f center-voxel offset
        positions[idx * 3]     = static_cast<float>(c.x) + 0.5f;
        positions[idx * 3 + 1] = static_cast<float>(c.y) + 0.5f;
        positions[idx * 3 + 2] = static_cast<float>(c.z) + 0.5f;

        // Color by state
        if (v.state == 1) {
            // Green (positive)
            colors[idx * 3]     = 0.29f;
            colors[idx * 3 + 1] = 0.87f;
            colors[idx * 3 + 2] = 0.50f;
        } else { // v.state == -1
            // Red (negative)
            colors[idx * 3]     = 0.97f;
            colors[idx * 3 + 1] = 0.44f;
            colors[idx * 3 + 2] = 0.44f;
        }

        // Size matches WASM particle size
        sizes[idx] = 6.0f;
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

std::string json_flux_slice(ftd::RenderBridge& rb, int axis, int index) {
    const int N = rb.lattice().size();
    const auto& voxels = rb.voxels();
    std::ostringstream ss;
    ss << std::setprecision(6);
    ss << "{\"type\":\"flux_slice\",\"axis\":" << axis << ",\"index\":" << index << ",\"data\":[";
    bool first = true;
    for (int a = 0; a < N; ++a) {
        for (int b = 0; b < N; ++b) {
            int x, y, z;
            if (axis == 0)      { x = index; y = a; z = b; }
            else if (axis == 1) { x = a; y = index; z = b; }
            else                { x = a; y = b; z = index; }
            int idx = rb.lattice().index(x, y, z);
            if (!first) ss << ",";
            ss << voxels[idx].density();
            first = false;
        }
    }
    ss << "]}";
    return ss.str();
}

std::string json_flux_volume(ftd::RenderBridge& rb) {
    const int N = rb.lattice().size();
    const auto& voxels = rb.voxels();
    std::ostringstream ss;
    ss << std::setprecision(6);
    ss << "{\"type\":\"flux_volume\",\"data\":[";
    bool first = true;
    // Layout transpose matching JS Z-slowest ordering
    for (int z = 0; z < N; ++z) {
        for (int y = 0; y < N; ++y) {
            for (int x = 0; x < N; ++x) {
                int cpp_idx = rb.lattice().index(x, y, z);
                if (!first) ss << ",";
                ss << voxels[cpp_idx].density();
                first = false;
            }
        }
    }
    ss << "]}";
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
    if (name == "absorbing_boundary") return &t.absorbing_boundary;
    if (name == "field_energy_gravity") return &t.field_energy_gravity;
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
    std::string cmd = ftd::json_string(json, "cmd");

    if (cmd == "tick") {
        rb->tick();
        return true;  // No response -- fire-and-forget for speed
    }
    else if (cmd == "run") {
        int n = static_cast<int>(ftd::json_number(json, "n"));
        if (n < 1) n = 1;
        if (n > 100000) n = 100000;
        rb->run(n);
        return true;  // No response -- fire-and-forget for speed
    }
    else if (cmd == "get_particles") {
        auto data = pack_particle_data(*rb);
        return ftd::ws_send_binary(client, data);
    }
    else if (cmd == "get_diagnostics") {
        return ftd::ws_send_text(client, json_diagnostics(*rb));
    }
    else if (cmd == "get_energy_audit") {
        return ftd::ws_send_text(client, json_energy_audit(*rb));
    }
    else if (cmd == "get_flux_slice") {
        int axis = static_cast<int>(ftd::json_number(json, "axis"));
        int index = static_cast<int>(ftd::json_number(json, "index"));
        return ftd::ws_send_text(client, json_flux_slice(*rb, axis, index));
    }
    else if (cmd == "get_flux_volume") {
        return ftd::ws_send_text(client, json_flux_volume(*rb));
    }
    else if (cmd == "set_toggle") {
        std::string name = ftd::json_string(json, "name");
        bool value = ftd::json_bool(json, "value");
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
        std::string name = ftd::json_string(json, "name");
        double value = ftd::json_number(json, "value");
        if (name == "dt") rb->set_dt(value);
        return true;  // Fire-and-forget
    }
    else if (cmd == "inject_flux") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_flux(x, y, z, {fx, fy, fz});
        return true;
    }
    else if (cmd == "inject_flux_add") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_flux_add(x, y, z, {fx, fy, fz});
        return true;
    }
    else if (cmd == "inject_wave_vel_add") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double wx = ftd::json_number(json, "wx");
        double wy = ftd::json_number(json, "wy");
        double wz = ftd::json_number(json, "wz");
        rb->inject_wave_vel_add(x, y, z, {wx, wy, wz});
        return true;
    }
    else if (cmd == "inject_particle") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        int8_t state = static_cast<int8_t>(ftd::json_number(json, "state"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->inject_particle(x, y, z, state, {fx, fy, fz});
        return true;
    }
    else if (cmd == "inject_wavepacket") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        int8_t state = static_cast<int8_t>(ftd::json_number(json, "state"));
        rb->inject_wavepacket(x, y, z, state);
        return true;
    }
    else if (cmd == "create_pair") {
        int x = static_cast<int>(ftd::json_number(json, "x"));
        int y = static_cast<int>(ftd::json_number(json, "y"));
        int z = static_cast<int>(ftd::json_number(json, "z"));
        double fx = ftd::json_number(json, "fx");
        double fy = ftd::json_number(json, "fy");
        double fz = ftd::json_number(json, "fz");
        rb->create_entangled_pair(x, y, z, {fx, fy, fz});
        return true;
    }
    else if (cmd == "resize") {
        int new_size = static_cast<int>(ftd::json_number(json, "size"));
        if (new_size < 4) new_size = 4;
        if (new_size > 256) new_size = 256;
        lattice_size = new_size;
        rb = std::make_unique<ftd::RenderBridge>(lattice_size);
        std::cout << "[ws_server] Resized lattice to " << lattice_size << "^3\n";
        return ftd::ws_send_text(client, json_ok(rb->current_tick()));
    }
    else if (cmd == "reset") {
        rb = std::make_unique<ftd::RenderBridge>(lattice_size);
        std::cout << "[ws_server] Reset engine (" << lattice_size << "^3)\n";
        return ftd::ws_send_text(client, json_ok(rb->current_tick()));
    }
    else if (cmd == "setup_scenario") {
        std::string name = ftd::json_string(json, "name");
        rb = std::make_unique<ftd::RenderBridge>(lattice_size); // reset first, matching JS setupScenario contract
        bool success = ftd::dispatch_scenario(*rb, name);
        if (success) {
            std::cout << "[ws_server] Loaded scenario natively: " << name << "\n";
            return ftd::ws_send_text(client, json_ok(rb->current_tick()));
        } else {
            std::cout << "[ws_server] Warning: dispatch_scenario failed for: " << name << "\n";
            return ftd::ws_send_text(client, json_error("failed to dispatch scenario: " + name));
        }
    }
    else if (cmd == "info") {
        return ftd::ws_send_text(client, json_info(*rb, gpu_active));
    }
    else {
        return ftd::ws_send_text(client, json_error("unknown command: " + cmd));
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
        if (!ftd::ws_handshake(client)) {
            std::cerr << "[ws_server] Handshake failed\n";
            closesocket(client);
            continue;
        }

        std::cout << "[ws_server] WebSocket handshake complete\n";

        // Message loop
        bool connected = true;
        while (connected) {
            std::vector<uint8_t> payload;
            uint8_t opcode = ftd::ws_read_frame(client, payload);

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
