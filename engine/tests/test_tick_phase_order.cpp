// ============================================================================
// test_tick_phase_order.cpp  (ticket W4-2)
// ----------------------------------------------------------------------------
// Regression test that PINS the RenderBridge tick-cycle phase ORDER.
//
// The documented order (CLAUDE.md, SPEC_ENGINE.md, render_bridge.cpp::tick):
//
//     phase_read  →  phase_write  →  gauss_project  →  phase_forces
//                 →  phase_movement  →  weak_transmutation
//                 →  accumulate_proper_time  →  update_energy_ledger  →  tick++
//
// Until now NO C++ test pinned this ordering. A future reorder (e.g. moving
// the energy ledger before movement, or forces before manifestation) would
// silently corrupt physics while every existing test — including the golden
// hash — kept passing for the wrong reason. (The golden hash in
// test_render_bridge_golden pins the *final state* of one fixed scenario; it
// does NOT isolate which phase produced which effect, and a clever reorder
// that happens to land on the same 100-tick hash would slip through. This
// file targets the ORDER directly.)
//
// ── DESIGN TIER (see ticket W4-2 step 2) ────────────────────────────────────
// Achieved tier: (b) — assert order via CAUSAL CONSEQUENCES.
//
// WHY NOT (a): there is no production phase-trace / telemetry hook. A grep of
// engine/ for phase_log|phase_trace|phase_order|phase_sequence|… returns
// nothing; RenderBridge::tick() (render_bridge.cpp:351-467) calls the phase
// methods directly with no recorded order vector, and ftd::test_telemetry is a
// test-harness NDJSON emitter wired into test main()s, NOT into tick(). So the
// strongest *available* assertion is the consequence-based one below, not a
// recorded-sequence equality. (Recommended follow-up: a compile-out
// production "phase trace" ring buffer behind a debug flag would let a future
// version of this test upgrade to tier (a) and assert the exact sequence.)
//
// Each invariant constructs a state where the WRONG order would produce a
// measurably different, sign-determinate result, then asserts the
// documented-order result. All three are CPU-only (force_cpu), deterministic
// (seed_rng / no RNG), and fast (<<2s at L≤16).
//
// Invariants:
//   PO-1  phase_read  runs BEFORE phase_write   (leapfrog kick-then-drift):
//         after one tick flux == flux_init + delta_j(flux_init); i.e. the
//         WRITE consumed exactly what the READ produced, this tick.
//   PO-2  phase_write (genesis) runs BEFORE phase_forces: a particle that
//         manifests this tick is force-integrated this tick (its velocity is
//         charge-dependent on a neighbour, which is impossible if forces ran
//         before it existed → it would still be state 0 and be skipped).
//   PO-3  update_energy_ledger is the LAST (state-reading) phase, AFTER
//         phase_movement: the ledger's E_curr equals the post-movement
//         energy_audit().total_energy even when movement (annihilation) just
//         changed the energy — so the ledger snapshot reflects post-movement
//         state, not pre-movement.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/test_telemetry.h"

#include <cmath>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Shared: a minimal, validator-clean toggle baseline.
//
// disable_all() clears every bulk-managed toggle to false (term_toggles.h:281),
// which simultaneously satisfies every TermToggles::validate() dependency
// (selective_damping→damping, lorentz_force→forces, weak_transmutation→
// dual_substrate, triad_binding→…). Each sub-test then turns ON only the
// phases it needs, so tick() runs a known-minimal phase ladder and the causal
// chain under test is the ONLY thing happening.
// ---------------------------------------------------------------------------
static void minimal_toggles(RenderBridge& rb) {
    rb.toggles.disable_all();          // all bulk toggles → false (valid config)
    rb.toggles.strict_validation = true; // fail loudly if a config slips invalid
}

// ===========================================================================
// PO-1: phase_read runs BEFORE phase_write  (leapfrog ordering)
//
// tick() ordering under test (render_bridge.cpp:404-409):
//     if (wave_propagation || coupling) phase_read();   // writes delta_j_[i]
//     phase_write();                                     // wave_vel += delta_j_;
//                                                        // flux     += wave_vel;
//
// phase_read_main_loop (phase_read.cpp) is a PURE function of (voxels,toggles)
// with no tick_ dependence: it fills delta_j_ from the CURRENT flux via the
// 18-pt Laplacian (+ coupling). phase_write_main_loop (phase_write.cpp:231-237,
// non-dual, non-symplectic) then does exactly:
//     v.wave_vel += delta_j_[i];
//     v.flux     += v.wave_vel;
//
// So with a fresh field (wave_vel == 0), ONE tick must leave:
//     wave_vel == delta_j(flux_init)
//     flux     == flux_init + delta_j(flux_init)
//
// If WRITE ran before READ, `flux += wave_vel` would add the stale wave_vel
// (==0) → flux unchanged this tick, and the equality below would fail. This
// pins read-before-write.
//
// The predicted delta_j is obtained independently on a TWIN bridge via the
// public prepare_delta_j() + delta_j() accessors (render_bridge.h:201-206,
// "Calls phase_read() without advancing the tick — state is NOT modified"),
// then a SECOND identical bridge is advanced one real tick and compared.
// Fully deterministic — no RNG, no genesis.
// ===========================================================================
static void configure_po1(RenderBridge& rb) {
    minimal_toggles(rb);
    rb.toggles.wave_propagation = true;  // phase_read: Laplacian → delta_j
    rb.toggles.coupling         = true;  // phase_read: g_c·∇s source (state==0 here ⇒ 0, but exercises the path)
    // genesis OFF ⇒ the flux blob never manifests (stays a pure field);
    // damping/gauss/forces/movement OFF ⇒ phase_write's leapfrog is the only
    // thing that touches flux/wave_vel. dual_substrate OFF ⇒ single-substrate
    // delta_j_ buffer (the one delta_j() exposes). symplectic OFF + dt=1 ⇒ the
    // bare `+= delta_j` / `+= wave_vel` form with no dt scaling.
}

// Inject an asymmetric flux pattern so the Laplacian (delta_j) is non-trivial
// and varies site-to-site (a single isotropic blob would give a clean but
// less discriminating pattern; an L-shaped 3-voxel seed gives a richer
// delta_j we can sample at several sites).
static void inject_po1_field(RenderBridge& rb) {
    const int c = rb.lattice().size() / 2;
    rb.voxel_at(c,     c, c).flux = Vec3{1.0,  0.0,  0.0};
    rb.voxel_at(c + 1, c, c).flux = Vec3{0.0,  0.7,  0.0};
    rb.voxel_at(c, c + 1, c).flux = Vec3{0.0,  0.0, -0.4};
}

void test_read_before_write() {
    section("PO-1: phase_read BEFORE phase_write (leapfrog kick-then-drift)");

    const int L = 12;
    const int c = L / 2;

    // --- Twin A: predict delta_j(flux_init) WITHOUT advancing a tick. ---
    RenderBridge twin(L);
    twin.force_cpu();
    configure_po1(twin);
    inject_po1_field(twin);
    twin.prepare_delta_j();              // == phase_read(); state untouched

    // Snapshot the predicted delta_j and the initial flux at the sample sites
    // immediately (before any further voxel access mutates buffers).
    const int idx0 = twin.lattice().index(c,     c, c);
    const int idx1 = twin.lattice().index(c + 1, c, c);
    const int idx2 = twin.lattice().index(c, c + 1, c);

    const Vec3 dj0 = twin.delta_j()[idx0];
    const Vec3 dj1 = twin.delta_j()[idx1];
    const Vec3 dj2 = twin.delta_j()[idx2];

    const Vec3 f0_init = twin.voxels()[idx0].flux;
    const Vec3 f1_init = twin.voxels()[idx1].flux;
    const Vec3 f2_init = twin.voxels()[idx2].flux;

    // Sanity: the predicted delta_j is genuinely non-zero (otherwise the test
    // is vacuous — every equality would trivially hold at 0).
    const double dj_total = dj0.mag() + dj1.mag() + dj2.mag();
    check("delta_j is non-trivial (test is non-vacuous)",
          dj_total > 1e-9,
          "Predicted delta_j ~ 0 at every sample site; field setup is degenerate.");

    // --- Twin B: identical state, advance exactly ONE real tick. ---
    RenderBridge rb(L);
    rb.force_cpu();
    configure_po1(rb);
    inject_po1_field(rb);
    rb.tick();   // read → write (→ no-op gauss/forces/movement, all OFF)

    const Vec3 wv0 = rb.voxels()[idx0].wave_vel;
    const Vec3 f0  = rb.voxels()[idx0].flux;
    const Vec3 f1  = rb.voxels()[idx1].flux;
    const Vec3 f2  = rb.voxels()[idx2].flux;

    // wave_vel == delta_j  (the kick: wave_vel started at 0, += delta_j once).
    check_close("wave_vel.x == delta_j.x  (kick applied)", wv0.x, dj0.x, 1e-12);
    check_close("wave_vel.y == delta_j.y  (kick applied)", wv0.y, dj0.y, 1e-12);
    check_close("wave_vel.z == delta_j.z  (kick applied)", wv0.z, dj0.z, 1e-12);

    // flux == flux_init + delta_j  (the drift consumed THIS tick's kick).
    // This is the load-bearing assertion: it can only hold if phase_read
    // (which produced delta_j) ran BEFORE phase_write (which added wave_vel
    // to flux) within the SAME tick.
    check_close("flux.x == flux_init.x + delta_j.x (read→write)", f0.x, f0_init.x + dj0.x, 1e-12);
    check_close("flux.y == flux_init.y + delta_j.y (read→write)", f0.y, f0_init.y + dj0.y, 1e-12);
    check_close("flux.z == flux_init.z + delta_j.z (read→write)", f0.z, f0_init.z + dj0.z, 1e-12);

    // Two more sample sites, to guard against an accidental single-site match.
    check_close("flux[idx1].y == init+dj (read→write)", f1.y, f1_init.y + dj1.y, 1e-12);
    check_close("flux[idx2].z == init+dj (read→write)", f2.z, f2_init.z + dj2.z, 1e-12);
}

// ===========================================================================
// PO-2: phase_write (genesis) runs BEFORE phase_forces
//
// tick() ordering under test (render_bridge.cpp:409, 433-434):
//     phase_write();    // genesis: void site with |J|>K_GENESIS → state ±1
//     ...
//     if (forces) phase_forces();   // skips state==0; integrates velocity of
//                                   // every manifested particle THIS tick
//
// Construct: a strong static flux blob at C that manifests via genesis on
// tick 1 (deterministic polarity: a single-voxel blob has zero divergence at
// its own center — the divergence stencil reads its 6 neighbours, all zero —
// so manifest_at() assigns state = -1, phase_write.cpp:75 with div==0). An
// EXTERNAL static charge sits 3 voxels away along +y, breaking the field
// symmetry so the genesis particle feels a non-zero, charge-dependent Coulomb
// force on its birth tick.
//
// If forces ran BEFORE write, the genesis voxel would still be state 0 when
// phase_forces iterated (continue on state==0, phase_forces.cpp:72) → it would
// acquire NO velocity on its birth tick. movement is OFF so the particle stays
// put and we read its velocity directly at C.
//
// To avoid hand-deriving the absolute Poisson sign, the assertion is a
// CHARGE-FLIP comparison (idiom mirrored from test_audit_regression.cpp G-1):
//   - external = -1  (same sign as genesis -1) → repulsion → genesis pushed -y
//   - external = +1  (opposite)                → attraction → genesis pulled +y
// The genesis particle's velocity.y must (a) be non-zero in both and (b) FLIP
// sign between the two — proving a real, charge-dependent force was applied to
// a particle that exists only because phase_write already ran this tick.
// ===========================================================================
static void configure_po2(RenderBridge& rb) {
    minimal_toggles(rb);
    rb.toggles.genesis          = true;  // phase_write manifestation
    rb.toggles.forces           = true;  // phase_forces master
    rb.toggles.poisson_coulomb  = true;  // F = -α·s·∇φ_C (charge-dependent)
    rb.toggles.gauss_projection = true;  // physical Coulomb path companion
    // OFF: wave_propagation + coupling ⇒ phase_read skipped ⇒ the injected
    //      blob's |J| is unchanged at genesis time ⇒ deterministic manifest.
    // OFF: movement ⇒ genesis particle stays at C, velocity read directly.
    // OFF: damping/gravity/lorentz/dual ⇒ pure electrostatics.
}

// Returns velocity.y of the genesis particle at C after one tick, or NAN if it
// failed to manifest (so the caller can record a skip rather than mis-assert).
static double genesis_particle_vy(int external_charge) {
    const int L = 16;
    const int c = L / 2;

    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(7);                      // deterministic Born-rule draw
    configure_po2(rb);

    // Strong flux blob at C: |J| = 3.0 ≫ K_GENESIS (~0.507) ⇒ manifest
    // probability p = 1 - exp(-(3.0-K_GENESIS)/K_B) ≈ 0.992. Dominant flux axis
    // is x (sets color; irrelevant here). Divergence at C is 0 ⇒ state -1.
    rb.voxel_at(c, c, c).flux = Vec3{3.0, 0.0, 0.0};

    // External STATIC charge 3 voxels along +y (a manifested particle, no flux,
    // velocity 0). With movement OFF it never moves; it only sources φ_C.
    rb.inject_particle(c, c + 3, c, static_cast<int8_t>(external_charge),
                       Vec3{0.0, 0.0, 0.0});

    rb.tick();   // write(genesis) → gauss → forces → (movement OFF)

    const int idxC = rb.lattice().index(c, c, c);
    const auto& vC = rb.voxels()[idxC];
    if (vC.state == 0) return std::nan("");   // did not manifest → signal skip
    return vC.velocity.y;
}

void test_genesis_before_forces() {
    section("PO-2: phase_write (genesis) BEFORE phase_forces");

    const double vy_repel  = genesis_particle_vy(-1);  // same-sign → repel (−y)
    const double vy_attract = genesis_particle_vy(+1); // opposite  → attract (+y)

    // Genesis must have happened in both runs.
    if (std::isnan(vy_repel) || std::isnan(vy_attract)) {
        check("genesis particle manifested in both runs", false,
              "Blob did not manifest at the chosen seed — adjust seed/amplitude. "
              "Not a phase-order failure.");
        return;
    }

    // (a) The genesis particle was force-integrated on its birth tick: its
    // velocity.y is non-zero. This is impossible if phase_forces ran before
    // phase_write (it would have skipped the still-void site).
    check("genesis particle gained velocity on birth tick (same-sign cfg)",
          std::abs(vy_repel) > 1e-9,
          "Newly-manifested particle has zero velocity ⇒ forces did NOT act on "
          "it this tick ⇒ forces ran before genesis (WRONG ORDER) or no force.");
    check("genesis particle gained velocity on birth tick (opp-sign cfg)",
          std::abs(vy_attract) > 1e-9,
          "Newly-manifested particle has zero velocity ⇒ forces ran before "
          "genesis (WRONG ORDER) or no force.");

    // (b) The force is genuinely charge-dependent (not numerical drift): the
    // genesis (−1) particle is pushed the OPPOSITE way when the external charge
    // flips sign. Same-sign ⇒ repelled (−y); opposite ⇒ attracted (+y).
    check("velocity.y flips sign when external charge flips",
          vy_repel * vy_attract < 0.0,
          "Genesis-particle velocity.y did not invert under external charge "
          "sign flip ⇒ the velocity is not the Coulomb force on the new "
          "particle (force-before-genesis would give 0 in both).");

    // Direction sanity for the like-sign case: two negatives repel, so the
    // genesis particle at C is pushed in −y (away from the +y external charge).
    check("same-sign genesis particle repelled in −y",
          vy_repel < 0.0,
          "Like-charge pair did not repel along the connecting axis.");
}

// ===========================================================================
// PO-3: update_energy_ledger is the LAST state-reading phase (AFTER movement)
//
// tick() ordering under test (render_bridge.cpp:437-466):
//     if (movement) phase_movement();   // annihilation zeroes KE + bursts flux
//     ...
//     ++tick_;
//     update_energy_ledger();           // E_curr = ½Σ|J|² + ½Σ|wv|² + Σ½|v|²
//
// update_energy_ledger_cpu (energy_ledger_compute.cpp:14-24) computes E_curr
// with the SAME formula as compute_energy_audit's total_energy
// (diagnostics_compute.cpp:98-99,123,134):
//     E = ½Σ|flux|² + ½Σ|wave_vel|² + Σ_state≠0 ½|v|²
// So after a tick, energy_ledger().E_curr must equal energy_audit()
// .total_energy to floating-point exactness — BOTH read the same final voxel
// state. The diagnostic bite: we force phase_movement to CHANGE the energy
// (an annihilation removes both particles' KE and redistributes their flux
// over 6 neighbours each, which lowers ½Σ|J|²). If the ledger had been
// computed BEFORE movement, its E_curr would reflect the pre-annihilation
// energy and would NOT match the post-annihilation audit.
//
// Movement-only config (everything else OFF): a +1 at C with velocity {1,0,0}
// performs a single integer jump into a static −1 at C+x on the annihilation
// tick. genesis OFF ⇒ no evaporation/manifestation perturbs the count.
// ===========================================================================
static void configure_po3(RenderBridge& rb) {
    minimal_toggles(rb);
    rb.toggles.movement = true;          // the ONLY active phase
    // Everything else OFF: no wave/coupling (flux static), no gauss (flux
    // untouched), no forces (velocity static), no damping, no genesis (so the
    // two charges persist until they collide; no spontaneous evaporation).
}

void test_energy_ledger_is_last() {
    section("PO-3: update_energy_ledger LAST (reflects post-movement state)");

    const int L = 16;
    const int c = L / 2;

    RenderBridge rb(L);
    rb.force_cpu();
    configure_po3(rb);

    // +1 at C, −1 one voxel along +x. Charges only (no flux) keeps the energy
    // budget purely kinetic until annihilation, so the change is unambiguous.
    rb.inject_particle(c,     c, c, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(c + 1, c, c, -1, Vec3{0.0, 0.0, 0.0});

    // Prime the ledger with one quiescent tick (velocities still 0 ⇒ nothing
    // moves). On the FIRST ledger update E_prev is seeded to E_curr
    // (energy_ledger_compute.cpp:27-36); this tick gets that bookkeeping out of
    // the way so the NEXT tick has a meaningful E_prev.
    rb.tick();

    // Arm the +1 particle: exactly one site/tick along +x ⇒ on the next tick
    // remainder.x reaches 1.0 and it jumps into the −1 ⇒ annihilation.
    rb.voxel_at(c, c, c).velocity = Vec3{1.0, 0.0, 0.0};

    // Energy the system carries going INTO the annihilation tick (now includes
    // the +1 particle's KE = ½·1² = 0.5). After annihilation this KE is gone
    // and the (zero) particle flux is redistributed ⇒ total energy must drop.
    const double e_before = compute_energy_audit(rb).total_energy;

    // The annihilation tick: movement runs, THEN the ledger is taken.
    rb.tick();

    const auto& ledger = rb.energy_ledger();
    const double e_audit_after = compute_energy_audit(rb).total_energy;

    // Confirm the annihilation actually fired (both particles gone) — otherwise
    // the energy would not have changed and the test would be vacuous.
    int manifested = 0;
    const int N = static_cast<int>(rb.lattice().total_sites());
    const auto& voxels = rb.voxels();
    for (int i = 0; i < N; ++i) if (voxels[i].state != 0) ++manifested;
    check("annihilation fired (0 particles remain)", manifested == 0,
          "The +1 did not reach/annihilate the −1; the energy-change premise "
          "(needed to make this test diagnostic) does not hold.");

    // Non-vacuity: movement genuinely changed the total energy this tick.
    check("movement changed total energy (KE removed by annihilation)",
          e_audit_after < e_before - 1e-9,
          "Total energy did not drop across the annihilation tick; PO-3 cannot "
          "distinguish pre- vs post-movement ledger timing without a change.");

    // THE ordering assertion: the ledger's recorded current energy equals the
    // POST-movement audit. Equal ⇒ the ledger snapshot was taken AFTER movement
    // mutated the state (i.e. the ledger is the last state-reading phase). If
    // the ledger had run before movement, E_curr would equal the (larger)
    // pre-annihilation energy and this equality would fail.
    check_close("ledger.E_curr == post-movement audit.total_energy",
                ledger.E_curr, e_audit_after, 1e-9);

    // And it must NOT equal the pre-movement energy (belt-and-suspenders: makes
    // explicit that a pre-movement ledger would be detected here).
    check("ledger.E_curr != pre-movement energy",
          std::abs(ledger.E_curr - e_before) > 1e-9,
          "ledger.E_curr matched the PRE-movement energy ⇒ the ledger ran "
          "before phase_movement (WRONG ORDER).");

    // The ledger also ran after ++tick_: after two ticks its tick_prev == 1
    // (it stamps tick_prev = tick_ - 1 using the post-increment tick counter,
    // energy_ledger_compute.cpp:39). A cheap corroboration that the ledger is
    // downstream of the tick++ as documented.
    check("ledger.tick_prev == 1 after two ticks (ran after tick++)",
          ledger.tick_prev == 1,
          "Ledger tick stamp inconsistent with running after ++tick_.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_tick_phase_order");

    ftd::test::test_read_before_write();      // PO-1
    ftd::test::test_genesis_before_forces();  // PO-2
    ftd::test::test_energy_ledger_is_last();  // PO-3

    return ftd::test::finalize();
}
