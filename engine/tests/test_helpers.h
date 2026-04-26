#pragma once
/**
 * @file test_helpers.h
 * @brief Shared test utilities — checks, inspectors, toggle presets.
 *
 * The audit (refactoring-analyst, 2026-04-25) found `check`, `check_close`
 * redefined in 112 / 48 test files respectively, plus repeated inspector
 * helpers (`total_flux`, `count_manifested`, `count_by_sign`) and toggle
 * preset boilerplate. This header consolidates them.
 *
 * Usage:
 *   #include "test_helpers.h"
 *   using ftd::test::check;
 *   using ftd::test::check_close;
 *   using ftd::test::enable_minimal_dynamics;
 *
 * Migration: tests that already define a local `check` can:
 *   (a) Delete the local definition and add `using ftd::test::check;` near
 *       the top of main(), OR
 *   (b) Leave the local definition (header-only, inline; no symbol clash
 *       since the local is at file scope and ftd::test is namespaced).
 *
 * The shared `failures` counter pattern is replaced by a small Counter struct
 * passed by reference. New tests should prefer the Counter form; legacy tests
 * can keep their `int failures = 0` global until migrated.
 */

#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

namespace ftd {
namespace test {

// ─── Counters ────────────────────────────────────────────────────────────
struct Counter {
    int passes = 0;
    int failures = 0;
    int total() const { return passes + failures; }
    bool ok() const { return failures == 0; }
};

// Module-level fallback counter for tests that don't pass one explicitly.
// Tests using the global form should declare `inline Counter g_counter;` in
// a .cpp file or rely on the static below at file scope.
inline Counter& global_counter() {
    static Counter c;
    return c;
}

// ─── Boolean checks ──────────────────────────────────────────────────────
inline void check(const char* name, bool condition, Counter* counter = nullptr) {
    Counter& c = counter ? *counter : global_counter();
    if (condition) {
        ++c.passes;
        std::cout << "  PASS  " << name << "\n";
    } else {
        ++c.failures;
        std::cout << "  FAIL  " << name << "\n";
    }
}

// ─── Closeness checks ────────────────────────────────────────────────────
inline void check_close(const char* name, double actual, double expected,
                        double tol, Counter* counter = nullptr) {
    Counter& c = counter ? *counter : global_counter();
    const bool ok = std::abs(actual - expected) <= tol;
    if (ok) {
        ++c.passes;
        std::cout << "  PASS  " << name << "\n";
    } else {
        ++c.failures;
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(8) << actual
                  << ", expected " << expected
                  << ", diff " << std::abs(actual - expected) << ")\n";
    }
}

// ─── Voxel-array inspectors ──────────────────────────────────────────────
inline double total_flux_density(const ftd::RenderBridge& rb) {
    double sum = 0.0;
    const auto& vox = rb.voxels();
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        sum += vox[i].density();
    return sum;
}

inline int count_manifested(const ftd::RenderBridge& rb) {
    int n = 0;
    const auto& vox = rb.voxels();
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        if (vox[i].state != 0) ++n;
    return n;
}

inline int count_by_sign(const ftd::RenderBridge& rb, int sign) {
    int n = 0;
    const auto& vox = rb.voxels();
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        if (vox[i].state == sign) ++n;
    return n;
}

inline int count_by_color(const ftd::RenderBridge& rb, int color) {
    int n = 0;
    const auto& vox = rb.voxels();
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        if (vox[i].color == color) ++n;
    return n;
}

inline int count_locked(const ftd::RenderBridge& rb) {
    int n = 0;
    const auto& vox = rb.voxels();
    for (int i = 0; i < rb.lattice().total_sites(); ++i)
        if (vox[i].locked) ++n;
    return n;
}

// ─── Toggle presets (RF-2 lightweight) ───────────────────────────────────
// Each preset starts from disable_all() so the caller's intent is explicit.
inline void enable_minimal_dynamics(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.movement         = true;
}

inline void enable_with_genesis(ftd::TermToggles& t) {
    enable_minimal_dynamics(t);
    t.genesis = true;
}

inline void enable_with_forces(ftd::TermToggles& t) {
    enable_minimal_dynamics(t);
    t.forces           = true;
    t.poisson_coulomb  = true;
}

inline void enable_emergent_eft(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.forces           = true;
    t.emergent_forces  = true;  // requires poisson_coulomb=false
    t.movement         = true;
}

inline void enable_dual_substrate_baseline(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation  = true;
    t.gauss_projection  = true;
    t.dual_substrate    = true;
    t.weak_transmutation= true;
    t.movement          = true;
}

// ─── Construction helper ─────────────────────────────────────────────────
// RenderBridge is non-copyable / non-movable (std::atomic + cuda handles +
// large vectors), so we can't return one by value. The helper instead
// initialises an existing bridge in place: typical use is
//
//   ftd::RenderBridge rb(L);
//   ftd::test::prepare_bridge(rb, /*force_cpu=*/true, /*seed=*/0xCAFEBABE);
//   ftd::test::enable_with_genesis(rb.toggles);
//
inline void prepare_bridge(ftd::RenderBridge& rb, bool force_cpu = true,
                           unsigned int seed = 0xCAFEBABEu) {
    rb.toggles.disable_all();
    if (force_cpu) rb.force_cpu();
    rb.seed_rng(seed);
}

// ─── Result reporter ─────────────────────────────────────────────────────
inline int report_and_exit_code(const Counter& c, const char* test_name = nullptr) {
    std::cout << "\n================================================================\n";
    if (test_name) std::cout << "  " << test_name << "\n";
    if (c.failures == 0) {
        std::cout << "  RESULT: " << c.passes << "/" << c.total() << " checks PASS\n";
    } else {
        std::cout << "  RESULT: FAILURES DETECTED ("
                  << c.failures << " of " << c.total() << ")\n";
    }
    std::cout << "================================================================\n";
    return c.ok() ? 0 : 1;
}

} // namespace test
} // namespace ftd
