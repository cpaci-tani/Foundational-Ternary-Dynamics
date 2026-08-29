/**
 * Native CPU qualification for the Scale-0 `empty` null control.
 *
 * Scope is deliberately narrow: after an explicit empty-scenario setup, every
 * stored lattice channel must remain in its exact default state while all
 * observer/telemetry reads remain finite and report no activity.  This test
 * makes no claim about physical vacuum structure or any non-null scenario.
 */

#include "ftd/dynamical_state_digest.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/test_telemetry.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>

namespace {

constexpr std::array<int, 5> k_lattice_sizes = {8, 17, 33, 65, 97};
constexpr std::array<int, 5> k_checkpoints = {0, 1, 2, 8, 16};
constexpr std::array<int, 3> k_long_lattice_sizes = {8, 17, 33};
constexpr std::array<int, 3> k_long_checkpoints = {64, 256, 1024};

struct BoundaryCase {
    ftd::FluxBoundaryMode mode;
    const char* name;
};

constexpr std::array<BoundaryCase, 3> k_boundaries = {{
    {ftd::FluxBoundaryMode::Periodic, "periodic"},
    {ftd::FluxBoundaryMode::Reflective, "reflective"},
    {ftd::FluxBoundaryMode::Dispersal, "dispersal"},
}};

bool finite(double value) {
    return std::isfinite(value);
}

bool finite(const ftd::Vec3& value) {
    return finite(value.x) && finite(value.y) && finite(value.z);
}

bool zero(const ftd::Vec3& value) {
    return value.x == 0.0 && value.y == 0.0 && value.z == 0.0;
}

bool finite_and_zero(const std::vector<double>& values) {
    for (double value : values) {
        if (!finite(value) || value != 0.0) return false;
    }
    return true;
}

ftd::DynamicalStateDigest capture_digest(
    ftd::RenderBridge& bridge, const std::string& label) {
    ftd::DynamicalStateDigest digest{};
    ftd::test::check((label + " shared digest capture succeeds").c_str(),
        bridge.capture_dynamical_state_digest(digest));
    return digest;
}

bool same_digest_values(const ftd::DynamicalStateDigest& lhs,
                        const ftd::DynamicalStateDigest& rhs) {
    return lhs.hash_lo == rhs.hash_lo
        && lhs.hash_hi == rhs.hash_hi
        && lhs.nonfinite_value_count == rhs.nonfinite_value_count
        && lhs.nondefault_value_count == rhs.nondefault_value_count;
}

bool exact_cpu_default_digest(const ftd::DynamicalStateDigest& digest,
                              const ftd::RenderBridge& bridge,
                              int tick) {
    return digest.schema_version == ftd::DYNAMICAL_STATE_DIGEST_SCHEMA
        && digest.lattice_size == bridge.lattice().size()
        && digest.site_count == bridge.lattice().total_sites()
        && digest.tick == tick
        && digest.state_version == 0
        && digest.device_to_host_bytes == 0
        && digest.nonfinite_value_count == 0
        && digest.nondefault_value_count == 0
        && digest.exact_default_record();
}

bool voxel_is_finite(const ftd::Voxel& voxel) {
    return finite(voxel.flux)
        && finite(voxel.wave_vel)
        && finite(voxel.flux_L)
        && finite(voxel.flux_R)
        && finite(voxel.wave_vel_L)
        && finite(voxel.wave_vel_R)
        && finite(voxel.velocity)
        && finite(voxel.remainder)
        && finite(voxel.latency)
        && finite(voxel.tau)
        && finite(voxel.phase)
        && finite(voxel.accel_mag)
        && finite(voxel.flux_strong)
        && finite(voxel.wave_vel_strong)
        && finite(voxel.flux_weak)
        && finite(voxel.wave_vel_weak)
        && finite(voxel.chirality_density())
        && finite(voxel.density())
        && finite(voxel.speed())
        && finite(voxel.bandwidth_used())
        && finite(voxel.causal_budget())
        && finite(voxel.gamma_ftd())
        && finite(voxel.born_infeld_core());
}

bool voxel_is_exact_null(const ftd::Voxel& voxel) {
    return voxel.state == 0
        && zero(voxel.flux)
        && zero(voxel.wave_vel)
        && zero(voxel.flux_L)
        && zero(voxel.flux_R)
        && zero(voxel.wave_vel_L)
        && zero(voxel.wave_vel_R)
        && zero(voxel.velocity)
        && zero(voxel.remainder)
        && voxel.latency == 0.0
        && voxel.tau == 0.0
        && voxel.phase == 0.0
        && !voxel.locked
        && voxel.particle_id == -1
        && voxel.pair_id == -1
        && voxel.spin == 0
        && voxel.color == 0
        && voxel.flavor == 0
        && voxel.accel_mag == 0.0
        && zero(voxel.flux_strong)
        && zero(voxel.wave_vel_strong)
        && zero(voxel.flux_weak)
        && zero(voxel.wave_vel_weak);
}

bool force_diag_is_exact_null(const ftd::ForceDiag& force) {
    return zero(force.f_coulomb)
        && zero(force.f_strong)
        && zero(force.f_magnetic)
        && zero(force.f_gravity)
        && zero(force.f_exchange);
}

bool diagnostics_are_finite(const ftd::Diagnostics& diag) {
    if (!finite(diag.total_flux)
        || !finite(diag.total_energy)
        || !finite(diag.avg_drag)
        || !finite(diag.max_bandwidth)
        || !finite(diag.max_causal_budget)
        || !finite(diag.total_entropy)
        || !finite(diag.total_angular_momentum)) {
        return false;
    }
    for (int count : diag.color_count) {
        if (count < 0) return false;
    }
    return true;
}

bool diagnostics_report_no_activity(const ftd::Diagnostics& diag) {
    return diag.total_flux == 0.0
        && diag.avg_drag == 0.0
        && diag.max_bandwidth == 0.0
        && diag.max_causal_budget == 0.0
        && diag.causal_projection_events == 0
        && diag.manifested_count == 0
        && diag.positive_count == 0
        && diag.negative_count == 0
        && diag.total_entropy == 0.0
        && diag.spin_up_count == 0
        && diag.spin_down_count == 0
        && diag.color_count[0] == 0
        && diag.color_count[1] == 0
        && diag.color_count[2] == 0
        && diag.color_count[3] == 0
        && zero(diag.total_angular_momentum);
}

bool audit_is_finite(const ftd::EnergyAudit& audit) {
    return finite(audit.field_energy)
        && finite(audit.wave_energy)
        && finite(audit.particle_ke)
        && finite(audit.total_energy)
        && finite(audit.gauss_violation)
        && finite(audit.max_gauss_error)
        && finite(audit.self_field_injection)
        && finite(audit.coulomb_pe)
        && finite(audit.E_field_energy)
        && finite(audit.B_field_energy)
        && finite(audit.total_poynting)
        && finite(audit.E_L_total)
        && finite(audit.E_R_total)
        && finite(audit.wv_L_total)
        && finite(audit.wv_R_total)
        && finite(audit.chirality_total)
        && finite(audit.strong_energy)
        && finite(audit.weak_energy)
        && finite(audit.particle_rest_energy)
        && finite(audit.particle_energy)
        && finite(audit.particle_momentum)
        && finite(audit.dynamic_energy)
        && finite(audit.cell_volume)
        && finite(audit.field_energy_density_sum)
        && finite(audit.wave_energy_density_sum)
        && finite(audit.strong_potential_energy)
        && finite(audit.strong_gravitational_mass)
        && finite(audit.strong_projection_residual)
        && finite(audit.strong_projection_lambda);
}

bool audit_reports_no_activity(const ftd::EnergyAudit& audit) {
    return audit.field_energy == 0.0
        && audit.wave_energy == 0.0
        && audit.particle_ke == 0.0
        && audit.total_energy == 0.0
        && audit.gauss_violation == 0.0
        && audit.max_gauss_error == 0.0
        && audit.self_field_injection == 0.0
        && audit.coulomb_pe == 0.0
        && audit.E_field_energy == 0.0
        && audit.B_field_energy == 0.0
        && audit.charge_total == 0
        && audit.manifested_count == 0
        && zero(audit.total_poynting)
        && audit.E_L_total == 0.0
        && audit.E_R_total == 0.0
        && audit.wv_L_total == 0.0
        && audit.wv_R_total == 0.0
        && audit.chirality_total == 0.0
        && audit.strong_energy == 0.0
        && audit.weak_energy == 0.0
        && audit.particle_rest_energy == 0.0
        && audit.particle_energy == 0.0
        && zero(audit.particle_momentum)
        && audit.dynamic_energy == 0.0
        && audit.field_energy_density_sum == 0.0
        && audit.wave_energy_density_sum == 0.0
        && audit.strong_potential_energy == 0.0
        && audit.strong_gravitational_mass == 0.0
        && audit.strong_projection_residual == 0.0
        && audit.strong_projection_events == 0
        && audit.strong_projection_failures == 0
        && audit.strong_topology_failures == 0;
}

bool lagrangian_is_finite(const ftd::LagrangianDiag& diag) {
    return finite(diag.field_kinetic_sum)
        && finite(diag.field_gradient_sum)
        && finite(diag.born_infeld_sum)
        && finite(diag.coupling_sum)
        && finite(diag.velocity_coupling_sum)
        && finite(diag.gauss_sum)
        && finite(diag.dissipation_sum)
        && finite(diag.total_lagrangian)
        && finite(diag.total_hamiltonian)
        && finite(diag.total_action)
        && finite(diag.gauss_violation)
        && finite(diag.max_gauss_error)
        && finite(diag.total_flux_mag)
        && finite(diag.total_wave_energy)
        && finite(diag.cell_volume);
}

bool lagrangian_reports_no_activity(const ftd::LagrangianDiag& diag) {
    return diag.field_kinetic_sum == 0.0
        && diag.field_gradient_sum == 0.0
        && diag.coupling_sum == 0.0
        && diag.velocity_coupling_sum == 0.0
        && diag.gauss_sum == 0.0
        && diag.dissipation_sum == 0.0
        && diag.gauss_violation == 0.0
        && diag.max_gauss_error == 0.0
        && diag.total_flux_mag == 0.0
        && diag.total_wave_energy == 0.0
        && diag.manifested_count == 0
        && diag.locked_count == 0;
}

bool state_independent_baseline_is_exact(
    const ftd::Diagnostics& diagnostics,
    const ftd::LagrangianDiag& lagrangian) {
    // The legacy-named diagnostics.total_energy and the kinematic
    // Lagrangian/Hamiltonian evaluate the documented state-independent Born
    // core at every site.  That structural observer baseline is not lattice
    // activity and must not be relabelled as such merely to make an empty
    // scenario read zero.
    return lagrangian.born_infeld_sum < 0.0
        && diagnostics.total_energy == -lagrangian.born_infeld_sum
        && lagrangian.total_lagrangian == lagrangian.born_infeld_sum
        && lagrangian.total_action == lagrangian.total_lagrangian
        && lagrangian.total_hamiltonian == -lagrangian.born_infeld_sum;
}

bool ledger_is_finite(const ftd::EnergyLedger& ledger) {
    return finite(ledger.E_prev)
        && finite(ledger.E_curr)
        && finite(ledger.dE_dt)
        && finite(ledger.drift_frac)
        && finite(ledger.expected_rate)
        && finite(ledger.residual)
        && finite(ledger.cumulative_injection)
        && finite(ledger.cumulative_dissipation)
        && finite(ledger.max_residual_seen);
}

bool ledger_reports_no_activity(const ftd::EnergyLedger& ledger, int tick) {
    const int expected_previous_tick = tick == 0 ? -1 : (tick == 1 ? 1 : tick - 1);
    return ledger.updates == static_cast<std::uint64_t>(tick)
        // The first ledger sample seeds both endpoints at the current tick;
        // later samples describe the preceding completed interval.
        && ledger.tick_prev == expected_previous_tick
        && ledger.E_prev == 0.0
        && ledger.E_curr == 0.0
        && ledger.dE_dt == 0.0
        && ledger.drift_frac == 0.0
        && ledger.expected_rate == 0.0
        && ledger.residual == 0.0
        && ledger.cumulative_injection == 0.0
        && ledger.cumulative_dissipation == 0.0
        && ledger.max_residual_seen == 0.0;
}

bool gravity_is_finite(const ftd::GravityMetricAgg& gravity) {
    return finite(gravity.latency_max)
        && finite(gravity.latency_mean)
        && finite(gravity.f_min)
        && finite(gravity.gamma_max)
        && finite(gravity.dilation_max_pct);
}

bool gravity_reports_no_activity(const ftd::GravityMetricAgg& gravity) {
    return gravity.latency_max == 0.0
        && gravity.latency_mean == 0.0
        && gravity.f_min == 1.0
        && gravity.gamma_max == 1.0
        && gravity.dilation_max_pct == 0.0
        && gravity.voxel_count == 0
        && !gravity.requested
        && !gravity.active;
}

bool all_production_terms_are_off(const ftd::RenderBridge& bridge) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (bridge.toggles.*(spec.field)) return false;
    }
    return true;
}

std::string case_name(int lattice_size, const char* boundary, int tick) {
    return "L=" + std::to_string(lattice_size)
        + " boundary=" + boundary
        + " tick=" + std::to_string(tick);
}

void verify_checkpoint(const ftd::RenderBridge& bridge,
                       int lattice_size,
                       const BoundaryCase& boundary,
                       int tick) {
    const std::string prefix = case_name(lattice_size, boundary.name, tick);
    bool finite_state = true;
    bool exact_state = true;
    const std::size_t site_count = bridge.lattice().total_sites();
    finite_state = bridge.voxels().size() == site_count
        && bridge.force_diag().size() == site_count
        && bridge.delta_j().size() == site_count
        && bridge.dJ().size() == site_count
        && bridge.phi_coulomb().size() == site_count
        && bridge.phi_latency().size() == site_count;
    exact_state = finite_state
        && finite_and_zero(bridge.phi_coulomb())
        && finite_and_zero(bridge.phi_latency());
    for (const auto& voxel : bridge.voxels()) {
        finite_state = finite_state && voxel_is_finite(voxel);
        exact_state = exact_state && voxel_is_exact_null(voxel);
        if (!finite_state && !exact_state) break;
    }
    for (const auto& force : bridge.force_diag()) {
        exact_state = exact_state && force_diag_is_exact_null(force);
        if (!exact_state) break;
    }
    for (const auto& value : bridge.delta_j()) {
        finite_state = finite_state && finite(value);
        exact_state = exact_state && zero(value);
    }
    for (const auto& value : bridge.dJ()) {
        finite_state = finite_state && finite(value);
        exact_state = exact_state && zero(value);
    }

    ftd::test::check((prefix + " complete state is finite").c_str(), finite_state);
    ftd::test::check((prefix + " complete state is exact null").c_str(), exact_state);
    ftd::test::check((prefix + " state indexes and events are empty").c_str(),
        bridge.charge_sum() == 0
        && bridge.active_indices().empty()
        && bridge.ordered_active_indices().empty()
        && bridge.genesis_events_this_tick() == 0
        && bridge.evaporation_events_this_tick() == 0
        && bridge.causal_projection_events_this_tick() == 0);
    ftd::test::check((prefix + " clocks are finite and exact").c_str(),
        bridge.current_tick() == tick
        && finite(bridge.physical_time())
        && finite(bridge.dt())
        && bridge.physical_time() == static_cast<double>(tick) * bridge.dt());

    const ftd::Diagnostics diagnostics = bridge.diagnostics();
    ftd::test::check((prefix + " diagnostics are finite").c_str(),
        diagnostics_are_finite(diagnostics));
    ftd::test::check((prefix + " diagnostics report no activity").c_str(),
        diagnostics.tick == tick && diagnostics_report_no_activity(diagnostics));

    const ftd::EnergyAudit audit = bridge.energy_audit();
    ftd::test::check((prefix + " energy audit is finite").c_str(),
        audit_is_finite(audit));
    ftd::test::check((prefix + " energy audit reports no activity").c_str(),
        audit_reports_no_activity(audit));

    const ftd::LagrangianDiag lagrangian =
        ftd::compute_lagrangian_diagnostics(bridge);
    ftd::test::check((prefix + " Lagrangian telemetry is finite").c_str(),
        lagrangian_is_finite(lagrangian));
    ftd::test::check((prefix + " Lagrangian telemetry reports no activity").c_str(),
        lagrangian_reports_no_activity(lagrangian));
    ftd::test::check((prefix + " state-independent observer baseline is exact").c_str(),
        state_independent_baseline_is_exact(diagnostics, lagrangian));

    const ftd::EnergyLedger& ledger = bridge.energy_ledger();
    ftd::test::check((prefix + " energy ledger is finite").c_str(),
        ledger_is_finite(ledger));
    ftd::test::check((prefix + " energy ledger reports no activity").c_str(),
        ledger_reports_no_activity(ledger, tick));

    const ftd::GravityMetricAgg gravity = bridge.gravity_metric_agg();
    ftd::test::check((prefix + " gravity telemetry is finite").c_str(),
        gravity_is_finite(gravity));
    ftd::test::check((prefix + " gravity telemetry reports no activity").c_str(),
        gravity_reports_no_activity(gravity));
}

std::unique_ptr<ftd::RenderBridge> make_empty_bridge(
    int lattice_size, const BoundaryCase& boundary) {
    auto bridge = std::make_unique<ftd::RenderBridge>(lattice_size);
    bridge->force_cpu();
    ftd::test::check(
        ("L=" + std::to_string(lattice_size) + " CPU backend is active").c_str(),
        bridge->backend_kind() == ftd::Backend::Kind::Cpu);
    const bool dispatched = ftd::dispatch_scenario(*bridge, "empty");
    ftd::test::check(
        ("L=" + std::to_string(lattice_size) + " empty dispatch succeeds").c_str(),
        dispatched);
    bridge->toggles.flux_boundary = boundary.mode;

    std::string validation_error;
    ftd::test::check(
        ("L=" + std::to_string(lattice_size) + " " + boundary.name
         + " toggle profile is valid").c_str(),
        bridge->toggles.validate(&validation_error), validation_error.c_str());
    ftd::test::check(
        ("L=" + std::to_string(lattice_size) + " " + boundary.name
         + " keeps every production term off").c_str(),
        all_production_terms_are_off(*bridge));
    ftd::test::check(
        ("L=" + std::to_string(lattice_size) + " " + boundary.name
         + " boundary selection is exact").c_str(),
        bridge->toggles.flux_boundary == boundary.mode);
    return bridge;
}

void run_resolution_boundary_matrix() {
    ftd::test::section("Exact null-control matrix");
    for (int lattice_size : k_lattice_sizes) {
        for (const BoundaryCase& boundary : k_boundaries) {
            auto bridge = make_empty_bridge(lattice_size, boundary);
            int completed_ticks = 0;
            for (int checkpoint : k_checkpoints) {
                bridge->run(checkpoint - completed_ticks);
                completed_ticks = checkpoint;
                verify_checkpoint(*bridge, lattice_size, boundary, checkpoint);
            }
        }
    }
}

void run_long_duration_digest_matrix() {
    ftd::test::section("Long-duration exact-null and canonical-digest matrix");

    // Schema probes make the inclusion boundary executable: signed zero,
    // clocks, and identity bookkeeping cannot move the digest, while an
    // included dynamical field must move it with exact counters. Raw padding
    // is unobservable by the shared API's named-field schema.
    const BoundaryCase& schema_boundary = k_boundaries[0];
    auto schema_baseline = make_empty_bridge(8, schema_boundary);
    const auto schema_digest = capture_digest(
        *schema_baseline, "CPU schema baseline");
    ftd::test::check("CPU schema baseline has exact shared provenance/counters",
        exact_cpu_default_digest(schema_digest, *schema_baseline, 0));

    auto signed_zero_probe = make_empty_bridge(8, schema_boundary);
    signed_zero_probe->voxel_at(0, 0, 0).flux.x = -0.0;
    const auto signed_zero_digest = capture_digest(
        *signed_zero_probe, "CPU signed-zero probe");
    ftd::test::check("canonical digest treats an included -0 field as zero",
        same_digest_values(signed_zero_digest, schema_digest)
        && exact_cpu_default_digest(signed_zero_digest, *signed_zero_probe, 0));

    auto excluded_probe = make_empty_bridge(8, schema_boundary);
    auto& excluded_voxel = excluded_probe->voxel_at(0, 0, 0);
    excluded_voxel.tau = 3.0;
    excluded_voxel.phase = 4.0;
    excluded_voxel.particle_id = 17;
    excluded_voxel.pair_id = 23;
    const auto excluded_digest = capture_digest(
        *excluded_probe, "CPU clock/identity probe");
    ftd::test::check("canonical digest excludes clocks and identity bookkeeping",
        same_digest_values(excluded_digest, schema_digest)
        && exact_cpu_default_digest(excluded_digest, *excluded_probe, 0));

    auto included_probe = make_empty_bridge(8, schema_boundary);
    included_probe->voxel_at(0, 0, 0).flux.x = 1.0;
    const auto included_digest = capture_digest(
        *included_probe, "CPU included-field probe");
    ftd::test::check("canonical digest is sensitive to included dynamical fields",
        !same_digest_values(included_digest, schema_digest)
        && included_digest.nonfinite_value_count == 0
        && included_digest.nondefault_value_count == 1
        && !included_digest.exact_default_record());

    auto nonfinite_probe = make_empty_bridge(8, schema_boundary);
    nonfinite_probe->voxel_at(0, 0, 0).flux.x =
        std::numeric_limits<double>::quiet_NaN();
    const auto nonfinite_digest = capture_digest(
        *nonfinite_probe, "CPU nonfinite-field probe");
    ftd::test::check("canonical digest counts one nonfinite nondefault value",
        nonfinite_digest.nonfinite_value_count == 1
        && nonfinite_digest.nondefault_value_count == 1
        && !nonfinite_digest.exact_default_record());

    for (int lattice_size : k_long_lattice_sizes) {
        ftd::DynamicalStateDigest size_canonical_digest{};
        for (std::size_t boundary_index = 0;
             boundary_index < k_boundaries.size(); ++boundary_index) {
            const BoundaryCase& boundary = k_boundaries[boundary_index];
            auto bridge = make_empty_bridge(lattice_size, boundary);
            const auto initial_digest = capture_digest(
                *bridge, case_name(lattice_size, boundary.name, 0));
            ftd::test::check(
                (case_name(lattice_size, boundary.name, 0)
                 + " shared digest has exact default counters/provenance").c_str(),
                exact_cpu_default_digest(initial_digest, *bridge, 0));

            if (boundary_index == 0) {
                size_canonical_digest = initial_digest;
            }
            ftd::test::check(
                ("L=" + std::to_string(lattice_size) + " " + boundary.name
                 + " initial canonical digest is boundary-independent").c_str(),
                same_digest_values(initial_digest, size_canonical_digest));

            int completed_ticks = 0;
            for (int checkpoint : k_long_checkpoints) {
                bridge->run(checkpoint - completed_ticks);
                completed_ticks = checkpoint;
                verify_checkpoint(*bridge, lattice_size, boundary, checkpoint);
                const auto checkpoint_digest = capture_digest(
                    *bridge, case_name(lattice_size, boundary.name, checkpoint));
                ftd::test::check(
                    (case_name(lattice_size, boundary.name, checkpoint)
                     + " canonical dynamical digest is invariant").c_str(),
                    same_digest_values(checkpoint_digest, initial_digest)
                    && exact_cpu_default_digest(
                        checkpoint_digest, *bridge, checkpoint));
            }
        }
    }
}

void run_reload_reset_checks() {
    ftd::test::section("Native reconstruction and reload idempotence");
    constexpr int lattice_size = 33;
    constexpr int checkpoint = 8;

    for (const BoundaryCase& boundary : k_boundaries) {
        auto first = make_empty_bridge(lattice_size, boundary);
        first->run(checkpoint);
        verify_checkpoint(*first, lattice_size, boundary, checkpoint);

        // Native reset/setup transactions replace RenderBridge atomically.
        // Reconstructing here exercises that CPU contract without importing a
        // WASM or WebSocket lifecycle into this native unit test.
        first = make_empty_bridge(lattice_size, boundary);
        verify_checkpoint(*first, lattice_size, boundary, 0);
        first->run(checkpoint);
        verify_checkpoint(*first, lattice_size, boundary, checkpoint);

        // A repeated setup on an already-null, zero-tick bridge is itself
        // idempotent and must not seed state or advance either clock.
        auto repeated = make_empty_bridge(lattice_size, boundary);
        ftd::test::check(
            (std::string(boundary.name) + " repeated empty dispatch succeeds").c_str(),
            ftd::dispatch_scenario(*repeated, "empty"));
        repeated->toggles.flux_boundary = boundary.mode;
        verify_checkpoint(*repeated, lattice_size, boundary, 0);
    }
}

}  // namespace

int main() {
    ftd::test::init("test_empty_scenario_qualification");
    ftd::test::contract({
        "Scale-0 empty null control",
        "[IMPOSED] all-zero initial record; native-operator invariance is tested",
        "scenario id empty; explicit finite lattice size and boundary law",
        "none",
        "complete voxel state, canonical fieldwise dynamical digest, Diagnostics, EnergyAudit, EnergyLedger, LagrangianDiag, and GravityMetricAgg",
        "native CPU; short matrix L in {8,17,33,65,97}, ticks in {0,1,2,8,16}; long matrix L in {8,17,33}, ticks in {64,256,1024}; all three computational boundary laws",
        "CPU only in this target; cross-backend parity is a separate gate",
        "exact default lattice state, invariant clock-independent canonical digest, and zero activity at every checkpoint",
        "any non-default stored channel, non-finite readout, activity report, or reload drift rejects the null-control contract",
    });

    run_resolution_boundary_matrix();
    run_long_duration_digest_matrix();
    run_reload_reset_checks();
    return ftd::test::finalize();
}
