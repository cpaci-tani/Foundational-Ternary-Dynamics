#pragma once

#include "scale_engine.h"
#include "dag_lattice.h"
#include "constants.h"
#include "term_toggles.h"
#include <memory>
#include <vector>

namespace ftd {

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
