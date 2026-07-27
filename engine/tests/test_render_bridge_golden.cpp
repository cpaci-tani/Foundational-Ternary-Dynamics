// ============================================================================
// test_render_bridge_golden.cpp
// ----------------------------------------------------------------------------
// Phase 4 PRE-FLIGHT REGRESSION GATE
//
// Before any further phase extraction is allowed out of render_bridge.cpp
// (Phase 4a/4b/4c — see plan `.claude/plans/i-want-to-try-crispy-charm.md`),
// every commit MUST reproduce the byte-hash captured here. If a commit
// changes the hash, it has changed engine physics — extraction commits
// MUST be bit-exact preserving.
//
// Pattern is the R1-R5 phase extraction precedent (poisson_solvers.cpp,
// transmutation_phases.cpp, injection.cpp; see ADR-0008).
//
// Setup (deterministic):
//   - L = 17 lattice (ODD — 2026-06-03: all lattices are odd so the flux
//     pulse at (8,8,8) lands on the true center voxel (N-1)/2 = 8)
//   - rb.force_cpu()                       (pin to CPU backend; bit-exactness)
//   - rb.seed_rng(42)                      (genesis Born-rule reproducible)
//   - 3 manifested particles at well-separated coordinates with known charges
//   - 1 flux pulse at lattice center
//   - Toggle profile (see set_toggle_profile() below): clean physics path
//
// Drive:
//   - rb.tick() x 100
//
// Hash (xor-fold of bit representations of every double we care about):
//   - voxels[*].state              (int8_t, cast to int64)
//   - voxels[*].flux               (Vec3)
//   - voxels[*].wave_vel           (Vec3)
//   - voxels[*].velocity           (Vec3)
//   - audit fields (22 doubles + 2 ints + Vec3 poynting)
//   - manifested particle state: (x, y, z, charge, vel) for each manifested site
//
// The xor-fold uses the FNV-1a 64-bit constant (0x100000001b3) as the mixer
// so each contribution is permuted before xoring — pure XOR is order-
// independent which would mask voxel-permutation bugs.
//
// Frozen golden hash:
//   GOLDEN_HASH = (computed on first run, then hardcoded — see below)
//
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"   // shared fold harness (revision 0.5 extraction)

#include <cstdint>
#include <cstdio>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Toggle profile — exercises the cleanest physics path.
// ---------------------------------------------------------------------------
static void set_toggle_profile(RenderBridge& rb) {
    auto& t = rb.toggles;

    // ON
    t.wave_propagation  = true;
    t.coupling          = true;
    t.gauss_projection  = true;
    t.forces            = true;
    t.movement          = true;
    t.poisson_coulomb   = true;

    // OFF (damping path)
    t.damping           = false;
    t.selective_damping = false;  // must be off when damping is off (validate())
    t.larmor_radiation  = false;

    // OFF (extra force channels)
    t.gravity           = false;
    t.lorentz_force     = false;
    t.color_forces      = false;
    t.strong_force      = false;
    t.exchange_force    = false;
    t.confinement       = false;

    // OFF (substrate / extension toggles)
    t.dual_substrate    = false;
    t.weak_transmutation= false;  // requires dual_substrate when on (validate())
    t.triad_binding     = false;
    t.pair_production   = false;
    t.latency_field     = false;
    t.langevin          = false;
    t.exact_dual_gauss  = false;
    t.emergent_forces   = false;  // mutually exclusive with poisson_coulomb

    // genesis: leave default ON. Born-rule manifestation is RNG-driven; we
    // seed_rng(42) explicitly so it's reproducible.
    t.genesis           = true;
}

// ---------------------------------------------------------------------------
// Inject a deterministic initial state.
// ---------------------------------------------------------------------------
static void inject_initial_state(RenderBridge& rb) {
    // 3 manifested particles, well-separated, charges {+1, -1, +1}.
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});

    // Flux pulse at lattice centre.
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// ---------------------------------------------------------------------------
// State hash: compute_state_hash() now lives in support/golden_hash.h
// (revision 0.5 extraction — verified to reproduce GOLDEN_HASH bit-exact).
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// FROZEN GOLDEN HASH.
//   - 2026-04-27: original capture on main @ HEAD at L=16 (0xcd957b601d47868a).
//   - 2026-06-03: RECAPTURED at L=17 — intentional config change (all lattice
//     sizes are now odd so phenomena/flux center on a true center voxel). This
//     is NOT a phase-extraction regression; the lattice changed 16→17, so the
//     byte-hash necessarily changed (different voxel count + center).
//   - 2026-06-10: re-pinned to 0xebaa6f314f66db3f (origin/main baseline).
//   - 2026-06-11: RE-PINNED to 0x56fa28acb5b9fe88 — DETERMINISM RESTORATION,
//     not a physics change. The 0xebaa6f value was captured while the engine was
//     non-deterministic run-to-run: three multi-thread races floated the hash —
//     (1) OpenMP float `reduction(+)` on the Poisson phi-sum / field-energy
//     loops (gauge-irrelevant to grad φ, but leaked into absolute-φ audit
//     scalars such as coulomb_pe); (2) a genesis/evaporation read-write race
//     (evaporation read neighbour flux live while genesis wrote a firing voxel's
//     flux); (3) the 8-colour SOR sweep racing at the PERIODIC seam on the ODD
//     L=17 lattice (wrap maps Nm1→0, both even ⇒ same colour ⇒ two boundary
//     cells become racing stencil-neighbours). All three are now fixed in
//     poisson_solvers.cpp + phase_write.cpp so the parallel CPU path is bit-
//     exact to a fully-sequential lexicographic sweep. 0x56fa28acb5b9fe88 is
//     that canonical sequential value (single-thread produced it throughout);
//     it is now reproduced identically by OMP_NUM_THREADS=1 and the full thread
//     pool. The per-voxel state/flux field is unchanged where it was already
//     deterministic; the race-affected boundary φ and audit scalars now take
//     their well-defined sequential values.
//   - 2026-06-17: RE-PINNED to 0xb604d81a3d79366e — intentional DIAGNOSTIC
//     correctness fix (audit finding m1), NOT an engine-dynamics change. The
//     audited `EnergyAudit.gauss_violation` / `max_gauss_error` scalars (which
//     this hash mixes) previously summed the residual (div(J) − s)² over EVERY
//     voxel, but the Gauss/SOR projection (poisson_solvers.cpp gauss_project_cpu)
//     only constrains VACUUM sites (state==0) and targets the mean-subtracted,
//     coupling-scaled source div(J)=charge_coupling·(s − mean_charge). The metric
//     now mirrors that exact constraint (sum over state==0, target −coupling·
//     mean_charge), so it reports the residual the solver actually drives to zero
//     rather than conflating the intentionally-unconstrained manifested-site term
//     and the periodic-BC neutralizing offset. The per-voxel state/flux/wave_vel/
//     velocity field is BYTE-IDENTICAL (verified); only the two gauss audit
//     scalars changed. New value is deterministic: reproduced identically by 2×
//     default-pool runs and OMP_NUM_THREADS=1 (the accumulation loop is the
//     existing sequential lexicographic pass in compute_energy_audit, no parallel
//     reduction). See diagnostics_compute.cpp gauss_violation block.
//   - 2026-07-18: RE-PINNED to 0x1343f31fc0163a84 — INTENTIONAL ENGINE-DYNAMICS
//     change: electric state-flux coupling sign amendment (lagrangian.h Term 2,
//     −g_c·s·(∇·J) → +g_c·s·(∇·J), phase_read source +g_c·∇s → −g_c·∇s). The
//     previous sign was in internal conflict with the L_GAUSS constraint at
//     charge sites (the Hamiltonian's coupling energy preferred s·divJ < 0
//     while Gauss demands div J = ρ ∝ s); the live engine settled the
//     compromise at f = −0.095 of the Gauss target, WRONG-SIGNED (inward flux
//     at a +1 charge). Post-fix the live attractor is constraint-aligned,
//     f = +0.114 (see test_gauss_law_fidelity.cpp; projector fixed point
//     bit-identical, EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS.md §9.1 prediction
//     upheld). The golden profile contains injected particles (s ≠ 0), so the
//     trajectory legitimately changes. Old: 0xb604d81a3d79366e. Deterministic:
//     identical across battery + standalone reruns.
//   - 2026-07-21: RE-PINNED to 0x450fca908f536e36 for FTD-0402 causal
//     normalization and mass-role separation. The pre-lock/current manifest
//     comparison (`dump_causal_golden_manifest`) found unchanged hashes for
//     state, latency, tau, phase, identity/lock/spin/color/flavor, and the
//     strong/weak sectors. Raw velocity and remainder changed directly because
//     force integration now uses M_INERTIAL and the C_SPEED-normalized causal
//     map; flux/wave fields and their audit scalars changed secondarily through
//     that deterministic trajectory. particle_ke now uses (gamma0-1)E_REST,
//     and total_energy includes accounted particle energy. No toggle, source,
//     boundary, or discrete-state change was observed. Reproduced twice.
//
//   - 2026-07-27: RE-PINNED to 0xc54ffbeda5a3ea63 for the EM-diagnostic c^2
//     restoration. B_field_energy and total_poynting were missing the factor
//     c^2 that the engine's own Lagrangian (lagrangian.h:145) and
//     test_em_energy_conservation.cpp both carry; with J as vector potential
//     and d^2J/dt^2 = c^2 grad^2 J the consistent energy is
//     1/2|E|^2 + (c^2/2)|B|^2 with flux c^2 (E x B). Uncorrected, a pure
//     transverse wave reported a FIXED B/E ratio of 1/c^2 = 3 and the browser
//     drew it as two bars in the same unit.
//     MEASURED, not inferred: reverting ONLY these four factors returns the
//     combined hash to the previous pin 0x450fca908f536e36 exactly, and the
//     state-only fold is byte-identical either way (0xe9633be07656e741).
//     The trajectory therefore did NOT move -- this is a readout correction.
//     That also proves the other 2026-07-27 engine edits (dipole sign in
//     atom_forces.cpp, energy-ledger expected_rate, remove_wave_mean in three
//     scenarios, latency clamp(-phi), gravity requested/active split) are all
//     inert for this golden. Old: 0x450fca908f536e36.
//
// If this changes WITHOUT a stated config/physics rationale, ENGINE PHYSICS
// CHANGED unexpectedly. To change it intentionally: (1) state the rationale in
// the commit, (2) update the constant below to the new captured value.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_HASH = 0xc54ffbeda5a3ea63ULL;  // L=17, EM-diagnostic c^2 restoration, 2026-07-27

// SPLIT GATE (2026-07-27). The combined GOLDEN_HASH above cannot distinguish a
// trajectory change from a corrected diagnostic. These two halves can:
//   GOLDEN_STATE_HASH -- per-voxel fields + manifested list. A mismatch here is
//                        a REAL physics change and should block.
//   GOLDEN_AUDIT_HASH -- the reported energy-audit scalars. A mismatch here on
//                        its own means a readout changed while the simulation
//                        did not, which is frequently intended.
static constexpr std::uint64_t GOLDEN_STATE_HASH = 0xe9633be07656e741ULL;
static constexpr std::uint64_t GOLDEN_AUDIT_HASH = 0x48bd8b3fc2efdba3ULL;

// ---------------------------------------------------------------------------
// Test driver
// ---------------------------------------------------------------------------
void test_golden_tick_hash() {
    section("100-tick byte-hash regression");

    RenderBridge rb(17);     // odd lattice — true center voxel at (17-1)/2 = 8
    rb.force_cpu();          // CPU-only — bit-exact reference
    rb.seed_rng(42);         // deterministic genesis Born-rule sampling
    set_toggle_profile(rb);
    inject_initial_state(rb);

    // Re-seed AFTER injection: inject_particle does not consume RNG, but
    // we want any future implementation that DOES consume RNG during
    // injection to leave the tick-loop RNG in a well-defined state.
    rb.seed_rng(42);

    // Drive 100 ticks.
    for (int t = 0; t < 100; ++t) {
        rb.tick();
    }

    const std::uint64_t hash       = compute_state_hash(rb);
    const std::uint64_t state_hash = compute_state_only_hash(rb);
    const std::uint64_t audit_hash = compute_audit_only_hash(rb);

    std::printf("[golden] combined     = 0x%016llx  (expected 0x%016llx)\n",
                static_cast<unsigned long long>(hash),
                static_cast<unsigned long long>(GOLDEN_HASH));
    std::printf("[golden]   state fold = 0x%016llx  (expected 0x%016llx)\n",
                static_cast<unsigned long long>(state_hash),
                static_cast<unsigned long long>(GOLDEN_STATE_HASH));
    std::printf("[golden]   audit fold = 0x%016llx  (expected 0x%016llx)\n",
                static_cast<unsigned long long>(audit_hash),
                static_cast<unsigned long long>(GOLDEN_AUDIT_HASH));

    // The trajectory gate -- this is the one that means "the physics moved".
    check("TRAJECTORY unchanged (state-only fold: voxels + manifested list)",
          state_hash == GOLDEN_STATE_HASH,
          "Per-voxel state/flux/wave_vel/velocity or the manifested-particle "
          "list changed. This is a REAL engine-physics change -- do not re-pin "
          "without a stated rationale.");

    // The readout gate -- a mismatch here ALONE means a reported number moved
    // while the simulation did not.
    check("reported diagnostics unchanged (audit-only fold)",
          audit_hash == GOLDEN_AUDIT_HASH,
          "The energy-audit scalars changed. If the state fold above still "
          "passes, the trajectory is intact and only a REPORTED quantity moved "
          "(e.g. a corrected unit or a restored factor). Re-pin with a rationale.");

    check("combined hash matches frozen GOLDEN_HASH",
          hash == GOLDEN_HASH,
          "Combined fold moved. Read the two sub-folds above to see which half "
          "-- trajectory or reported diagnostics -- is responsible.");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_render_bridge_golden");
    ftd::test::test_golden_tick_hash();
    return ftd::test::finalize();
}
