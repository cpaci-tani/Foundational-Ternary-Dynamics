#pragma once

#include "scale_engine.h"
#include "dag_lattice.h"
#include "constants.h"
#include "term_toggles.h"
#include <memory>
#include <vector>

namespace ftd {

/**
 * @brief DagEngine — EXPERIMENTAL / DEPRECATED sparse-voxel-DAG prototype.
 *
 * ════════════════════════════════════════════════════════════════════════
 *           ⚠  EXPERIMENTAL — DEPRECATED — DO NOT USE IN PRODUCTION  ⚠
 *                          (ticket W6 — deprecate-clearly)
 * ════════════════════════════════════════════════════════════════════════
 *
 * STATUS: experimental / deprecated, non-production. Partially built and
 * intentionally NOT completed. This is a data-structure prototype, not a
 * physics engine.
 *
 * Original ambition: a sparse-voxel DAG engine that efficiently skips over
 * vast regions of void (unmanifested space) using a COW-backed octree.
 *
 * WHAT WORKS (genuinely implemented, exercised by test_dag_engine):
 *   - `phase_read`   — 18-point Moore Laplacian + coupling source, walked
 *                      via the octree-recursive `recursive_read`.
 *   - `phase_write`  — Störmer–Verlet leapfrog update + damping, walked via
 *                      `recursive_write` with copy-on-write voxel writes.
 *   - `inject_flux`, `dag()`, `clear()`, the ScaleEngine bookkeeping
 *      accessors (`scale_level`, `scale_name`, `current_tick`, `dt`,
 *      `set_dt`, `entity_count`).
 *   These remain fully functional and MUST keep working.
 *
 * WHAT DOES NOT WORK (unimplemented stubs — were silent no-ops, now LOUD):
 *   - `gauss_project`  — no U(1) charge-conservation projection.
 *   - `phase_forces`   — no Coulomb / Lorentz / colour forces.
 *   - `phase_movement` — no particle movement / collisions.
 *   `tick()` therefore runs WAVE-ONLY: nothing gauge-couples, nothing
 *   forces, nothing moves. As of W6, `tick()` no longer silently invokes
 *   these stubs — it emits a one-time runtime warning and skips them, and
 *   each stub body `assert`s false if called directly (loud in debug).
 *
 * THE GOLDEN / PRODUCTION PHYSICS PATH is `RenderBridge`
 * (include/ftd/render_bridge.h, src/render_bridge.cpp): a flat voxel array
 * with all six phases implemented, tested, and golden-hash locked. It is
 * what the WASM build, the benchmarks, and the browser dashboard use.
 * DagEngine is NOT in the golden tick path and has no WASM/JS binding
 * (the Embind binding was intentionally removed — see ftd_wasm.cpp).
 *
 * This class is retained ONLY because:
 *   1. `SparseVoxelDAG` (in dag_lattice.h) is useful future infrastructure
 *      for sparse-aware experiments (deep cosmological sims, mostly vacuum).
 *   2. `test_dag_engine` exercises the DAG's read/write semantics as a
 *      structural-parity check on the data structure itself.
 *   3. `phase_read` / `phase_write` document the octree-recursive pattern.
 *
 * To upgrade this class to production (multi-week, currently NO consumer):
 *   - Implement the three stub phases on top of SparseVoxelDAG traversal
 *     (mirror RenderBridge's algorithms, but make writes COW-aware).
 *   - Add Gauss SOR that respects sparse topology (skip pure-void
 *     subtrees, which is the whole point of the DAG).
 *   - Add particle movement that uses the DAG's spatial index.
 *   - Add a comprehensive test suite parallel to RenderBridge's.
 *   - Remove the W6 [[deprecated]] markers, the runtime warning in tick(),
 *     the stub asserts, and this banner.
 *
 * Until then: use `RenderBridge`.
 *
 * NOTE ON [[deprecated]] GRANULARITY (W6): the attribute is placed on the
 * three unimplemented stub methods only, NOT on the class. The class is
 * legitimately constructed and ticked by test_dag_engine (a deliberate
 * keeper); a class-level attribute would spam unavoidable warnings on that
 * kept test. The stubs are private and no longer called, so the markers
 * emit zero warnings today while still flagging any future direct use.
 * ════════════════════════════════════════════════════════════════════════
 */
class DagEngine : public ScaleEngine {
public:
    explicit DagEngine(int lattice_size);
    ~DagEngine() override = default;

    int scale_level() const override { return 0; }
    const char* scale_name() const override { return "DagEngine"; }

    void tick() override;
    void clear() override;

    // ScaleEngine pure virtual overrides (stubs for DagEngine)
    int current_tick() const override { return tick_; }
    double dt() const override { return dt_; }
    void set_dt(double d) override { dt_ = d; }
    int entity_count() const override { return static_cast<int>(active_indices_.size()); }

    // Test API: force flux injection into the DAG
    void inject_flux(int x, int y, int z, double fx, double fy, double fz);

    // Provide read access to the DAG for tests
    const SparseVoxelDAG& dag() const { return *dag_; }

    ScaleBaseDiagnostics base_diagnostics() const override;
    bool get_toggle(const std::string& name) const override;
    void set_toggle(const std::string& name, bool value) override;

    // Structural read/write loops
    void recursive_read(int x, int y, int z, int current_size);
    void recursive_write(int x, int y, int z, int current_size);

private:
    void phase_read();
    void phase_write();

    // ── UNIMPLEMENTED STUBS (W6 deprecate-clearly) ─────────────────────────
    // These three phases were empty silent no-ops ("looks implemented but
    // isn't"). They are NOT implemented and are NOT wired into tick() any
    // more. Each body asserts false if invoked directly (loud in debug).
    // Marked [[deprecated]] at method granularity (see class banner) so any
    // future caller that wires them up gets a compile-time warning pointing
    // at this ticket. Do NOT remove the marker until the phase is actually
    // implemented. Use RenderBridge for real Gauss/force/movement physics.
    [[deprecated("DagEngine is a non-production experimental engine; RenderBridge "
                 "is the production path. gauss_project/phase_forces/phase_movement "
                 "are unimplemented stubs — see ticket W6 (deprecate-clearly).")]]
    void gauss_project();
    [[deprecated("DagEngine is a non-production experimental engine; RenderBridge "
                 "is the production path. gauss_project/phase_forces/phase_movement "
                 "are unimplemented stubs — see ticket W6 (deprecate-clearly).")]]
    void phase_forces();
    [[deprecated("DagEngine is a non-production experimental engine; RenderBridge "
                 "is the production path. gauss_project/phase_forces/phase_movement "
                 "are unimplemented stubs — see ticket W6 (deprecate-clearly).")]]
    void phase_movement();

    // Discrete physics operators over DAG
    Vec3 laplacian_flux(int x, int y, int z) const;
    Vec3 gradient_state(int x, int y, int z) const;
    Vec3 curl_state_velocity(int x, int y, int z) const;

    std::unique_ptr<SparseVoxelDAG> dag_;
    int tick_ = 0;
    double dt_ = 1.0;
    TermToggles toggles_;

    // Store deltas between phases
    std::vector<Vec3> delta_j_;
    // active_indices_ is the only one of the originally-declared trio that's
    // actually read (by entity_count). sweep_active_bounds() and
    // has_active_flag_ were declared but never defined/written -- removed.
    std::vector<int> active_indices_;
};

} // namespace ftd
