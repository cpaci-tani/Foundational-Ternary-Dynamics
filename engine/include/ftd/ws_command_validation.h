#pragma once
#include "ftd/ws_json.h"
#include "ftd/term_toggles.h"
#include <limits>
#include <set>
#include <stdexcept>

namespace ftd {
// Pure preflight: every supplied command field is validated before dispatch,
// including fields a conditional branch might otherwise ignore. Semantic
// engine/backend toggle validation still applies to a staged profile later.
inline void validate_ws_command(const JsonValue& j) {
    const auto cmd = j.string("cmd");
    std::set<std::string> allowed{"cmd", "_requestId", "_binaryVersion"};
    if (j.has("_requestId")) j.integer("_requestId", 1, kJsonSafeInteger);
    if (j.has("_binaryVersion") && j.integer("_binaryVersion", 2, 3) == 3 && !j.has("_requestId"))
        throw std::invalid_argument("binary version 3 requires _requestId");
    const bool binary_command = cmd == "get_particles" || cmd == "get_flux_volume"
        || cmd == "get_field_sample" || cmd == "get_field_slices";
    if (binary_command && j.has("_requestId") && j.integer_or("_binaryVersion", 2, 2, 3) != 3)
        throw std::invalid_argument("correlated binary requests require _binaryVersion 3");
    auto add = [&](std::initializer_list<const char*> names) {
        for (auto name : names) allowed.insert(name);
    };
    auto str = [&](const char* key) { allowed.insert(key); return j.string(key); };
    auto integer = [&](const char* key, std::int64_t lo, std::int64_t hi) {
        allowed.insert(key); return j.integer(key, lo, hi);
    };
    constexpr auto imin = std::numeric_limits<int>::min();
    constexpr auto imax = std::numeric_limits<int>::max();
    auto coords = [&] { for (auto key : {"x", "y", "z"}) integer(key, imin, imax); };
    auto vector = [&](std::initializer_list<const char*> keys) {
        for (auto key : keys) { allowed.insert(key); j.number(key); }
    };
    if (cmd == "run") {
        add({"n"}); if (j.has("n")) j.integer("n", 1, imax);
    } else if (cmd == "inspect_voxel" || cmd == "get_force_at") coords();
    else if (cmd == "get_flux_slice") {
        integer("axis", 0, 2); integer("index", imin, imax);
    } else if (cmd == "get_flux_volume") {
        add({"axisSamples"}); if (j.has("axisSamples")) j.integer("axisSamples", imin, imax);
    } else if (cmd == "get_field_sample" || cmd == "get_field_slices") {
        str("kind"); add({"stride", "token"});
        // Preserve historical stride clamping and absent token default, after
        // exact decoding has ruled out undefined conversions.
        if (j.has("stride")) j.integer("stride", imin, imax);
        if (j.has("token")) j.integer("token", 0, 4294967295LL);
        if (cmd == "get_field_slices") { add({"mid"}); if (j.has("mid")) j.integer("mid", imin, imax); }
    } else if (cmd == "set_toggle") {
        str("name"); add({"value"}); j.boolean("value");
    } else if (cmd == "set_flux_boundary") integer("mode", 0, 2);
    else if (cmd == "set_flux_periodic_axis") integer("axis", 0, 3);
    else if (cmd == "set_param") {
        auto name = str("name"); add({"value"}); j.number("value");
        if (name == "langevin_seed") j.integer("value", 0, 4294967295LL);
    } else if (cmd == "inject_flux" || cmd == "inject_flux_add" || cmd == "create_pair") {
        coords(); vector({"fx", "fy", "fz"});
    } else if (cmd == "inject_wave_vel_add") {
        coords(); vector({"wx", "wy", "wz"});
    } else if (cmd == "inject_particle" || cmd == "inject_wavepacket") {
        coords(); integer("state", -1, 1);
        if (cmd == "inject_particle") vector({"fx", "fy", "fz"});
    } else if (cmd == "resize" || cmd == "preflight_resize") {
        integer("size", imin, imax); // Existing resource policy clamps [4,256].
    } else if (cmd == "setup_scenario" || cmd == "resize_scenario" || cmd == "apply_profile") {
        if (cmd == "resize_scenario") integer("size", imin, imax);
        add({"name", "applyProfile", "fluxBoundaryMode", "fluxPeriodicAxis"});
        if (cmd != "apply_profile" || j.has("name")) j.string("name");
        if (j.has("applyProfile")) j.boolean("applyProfile");
        bool profile_fields = false;
        for (const auto& spec : TOGGLE_SPECS) {
            const auto key = std::string("toggle_") + spec.name;
            allowed.insert(key);
            if (j.has(key)) { j.boolean(key); profile_fields = true; }
        }
        if (j.has("fluxBoundaryMode")) { j.integer("fluxBoundaryMode", 0, 2); profile_fields = true; }
        if (j.has("fluxPeriodicAxis")) { j.integer("fluxPeriodicAxis", 0, 3); profile_fields = true; }
        if ((cmd == "apply_profile" && j.has("applyProfile") && !j.boolean("applyProfile"))
            || (cmd != "apply_profile" && profile_fields && !j.boolean_or("applyProfile")))
            throw std::invalid_argument("profile fields conflict with disabled applyProfile");
        if (j.has("fluxBoundaryMode") && j.boolean_or("toggle_reflective_boundary")
            && j.integer("fluxBoundaryMode", 0, 2) != 1)
            throw std::invalid_argument("reflective boundary toggle conflicts with fluxBoundaryMode");
    } else if (cmd == "get_telemetry" || cmd == "set_telemetry_demand") {
        add({"diagnostics", "audit", "gravity", "lagrangian"});
        for (auto name : {"diagnostics", "audit", "gravity", "lagrangian"})
            if (j.has(name)) j.boolean(name);
        if (cmd == "set_telemetry_demand") {
            add({"mask", "everyTicks"});
            if (j.has("mask")) {
                const auto mask = j.integer("mask", 0, 15);
                unsigned bit = 1;
                for (auto name : {"diagnostics", "audit", "gravity", "lagrangian"}) {
                    if (j.has(name) && j.boolean(name) != ((mask & bit) != 0))
                        throw std::invalid_argument("telemetry mask conflicts with named group");
                    bit <<= 1;
                }
            }
            if (j.has("everyTicks")) {
                for (const auto& field : j.at("everyTicks").object()) {
                    if (field.first != "diagnostics" && field.first != "audit"
                        && field.first != "gravity" && field.first != "lagrangian")
                        throw std::invalid_argument("unknown cadence group");
                    field.second.integer(1, 65535);
                }
            }
        }
    } else if (cmd != "tick" && cmd != "get_particles" && cmd != "get_diagnostics"
               && cmd != "get_dynamical_state_digest" && cmd != "get_energy_audit"
               && cmd != "get_gravity_metric" && cmd != "get_lagrangian"
               && cmd != "reset" && cmd != "info") {
        throw std::invalid_argument("unknown command: " + cmd);
    }
    for (const auto& field : j.object()) {
        if (!allowed.count(field.first))
            throw std::invalid_argument("unexpected command field: " + field.first);
    }
}
} // namespace ftd
