/**
 * @file ws_server_telemetry.cpp
 * @brief Telemetry, inspection, and scalar JSON serialization.
 */

#include "ws_server_internal.h"

#include "ftd/lagrangian.h"
#include "ftd/ws_json.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ftd::ws_server_detail {

const char* telemetry_group_name(std::size_t index) {
    switch (index) {
    case 0: return "diagnostics";
    case 1: return "audit";
    case 2: return "gravity";
    default: return "lagrangian";
    }
}

constexpr std::array<std::uint32_t, 4> kTelemetryGroupBits{{
    ftd::TELEMETRY_DIAGNOSTICS,
    ftd::TELEMETRY_AUDIT,
    ftd::TELEMETRY_GRAVITY,
    ftd::TELEMETRY_LAGRANGIAN,
}};

const ftd::TelemetryGroupMeta& telemetry_group_meta(
    const ftd::TelemetrySnapshot& snapshot, std::size_t index) {
    switch (index) {
    case 0: return snapshot.diagnostics_meta;
    case 1: return snapshot.audit_meta;
    case 2: return snapshot.gravity_meta;
    default: return snapshot.lagrangian_meta;
    }
}

// RFC 8259 JSON has no NaN or Infinity literals. Preserve the channel's
// unavailability as JSON null so one unstable reduction cannot make the
// browser reject the entire telemetry frame (or be mistaken for zero).
std::string finite_json(double value, int precision = 17) {
    if (!std::isfinite(value)) return "null";
    std::ostringstream ss;
    ss << std::setprecision(precision) << value;
    return ss.str();
}

// All serializers below take immutable publisher values.  In particular, no
// function in this section accepts RenderBridge: `get_telemetry` and the
// legacy scalar commands must never turn a panel refresh into a CUDA reduction.
std::string json_diagnostics_value(
    const ftd::Diagnostics& d,
    const ftd::TelemetryGroupMeta& meta) {
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    ss << "\"tick\":"          << d.tick;
    ss << ",\"physicalTime\":" << finite_json(meta.physical_time, 10);
    ss << ",\"dt\":"           << finite_json(meta.dt, 10);
    ss << ",\"manifested\":"   << d.manifested_count;
    ss << ",\"positive\":"     << d.positive_count;
    ss << ",\"negative\":"     << d.negative_count;
    ss << ",\"totalFlux\":"    << finite_json(d.total_flux, 10);
    ss << ",\"totalEnergy\":"  << finite_json(d.total_energy, 10);
    ss << ",\"maxBandwidth\":" << finite_json(d.max_bandwidth, 10);
    ss << ",\"maxCausalBudget\":" << d.max_causal_budget;
    ss << ",\"causalProjectionEvents\":" << d.causal_projection_events;
    ss << ",\"avgDrag\":"      << finite_json(d.avg_drag, 10);
    ss << ",\"entropy\":"      << finite_json(d.total_entropy, 10);
    ss << ",\"chargeBalance\":" << (d.positive_count - d.negative_count);
    ss << ",\"spinUp\":"       << d.spin_up_count;
    ss << ",\"spinDown\":"     << d.spin_down_count;
    ss << ",\"colorless\":"    << d.color_count[0];
    ss << ",\"colorRed\":"     << d.color_count[1];
    ss << ",\"colorGreen\":"   << d.color_count[2];
    ss << ",\"colorBlue\":"    << d.color_count[3];
    ss << ",\"angMomX\":"      << finite_json(d.total_angular_momentum.x, 10);
    ss << ",\"angMomY\":"      << finite_json(d.total_angular_momentum.y, 10);
    ss << ",\"angMomZ\":"      << finite_json(d.total_angular_momentum.z, 10);
    ss << "}";
    return ss.str();
}

std::string json_energy_audit_value(const ftd::EnergyAudit& ea) {
    std::ostringstream ss;
    ss << std::setprecision(10);
    ss << "{";
    // NAMING (see diagnostics_compute.cpp): fieldEnergy is flux POTENTIAL energy
    // ½Σ|J|² (NOT E-field energy); EFieldEnergy below is byte-identical to
    // waveEnergy by construction (E = -wave_vel), and BFieldEnergy carries the
    // (c²/2) weight. Do not read "fieldEnergy vs BFieldEnergy" as "|E|² vs |B|²".
    ss << "\"fieldEnergy\":"        << finite_json(ea.field_energy, 10);
    ss << ",\"waveEnergy\":"        << finite_json(ea.wave_energy, 10);
    ss << ",\"particleKE\":"        << finite_json(ea.particle_ke, 10);
    ss << ",\"totalEnergy\":"       << finite_json(ea.total_energy, 10);
    ss << ",\"gaussViolation\":"    << finite_json(ea.gauss_violation, 10);
    ss << ",\"maxGaussError\":"     << finite_json(ea.max_gauss_error, 10);
    ss << ",\"selfFieldInjection\":" << finite_json(ea.self_field_injection, 10);
    ss << ",\"coulombPE\":"         << finite_json(ea.coulomb_pe, 10);
    ss << ",\"EFieldEnergy\":"      << finite_json(ea.E_field_energy, 10);
    ss << ",\"BFieldEnergy\":"      << finite_json(ea.B_field_energy, 10);
    ss << ",\"chargeTotal\":"       << ea.charge_total;
    ss << ",\"manifested\":"        << ea.manifested_count;
    ss << ",\"particleRestEnergy\":" << finite_json(ea.particle_rest_energy, 10);
    ss << ",\"particleEnergy\":"     << finite_json(ea.particle_energy, 10);
    ss << ",\"dynamicEnergy\":"      << finite_json(ea.dynamic_energy, 10);
    ss << ",\"particleMomentumX\":"  << finite_json(ea.particle_momentum.x, 10);
    ss << ",\"particleMomentumY\":"  << finite_json(ea.particle_momentum.y, 10);
    ss << ",\"particleMomentumZ\":"  << finite_json(ea.particle_momentum.z, 10);
    ss << ",\"poyntingX\":"         << finite_json(ea.total_poynting.x, 10);
    ss << ",\"poyntingY\":"         << finite_json(ea.total_poynting.y, 10);
    ss << ",\"poyntingZ\":"         << finite_json(ea.total_poynting.z, 10);
    ss << ",\"ELTotal\":"           << finite_json(ea.E_L_total, 10);
    ss << ",\"ERTotal\":"           << finite_json(ea.E_R_total, 10);
    ss << ",\"waveLTotal\":"        << finite_json(ea.wv_L_total, 10);
    ss << ",\"waveRTotal\":"        << finite_json(ea.wv_R_total, 10);
    ss << ",\"chiralityTotal\":"    << finite_json(ea.chirality_total, 10);
    ss << ",\"strongEnergy\":"      << finite_json(ea.strong_energy, 10);
    ss << ",\"weakEnergy\":"        << finite_json(ea.weak_energy, 10);
    ss << ",\"cellVolume\":"        << finite_json(ea.cell_volume, 10);
    ss << ",\"fieldEnergyDensitySum\":" << finite_json(ea.field_energy_density_sum, 10);
    ss << ",\"waveEnergyDensitySum\":" << finite_json(ea.wave_energy_density_sum, 10);
    ss << ",\"strongPotentialEnergy\":" << finite_json(ea.strong_potential_energy, 10);
    ss << ",\"strongGravitationalMass\":" << finite_json(ea.strong_gravitational_mass, 10);
    ss << ",\"strongProjectionResidual\":" << finite_json(ea.strong_projection_residual, 10);
    ss << ",\"strongProjectionLambda\":" << finite_json(ea.strong_projection_lambda, 10);
    ss << ",\"strongProjectionEvents\":" << ea.strong_projection_events;
    ss << ",\"strongProjectionFailures\":" << ea.strong_projection_failures;
    ss << ",\"strongTopologyFailures\":" << ea.strong_topology_failures;
    ss << "}";
    return ss.str();
}

std::string json_gravity_metric_value(const ftd::GravityMetricAgg& a) {
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"active\":" << (a.active ? "true" : "false")
       << ",\"requested\":" << (a.requested ? "true" : "false")
       << ",\"latencyMax\":" << finite_json(a.latency_max)
       << ",\"latencyMean\":" << finite_json(a.latency_mean)
       << ",\"fMin\":" << finite_json(a.f_min)
       << ",\"gammaMax\":" << finite_json(a.gamma_max)
       << ",\"dilationMaxPct\":" << finite_json(a.dilation_max_pct)
       << ",\"voxelCount\":" << a.voxel_count << "}";
    return ss.str();
}

std::string json_lagrangian_value(const ftd::TelemetryLagrangian& lag) {
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"fieldKinetic\":" << finite_json(lag.field_kinetic_sum)
       << ",\"fieldGradient\":" << finite_json(lag.field_gradient_sum)
       << ",\"bornInfeld\":" << finite_json(lag.born_infeld_sum)
       << ",\"coupling\":" << finite_json(lag.coupling_sum)
       << ",\"velocity\":" << finite_json(lag.velocity_coupling_sum)
       << ",\"gauss\":" << finite_json(lag.gauss_sum)
       << ",\"dissipation\":" << finite_json(lag.dissipation_sum)
       << ",\"total\":" << finite_json(lag.total_lagrangian)
       << ",\"hamiltonian\":" << finite_json(lag.total_hamiltonian)
       << ",\"totalAction\":" << finite_json(lag.total_action)
       << ",\"gaussViolation\":" << finite_json(lag.gauss_violation)
       << ",\"maxGaussError\":" << finite_json(lag.max_gauss_error)
       << ",\"totalFluxMag\":" << finite_json(lag.total_flux_mag)
       << ",\"totalWaveEnergy\":" << finite_json(lag.total_wave_energy)
       << ",\"manifested\":" << lag.manifested_count
       << ",\"locked\":" << lag.locked_count
       << ",\"cellVolume\":" << finite_json(lag.cell_volume) << "}";
    return ss.str();
}

void append_telemetry_groups(
    std::ostringstream& ss,
    const ftd::TelemetrySnapshot& snapshot,
    std::uint32_t mask) {
    bool first = true;
    const auto append = [&](std::size_t index, const std::string& value) {
        if (!first) ss << ',';
        first = false;
        ss << '"' << telemetry_group_name(index) << "\":" << value;
    };
    if (mask & ftd::TELEMETRY_DIAGNOSTICS)
        append(0, json_diagnostics_value(snapshot.diagnostics,
                                         snapshot.diagnostics_meta));
    if (mask & ftd::TELEMETRY_AUDIT)
        append(1, json_energy_audit_value(snapshot.audit));
    if (mask & ftd::TELEMETRY_GRAVITY)
        append(2, json_gravity_metric_value(snapshot.gravity));
    if (mask & ftd::TELEMETRY_LAGRANGIAN)
        append(3, json_lagrangian_value(snapshot.lagrangian));
}

void append_telemetry_group_meta(
    std::ostringstream& ss,
    const ftd::TelemetrySnapshot& snapshot,
    const std::array<std::uint64_t, 4>& group_snapshot_versions,
    std::uint32_t mask,
    std::uint64_t current_epoch) {
    bool first = true;
    for (std::size_t index = 0; index < kTelemetryGroupBits.size(); ++index) {
        const std::uint32_t bit = kTelemetryGroupBits[index];
        if ((mask & bit) == 0) continue;
        const auto& meta = telemetry_group_meta(snapshot, index);
        if (!first) ss << ',';
        first = false;
        ss << '"' << telemetry_group_name(index) << "\":{"
           << "\"epoch\":" << ftd::json_exact_uint64(meta.epoch)
           // `epoch` is the cross-backend mutation/version contract. The
           // GPU additionally exposes a device state counter; CPU's internal
           // count is merely a snapshot sequence, so keep it diagnostic-only.
           << ",\"stateVersion\":" << ftd::json_exact_uint64(meta.epoch)
           << ",\"backendStateVersion\":" << ftd::json_exact_uint64(meta.state_version)
           << ",\"tick\":" << meta.tick
           << ",\"snapshotVersion\":" << ftd::json_exact_uint64(group_snapshot_versions[index])
           << ",\"stale\":" << (meta.epoch == current_epoch ? "false" : "true")
           << ",\"physicalTime\":" << std::setprecision(17)
           << finite_json(meta.physical_time)
           << ",\"dt\":" << finite_json(meta.dt)
           << ",\"latticeSize\":" << meta.lattice_size
           << '}';
    }
}

std::string json_telemetry_envelope(
    const char* type,
    std::uint64_t snapshot_version,
    std::uint64_t source_epoch,
    std::uint64_t epoch,
    int tick,
    std::uint32_t available_mask,
    std::uint32_t fresh_mask,
    std::uint32_t pending_mask,
    std::uint32_t payload_mask,
    const ftd::TelemetrySnapshot& snapshot,
    const std::array<std::uint64_t, 4>& group_snapshot_versions,
    const std::array<std::uint32_t, 4>& min_interval_ms,
    std::optional<std::uint32_t> requested_mask = std::nullopt,
    std::optional<std::uint32_t> published_mask = std::nullopt) {
    std::ostringstream ss;
    ss << "{\"type\":\"" << type << "\""
       << ",\"nativeInstanceId\":\"" << ftd::native_instance_id() << "\""
       << ",\"snapshotVersion\":" << ftd::json_exact_uint64(snapshot_version)
       << ",\"sourceEpoch\":" << ftd::json_exact_uint64(source_epoch)
       << ",\"epoch\":" << ftd::json_exact_uint64(epoch)
       << ",\"tick\":" << tick
       << ",\"availableMask\":" << available_mask
       << ",\"freshMask\":" << fresh_mask
       << ",\"pendingMask\":" << pending_mask
       // This is the native producer's minimum wall-clock spacing for each
       // full-grid group.  It is a backpressure policy, not a browser-panel
       // refresh rate, and makes a delayed group explainable to the UI.
       << ",\"minIntervalMs\":{\"diagnostics\":" << min_interval_ms[0]
       << ",\"audit\":" << min_interval_ms[1]
       << ",\"gravity\":" << min_interval_ms[2]
       << ",\"lagrangian\":" << min_interval_ms[3] << '}';
    if (requested_mask) ss << ",\"requestedMask\":" << *requested_mask;
    if (published_mask) ss << ",\"publishedMask\":" << *published_mask;
    ss << ",\"groups\":{";
    append_telemetry_groups(ss, snapshot, payload_mask);
    ss << "},\"groupMeta\":{";
    append_telemetry_group_meta(ss, snapshot, group_snapshot_versions,
                                payload_mask, epoch);
    ss << '}';
    ss << "}";
    return ss.str();
}

std::string json_telemetry_cached(
    const ftd::NativeTelemetryScheduler::CachedView& view,
    std::uint32_t requested_mask) {
    const std::uint32_t payload_mask = requested_mask & view.available_mask;
    return json_telemetry_envelope(
        "telemetry", view.snapshot_version, view.source_epoch, view.epoch, view.tick,
        view.available_mask, view.fresh_mask, view.pending_mask, payload_mask,
        view.snapshot, view.group_snapshot_versions, view.min_interval_ms,
        requested_mask, std::nullopt);
}

std::string json_telemetry_publication(
    const ftd::NativeTelemetryScheduler::Publication& publication) {
    return json_telemetry_envelope(
        "telemetry_snapshot", publication.snapshot_version,
        publication.source_epoch, publication.epoch, publication.tick,
        publication.available_mask,
        publication.fresh_mask, publication.pending_mask,
        publication.published_mask, publication.snapshot,
        publication.group_snapshot_versions, publication.min_interval_ms,
        std::nullopt, publication.published_mask);
}

std::string json_telemetry_invalidation(
    const ftd::NativeTelemetryScheduler::Invalidation& invalidation) {
    // This is deliberately separate from telemetry_snapshot: it changes
    // freshness only. In particular, snapshotVersion remains the last
    // completed full-grid publication and must not be mistaken for a sample
    // of the newly-mutated state.
    std::ostringstream ss;
    ss << "{\"type\":\"telemetry_invalidated\""
       << ",\"nativeInstanceId\":\"" << ftd::native_instance_id() << "\""
       << ",\"sourceEpoch\":" << ftd::json_exact_uint64(invalidation.source_epoch)
       << ",\"epoch\":" << ftd::json_exact_uint64(invalidation.epoch)
       << ",\"tick\":" << invalidation.tick
       << ",\"snapshotVersion\":" << ftd::json_exact_uint64(invalidation.snapshot_version)
       << ",\"availableMask\":" << invalidation.available_mask
       << ",\"freshMask\":0"
       << ",\"pendingMask\":" << invalidation.pending_mask
       << ",\"reason\":\"" << json_escape(invalidation.reason) << "\"}";
    return ss.str();
}

std::string json_cached_legacy_group(
    const ftd::NativeTelemetryScheduler::CachedView& view,
    std::uint32_t bit) {
    if ((view.available_mask & bit) == 0) return {};
    if (bit == ftd::TELEMETRY_DIAGNOSTICS)
        return json_diagnostics_value(view.snapshot.diagnostics,
                                      view.snapshot.diagnostics_meta);
    if (bit == ftd::TELEMETRY_AUDIT)
        return json_energy_audit_value(view.snapshot.audit);
    if (bit == ftd::TELEMETRY_GRAVITY)
        return json_gravity_metric_value(view.snapshot.gravity);
    return json_lagrangian_value(view.snapshot.lagrangian);
}

std::string json_voxel(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto& read_rb = std::as_const(rb);
    const auto sample = read_rb.inspect_voxel(x, y, z);
    const auto& v = sample.voxel;
    const double div = sample.divergence;
    const auto& curl = sample.curl;
    const auto& em = sample.em;
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"x\":" << read_rb.lattice().wrap(x)
       << ",\"y\":" << read_rb.lattice().wrap(y)
       << ",\"z\":" << read_rb.lattice().wrap(z)
       << ",\"state\":" << static_cast<int>(v.state)
       << ",\"particleId\":" << v.particle_id
       << ",\"pairId\":" << v.pair_id
       << ",\"locked\":" << (v.locked ? "true" : "false")
       << ",\"spin\":" << static_cast<int>(v.spin)
       << ",\"color\":" << static_cast<int>(v.color)
       << ",\"fluxX\":" << finite_json(v.flux.x) << ",\"fluxY\":" << finite_json(v.flux.y)
       << ",\"fluxZ\":" << finite_json(v.flux.z) << ",\"density\":" << finite_json(v.density())
       << ",\"phase\":" << finite_json(v.phase) << ",\"tau\":" << finite_json(v.tau)
       << ",\"latency\":" << finite_json(v.latency)
       << ",\"waveVelX\":" << finite_json(v.wave_vel.x) << ",\"waveVelY\":" << finite_json(v.wave_vel.y)
       << ",\"waveVelZ\":" << finite_json(v.wave_vel.z)
       << ",\"velX\":" << finite_json(v.velocity.x) << ",\"velY\":" << finite_json(v.velocity.y)
       << ",\"velZ\":" << finite_json(v.velocity.z) << ",\"speed\":" << finite_json(v.speed())
       << ",\"accelMag\":" << finite_json(v.accel_mag)
       << ",\"divJ\":" << finite_json(div) << ",\"curlX\":" << finite_json(curl.x)
       << ",\"curlY\":" << finite_json(curl.y) << ",\"curlZ\":" << finite_json(curl.z)
       << ",\"Ex\":" << finite_json(em.E.x) << ",\"Ey\":" << finite_json(em.E.y)
       << ",\"Ez\":" << finite_json(em.E.z) << ",\"Emag\":" << finite_json(em.E_mag)
       << ",\"Bx\":" << finite_json(em.B.x) << ",\"By\":" << finite_json(em.B.y)
       << ",\"Bz\":" << finite_json(em.B.z) << ",\"Bmag\":" << finite_json(em.B_mag) << "}";
    return ss.str();
}

std::string json_force_at(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto fd = std::as_const(rb).inspect_force(x, y, z);
    std::ostringstream ss;
    ss << std::setprecision(17)
       << "{\"x\":" << rb.lattice().wrap(x)
       << ",\"y\":" << rb.lattice().wrap(y)
       << ",\"z\":" << rb.lattice().wrap(z)
       << ",\"coulombX\":" << finite_json(fd.f_coulomb.x)
       << ",\"coulombY\":" << finite_json(fd.f_coulomb.y)
       << ",\"coulombZ\":" << finite_json(fd.f_coulomb.z)
       << ",\"coulombMag\":" << finite_json(fd.f_coulomb.mag())
       << ",\"strongX\":" << finite_json(fd.f_strong.x)
       << ",\"strongY\":" << finite_json(fd.f_strong.y)
       << ",\"strongZ\":" << finite_json(fd.f_strong.z)
       << ",\"strongMag\":" << finite_json(fd.f_strong.mag())
       << ",\"magneticX\":" << finite_json(fd.f_magnetic.x)
       << ",\"magneticY\":" << finite_json(fd.f_magnetic.y)
       << ",\"magneticZ\":" << finite_json(fd.f_magnetic.z)
       << ",\"magneticMag\":" << finite_json(fd.f_magnetic.mag())
       << ",\"gravityX\":" << finite_json(fd.f_gravity.x)
       << ",\"gravityY\":" << finite_json(fd.f_gravity.y)
       << ",\"gravityZ\":" << finite_json(fd.f_gravity.z)
       << ",\"gravityMag\":" << finite_json(fd.f_gravity.mag())
       << ",\"exchangeX\":" << finite_json(fd.f_exchange.x)
       << ",\"exchangeY\":" << finite_json(fd.f_exchange.y)
       << ",\"exchangeZ\":" << finite_json(fd.f_exchange.z)
       << ",\"exchangeMag\":" << finite_json(fd.f_exchange.mag()) << "}";
    return ss.str();
}

std::string json_info(ftd::RenderBridge& rb,
                      const ftd::NativeTelemetryScheduler& telemetry) {
    const bool gpu_active = rb.backend_kind() == ftd::Backend::Kind::Gpu;
    const ResourceBudget current_budget = resource_budget(rb.lattice().size());
    std::ostringstream ss;
    ss << "{";
    ss << "\"latticeSize\":" << rb.lattice().size();
    ss << ",\"tick\":"       << rb.current_tick();
    ss << ",\"gpu\":"        << (gpu_active ? "true" : "false");
    ss << ",\"backend\":\"" << (gpu_active ? "cuda" : "cpu") << "\"";
    ss << ",\"version\":\"" << ftd::ENGINE_VERSION << "\"";
    ss << ",\"maxLatticeSize\":" << kMaxLatticeSize;
    ss << ",\"availableHostBytes\":" << current_budget.host_available;
    ss << ",\"availableGpuBytes\":" << current_budget.gpu_available;
    ss << ",\"interactiveGpuMode\":"
       << (rb.interactive_gpu_mode() ? "true" : "false");
    ss << ",\"maxVisualParticles\":" << kMaxVisualParticles;
    // Protocol v2 turns telemetry into a native publisher/cache rather than
    // an RPC that performs a fresh reduction for every side panel.
    ss << ",\"telemetryProtocolVersion\":2"
       << ",\"nativeProtocolVersion\":3,\"nativeBinaryVersions\":[2,3]"
       << ",\"nativeInstanceId\":\"" << ftd::native_instance_id() << "\""
       << ",\"exactIntegerEncoding\":\"safe-number-or-decimal-string\""
       << ",\"telemetryPush\":true"
       << ",\"telemetryRecoveryRequired\":"
       << (telemetry.suspended() ? "true" : "false")
       << ",\"restartRequired\":"
       << (telemetry.restart_required() ? "true" : "false")
       << ",\"sourceEpoch\":" << ftd::json_exact_uint64(telemetry.source_epoch())
       << ",\"telemetrySourceEpoch\":" << ftd::json_exact_uint64(telemetry.source_epoch())
       << ",\"telemetryEpoch\":" << ftd::json_exact_uint64(telemetry.epoch())
       << ",\"telemetrySnapshotVersion\":" << ftd::json_exact_uint64(telemetry.snapshot_version());
    if (telemetry.suspended()) {
        ss << ",\"telemetryRecoveryReason\":\""
           << json_escape(telemetry.suspension_reason()) << "\"";
    }
    ss << "}";
    return ss.str();
}

std::string json_visual_deferred(
    const char* operation,
    const ftd::NativeTelemetryScheduler& telemetry) {
    const auto view = telemetry.latest();
    std::ostringstream ss;
    ss << "{\"type\":\"visual_deferred\""
       << ",\"nativeInstanceId\":\"" << ftd::native_instance_id() << "\""
       << ",\"operation\":\"" << operation << "\""
       << ",\"reason\":\"telemetry_priority\""
       << ",\"sourceEpoch\":" << ftd::json_exact_uint64(view.source_epoch)
       << ",\"epoch\":" << ftd::json_exact_uint64(view.epoch)
       << ",\"tick\":" << view.tick
       << ",\"snapshotVersion\":" << ftd::json_exact_uint64(view.snapshot_version)
       << ",\"pendingMask\":" << view.pending_mask
       // Hint only: a GPU fence can take longer; the normal retry trigger is
       // the next telemetry_snapshot publication.
       << ",\"retryAfterMs\":16}";
    return ss.str();
}

// --------------------------------------------------------------------------
// Telemetry control-plane JSON helpers
// --------------------------------------------------------------------------
// Demand and selection use the bounded typed JSON parser shared by native
// commands. Validate into a candidate before publishing a changed demand.

std::uint32_t telemetry_selection_mask(const std::string& json) {
    const auto request = ftd::parse_json_object(json);
    std::uint32_t mask = 0;
    bool explicit_selection = false;
    const std::array<const char*, 4> names{{
        "diagnostics", "audit", "gravity", "lagrangian",
    }};
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (!request.has(names[i])) continue;
        explicit_selection = true;
        if (request.boolean(names[i])) mask |= kTelemetryGroupBits[i];
    }
    return explicit_selection ? mask : ftd::TELEMETRY_DIAGNOSTICS;
}

bool parse_telemetry_demand(const std::string& json,
                            const ftd::NativeTelemetryScheduler& scheduler,
                            ftd::NativeTelemetryScheduler::Demand& out,
                            std::string& error) {
    // Optional means absent. A supplied malformed field must not silently
    // inherit a prior value, and failure must leave the caller's output intact.
    try {
        const auto request = ftd::parse_json_object(json);
        auto candidate = scheduler.demand();
        const bool has_mask = request.has("mask");
        if (has_mask) {
            candidate.enabled_mask = static_cast<std::uint32_t>(
                request.integer("mask", 0, ftd::TELEMETRY_ALL));
        }
        const std::array<const char*, 4> names{{
            "diagnostics", "audit", "gravity", "lagrangian",
        }};
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (!request.has(names[i])) continue;
            const bool enabled = request.boolean(names[i]);
            if (has_mask && enabled != ((candidate.enabled_mask & kTelemetryGroupBits[i]) != 0)) {
                throw std::invalid_argument("telemetry mask conflicts with group selection");
            }
            if (enabled) candidate.enabled_mask |= kTelemetryGroupBits[i];
            else candidate.enabled_mask &= ~kTelemetryGroupBits[i];
        }
        if (request.has("everyTicks")) {
            const auto& cadence = request.at("everyTicks");
            (void)cadence.object();
            for (std::size_t i = 0; i < names.size(); ++i) {
                if (!cadence.has(names[i])) continue;
                candidate.every_ticks[i] = static_cast<std::uint32_t>(
                    cadence.integer(names[i], 1, ftd::NativeTelemetryScheduler::kMaxCadenceTicks));
            }
        }
        out = candidate;
        error.clear();
        return true;
    } catch (const std::invalid_argument& ex) {
        error = ex.what();
        return false;
    }
}

std::string json_telemetry_demand_ack(
    const ftd::NativeTelemetryScheduler& scheduler) {
    const auto& demand = scheduler.demand();
    const auto view = scheduler.latest();
    std::ostringstream ss;
    ss << "{\"type\":\"telemetry_demand\""
       << ",\"nativeInstanceId\":\"" << ftd::native_instance_id() << "\""
       << ",\"enabledMask\":" << demand.enabled_mask
       << ",\"everyTicks\":{"
       << "\"diagnostics\":" << demand.every_ticks[0]
       << ",\"audit\":" << demand.every_ticks[1]
       << ",\"gravity\":" << demand.every_ticks[2]
       << ",\"lagrangian\":" << demand.every_ticks[3]
       << "},\"minIntervalMs\":{"
       << "\"diagnostics\":" << view.min_interval_ms[0]
       << ",\"audit\":" << view.min_interval_ms[1]
       << ",\"gravity\":" << view.min_interval_ms[2]
       << ",\"lagrangian\":" << view.min_interval_ms[3]
       << "},\"snapshotVersion\":" << ftd::json_exact_uint64(view.snapshot_version)
       << ",\"sourceEpoch\":" << ftd::json_exact_uint64(view.source_epoch)
       << ",\"telemetrySourceEpoch\":" << ftd::json_exact_uint64(view.source_epoch)
       << ",\"epoch\":" << ftd::json_exact_uint64(view.epoch)
       << ",\"tick\":" << view.tick << '}';
    return ss.str();
}

bool flush_telemetry_publications(
    SOCKET client, ftd::NativeTelemetryScheduler& scheduler) {
    // Invalidation is a smaller, earlier state-boundary signal. Write it
    // before a later snapshot delta from the same single writer so a browser
    // cannot briefly label an old cached group fresh after a direct edit.
    while (const auto invalidation = scheduler.take_invalidation()) {
        if (!ftd::ws_send_text(client, json_telemetry_invalidation(*invalidation))) {
            return false;
        }
    }
    while (const auto publication = scheduler.take_publication()) {
        if (!ftd::ws_send_text(client, json_telemetry_publication(*publication))) {
            return false;
        }
    }
    return true;
}

std::string json_flux_slice(ftd::RenderBridge& rb, int axis, int index) {
    std::vector<float> magnitudes;
    rb.copy_visual_flux_magnitude_plane(axis, index, magnitudes);
    std::ostringstream ss;
    ss << std::setprecision(6);
    ss << "{\"type\":\"flux_slice\",\"axis\":" << axis << ",\"index\":" << index << ",\"data\":[";
    bool first = true;
    for (const float magnitude : magnitudes) {
        if (!first) ss << ",";
        // Keep the existing stream/precision for bulk samples; constructing
        // one temporary string stream per cell would add avoidable overhead.
        if (std::isfinite(magnitude)) ss << magnitude;
        else ss << "null";
        first = false;
    }
    ss << "]}";
    return ss.str();
}

}  // namespace ftd::ws_server_detail
