#pragma once

#include "scale_engine.h"
#include "dag_lattice.h"
#include "constants.h"
#include "term_toggles.h"
#include <memory>
#include <vector>

namespace ftd {

/**
 * @brief DagEngine — EXPERIMENTAL sparse-voxel-DAG prototype.
 *
 * ════════════════════════════════════════════════════════════════════════
 *                        ⚠  EXPERIMENTAL — DO NOT USE IN PRODUCTION  ⚠
 * ════════════════════════════════════════════════════════════════════════
 *
 * Original ambition: a sparse-voxel DAG engine that efficiently skips over
 * vast regions of void (unmanifested space) using a COW-backed octree.
 *
 * Current reality: `phase_read` and `phase_write` are implemented against
 * the SparseVoxelDAG, but `gauss_project`, `phase_forces`, and
 * `phase_movement` are empty `[OPEN]` stubs. Ticks run but nothing
 * gauge-couples, nothing forces, nothing moves.
 *
 * THE PRODUCTION ENGINE is `RenderBridge` (include/ftd/render_bridge.h,
 * src/render_bridge.cpp). It operates on a flat voxel array, has all
 * six phases implemented and tested, and is what the WASM build, the
 * benchmarks, and the browser dashboard actually use.
 *
 * This class is retained because:
 *   1. `SparseVoxelDAG` (in dag_lattice.h) is a useful data structure
 *      for future sparse-aware experiments (e.g., deep cosmological
 *      simulations where most of space is vacuum).
 *   2. `test_dag_engine` exercises the DAG's read/write semantics as
 *      a structural-parity check on the data structure itself.
 *   3. The phase_read / phase_write implementations document the
 *      octree-recursive dispatch pattern.
 *
 * To upgrade this class to production:
 *   - Implement the three stub phases on top of SparseVoxelDAG traversal
 *     (mirror RenderBridge's algorithms, but make writes COW-aware).
 *   - Add Gauss SOR that respects sparse topology (skip pure-void
 *     subtrees, which is the whole point of the DAG).
 *   - Add particle movement that uses the DAG's spatial index.
 *   - Add a comprehensive test suite parallel to RenderBridge's.
 *   - Remove this warning banner.
 *
 * Until then: use `RenderBridge`.
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
    void gauss_project();
    void phase_forces();
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
