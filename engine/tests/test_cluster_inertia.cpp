// ============================================================================
// test_cluster_inertia.cpp  (unified-mass Phase 2, 2026-06-06)
// ----------------------------------------------------------------------------
// Verifies the [IMPOSED] rigid-body cluster-inertia mechanism committed on
// branch engine/unified-mass-2026-06-06 (commit 0e21b8f5). The free function
//
//     phase_forces_integrate_clusters(RenderBridge&)
//         engine/src/render_bridge_phases/phase_forces.cpp:277
//
// flood-fills a connected (26-Moore) cluster of LOCKED, same-sign manifested
// voxels, reconstructs the cluster force
//
//     F_cluster = Σ_members (f_coulomb + f_gravity + f_strong + f_magnetic)
//
// from force_diag_, and integrates the centre-of-mass at INERTIAL MASS
// m = N·M_INERTIAL (N = member count, M_INERTIAL = K_B = 0.511, the
// separately named imposed inertial calibration) via the same γ_FTD
// momentum scheme as the per-voxel loop, with the per-mass acceleration
// a_COM = F_cluster/(N·M_INERTIAL). The resulting V_COM is written to every member
// (rigid body).
//
// ── THE FALSIFIER: a ∝ 1/N ─────────────────────────────────────────────────
// The mechanism's whole content is that a heavier cluster (more voxels)
// resists a GIVEN force proportionally: a = F/(N·M_INERTIAL). So if F is held
// FIXED while N varies, then a·N is constant.
//
// CRITICAL — why a∝1/N is NOT field-testable (do not "fix" this by driving the
// cluster with a real field): every FTD field force already scales ∝N
// (gravity ∝ mass ∝ N; EM ∝ charge ∝ N). Driving the cluster with a field
// therefore gives a = F(N)/(N·M) = const — that is the EQUIVALENCE PRINCIPLE
// (a separate Phase-3 test: same a regardless of N), the exact OPPOSITE of the
// 1/N inertia signature. To isolate inertia we must hold the TOTAL force fixed
// as N grows, which is unphysical for any field and so is done by injecting a
// fixed force directly into force_diag_ and calling the cluster pass DIRECTLY
// (NOT via tick()).
//
// ── Newtonian-clean configuration ──────────────────────────────────────────
//   latency_field OFF, gravity OFF  ⇒ every member latency L = 0
//                                    ⇒ γ_FTD = 1 ⇒ clean Newtonian a = F/m.
//   F = {1e-3, 0, 0} (small)         ⇒ q = F/m ≪ C_SPEED, so the relativistic
//                                      γ_FTD momentum correction is < 6 ppm
//                                      (negligible against the 1% tolerance).
//
// At F = 1e-3, M_INERTIAL = 0.511, dt = 1 the predicted numbers are:
//   N= 1: a_COM = 1.95694e-3,  a·N = 1.95694e-3
//   N= 8: a_COM = 2.44618e-4,  a·N = 1.95695e-3
//   N=27: a_COM = 7.24795e-5,  a·N = 1.95695e-3
// i.e. a·N is constant to ~6 ppm across N — the 1/N inertia signature.
//
// All assertions are CPU-only (force_cpu), deterministic, and fast (<1s, L=12).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace ftd {

// ── Friend test hook (declared in render_bridge.h) ──────────────────────────
// force_diag_ has no public mutator (only the const force_diag() / _at()
// readers), so the falsifier writes the fixed injected force through this
// friend. dt_/V_COM are read through public dt() / voxel velocities.
void test_cluster_inertia_inject_force(RenderBridge& rb, int idx, const Vec3& f) {
    // Drive the WHOLE cluster force through one diagnostic channel; the cluster
    // pass sums f_coulomb + f_gravity + f_strong + f_magnetic, so any single
    // channel reproduces F_cluster exactly.
    rb.force_diag_[idx].f_coulomb = f;
}

namespace test {

// ---------------------------------------------------------------------------
// Build a solid (2k+1 isn't required — any contiguous cube) locked +1 cube of
// edge `edge` centred in the lattice and return the flat indices of its voxels.
// A solid cube is fully 26-connected, so phase_forces_integrate_clusters sees
// it as a single cluster of N = edge³ members.
//
// Each member is injected as a manifested +1 charge (state≠0 ⇒ it lands in
// ordered_active_indices(), which the flood-fill seeds from), locked=true (the
// cluster-membership predicate), velocity zeroed (so V_COM starts at 0), and
// its force_diag_ zeroed. The caller then injects the fixed total force.
// ---------------------------------------------------------------------------
static std::vector<int> build_locked_cube(RenderBridge& rb, int edge, int origin) {
    std::vector<int> members;
    members.reserve(static_cast<size_t>(edge) * edge * edge);
    for (int dx = 0; dx < edge; ++dx)
        for (int dy = 0; dy < edge; ++dy)
            for (int dz = 0; dz < edge; ++dz) {
                const int x = origin + dx, y = origin + dy, z = origin + dz;
                rb.inject_particle(x, y, z, +1, Vec3{0.0, 0.0, 0.0});
                Voxel& v = rb.voxel_at(x, y, z);
                v.locked   = true;
                v.velocity = Vec3{0.0, 0.0, 0.0};
                members.push_back(rb.lattice().index(x, y, z));
            }
    return members;
}

// Configure a Newtonian-clean bridge: forces + cluster_inertia ON, gravity and
// latency_field OFF (⇒ L=0 ⇒ γ_FTD=1), everything else OFF. dt left at 1.0.
static void configure_clean(RenderBridge& rb) {
    rb.toggles.disable_all();          // clears bulk toggles (cluster_inertia is
                                       // non-bulk, so set it explicitly below)
    rb.toggles.forces          = true; // injected-force falsifier uses f_coulomb
    rb.toggles.cluster_inertia = true;
    rb.toggles.gravity         = false; // no real gravity (we inject force directly)
    rb.toggles.latency_field   = false; // ⇒ every member latency stays 0
}

// One falsifier sample: an edge³ locked cube driven by a FIXED total force F,
// integrated by ONE direct call to phase_forces_integrate_clusters.
struct Sample {
    int    N        = 0;       // member count = edge³
    double a_com    = 0.0;     // V_COM.x / dt  (Newtonian a at L=0, dt scaling removed)
    double a_times_N = 0.0;    // a_COM · N     (the 1/N invariant)
    bool   rigid    = false;   // every member ended with identical velocity
};

static Sample run_sample(int edge, const Vec3& F) {
    const int L = 12;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);
    configure_clean(rb);

    // Centre the cube; for edge<=3 and L=12 this stays well inside [0,L).
    const int origin = L / 2 - edge / 2;
    const std::vector<int> members = build_locked_cube(rb, edge, origin);

    // Inject the FIXED total force so the members sum to F: each member gets
    // F/N, hence ΣF = F regardless of N (this is what makes a∝1/N visible —
    // the total is held constant as N grows).
    const int N = static_cast<int>(members.size());
    const Vec3 f_each = F * (1.0 / N);
    for (int idx : members) test_cluster_inertia_inject_force(rb, idx, f_each);

    // The single direct integration step under test.
    phase_forces_integrate_clusters(rb);

    Sample s;
    s.N = N;
    // V_COM is written to every member; read it from the first.
    const Vec3 v0 = rb.voxel_at(origin, origin, origin).velocity;
    s.a_com     = v0.x / rb.dt();
    s.a_times_N = s.a_com * N;

    // Rigid-body check: all members share the identical velocity vector.
    s.rigid = true;
    for (int idx : members) {
        const Vec3 vm = rb.voxels()[idx].velocity;
        if (vm.x != v0.x || vm.y != v0.y || vm.z != v0.z) { s.rigid = false; break; }
    }
    return s;
}

// ===========================================================================
// CI-1: a_COM = F/(N·M_INERTIAL) per cluster, and a_COM·N is constant (a ∝ 1/N).
// ===========================================================================
void test_inertia_scales_as_inverse_N() {
    section("CI-1: a_COM = F/(N*M_INERTIAL); a_COM*N constant across N (a proportional to 1/N)");

    const Vec3 F{1e-3, 0.0, 0.0};
    const Sample s1  = run_sample(1, F);   // single voxel
    const Sample s8  = run_sample(2, F);   // 2x2x2 cube
    const Sample s27 = run_sample(3, F);   // 3x3x3 cube

    std::printf("    [CI-1] F.x=%.3e  M_INERTIAL=%.3f  dt=1\n", F.x, M_INERTIAL);
    std::printf("    [CI-1]   N    a_COM            F/(N*M_INERTIAL) a_COM*N\n");
    auto row = [&](const Sample& s) {
        std::printf("    [CI-1]  %3d   %.10e   %.10e   %.10e\n",
                    s.N, s.a_com, F.x / (s.N * M_INERTIAL), s.a_times_N);
    };
    row(s1); row(s8); row(s27);

    // (a) Per-cluster Newtonian law: a_COM ≈ F.x/(N·M_INERTIAL) within ~1%.
    auto check_newton = [&](const Sample& s) {
        const double predicted = F.x / (s.N * M_INERTIAL);
        const double rel = std::abs(s.a_com - predicted) / predicted;
        char name[96];
        std::snprintf(name, sizeof(name),
                      "CI-1a: N=%d  a_COM == F/(N*M_INERTIAL) within 1%% (rel=%.2e)",
                      s.N, rel);
        check(name, rel < 0.01,
              "a_COM departed from the imposed Newtonian law a = F/(N*M_INERTIAL) by "
              ">1%. At L=0 (gravity/latency OFF) and F=1e-3 the gamma_FTD "
              "correction is <6 ppm, so this is a real deviation in the cluster "
              "integrator, not relativistic curvature.");
    };
    check_newton(s1);
    check_newton(s8);
    check_newton(s27);

    // (b) THE FALSIFIER: a_COM·N is constant across N within ~1% — heavier
    //     clusters resist the same total force proportionally (a ∝ 1/N).
    const double spread_8  = std::abs(s8.a_times_N  - s1.a_times_N) / s1.a_times_N;
    const double spread_27 = std::abs(s27.a_times_N - s1.a_times_N) / s1.a_times_N;
    std::printf("    [CI-1] a_COM*N spread vs N=1:  N=8 -> %.2e   N=27 -> %.2e\n",
                spread_8, spread_27);
    check("CI-1b: a_COM*N constant N=1 vs N=8 within 1% (a proportional to 1/N)",
          spread_8 < 0.01,
          "a_COM*N changed by >1% from N=1 to N=8: the cluster inertial mass is "
          "NOT scaling as N*M_INERTIAL, so acceleration is not proportional to 1/N.");
    check("CI-1b: a_COM*N constant N=1 vs N=27 within 1% (a proportional to 1/N)",
          spread_27 < 0.01,
          "a_COM*N changed by >1% from N=1 to N=27: the cluster inertial mass is "
          "NOT scaling as N*M_INERTIAL, so acceleration is not proportional to 1/N.");

    // Non-vacuity: there must actually be acceleration.
    check("CI-1: acceleration is non-trivial (test is non-vacuous)",
          s1.a_com > 1e-9,
          "a_COM ~ 0 for the single-voxel cluster; the injected force never "
          "reached the integrator and the 1/N comparison is vacuous.");
}

// ===========================================================================
// CI-2: rigid body — every member of a cluster ends with the IDENTICAL velocity.
// ===========================================================================
void test_rigid_body_velocity() {
    section("CI-2: rigid body — all members of a cluster share one V_COM");

    const Vec3 F{1e-3, 0.0, 0.0};
    const Sample s8  = run_sample(2, F);
    const Sample s27 = run_sample(3, F);

    check("CI-2: 2x2x2 (N=8) cluster is rigid (all member velocities identical)",
          s8.rigid,
          "Members of the N=8 locked cluster ended with DIFFERENT velocities; "
          "the cluster pass must write the single V_COM to every member.");
    check("CI-2: 3x3x3 (N=27) cluster is rigid (all member velocities identical)",
          s27.rigid,
          "Members of the N=27 locked cluster ended with DIFFERENT velocities; "
          "the cluster pass must write the single V_COM to every member.");
}

// ===========================================================================
// CI-3: zero force ⇒ zero V_COM (no spurious self-acceleration).
// ===========================================================================
void test_zero_force_no_motion() {
    section("CI-3: F_cluster = 0 ==> V_COM stays 0 (no spurious self-acceleration)");

    const int L = 12;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);
    configure_clean(rb);

    const int edge = 3, origin = L / 2 - edge / 2;
    const std::vector<int> members = build_locked_cube(rb, edge, origin);
    // Deliberately inject NOTHING into force_diag_ (build_locked_cube left every
    // member's force_diag_ zeroed via inject_particle), so F_cluster = 0.

    phase_forces_integrate_clusters(rb);

    double vmax = 0.0;
    for (int idx : members) vmax = std::max(vmax, rb.voxels()[idx].velocity.mag());
    std::printf("    [CI-3] max |V_COM| with F_cluster=0:  %.3e\n", vmax);

    check("CI-3: zero cluster force leaves V_COM exactly 0",
          vmax == 0.0,
          "A locked cluster with ZERO total force acquired non-zero velocity: "
          "the cluster integrator is injecting spurious self-acceleration.");
}

// ===========================================================================
// CI-4: NO-OP GUARD — with cluster_inertia=false the pass never runs, so a tick
// leaves V_COM = 0 (defends the additive / golden-safe property).
//
// The toggle gate `if (toggles.cluster_inertia)` lives in
// RenderBridge::phase_forces() (render_bridge.cpp:469), NOT inside the free
// function — so this guard must be exercised at the gating level. We drive it
// through the public tick(): forces ON, cluster_inertia OFF, everything else
// (wave/genesis/movement/gravity/gauss) OFF so the tick is minimal. The locked
// members are skipped by the per-voxel integrator (`if (!v.locked)`) AND the
// cluster pass is gated off, so V_COM must stay 0.
// ===========================================================================
void test_noop_guard_when_toggle_off() {
    section("CI-4: cluster_inertia=false ==> tick() does not move the cluster (golden-safe)");

    const int L = 12;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);
    rb.toggles.disable_all();
    rb.toggles.forces          = true;   // the gate lives on the forces path
    rb.toggles.cluster_inertia = false;  // THE guard under test
    rb.toggles.gravity         = false;
    rb.toggles.latency_field   = false;

    const int edge = 3, origin = L / 2 - edge / 2;
    const std::vector<int> members = build_locked_cube(rb, edge, origin);
    // Even with a non-trivial injected force, the OFF gate must keep V_COM = 0.
    const Vec3 f_each = Vec3{1e-3, 0.0, 0.0} * (1.0 / members.size());
    for (int idx : members) test_cluster_inertia_inject_force(rb, idx, f_each);

    rb.tick();   // full cycle, but the cluster pass is gated OFF

    double vmax = 0.0;
    for (int idx : members) vmax = std::max(vmax, rb.voxels()[idx].velocity.mag());
    std::printf("    [CI-4] max |V_COM| after tick with cluster_inertia OFF:  %.3e\n", vmax);

    check("CI-4: cluster_inertia OFF leaves the locked cluster at rest after a tick",
          vmax == 0.0,
          "With cluster_inertia=false a tick moved the locked cluster: the toggle "
          "gate in phase_forces() is not protecting the additive / golden-safe "
          "property (the pass must be a strict no-op when the toggle is off).");
}

// ===========================================================================
// CI-5: THE EQUIVALENCE PRINCIPLE — universal free-fall, a INDEPENDENT of N.
// ---------------------------------------------------------------------------
// CI-1 (above) held the TOTAL force F fixed as N grew, isolating the inertial
// 1/N signature: a = F/(N·M_INERTIAL) ⇒ a ∝ 1/N. CI-5 is the exact COMPLEMENT and
// the textbook EP discriminator: hold the PER-VOXEL force f fixed instead. Then
// the cluster force scales with the cluster's own mass,
//
//     F_cluster = Σ_members f = N·f         (f = per-voxel force, same for all)
//
// and the inertial calibration N·M_INERTIAL CANCELS it exactly:
//
//     a_COM = F_cluster/(N·M_INERTIAL) = (N·f)/(N·M_INERTIAL) = f/M_INERTIAL.
//
// The N's cancel ⇒ a_COM is the SAME for every cluster regardless of how many
// voxels it contains. Two clusters of DIFFERENT N released in the SAME uniform
// per-voxel field fall with the SAME acceleration — universal free-fall, the
// operational equivalence principle. (For a real FTD field this f is exactly
// what gravity/EM deliver: a uniform per-voxel pull whose cluster total ∝ N,
// see the CI-1 header. CI-5 supplies that uniform f directly so latency/gravity
// can stay OFF and the arithmetic is Newtonian-clean.)
//
// EPISTEMIC STATUS — this is a DEMONSTRATION, not a derivation. One raw
// M_INERTIAL scalar is [IMPOSED]. M_GRAVITATIONAL currently has the same
// numerical value but a separate role; their equality is not an action theorem.
// The ENGINE exhibiting the EP
// relation here is CONDITIONAL
// on the [IMPOSED] cluster-inertia mechanism (FTD-0250): the rigid-body
// collective-coordinate reduction a_COM = F_cluster/(N·M_INERTIAL) is imposed on the
// engine, not derived from the per-voxel dynamics (that reduction is [OPEN]).
// So CI-5 shows the imposed mechanism is internally EP-consistent; it does not
// derive the EP from the substrate.
//
// CONSTRUCTION: two well-separated LOCKED cubes in ONE bridge — N=8 (edge 2) at
// a low corner, N=27 (edge 3) at a high corner. At L=16 the cubes occupy
// [1,2]³ and [11,13]³; their nearest faces are 9 voxels apart, far beyond
// 26-Moore adjacency (≤1), so the flood-fill resolves them as TWO distinct
// clusters. The SAME uniform per-voxel f = {1e-3,0,0} is injected into EVERY
// member of BOTH, so F_cluster = N·f for each. One direct call integrates both.
// ===========================================================================
void test_equivalence_principle_universal_freefall() {
    section("CI-5: EQUIVALENCE PRINCIPLE — a_COM independent of N (universal free-fall) [DEMONSTRATION, not derivation; conditional on [IMPOSED] FTD-0250]");

    const int L = 16;
    RenderBridge rb(L);
    rb.force_cpu();
    rb.seed_rng(1234);
    configure_clean(rb);   // forces+cluster_inertia ON, gravity+latency OFF ⇒ L=0

    // Two separated locked cubes of DIFFERENT N in the same lattice.
    const std::vector<int> cubeA = build_locked_cube(rb, /*edge=*/2, /*origin=*/1);   // N=8,  [1,2]^3
    const std::vector<int> cubeB = build_locked_cube(rb, /*edge=*/3, /*origin=*/11);  // N=27, [11,13]^3
    const int NA = static_cast<int>(cubeA.size());
    const int NB = static_cast<int>(cubeB.size());

    // SAME uniform per-voxel force into EVERY member of BOTH cubes ⇒
    // F_cluster(A) = NA·f, F_cluster(B) = NB·f (the gravitational/source side
    // scales with N, exactly as a real field would).
    const Vec3 f{1e-3, 0.0, 0.0};
    for (int idx : cubeA) test_cluster_inertia_inject_force(rb, idx, f);
    for (int idx : cubeB) test_cluster_inertia_inject_force(rb, idx, f);

    // One direct integration step resolves both clusters.
    phase_forces_integrate_clusters(rb);

    // Read each cluster's COM acceleration (V_COM written to every member; L=0,
    // so a = V_COM.x/dt is the clean Newtonian acceleration).
    const double aA = rb.voxel_at(1, 1, 1).velocity.x   / rb.dt();
    const double aB = rb.voxel_at(11, 11, 11).velocity.x / rb.dt();
    const double a_predicted = f.x / M_INERTIAL;             // f/M_INERTIAL, N-independent
    const double Fclus_A = NA * f.x;                         // source/gravitational side
    const double Fclus_B = NB * f.x;

    std::printf("    [CI-5] uniform per-voxel f.x=%.3e   M_INERTIAL=%.3f   dt=1\n", f.x, M_INERTIAL);
    std::printf("    [CI-5]   cluster   N    F_cluster=N*f    a_COM            f/M_INERTIAL\n");
    std::printf("    [CI-5]      A     %3d   %.10e   %.10e   %.10e\n", NA, Fclus_A, aA, a_predicted);
    std::printf("    [CI-5]      B     %3d   %.10e   %.10e   %.10e\n", NB, Fclus_B, aB, a_predicted);

    // (a) EP — the two clusters of different N accelerate EQUALLY (≤1%).
    const double rel_AB = std::abs(aA - aB) / std::abs(aB);
    std::printf("    [CI-5] |a(N=8) - a(N=27)| / a(N=27) = %.2e  (EP: should be ~0)\n", rel_AB);
    check("CI-5a: EP — a_COM(N=8) == a_COM(N=27) within 1% (universal free-fall, a independent of N)",
          rel_AB < 0.01,
          "Two locked clusters of different N in the SAME uniform per-voxel field "
          "accelerated by DIFFERENT amounts: the inertial calibration N*M_INERTIAL is "
          "NOT cancelling the N-scaling of F_cluster=N*f, so the imposed "
          "cluster-inertia mechanism violates the equivalence principle.");

    // (b) Each cluster's a_COM equals the N-independent value f.x/M_INERTIAL (≤1%).
    auto check_universal = [&](const char* tag, int N, double a) {
        const double rel = std::abs(a - a_predicted) / a_predicted;
        char name[112];
        std::snprintf(name, sizeof(name),
                      "CI-5b: %s (N=%d) a_COM == f/M_INERTIAL within 1%% (rel=%.2e)", tag, N, rel);
        check(name, rel < 0.01,
              "Cluster a_COM departed from the N-independent free-fall value "
              "f/M_INERTIAL by >1%. At L=0 and f=1e-3 the gamma_FTD correction is "
              "<6 ppm, so this is a real deviation in the cluster integrator.");
    };
    check_universal("cube A", NA, aA);
    check_universal("cube B", NB, aB);

    // (c) The SOURCE side scales with N while a stays constant — i.e. the two
    //     N-dependencies (F_cluster ∝ N on the force side, mass ∝ N on the
    //     inertia side) genuinely cancel. Assert F_cluster ∝ N exactly.
    const double ratio_F = Fclus_B / Fclus_A;     // = NB/NA = 27/8 = 3.375
    const double ratio_N = static_cast<double>(NB) / NA;
    std::printf("    [CI-5] F_cluster(B)/F_cluster(A) = %.6f   NB/NA = %.6f\n", ratio_F, ratio_N);
    check("CI-5c: F_cluster proportional to N (NB/NA = 27/8) while a_COM is N-independent",
          std::abs(ratio_F - ratio_N) < 1e-9,
          "F_cluster did not scale as N: the source/gravitational side is not "
          "growing with cluster mass, so the EP cancellation a=f/M_INERTIAL is "
          "coincidental rather than the N's genuinely cancelling.");

    // Non-vacuity: there must actually be acceleration, and the two cubes must
    // really differ in N (otherwise CI-5a is trivially satisfied).
    check("CI-5: acceleration is non-trivial and N differs (test is non-vacuous)",
          aA > 1e-9 && aB > 1e-9 && NA != NB,
          "Either a_COM ~ 0 (force never reached the integrator) or the two "
          "cubes had equal N — the EP comparison would be vacuous.");
}

}  // namespace test
}  // namespace ftd

int main() {
    ftd::test::init("test_cluster_inertia");

    ftd::test::test_inertia_scales_as_inverse_N();           // CI-1 (a) + (b): a ∝ 1/N (inertia)
    ftd::test::test_rigid_body_velocity();                   // CI-2
    ftd::test::test_zero_force_no_motion();                  // CI-3
    ftd::test::test_noop_guard_when_toggle_off();            // CI-4
    ftd::test::test_equivalence_principle_universal_freefall(); // CI-5: a independent of N (EP)

    return ftd::test::finalize();
}
