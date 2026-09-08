/**
 * @file test_toggle_matrix.cpp
 * @brief Pairwise toggle-combination smoke test.
 *
 * Closes TEST-005 from CHECKLIST_ENGINE.md. The audit found that virtually
 * all existing tests use enable_all() / disable_all() ± a few flips. This
 * gate derives its inventory from TOGGLE_SPECS so new terms cannot silently
 * fall outside the interaction matrix.
 *
 * We enumerate every pair explicitly: each pair (i, j) with both ON,
 * everything else OFF except declared prerequisites. Cross-cutting setup that
 * cannot be represented by a simple requires_ edge (triad dual mode, cluster
 * force channel, and the legacy reflective boundary mode) is applied here.
 *
 * Pass criteria for each combination:
 *   (a) `validate()` either accepts the combo or rejects it cleanly (no
 *       crash); rejected combos are SKIPPED.
 *   (b) Engine ticks 5 times without throwing, NaN, or signal.
 *   (c) Energy ledger stays finite.
 *
 * Catches bugs of the form "toggle X + toggle Y silently misbehave when
 * combined" — the most common failure mode for new engine extensions.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <set>
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <string_view>

#include "ftd/render_bridge.h"
#include "ftd/scenario_profiles.h"
#include "ftd/scenarios.h"

namespace {

void clear_all(ftd::TermToggles& toggles) {
    for (const auto& spec : ftd::TOGGLE_SPECS)
        toggles.*(spec.field) = false;
    toggles.bcc_stencil = ftd::BccStencilMode::FULL;
    toggles.langevin_site_filter = ftd::SiteClass::ALL_SITES;
    toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    toggles.periodic_axis = ftd::PeriodicAxis::Z;
}

void enable_with_requirements(ftd::TermToggles& toggles,
                              const ftd::ToggleSpec& spec) {
    toggles.*(spec.field) = true;
    ftd::term_toggles_detail::for_each_csv(
        spec.requires_, [&](std::string_view dependency) {
            const auto* dependency_spec =
                ftd::term_toggles_detail::find_spec(dependency);
            if (dependency_spec && !(toggles.*(dependency_spec->field)))
                enable_with_requirements(toggles, *dependency_spec);
        });

    const std::string_view name(spec.name);
    if (name == "triad_binding")
        toggles.dual_substrate = true;
    if (name == "cluster_inertia")
        toggles.forces = true;
    if (name == "reflective_boundary")
        toggles.flux_boundary = ftd::FluxBoundaryMode::Reflective;
    if (name == "langevin") {
        toggles.langevin_T = 0.005;
        toggles.langevin_gamma = 0.02;
    }
}

bool finite_audit(const ftd::EnergyLedger& l) {
    return std::isfinite(l.E_curr) && std::isfinite(l.dE_dt) && std::isfinite(l.residual);
}

std::string enabled_terms(const ftd::TermToggles& toggles) {
    std::string out;
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (!(toggles.*(spec.field))) continue;
        if (!out.empty()) out += ',';
        out += spec.name;
    }
    return out.empty() ? "none" : out;
}

int audit_interactive_gpu_contract() {
    int failures = 0;
    std::string error;

    ftd::TermToggles clean;
    clean.disable_all();
    if (!clean.validate_backend(ftd::ToggleBackend::GPU, true, &error)) {
        std::printf("  FAIL  all-off profile rejected by interactive GPU validator: %s",
                    error.c_str());
        ++failures;
    }

    // Every registry row that excludes CUDA must fail before the server can
    // acknowledge it. This includes the CPU movement-order variant as well as
    // the selected CPU-only research integrators.
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (spec.backends & ftd::ToggleBackend::GPU) continue;
        ftd::TermToggles staged;
        staged.disable_all();
        staged.*(spec.field) = true;
        error.clear();
        if (staged.validate_backend(ftd::ToggleBackend::GPU, true, &error)) {
            std::printf("  FAIL  interactive GPU validator accepted CPU-only term %s\n",
                        spec.name);
            ++failures;
        }
    }

    for (const auto* name : {"knot_tracking"}) {
        const auto* spec = ftd::term_toggles_detail::find_spec(name);
        ftd::TermToggles staged;
        staged.disable_all();
        staged.*(spec->field) = true;
        error.clear();
        if (staged.validate_backend(ftd::ToggleBackend::GPU, true, &error)) {
            std::printf("  FAIL  full-GPU interactive validator accepted %s\n", name);
            ++failures;
        }
        error.clear();
        if (!staged.validate_backend(ftd::ToggleBackend::GPU, false, &error)) {
            std::printf("  FAIL  hybrid GPU campaign validator rejected %s: %s",
                        name, error.c_str());
            ++failures;
        }
    }

    for (const auto* name : {"cluster_inertia", "confinement"}) {
        const auto* spec = ftd::term_toggles_detail::find_spec(name);
        ftd::TermToggles staged;
        staged.disable_all();
        staged.*(spec->field) = true;
        if (std::string_view(name) == "cluster_inertia")
            staged.color_forces = true;
        if (std::string_view(name) == "confinement") staged.color_forces = true;
        error.clear();
        if (!staged.validate_backend(ftd::ToggleBackend::GPU, true, &error)) {
            std::printf("  FAIL  full-GPU interactive validator rejected %s: %s",
                        name, error.c_str());
            ++failures;
        }
    }

    ftd::TermToggles stochastic;
    stochastic.disable_all();
    stochastic.langevin = true;
    stochastic.dual_substrate = true;
    error.clear();
    if (stochastic.validate(&error)) {
        std::printf("  FAIL  validator accepted silently skipped dual-substrate Langevin\n");
        ++failures;
    }

    for (const char* writer : {"flux_pump", "flux_cell_port"}) {
        ftd::TermToggles staged;
        clear_all(staged);
        staged.matched_gauss_dynamics = true;
        const auto* writer_spec = ftd::term_toggles_detail::find_spec(writer);
        staged.*(writer_spec->field) = true;
        error.clear();
        if (staged.validate(&error)
            || error.find("isolated conservative movement sector") == std::string::npos) {
            std::printf("  FAIL  matched-Gauss validator accepted unjournaled writer %s\n",
                        writer);
            ++failures;
        }
    }

    return failures;
}

template <std::size_t N>
std::string profile_csv(const std::array<std::string_view, N>& terms) {
    std::string result;
    for (const std::string_view term : terms) {
        if (!result.empty()) result += ',';
        result += term;
    }
    return result;
}

std::map<std::string_view, std::string> expected_scenario_profiles() {
    std::map<std::string_view, std::string> expected;
    const auto add = [&](std::initializer_list<const char*> ids,
                         std::string profile) {
        for (const char* id : ids) expected.emplace(id, profile);
    };

    add({
        "empty", "flux-vortex", "flux-screening", "flux-triad",
        "quantum-entangle", "s0-seed-wilson-loop", "s0-seed-flux-tube",
        "s0-seed-monopole", "s0-seed-instanton", "s0-seed-schwarzschild",
        "s0-seed-time-horizon", "s0-seed-sloop", "s0-seed-observer-cell",
        "s0-field-uniform-e", "s0-field-uniform-b",
        "s0-field-electric-dipole", "s0-field-magnetic-dipole",
        "s0-field-vortex-line", "s0-seed-octahedron",
        "s0-seed-cuboctahedron", "s0-seed-stella-octangula",
        "s0-seed-moore-cell", "s0-seed-moore-decomposition",
    }, "none");

    add({
        "flux-pulse", "flux-dipole", "flux-standing",
        "flux-nested-standing", "flux-interference", "flux-dual-substrate",
        "flux-vacuum-foam", "flux-thermalization", "flux-zero-point",
        "quantum-well", "quantum-casimir",
        "s0-seed-up-quark", "s0-seed-down-quark",
        "s0-seed-strange-quark", "s0-seed-charm-quark",
        "s0-seed-bottom-quark", "s0-seed-top-quark",
        "s0-seed-anti-up-quark", "s0-seed-anti-down-quark",
        "s0-seed-anti-strange-quark", "s0-seed-anti-charm-quark",
        "s0-seed-anti-bottom-quark", "s0-seed-anti-top-quark",
        "s0-seed-higgs-field", "s0-seed-gluon",
        "s0-seed-gravitational-lensing", "s0-seed-gravitational-wave",
        "s0-seed-time-gravity-well", "s0-seed-time-twin-clocks",
        "s0-field-plane-wave", "s0-field-standing-wave",
        "s0-field-rf-lattice-wave", "s0-field-light-lattice-wave",
        "s0-field-sound-lattice-wave", "s0-field-sound-collision",
        "s0-field-spacetime-forcing-boundary",
        "s0-vacuum-electron", "s0-vacuum-muon", "s0-vacuum-tau",
        "s0-vacuum-positron", "s0-vacuum-antimuon", "s0-vacuum-antitau",
        "s0-vacuum-w-boson", "s0-vacuum-w-minus-boson",
        "s0-vacuum-z-boson", "s0-vacuum-higgs",
    }, "wave_propagation");

    add({
        "flux-soliton", "light-rainbow", "light-dipole", "light-two-slit",
        "light-photon-race", "quantum-double-slit",
        "quantum-aharonov-bohm", "s0-field-photon-pulse",
        "s0-vacuum-photon", "s0-vacuum-electron-neutrino",
        "s0-vacuum-muon-neutrino", "s0-vacuum-tau-neutrino",
        "s0-vacuum-electron-antineutrino",
        "s0-vacuum-muon-antineutrino", "s0-vacuum-tau-antineutrino",
    }, "wave_propagation,gauss_projection");

    add({"quantum-eraser", "quantum-tunnel"},
        "wave_propagation,coupling,gauss_projection");
    add({"s0-seed-dynamical-flux-dressing", "s0-field-thomson-scattering"},
        "wave_propagation,coupling");
    add({
        "flux-cascade", "flux-random-genesis", "flux-genesis-between-gates",
        "quantum-born-rule", "quantum-zeno",
    }, "genesis");
    add({
        "flux-annihilation", "flux-meson", "flux-string-breaking",
        "flux-baryon", "s0-seed-ee-annihilation",
    }, "movement");
    add({"flux-pair-production"}, "pair_production");
    add({"flux-cyclotron"}, "forces,poisson_coulomb,movement,lorentz_force");
    add({"s0-seed-beta-decay"}, "damping,dual_substrate,weak_transmutation");
    add({"s0-seed-massive-body"}, "gravity,latency_field");
    add({"s0-seed-de-broglie-clock"},
        "wave_propagation,de_broglie_clock");
    add({
        "s0-vacuum-proton", "s0-vacuum-neutron",
        "s0-vacuum-pion-charged", "s0-vacuum-pion-neutral",
        "s0-vacuum-kaon-charged",
    }, "forces,movement,color_forces");
    add({
        "s0-seed-hydrogen", "s0-seed-helium", "s0-seed-h2-bond-formation",
    }, profile_csv(ftd::PREPARED_COULOMB_CANDIDATE_TERMS));
    add({"s0-seed-spark-of-life"},
        "wave_propagation,coupling,damping,genesis,gauss_projection,forces,movement");
    add({"s0-seed-ew-phase-transition"},
        "wave_propagation,genesis,gauss_projection,ew_background_sweep");
    add({"s0-seed-quark-gluon-plasma"},
        "wave_propagation,gauss_projection,movement,langevin");
    add({
        "s0-seed-emergent-ic1", "s0-seed-emergent-ic3-collision",
        "s0-seed-emergent-ic4-subthreshold",
        "s0-seed-emergent-ic2-thermal-runaway",
        "s0-seed-emergent-ic1-diagonal", "s0-seed-emergent-ic1-isotropic",
        "s0-seed-emergent-ic1-viz",
        "s0-seed-emergent-ic1-diagonal-viz",
        "s0-seed-emergent-ic1-isotropic-viz", "s0-seed-cluster-law",
        "s0-seed-cluster-law-subknee", "s0-seed-cluster-law-knee",
        "s0-seed-cluster-law-superknee", "s0-seed-thermal-ignition",
    }, "wave_propagation,genesis,gauss_projection,langevin");
    add({"s0-field-thomson-unlocked-recoil"},
        "wave_propagation,coupling,forces,movement,emergent_forces");
    add({"s0-seed-moving-source-reciprocity"},
        "wave_propagation,coupling,forces,movement,emergent_forces,strict_validation");
    // s0-cell-* flux cells: the capacitor is Gauss-charged on the wave map;
    // every ring/arm reservoir runs the bare wave map (boundary law differs).
    add({"s0-cell-capacitor"}, "wave_propagation,gauss_projection");
    add({
        "s0-cell-torus", "s0-cell-torus-reverse", "s0-cell-torus-scrambled",
        "s0-cell-torus-open", "s0-cell-torus-walled", "s0-cell-triad",
    }, "wave_propagation");
    // Membrane family: the [IMPOSED] de Broglie clock makes the locked shell a
    // mass-gap wall; the gated and pumped variants add their research term.
    add({"s0-cell-torus-membrane"}, "wave_propagation,de_broglie_clock");
    add({"s0-cell-torus-membrane-gated"}, "wave_propagation,de_broglie_clock,flux_cell_port");
    add({"s0-cell-membrane-pumped"}, "wave_propagation,de_broglie_clock,flux_pump");
    add({"s0-cell-membrane-pumped-resonant"}, "wave_propagation,de_broglie_clock,flux_pump");
    add({"s0-cell-membrane-transfer"}, "wave_propagation,de_broglie_clock,flux_cell_port");

    return expected;
}

int audit_native_scenario_profiles() {
    int failures = 0;
    const auto& ids = ftd::scale0_scenario_ids();
    std::set<std::string_view> unique(ids.begin(), ids.end());
    const auto expected = expected_scenario_profiles();
    if (ids.size() != 142 || unique.size() != ids.size()) {
        std::printf("  FAIL  native scenario registry: count=%zu unique=%zu (expected 142)\n",
                    ids.size(), unique.size());
        ++failures;
    }
    if (expected.size() != ids.size()) {
        std::printf("  FAIL  expected profile table has %zu entries for %zu scenarios\n",
                    expected.size(), ids.size());
        ++failures;
    }

    for (const std::string_view id_view : ids) {
        const std::string id(id_view);
        // Native setup constructs a fresh bridge transaction, so this mirrors
        // the real server contract while keeping the catalog audit fast and
        // independent of CUDA availability.
        ftd::RenderBridge rb(16);
        rb.force_cpu();
        const bool handled = ftd::dispatch_scenario(rb, id);
        if (!handled) {
            std::printf("  FAIL  scenario %s is registered but not dispatched\n", id.c_str());
            ++failures;
            continue;
        }

        std::string validation_error;
        if (!rb.toggles.validate(&validation_error)) {
            std::printf("  FAIL  scenario %s has invalid final toggles: %s",
                        id.c_str(), validation_error.c_str());
            ++failures;
        }

        // The desktop application is CUDA-first. A public scenario may not
        // silently select a toggle whose registry explicitly excludes GPU.
        for (const auto& spec : ftd::TOGGLE_SPECS) {
            if ((rb.toggles.*(spec.field))
                && !(spec.backends & ftd::ToggleBackend::GPU)) {
                std::printf("  FAIL  scenario %s enables non-GPU term %s\n",
                            id.c_str(), spec.name);
                ++failures;
            }
        }

        bool finite = true;
        for (const auto& voxel : std::as_const(rb).voxels()) {
            finite = finite
                && std::isfinite(voxel.flux.x)
                && std::isfinite(voxel.flux.y)
                && std::isfinite(voxel.flux.z)
                && std::isfinite(voxel.wave_vel.x)
                && std::isfinite(voxel.wave_vel.y)
                && std::isfinite(voxel.wave_vel.z)
                && std::isfinite(voxel.velocity.x)
                && std::isfinite(voxel.velocity.y)
                && std::isfinite(voxel.velocity.z);
        }
        if (!finite) {
            std::printf("  FAIL  scenario %s seeds a non-finite voxel\n", id.c_str());
            ++failures;
        }

        const std::string actual_profile = enabled_terms(rb.toggles);
        const auto expected_it = expected.find(id);
        if (expected_it == expected.end()) {
            std::printf("  FAIL  scenario %s has no expected profile entry\n", id.c_str());
            ++failures;
        } else if (actual_profile != expected_it->second) {
            std::printf("  FAIL  scenario %s profile expected [%s], got [%s]\n",
                        id.c_str(), std::string(expected_it->second).c_str(),
                        actual_profile.c_str());
            ++failures;
        }

        std::printf("  PROFILE %-43s %s\n", id.c_str(),
                    actual_profile.c_str());
    }

    // Prefixes are routing hints, not proof that a body exists. This locks out
    // the historic success-with-an-empty-lattice behavior for typo'd IDs.
    ftd::RenderBridge unknown(8);
    unknown.force_cpu();
    if (ftd::dispatch_scenario(unknown, "quantum-not-a-scenario")
        || ftd::setup_quantum_scenario(unknown, "quantum-not-a-scenario")
        || ftd::setup_flux_scenario(unknown, "flux-not-a-scenario")
        || ftd::setup_light_scenario(unknown, "light-not-a-scenario")
        || ftd::setup_vacuum_scenario(unknown, "s0-vacuum-not-a-scenario")
        || ftd::setup_s0_seed_scenario(unknown, "s0-seed-not-a-scenario")
        || ftd::setup_s0_field_scenario(unknown, "s0-field-not-a-scenario")) {
        std::printf("  FAIL  an unknown scenario prefix was falsely accepted\n");
        ++failures;
    }

    // Exercise the actual default backend once. quantum-well stages two L^2
    // marker planes on the host; compact visual readback must perform the one
    // lazy upload and expose every marker. On CUDA this guards the old
    // per-marker full-lattice transfer loop; on CPU it verifies the same seed.
    {
        constexpr int L = 16;
        ftd::RenderBridge backend(L);
        if (!ftd::dispatch_scenario(backend, "quantum-well")) {
            std::printf("  FAIL  quantum-well backend staging dispatch failed\n");
            ++failures;
        } else {
            backend.tick();
            std::vector<std::int8_t> states;
            backend.copy_visual_states(states);
            const auto manifested = std::count_if(
                states.begin(), states.end(), [](std::int8_t s) { return s != 0; });
            if (manifested != 2 * L * L) {
                std::printf("  FAIL  staged marker upload exposed %td states (expected %d)\n",
                            manifested, 2 * L * L);
                ++failures;
            }
        }
    }

    return failures;
}

} // namespace

int main() {
    const std::size_t toggle_count =
        sizeof(ftd::TOGGLE_SPECS) / sizeof(ftd::TOGGLE_SPECS[0]);
    const std::size_t pair_count = toggle_count * (toggle_count - 1) / 2;
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  TEST-005: Toggle Pairwise Smoke Matrix (%zu toggles, %zu pairs)\n",
                toggle_count, pair_count);
    std::printf("================================================================\n\n");

    int failures = 0;
    int rejected_by_validate = 0;
    int pair_failures = 0;
    int passed = 0;

    std::printf("  Auditing native full-GPU profile contract...\n");
    failures += audit_interactive_gpu_contract();
    std::printf("\n");

    std::printf("  Auditing every native Scale-0 scenario profile...\n");
    failures += audit_native_scenario_profiles();
    std::printf("\n");

    enum class PairResult { Passed, Rejected, Failed };
    auto run_pair = [&](std::size_t i, std::size_t j) -> PairResult {
        ftd::RenderBridge rb(8);
        clear_all(rb.toggles);
        enable_with_requirements(rb.toggles, ftd::TOGGLE_SPECS[i]);
        enable_with_requirements(rb.toggles, ftd::TOGGLE_SPECS[j]);
        rb.force_cpu();
        rb.seed_rng(0xBADC0FFEEu + i * 100u + j);

        // Skip combinations that the validator rejects. (Not a failure —
        // the validator's job is exactly this.)
        std::string err;
        if (!rb.toggles.validate(&err)) {
            return PairResult::Rejected;
        }

        // Inject a small charged pair so something interesting can happen
        // for force/exchange/triad/pair_production-on combos.
        rb.inject_particle(3, 4, 4, +1, ftd::Vec3{0, 0, 0}, 0, 1);
        rb.inject_particle(5, 4, 4, -1, ftd::Vec3{0, 0, 0}, 0, 2);
        if (rb.toggles.matched_gauss_dynamics) {
            const auto initialized = rb.initialize_matched_gauss_dynamics();
            if (!initialized.valid) {
                std::printf("  FAIL  pair (%s, %s): matched-Gauss initialization failed\n",
                            ftd::TOGGLE_SPECS[i].name,
                            ftd::TOGGLE_SPECS[j].name);
                return PairResult::Failed;
            }
        }

        try {
            for (int t = 0; t < 5; ++t) rb.tick();
        } catch (const std::exception& e) {
            std::printf("  FAIL  pair (%s, %s): exception: %s\n",
                        ftd::TOGGLE_SPECS[i].name,
                        ftd::TOGGLE_SPECS[j].name, e.what());
            return PairResult::Failed;
        }

        if (!finite_audit(rb.energy_ledger())) {
            std::printf("  FAIL  pair (%s, %s): non-finite energy ledger\n",
                        ftd::TOGGLE_SPECS[i].name,
                        ftd::TOGGLE_SPECS[j].name);
            return PairResult::Failed;
        }
        return PairResult::Passed;
    };

    for (std::size_t i = 0; i < toggle_count; ++i) {
        for (std::size_t j = i + 1; j < toggle_count; ++j) {
            switch (run_pair(i, j)) {
                case PairResult::Passed:   ++passed; break;
                case PairResult::Rejected: ++rejected_by_validate; break;
                case PairResult::Failed:   ++pair_failures; break;
            }
        }
    }
    failures += pair_failures;

    std::printf("\n  Combinations tested: %d (passed) + %d (validator-rejected) + %d (failed)\n",
                passed, rejected_by_validate, pair_failures);
    std::printf("================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: PASS — all %d toggle pairs either validate and survive 5 ticks or reject cleanly\n",
                    passed + rejected_by_validate);
    } else {
        std::printf("  RESULT: FAIL — %d pairs trigger crash, exception, or non-finite state\n",
                    failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
