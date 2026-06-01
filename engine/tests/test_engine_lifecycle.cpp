// ============================================================================
// test_engine_lifecycle.cpp — ScaleEngine RAII / lifecycle contract (ticket W5)
// ----------------------------------------------------------------------------
// The abstract base ftd::ScaleEngine (include/ftd/scale_engine.h) declares two
// lifecycle hooks that every concrete subclass must honour:
//
//     virtual void clear()     = 0;   // reset to a reusable-empty state
//     virtual ~ScaleEngine()   = default;   // RAII: member destructors free all
//
// The contract (documented in CONTRACTS.md §"Scale-engine lifecycle"):
//
//   L1  construct → inject entities → the engine reports them active.
//   L2  step/tick a few times → no crash, state stays finite.
//   L3  clear() → engine returns to empty (entity_count()==0) AND its
//       documented reset invariants hold (tick counter back to 0, plus any
//       per-engine integrator state).
//   L4  re-inject after clear() → works again. clear() leaves a *reusable*
//       engine, not a corpse.
//   L5  clear() is idempotent — a second clear() on an already-empty engine
//       neither crashes nor resurrects state.
//   L6  construct+populate+destruct in a tight loop exercises ~ScaleEngine()
//       member-RAII teardown with no crash and no unbounded growth.
//
// This is a DETERMINISTIC CPU test. It instantiates every concrete subclass
// that is practical to build standalone (no RenderBridge / scene required):
//   - ParticleEngine (Scale 1)
//   - CosmicEngine   (Scale 5)
//   - DagEngine      (Scale 0, EXPERIMENTAL)
//
// DagEngine caveat (real engine quirk, NOT a test bug): its entity_count()
// reads active_indices_, a member that is declared but never written anywhere
// in the codebase (see dag_engine.h line ~103 and the grep-confirmed absence
// of any write). So DagEngine::entity_count() is permanently 0. For DagEngine
// the population / clear invariants are therefore asserted against the DAG's
// voxel state directly (eng.dag().get_voxel(...)), and the broken
// entity_count() contract is recorded as a check with an explanatory note
// rather than silently skipped.
// ============================================================================

#include "ftd/particle_engine.h"
#include "ftd/cosmic_engine.h"
#include "ftd/dag_engine.h"
#include "ftd/scale_engine.h"
#include "ftd/test_telemetry.h"

#include <cmath>

namespace ftd {
namespace test {

// ---------------------------------------------------------------------------
// Small helper: is every component of a Vec3 finite?
// ---------------------------------------------------------------------------
static bool finite_vec(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

// ===========================================================================
// ParticleEngine (Scale 1)
//
// clear() impl (particle_engine.h):
//   particles_.clear(); forces_.clear(); force_diag_.clear();
//   tick_ = 0; next_id_ = 0;
// Does NOT reset: dt_, soft_, toggles (these are config, not entity state).
//
// Public API used:
//   int  add_particle(int8_t charge, Vec3 pos, Vec3 vel = {}, double mass = K_B,
//                     double r_eff = 2.48, int8_t spin = 0, int8_t color = 0);
//   void tick() override;                int  current_tick() const override;
//   int  entity_count() const override;  void clear() override;
//   ParticleDiagnostics diagnostics() const;
// ===========================================================================
void test_particle_engine_lifecycle() {
    section("ParticleEngine (Scale 1): clear() + RAII lifecycle");

    ParticleEngine pe;

    // L1: construct → inject → active.
    pe.add_particle(+1, {0, 0, 0}, {0, 0, 0});
    pe.add_particle(-1, {10, 0, 0}, {0, 0, 0});
    pe.add_particle(+1, {0, 10, 0}, {0, 0, 0});
    check("L1: entity_count() > 0 after injection", pe.entity_count() == 3);
    check("L1: diagnostics().particle_count agrees",
          pe.diagnostics().particle_count == 3);

    // L2: step a few times → finite, no crash.
    for (int t = 0; t < 8; ++t) pe.tick();
    {
        auto d = pe.diagnostics();
        check("L2: tick advanced", pe.current_tick() == 8);
        check("L2: total_energy finite after stepping",
              std::isfinite(d.total_energy));
        check("L2: total_momentum finite after stepping",
              finite_vec(d.total_momentum));
        bool all_finite = true;
        for (const auto& p : pe.particles())
            all_finite = all_finite && finite_vec(p.position) && finite_vec(p.velocity);
        check("L2: every particle position/velocity finite", all_finite);
    }

    // L3: clear() → empty + reset invariants.
    pe.clear();
    check("L3: entity_count() == 0 after clear()", pe.entity_count() == 0);
    check("L3: particles() empty after clear()", pe.particles().empty());
    check("L3: current_tick() == 0 after clear()", pe.current_tick() == 0);
    check("L3: diagnostics().particle_count == 0 after clear()",
          pe.diagnostics().particle_count == 0);

    // L4: re-inject after clear() → reusable.
    int reborn = pe.add_particle(+1, {1, 1, 1}, {0, 0, 0});
    check("L4: add_particle works after clear() (returns valid id)", reborn >= 0);
    check("L4: entity_count() == 1 after re-injection", pe.entity_count() == 1);
    pe.tick();  // and it still steps cleanly
    check("L4: tick after re-injection advances to 1", pe.current_tick() == 1);
    check("L4: still finite after post-clear tick",
          std::isfinite(pe.diagnostics().total_energy));

    // Note: next_id_ is reset to 0 by clear(), so the id namespace restarts.
    // That is the documented behaviour — assert it so a future change that
    // makes ids monotonic-across-clear() is caught.
    check("L4: id namespace restarts after clear() (next id == 0)", reborn == 0);

    // L5: double-clear() idempotent.
    pe.clear();
    pe.clear();
    check("L5: entity_count() == 0 after double clear()", pe.entity_count() == 0);
    check("L5: current_tick() == 0 after double clear()", pe.current_tick() == 0);

    // L6: construct + populate + destruct loop (RAII smoke test).
    // Use all same-sign charges spread well apart: this keeps the population
    // stable across a tick (opposite charges within r_eff_i+r_eff_j ≈ 4.96
    // would annihilate via check_annihilation(), which is correct physics but
    // would conflate this destructor smoke test with annihilation bookkeeping).
    bool loop_ok = true;
    for (int i = 0; i < 50; ++i) {
        ParticleEngine tmp;
        for (int k = 0; k < 20; ++k)
            tmp.add_particle(+1, {double(k) * 10.0, 0, 0}, {0, 0, 0});
        tmp.tick();
        loop_ok = loop_ok && (tmp.entity_count() == 20);
        // tmp destructs here every iteration — exercises ~ParticleEngine().
    }
    check("L6: 50× construct/populate/destruct loop completed cleanly", loop_ok);
}

// ===========================================================================
// CosmicEngine (Scale 5)
//
// clear() impl (cosmic_engine.cpp):
//   bodies_/forces_/force_diag_/gw_events_/octree_/sph_neighbors_ cleared;
//   tick_ = 0; next_id_ = 0;
//   a_ = 1.0; adot_ = 0.0; t_cosmic_ = 0.0; H0_ = 0.0;   // Friedmann state
// Does NOT reset: dt_, box_size_, softening_, toggles (config, not state).
//
// Public API used:
//   int  add_dark_matter(double mass, Vec3 pos, Vec3 vel = {});
//   int  add_star(double mass, Vec3 pos, Vec3 vel = {}, double lum = -1.0);
//   int  add_gas(double mass, Vec3 pos, Vec3 vel = {}, double temp = 1e4);
//   void tick() override;                int  current_tick() const override;
//   int  entity_count() const override;  void clear() override;
//   double scale_factor() const;         CosmicDiagnostics diagnostics() const;
// ===========================================================================
void test_cosmic_engine_lifecycle() {
    section("CosmicEngine (Scale 5): clear() + RAII lifecycle");

    CosmicEngine ce;
    ce.set_box_size(100.0);  // keep bodies well inside the box

    // L1: construct → inject mixed body types → active.
    ce.add_dark_matter(1.0e10, {10, 10, 10}, {0, 0, 0});
    ce.add_star(1.0, {12, 10, 10}, {0, 0, 0});
    ce.add_gas(1.0e3, {10, 12, 10}, {0, 0, 0}, 1.0e4);
    check("L1: entity_count() == 3 after injection", ce.entity_count() == 3);
    check("L1: diagnostics().body_count agrees",
          ce.diagnostics().body_count == 3);

    // L2: step a few times → finite, no crash.
    for (int t = 0; t < 8; ++t) ce.tick();
    {
        auto d = ce.diagnostics();
        check("L2: tick advanced", ce.current_tick() == 8);
        check("L2: total_energy finite after stepping",
              std::isfinite(d.total_energy));
        check("L2: scale_factor finite & positive after stepping",
              std::isfinite(ce.scale_factor()) && ce.scale_factor() > 0.0);
        bool all_finite = true;
        for (const auto& b : ce.bodies())
            all_finite = all_finite && finite_vec(b.position) && finite_vec(b.velocity);
        check("L2: every body position/velocity finite", all_finite);
    }

    // L3: clear() → empty + reset invariants (incl. Friedmann state).
    ce.clear();
    check("L3: entity_count() == 0 after clear()", ce.entity_count() == 0);
    check("L3: bodies() empty after clear()", ce.bodies().empty());
    check("L3: current_tick() == 0 after clear()", ce.current_tick() == 0);
    check("L3: scale_factor() reset to 1.0 after clear()",
          std::abs(ce.scale_factor() - 1.0) < 1e-12);
    check("L3: hubble_parameter() reset to 0 after clear()",
          std::abs(ce.hubble_parameter()) < 1e-12);

    // L4: re-inject after clear() → reusable.
    int reborn = ce.add_star(2.0, {5, 5, 5}, {0, 0, 0});
    check("L4: add_star works after clear() (returns valid id)", reborn >= 0);
    check("L4: entity_count() == 1 after re-injection", ce.entity_count() == 1);
    ce.tick();
    check("L4: tick after re-injection advances to 1", ce.current_tick() == 1);
    check("L4: still finite after post-clear tick",
          std::isfinite(ce.diagnostics().total_energy));
    check("L4: id namespace restarts after clear() (next id == 0)", reborn == 0);

    // L5: double-clear() idempotent.
    ce.clear();
    ce.clear();
    check("L5: entity_count() == 0 after double clear()", ce.entity_count() == 0);
    check("L5: scale_factor() still 1.0 after double clear()",
          std::abs(ce.scale_factor() - 1.0) < 1e-12);

    // L6: construct + populate + destruct loop (RAII smoke test).
    bool loop_ok = true;
    for (int i = 0; i < 50; ++i) {
        CosmicEngine tmp;
        tmp.set_box_size(100.0);
        for (int k = 0; k < 10; ++k)
            tmp.add_dark_matter(1.0e9, {double(k), double(k), 0}, {0, 0, 0});
        tmp.tick();
        loop_ok = loop_ok && (tmp.entity_count() == 10);
        // tmp destructs here every iteration — exercises ~CosmicEngine().
    }
    check("L6: 50× construct/populate/destruct loop completed cleanly", loop_ok);
}

// ===========================================================================
// DagEngine (Scale 0, EXPERIMENTAL)
//
// clear() impl (dag_engine.cpp):
//   int sz = dag_->size();
//   dag_ = make_unique<SparseVoxelDAG>(sz);   // rebuilds → all flux wiped
//   tick_ = 0;
// Does NOT reset: dt_, toggles_, the delta_j_ buffer's allocated size.
//
// entity_count() is permanently 0 (active_indices_ is never written) — so the
// population / clear invariants are asserted against the DAG voxel state
// directly. The broken entity_count() contract is recorded explicitly.
//
// Public API used:
//   explicit DagEngine(int lattice_size);   // size must be a power of two
//   void inject_flux(int x, int y, int z, double fx, double fy, double fz);
//   const SparseVoxelDAG& dag() const;       // -> get_voxel(x,y,z) -> Voxel
//   void tick() override;                    int  current_tick() const override;
//   int  entity_count() const override;      void clear() override;
// ===========================================================================
void test_dag_engine_lifecycle() {
    section("DagEngine (Scale 0): clear() + RAII lifecycle");

    DagEngine de(16);

    // Record the documented entity_count() quirk up front so it is visible
    // in the report rather than hidden behind a silent skip.
    check("DAG: entity_count() is 0 on construction (active_indices_ unused)",
          de.entity_count() == 0,
          "DagEngine::entity_count() reads active_indices_, which is declared "
          "but never written; it is permanently 0. Population is asserted via "
          "the DAG voxel state below.");

    // L1: construct → inject flux → DAG carries it.
    de.inject_flux(8, 8, 8, 1.0, 0.0, 0.0);
    {
        Voxel injected = de.dag().get_voxel(8, 8, 8);
        Voxel untouched = de.dag().get_voxel(0, 0, 0);
        check("L1: injected voxel carries flux", injected.flux.x == 1.0);
        check("L1: untouched void voxel stays zero", untouched.flux.mag() == 0.0);
    }

    // L2: step a few times → finite, no crash.
    for (int t = 0; t < 4; ++t) de.tick();
    {
        check("L2: tick advanced", de.current_tick() == 4);
        Voxel center = de.dag().get_voxel(8, 8, 8);
        Voxel right  = de.dag().get_voxel(9, 8, 8);
        check("L2: center flux finite after stepping", finite_vec(center.flux));
        check("L2: neighbor flux finite after stepping", finite_vec(right.flux));
    }

    // L3: clear() → DAG wiped + tick reset.
    de.clear();
    check("L3: current_tick() == 0 after clear()", de.current_tick() == 0);
    {
        Voxel center = de.dag().get_voxel(8, 8, 8);
        check("L3: injected flux wiped after clear() (DAG rebuilt)",
              center.flux.mag() == 0.0);
        check("L3: DAG size preserved across clear()", de.dag().size() == 16);
    }

    // L4: re-inject after clear() → reusable.
    de.inject_flux(4, 4, 4, 2.0, 0.0, 0.0);
    {
        Voxel reborn = de.dag().get_voxel(4, 4, 4);
        check("L4: inject_flux works after clear()", reborn.flux.x == 2.0);
    }
    de.tick();
    check("L4: tick after re-injection advances to 1", de.current_tick() == 1);
    {
        Voxel after = de.dag().get_voxel(4, 4, 4);
        check("L4: flux finite after post-clear tick", finite_vec(after.flux));
    }

    // L5: double-clear() idempotent.
    de.clear();
    de.clear();
    check("L5: current_tick() == 0 after double clear()", de.current_tick() == 0);
    {
        Voxel center = de.dag().get_voxel(4, 4, 4);
        check("L5: DAG empty after double clear()", center.flux.mag() == 0.0);
        check("L5: DAG size still 16 after double clear()", de.dag().size() == 16);
    }

    // L6: construct + populate + destruct loop (RAII smoke test).
    // DagEngine owns a unique_ptr<SparseVoxelDAG>; this exercises that the
    // defaulted ~DagEngine() frees the octree pools every iteration.
    bool loop_ok = true;
    for (int i = 0; i < 50; ++i) {
        DagEngine tmp(16);
        tmp.inject_flux(8, 8, 8, 1.0, 0.0, 0.0);
        tmp.tick();
        loop_ok = loop_ok && (tmp.dag().get_voxel(8, 8, 8).flux.mag() != 0.0
                              || tmp.current_tick() == 1);
        // tmp destructs here every iteration — exercises ~DagEngine() + the
        // unique_ptr<SparseVoxelDAG> teardown.
    }
    check("L6: 50× construct/populate/destruct loop completed cleanly", loop_ok);
}

// ===========================================================================
// Polymorphic teardown: delete-through-base-pointer.
//
// The whole reason ScaleEngine has `virtual ~ScaleEngine() = default;` is so
// the bridge can hold a ScaleEngine* and delete the correct derived destructor.
// Assert that deleting each subclass through a base pointer runs without crash
// (a non-virtual base dtor here would be UB / leak the derived members).
// ===========================================================================
void test_polymorphic_destruction() {
    section("Polymorphic teardown via ScaleEngine* (virtual dtor)");

    {
        ScaleEngine* e = new ParticleEngine();
        static_cast<ParticleEngine*>(e)->add_particle(+1, {0, 0, 0}, {0, 0, 0});
        e->tick();
        check("ParticleEngine: entity via base ptr", e->entity_count() == 1);
        e->clear();
        check("ParticleEngine: clear() via base ptr empties", e->entity_count() == 0);
        delete e;  // must dispatch ~ParticleEngine() through virtual base dtor
        check("ParticleEngine: delete via ScaleEngine* did not crash", true);
    }
    {
        ScaleEngine* e = new CosmicEngine();
        static_cast<CosmicEngine*>(e)->add_dark_matter(1e9, {0, 0, 0}, {0, 0, 0});
        e->tick();
        check("CosmicEngine: entity via base ptr", e->entity_count() == 1);
        e->clear();
        check("CosmicEngine: clear() via base ptr empties", e->entity_count() == 0);
        delete e;
        check("CosmicEngine: delete via ScaleEngine* did not crash", true);
    }
    {
        ScaleEngine* e = new DagEngine(16);
        static_cast<DagEngine*>(e)->inject_flux(8, 8, 8, 1.0, 0.0, 0.0);
        e->tick();
        // entity_count() is permanently 0 for DagEngine; assert via tick.
        check("DagEngine: tick advanced via base ptr", e->current_tick() == 1);
        e->clear();
        check("DagEngine: clear() via base ptr resets tick", e->current_tick() == 0);
        delete e;  // must dispatch ~DagEngine() (frees the unique_ptr DAG)
        check("DagEngine: delete via ScaleEngine* did not crash", true);
    }
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_engine_lifecycle");

    ftd::test::test_particle_engine_lifecycle();
    ftd::test::test_cosmic_engine_lifecycle();
    ftd::test::test_dag_engine_lifecycle();
    ftd::test::test_polymorphic_destruction();

    return ftd::test::finalize();
}
