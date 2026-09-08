#include "../src/ws_server_internal.h"
#include "support/ws_loopback_fixture.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {
int checks = 0, failures = 0;
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) { ++failures; std::cerr << "FAIL " << message << '\n'; }
}
std::uint64_t little(const std::vector<std::uint8_t>& bytes, std::size_t offset, int width) {
    std::uint64_t out = 0;
    for (int i = 0; i < width; ++i) out |= std::uint64_t(bytes.at(offset + i)) << (8 * i);
    return out;
}
std::vector<std::uint8_t> receive(SOCKET socket, std::uint8_t& opcode) {
    std::uint8_t header[2]{};
    if (!ftd::recv_exact(socket, header, 2)) throw std::runtime_error("missing frame header");
    opcode = header[0] & 15;
    if (header[0] != (0x80 | opcode) || (header[1] & 128)) throw std::runtime_error("invalid response frame");
    std::size_t size = header[1] & 127;
    if (size >= 126) {
        const auto width = size == 126 ? 2u : 8u;
        std::uint8_t bytes[8]{};
        if (!ftd::recv_exact(socket, bytes, width)) throw std::runtime_error("missing frame length");
        size = 0;
        for (unsigned i = 0; i < width; ++i) size = (size << 8) | bytes[i];
    }
    if (size > 1'000'000) throw std::runtime_error("unexpected fixture response size");
    std::vector<std::uint8_t> out(size);
    if (!ftd::recv_exact(socket, out.data(), out.size())) throw std::runtime_error("missing frame payload");
    return out;
}
}
int run_tests() {
    using namespace ftd;
    using namespace ftd::ws_server_detail;
#ifdef _WIN32
    _putenv_s("FTD_FORCE_CPU", "1");
#else
    setenv("FTD_FORCE_CPU", "1", 1);
#endif
    ws_test::Platform platform;
    for (auto value : {kWireSafeInteger, kWireSafeInteger + 1, kWireSafeInteger + 2,
                       std::numeric_limits<std::uint64_t>::max()}) {
        const auto decoded = parse_json_object("{\"v\":" + json_exact_uint64(value) + "}");
        check(decoded.at("v").text == std::to_string(value), "uint64 JSON digits remain exact");
        check(decoded.at("v").kind == (value <= kWireSafeInteger ? JsonValue::Kind::Number : JsonValue::Kind::String),
              "uint64 wire representation changes only outside safe interval");
        NativeTelemetryScheduler::CachedView view;
        view.source_epoch = value; view.epoch = value; view.snapshot_version = value;
        view.available_mask = view.fresh_mask = TELEMETRY_DIAGNOSTICS;
        view.snapshot.diagnostics_meta.epoch = value;
        view.snapshot.diagnostics_meta.state_version = value;
        view.group_snapshot_versions[0] = value;
        const auto telemetry = parse_json_object(json_telemetry_cached(view, TELEMETRY_DIAGNOSTICS));
        check(telemetry.at("sourceEpoch").text == std::to_string(value)
              && telemetry.at("epoch").text == std::to_string(value)
              && telemetry.at("snapshotVersion").text == std::to_string(value), "envelope preserves wide counters");
        const auto& group = telemetry.at("groupMeta").at("diagnostics");
        check(group.at("stateVersion").text == std::to_string(value)
              && group.at("backendStateVersion").text == std::to_string(value), "group versions preserve wide counters");
    }
    NativeObservation meta;
    meta.request_id = kWireSafeInteger; meta.sample_tick = 123;
    meta.source_epoch = std::numeric_limits<std::uint64_t>::max();
    meta.epoch = kWireSafeInteger + 2; meta.lattice_size = 4;
    meta.physical_time = 7.25; meta.dt = .125;
    meta.instance_nonce = {{0x0123456789abcdefull, 0xfedcba9876543210ull}};
    std::vector<std::uint8_t> wire(92, 0xaa);
    write_native_observation_header(wire, meta);
    check(little(wire, 0, 4) == kNativeObservationMagic && little(wire, 4, 4) == 88, "explicit envelope layout");
    check(little(wire, 8, 8) == meta.request_id && little(wire, 24, 8) == meta.source_epoch
          && little(wire, 32, 8) == meta.epoch && little(wire, 40, 8) == 4, "wide header fields exact");
    check(little(wire, 72, 8) == meta.instance_nonce[0] && little(wire, 80, 8) == meta.instance_nonce[1]
          && little(wire, 88, 4) == 0xaaaaaaaau, "namespace order and untouched nested payload");
    auto invalid = meta; invalid.request_id = 0;
    const auto before = wire;
    bool rejected = false;
    try { write_native_observation_header(wire, invalid); } catch (const std::invalid_argument&) { rejected = true; }
    check(rejected && wire == before, "invalid metadata rejects before writing header");

    int size = 4;
    auto bridge = std::make_unique<RenderBridge>(size);
    bridge->inject_particle(1, 1, 1, 1, {0.125, -0.0625, 0.03125});
    NativeTelemetryScheduler scheduler;
    scheduler.on_source_replaced(*bridge);
    const auto rng = bridge->rng_state_hash();
    ws_test::Pair pair;
    std::uint8_t opcode = 0;
    const auto command = [&](const std::string& request) {
        check(handle_command(request, pair.server, bridge, scheduler, size), "command stays connected");
        return receive(pair.client, opcode);
    };
    const auto info_bytes = command("{\"cmd\":\"info\",\"_requestId\":1}");
    const auto info = parse_json_object(std::string(info_bytes.begin(), info_bytes.end()));
    check(info.integer("nativeProtocolVersion", 0, 10) == 3 && info.string("nativeInstanceId") == native_instance_id(), "authoritative capability and namespace");
    for (const std::string request : {"\"cmd\":\"get_particles\"", "\"cmd\":\"get_flux_volume\",\"axisSamples\":4",
                                       "\"cmd\":\"get_field_sample\",\"kind\":\"e\",\"stride\":1,\"token\":27"}) {
        const auto legacy = command("{" + request + "}");
        check(opcode == WS_BINARY, "legacy sample is binary");
        const auto v3 = command("{" + request + ",\"_binaryVersion\":3,\"_requestId\":17}");
        if (opcode != WS_BINARY) std::cerr << "Unexpected v3 response: " << std::string(v3.begin(), v3.end()) << '\n';
        check(opcode == WS_BINARY && v3.size() == legacy.size() + 88
              && std::equal(legacy.begin(), legacy.end(), v3.begin() + 88), "v3 preserves exact legacy sampled bytes");
        check(little(v3, 8, 8) == 17 && little(v3, 16, 8) == 0 && little(v3, 24, 8) == scheduler.source_epoch()
              && little(v3, 32, 8) == scheduler.epoch(), "v3 stamps actual source and represented tick");
        check(little(v3, 72, 8) == native_instance_nonce()[0] && little(v3, 80, 8) == native_instance_nonce()[1], "v3 namespace matches info");
    }
    check(bridge->current_tick() == 0 && bridge->rng_state_hash() == rng, "observation namespace and encoding do not advance physics or RNG");
    bridge->inject_flux(3, 2, 2, {0.5, 0, 0});
    const auto final_plane = pack_field_sample(*bridge, VisualFieldKind::FluxVector, 1, 27, 3);
    check(little(final_plane, 16, 4) > 0, "last-plane witness is populated");
    check(pack_field_sample(*bridge, VisualFieldKind::FluxVector, 1, 27, std::numeric_limits<int>::max()) == final_plane,
          "INT_MAX slice mid clips before grid-rounding arithmetic");
    {
        RenderBridge even(8);
        even.inject_flux(7, 1, 1, {0.5, 0, 0});
        const auto edge = pack_field_sample(even, VisualFieldKind::FluxVector, 2, 27, 7);
        check(little(edge, 24, 4) == 1 && little(edge, 16, 4) == 1,
              "noninterior center-anchored even grid retains sampled final plane");
    }
    for (const std::string request : {
            "{\"cmd\":\"get_particles\",\"_binaryVersion\":3}",
            "{\"cmd\":\"tick\",\"_binaryVersion\":4,\"_requestId\":19}",
            "{\"cmd\":\"tick\",\"_binaryVersion\":2.5,\"_requestId\":20}",
            "{\"cmd\":\"get_particles\",\"_requestId\":21}",
            "{\"cmd\":\"get_flux_volume\",\"_binaryVersion\":2,\"_requestId\":22}",
            "{\"cmd\":\"get_field_sample\",\"kind\":\"e\",\"_requestId\":23}"}) {
        const auto bytes = command(request);
        const auto error = parse_json_object(std::string(bytes.begin(), bytes.end()));
        check(opcode == WS_TEXT && error.has("error") && bridge->current_tick() == 0, "malformed version rejects before mutation");
    }
    for (const std::string request : {
            "\"cmd\":\"tick\"", "\"cmd\":\"run\",\"n\":1",
            "\"cmd\":\"get_flux_slice\",\"axis\":0,\"index\":1",
            "\"cmd\":\"inspect_voxel\",\"x\":1,\"y\":1,\"z\":1",
            "\"cmd\":\"set_toggle\",\"name\":\"selective_damping\",\"value\":false",
            "\"cmd\":\"inject_flux\",\"x\":0,\"y\":0,\"z\":0,\"fx\":0.125,\"fy\":0,\"fz\":0"}) {
        const auto bytes = command("{" + request + ",\"_requestId\":9007199254740991}");
        const auto response = parse_json_object(std::string(bytes.begin(), bytes.end()));
        check(opcode == WS_TEXT && !response.has("error") && response.integer("_requestId", 1, kJsonSafeInteger) == kJsonSafeInteger,
              "special response and formerly silent mutator preserve correlation");
    }
    std::cout << checks - failures << '/' << checks << " checks passed\n";
    return failures ? 1 : 0;
}

int main() {
    try { return run_tests(); }
    catch (const std::exception& error) {
        std::cerr << "Unhandled fixture failure after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
