#include "../src/ws_server_internal.h"
#include "ftd/ws_json.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
int checks = 0, failures = 0;
std::ofstream json_output;
void check(bool condition, const std::string& label) {
    ++checks;
    if (!condition) { ++failures; std::cerr << "FAIL " << label << '\n'; }
}
bool same_demand(const ftd::NativeTelemetryScheduler::Demand& a,
                 const ftd::NativeTelemetryScheduler::Demand& b) {
    return a.enabled_mask == b.enabled_mask && a.every_ticks == b.every_ticks;
}
void null_field(const ftd::JsonValue& object, const std::string& key) {
    check(object.has(key) && object.at(key).kind == ftd::JsonValue::Kind::Null,
          key + " is explicit null, not zero or a non-JSON token");
}
void finite_tree(const ftd::JsonValue& value) {
    if (value.kind == ftd::JsonValue::Kind::Number)
        check(std::isfinite(value.number()), "all parsed numeric tokens are finite");
    for (const auto& entry : value.members) finite_tree(entry.second);
    for (const auto& entry : value.elements) finite_tree(entry);
}
ftd::JsonValue parse_output(const std::string& raw) {
    if (json_output.is_open()) {
        json_output << raw << '\n';
        if (!json_output) throw std::runtime_error("cannot write JSONL serializer output");
    }
    // Literal checks are independent of the new parser's numeric conversion.
    check(raw.find(":nan") == std::string::npos && raw.find(":inf") == std::string::npos
        && raw.find(":-inf") == std::string::npos && raw.find(":NaN") == std::string::npos
        && raw.find(":Infinity") == std::string::npos, "no non-JSON float literals emitted");
    auto parsed = ftd::parse_json_object(raw);
    finite_tree(parsed);
    return parsed;
}

void test_demand_validation() {
    using namespace ftd;
    using namespace ftd::ws_server_detail;
    NativeTelemetryScheduler scheduler;
    NativeTelemetryScheduler::Demand baseline;
    baseline.enabled_mask = TELEMETRY_DIAGNOSTICS | TELEMETRY_GRAVITY;
    baseline.every_ticks = {{2, 7, 11, 19}};
    scheduler.set_demand(baseline);
    const std::string before_ack = json_telemetry_demand_ack(scheduler);
    const auto before_view = json_telemetry_cached(scheduler.latest(), TELEMETRY_ALL);
    const std::vector<std::string> invalid{
        "[]", "null", "{", "{}trailing", R"({"mask":1,"mask":2})",
        R"({"mask":true})", R"({"mask":null})", R"({"mask":"3"})",
        R"({"mask":-1})", R"({"mask":16})", R"({"mask":1.5})",
        R"({"mask":1.00000000000000000001})", R"({"mask":1e999})",
        R"({"audit":0})", R"({"audit":1})", R"({"audit":"false"})",
        R"({"diagnostics":null})", R"({"gravity":[]})", R"({"lagrangian":{}})",
        R"({"mask":1,"audit":true})", R"({"mask":15,"gravity":false})",
        R"({"mask":0,"audit":true})", R"({"everyTicks":null})",
        R"({"everyTicks":[]})", R"({"everyTicks":true})", R"({"everyTicks":"audit"})",
        R"({"everyTicks":{"audit":0}})", R"({"everyTicks":{"audit":-1}})",
        R"({"everyTicks":{"audit":65536}})", R"({"everyTicks":{"audit":1.5}})",
        R"({"everyTicks":{"audit":1.00000000000000000001}})",
        R"({"everyTicks":{"audit":"2"}})", R"({"everyTicks":{"audit":true}})",
        R"({"everyTicks":{"audit":null}})", R"({"everyTicks":{"audit":2,"audit":3}})",
        // Failure occurs after candidate fields have already been modified.
        R"({"mask":15,"everyTicks":{"diagnostics":2,"lagrangian":false}})",
        R"({"audit":true,"gravity":false,"everyTicks":{"audit":0}})",
        R"({"audit":false,"\u0061udit":true})",
    };
    for (const auto& raw : invalid) {
        NativeTelemetryScheduler::Demand out;
        out.enabled_mask = TELEMETRY_LAGRANGIAN;
        out.every_ticks = {{31, 37, 41, 43}};
        const auto sentinel = out;
        std::string error = "old error";
        check(!parse_telemetry_demand(raw, scheduler, out, error), "reject typed/malformed demand " + raw);
        check(!error.empty(), "rejection has an explicit error");
        check(same_demand(out, sentinel), "failure leaves output wholly unchanged");
        check(same_demand(scheduler.demand(), baseline), "failure preserves scheduler demand");
        check(json_telemetry_demand_ack(scheduler) == before_ack
            && json_telemetry_cached(scheduler.latest(), TELEMETRY_ALL) == before_view,
            "failure preserves scheduler publication, epoch, freshness and cadence");
    }
    struct Valid { const char* json; std::uint32_t mask; std::array<std::uint32_t, 4> cadence; };
    const std::vector<Valid> valid{
        {"{}", 5, {{2, 7, 11, 19}}},
        {R"({"audit":true,"gravity":false})", 3, {{2, 7, 11, 19}}},
        {R"({"mask":0})", 0, {{2, 7, 11, 19}}},
        {R"({"mask":15,"diagnostics":true,"audit":true,"gravity":true,"lagrangian":true})", 15, {{2, 7, 11, 19}}},
        {R"({"mask":3.0,"diagnostics":true,"audit":true,"gravity":false})", 3, {{2, 7, 11, 19}}},
        {R"({"everyTicks":{"diagnostics":1,"audit":65535,"gravity":2e1,"lagrangian":3.0}})", 5, {{1, 65535, 20, 3}}},
        {R"({"everyTicks":{}})", 5, {{2, 7, 11, 19}}},
    };
    for (const auto& item : valid) {
        NativeTelemetryScheduler::Demand out;
        std::string error = "previous failure";
        check(parse_telemetry_demand(item.json, scheduler, out, error), "valid demand accepted");
        check(error.empty() && out.enabled_mask == item.mask && out.every_ticks == item.cadence,
              "valid demand preserves exact typed values and absent-field defaults");
        check(json_telemetry_demand_ack(scheduler) == before_ack,
              "parse success remains separate from scheduler mutation");
    }
    check(telemetry_selection_mask("{}") == TELEMETRY_DIAGNOSTICS, "default selection diagnostics only");
    check(telemetry_selection_mask(R"({"audit":false})") == 0, "explicit false does not select default");
    check(telemetry_selection_mask(R"({"audit":true,"gravity":true})") == (TELEMETRY_AUDIT | TELEMETRY_GRAVITY), "typed true selection union");
    for (const std::string raw : {R"({"audit":1})", R"({"gravity":null})", R"({"audit":"true"})"}) {
        bool rejected = false;
        try { (void)telemetry_selection_mask(raw); } catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "selection rejects nonboolean supplied field");
    }
}

void test_nonfinite_snapshot() {
    using namespace ftd;
    using namespace ftd::ws_server_detail;
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    NativeTelemetryScheduler::CachedView view;
    view.source_epoch = 3; view.epoch = 7; view.tick = 9;
    view.snapshot_version = 13; view.available_mask = TELEMETRY_ALL; view.fresh_mask = TELEMETRY_ALL;
    view.group_snapshot_versions = {{10, 11, 12, 13}};
    view.min_interval_ms = {{0, 100, 200, 300}};
    auto& s = view.snapshot;
    for (auto* meta : {&s.diagnostics_meta, &s.audit_meta, &s.gravity_meta, &s.lagrangian_meta}) {
        meta->epoch = 7; meta->tick = 9; meta->physical_time = nan; meta->dt = inf; meta->lattice_size = 4;
    }
    s.diagnostics.tick = 9; s.diagnostics.total_flux = nan; s.diagnostics.total_energy = inf;
    s.diagnostics.max_bandwidth = -inf; s.diagnostics.avg_drag = nan; s.diagnostics.total_entropy = inf;
    s.diagnostics.total_angular_momentum = {nan, inf, -inf};
    s.audit.field_energy = nan; s.audit.wave_energy = inf; s.audit.particle_ke = -inf;
    s.audit.total_energy = nan; s.audit.dynamic_energy = inf; s.audit.cell_volume = nan;
    s.audit.total_poynting = {nan, inf, -inf}; s.audit.particle_momentum = {nan, inf, -inf};
    s.audit.strong_projection_residual = nan; s.audit.strong_projection_lambda = inf;
    s.gravity.active = true; s.gravity.requested = false; s.gravity.latency_max = nan;
    s.gravity.latency_mean = inf; s.gravity.f_min = -inf; s.gravity.gamma_max = nan;
    s.gravity.dilation_max_pct = inf;
    s.lagrangian.field_kinetic_sum = nan; s.lagrangian.field_gradient_sum = inf;
    s.lagrangian.total_lagrangian = -inf; s.lagrangian.total_hamiltonian = nan;
    s.lagrangian.total_action = inf; s.lagrangian.cell_volume = nan;
    const auto parsed = parse_output(json_telemetry_cached(view, TELEMETRY_ALL));
    const auto& groups = parsed.at("groups");
    for (const char* group : {"diagnostics", "audit", "gravity", "lagrangian"}) {
        null_field(parsed.at("groupMeta").at(group), "physicalTime");
        null_field(parsed.at("groupMeta").at(group), "dt");
        check(parsed.at("groupMeta").at(group).integer("tick", 0, 100) == 9, "nonfinite fields do not corrupt actual group clock");
    }
    for (const char* field : {"totalFlux", "totalEnergy", "maxBandwidth", "avgDrag", "entropy", "angMomX", "angMomY", "angMomZ", "physicalTime", "dt"})
        null_field(groups.at("diagnostics"), field);
    for (const char* field : {"fieldEnergy", "waveEnergy", "particleKE", "totalEnergy", "dynamicEnergy", "cellVolume", "poyntingX", "poyntingY", "poyntingZ", "particleMomentumX", "particleMomentumY", "particleMomentumZ", "strongProjectionResidual", "strongProjectionLambda"})
        null_field(groups.at("audit"), field);
    for (const char* field : {"latencyMax", "latencyMean", "fMin", "gammaMax", "dilationMaxPct"}) null_field(groups.at("gravity"), field);
    for (const char* field : {"fieldKinetic", "fieldGradient", "total", "hamiltonian", "totalAction", "cellVolume"}) null_field(groups.at("lagrangian"), field);
    check(groups.at("audit").number("gaussViolation") == 0, "real measured zero remains numeric");
    check(groups.at("gravity").boolean("active") && !groups.at("gravity").boolean("requested"), "booleans remain typed");
    check(parsed.integer("sourceEpoch", 0, 100) == 3 && parsed.integer("snapshotVersion", 0, 100) == 13,
          "envelope identity survives nonfinite channel values");
    for (const auto bit : {TELEMETRY_DIAGNOSTICS, TELEMETRY_AUDIT, TELEMETRY_GRAVITY, TELEMETRY_LAGRANGIAN})
        (void)parse_output(json_cached_legacy_group(view, bit));
    view.available_mask = TELEMETRY_DIAGNOSTICS;
    const auto partial = parse_output(json_telemetry_cached(view, TELEMETRY_ALL));
    check(partial.at("groups").members.size() == 1 && !partial.at("groupMeta").has("audit"),
          "unavailable groups are omitted rather than fabricated");
}

void test_nonfinite_inspection() {
    using namespace ftd;
    using namespace ftd::ws_server_detail;
    RenderBridge bridge(4); bridge.force_cpu();
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    auto& v = bridge.voxels()[bridge.lattice().index(0, 0, 0)];
    v.flux = {nan, inf, -inf}; v.wave_vel = {nan, inf, -inf}; v.velocity = {nan, inf, -inf};
    v.phase = nan; v.tau = inf; v.latency = -inf; v.accel_mag = nan;
    const auto voxel = parse_output(json_voxel(bridge, 0, 0, 0));
    for (const char* field : {"fluxX", "fluxY", "fluxZ", "density", "phase", "tau", "latency", "waveVelX", "waveVelY", "waveVelZ", "velX", "velY", "velZ", "speed", "accelMag", "Ex", "Ey", "Ez", "Emag"}) null_field(voxel, field);
    check(voxel.integer("x", 0, 3) == 0 && voxel.integer("state", -1, 1) == 0, "integer voxel metadata survives nonfinite values");

    // This is a corruption fixture on a non-const CPU object. Seed the actual
    // retained diagnostic buffer without running a nonfinite physics tick.
    auto& force = const_cast<ForceDiag&>(bridge.force_diag_at(0, 0, 0));
    force.f_coulomb = {nan, inf, -inf}; force.f_strong = {inf, -inf, nan};
    force.f_magnetic = {-inf, nan, inf}; force.f_gravity = {nan, inf, -inf};
    force.f_exchange = {nan, inf, -inf};
    const auto encoded_force = parse_output(json_force_at(bridge, 0, 0, 0));
    for (const std::string prefix : {"coulomb", "strong", "magnetic", "gravity", "exchange"})
        for (const std::string suffix : {"X", "Y", "Z", "Mag"}) null_field(encoded_force, prefix + suffix);
    const auto slice = parse_output(json_flux_slice(bridge, 2, 0));
    const auto& samples = slice.at("data");
    check(samples.kind == JsonValue::Kind::Array && samples.elements.size() == 16, "slice shape preserved");
    check(samples.elements[0].kind == JsonValue::Kind::Null, "nonfinite plane magnitude is null");
    check(samples.elements[1].number() == 0, "neighboring empty voxel remains numeric zero");
    check(bridge.current_tick() == 0, "read-only serializers do not step physics");
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 1 && (argc != 3 || std::string(argv[1]) != "--json-output")) {
        std::cerr << "Usage: test_ws_telemetry_validation [--json-output <path>]\n";
        return 2;
    }
    if (argc == 3) {
        json_output.open(argv[2], std::ios::out | std::ios::trunc);
        if (!json_output) {
            std::cerr << "Cannot open JSONL output: " << argv[2] << '\n';
            return 2;
        }
    }
#ifdef _WIN32
    _putenv_s("FTD_FORCE_CPU", "1");
#else
    setenv("FTD_FORCE_CPU", "1", 1);
#endif
    try {
        test_demand_validation(); test_nonfinite_snapshot(); test_nonfinite_inspection();
    } catch (const std::exception& e) {
        check(false, std::string("unexpected exception: ") + e.what());
    }
    if (json_output.is_open()) {
        json_output.flush();
        check(static_cast<bool>(json_output), "JSONL output flush succeeds");
        json_output.close();
        check(!json_output.fail(), "JSONL output close succeeds");
    }
    std::cout << checks << " telemetry checks, " << failures << " failures\n";
    return failures ? 1 : 0;
}
