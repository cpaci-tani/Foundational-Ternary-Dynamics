// ==========================================================================
//  engine/src/scenarios/s0_seed.cpp
//
//  Group: s0-seed-* (50 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
//
//  Split out of engine/src/scenarios.cpp (ticket S1). The three internal
//  static helpers seed_lepton / dp / tri moved with this group because
//  they are only used by s0-seed-* scenarios.
//
//  Parameterization pass (2026-09): every meaningful hardcoded seed input is
//  now exposed via ftd::seed::{real,integer,choice,boolean}. With no
//  ftd::seed::Context installed (the legacy dispatch_scenario() path), every
//  call below returns its literal default unchanged, so behavior, arithmetic
//  order, centering/rounding, and RNG draw ordering are bit-identical to the
//  pre-parameterization file. Exposing an amplitude, radius fraction, or
//  count as a bounded, labeled input is a preparation-time convenience, not
//  a new physics claim; qualification language in each scenario's comment
//  block is unchanged.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

using detail::urand;

// (seed_lepton helper removed audit-4 2026-04-28: only callers were
// s0-seed-{electron, muon, tau} which are now canonical in vacuum.cpp.)

namespace {
// Per-axis construction center, read once per scenario body before any
// placement loop. Most s0-seed-* bodies previously hardcoded their
// construction center to the lattice midpoint (mc/midF) with no override —
// this is the shared fix threaded through the call sites below. Returns both
// the double-precision center (for envelope/Gaussian math, which used the
// unrounded midF historically) and its rounded integer form (for marker
// placement, which used the rounded mc historically).
struct SourceCenter { double x, y, z; int ix, iy, iz; };

inline SourceCenter read_source_center(int N, double default_center) {
    SourceCenter c;
    c.x = seed::real("source.x", default_center, 0, N - 1,
        "Center x", "Lattice x-coordinate of the construction's center.", "lattice units", 1.0);
    c.y = seed::real("source.y", default_center, 0, N - 1,
        "Center y", "Lattice y-coordinate of the construction's center.", "lattice units", 1.0);
    c.z = seed::real("source.z", default_center, 0, N - 1,
        "Center z", "Lattice z-coordinate of the construction's center.", "lattice units", 1.0);
    c.ix = RND(c.x); c.iy = RND(c.y); c.iz = RND(c.z);
    return c;
}
}  // namespace

bool setup_s0_seed_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-seed-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    // Reused across the shape/geometry seeds below: a ±1 polarity choice.
    static const seed::Options POLARITY = {{-1, "Negative (-1)"}, {1, "Positive (+1)"}};

    // Audit-4 2026-04-28: s0-seed-{electron, muon, tau, photon} removed —
    // mirrors of s0-vacuum-{electron, muon, tau, photon} which are now canonical.
    // s0-seed-proton-candidate also removed earlier (audit-3).

    // ── Native source-built flux response ──
    if (name == "s0-seed-dynamical-flux-dressing") {
        // Scenario ID: s0-seed-dynamical-flux-dressing
        // Physical purpose: visualize the field generated dynamically from
        // zero initial J/W by the existing -G_C*grad(s) source term.
        // Qualification: FTD-0476 source-built dressing probe.  This is not an
        // electromagnetic aura, a pilot wave, or a radiation demonstration.
        configure_locked_coupled_field_terms(rb);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const int source_polarity = seed::choice("source.polarity", +1, POLARITY,
            "Source polarity", "Ternary manifestation state of the single locked source voxel. Sign only flips the source-built field's overall sign.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, source_polarity);
        LOCK(rb, c.ix, c.iy, c.iz);
    }
    else if (name == "s0-seed-moving-source-reciprocity") {
        // Scenario ID: s0-seed-moving-source-reciprocity
        // Physical purpose: visualize the mechanical response of a previously
        // resting polarity to a spatially separate finite flux packet through
        // the selected G_C*s*grad|J| production extension.
        // Qualification: FTD-0477 measured a deterministic 0.203598-cell
        // subvoxel response but no integer hop through tick 72.  The source
        // velocity is never prescribed; this remains a selected-extension
        // discriminator, not native qE, electromagnetism, or radiation.
        configure_emergent_recoil_terms(rb);
        rb.toggles.strict_validation = true;
        const double width_divisor = seed::real("geometry.sourceWidthDivisor", 22.0, 4.0, 200.0,
            "Source width divisor", "N/divisor sets the resting-source patch half-width before the [1,3] safety clamp; a smaller divisor widens the patch.", "lattice units", 0.5);
        const double separation_fraction = seed::real("geometry.separationFraction", 0.31, 0.05, 0.9,
            "Separation fraction", "Fraction of N used for the packet-to-source separation before the safety clamp; larger values start the packet farther away.", "fraction of N", 0.01);
        const double packet_amplitude = seed::real("packet.amplitude", 0.5, 0.0, 5.0,
            "Packet peak amplitude", "Peak amplitude of the injected transverse flux packet, in units of C_SPEED-normalized sigma; scales the whole probe linearly.", "lattice units", 0.01);
        const int width = std::max(1, std::min(3, RND(N / width_divisor)));
        const int separation = std::min(
            std::max(6, RND(separation_fraction * N)), std::max(3, mc - 1));
        const int source_offset = N <= 9 ? std::min(mc, 4) : width;
        const int source_x = seed::integer("source.x", mc, 0, N-1, "Marker x", "Moves the resting marker along x independently of the probe.");
        const int source_y = seed::integer("source.y", (mc + source_offset) % N, 0, N-1, "Marker y", "Moves the resting marker along y; reset follows the source-width offset.");
        const int source_z = seed::integer("source.z", mc, 0, N-1, "Marker z", "Moves the resting marker along z independently of the probe.");
        const int polarity = seed::choice("source.polarity", 1, POLARITY, "Marker polarity", "Changes the resting marker's ternary sign.");
        IP(rb, source_x, source_y, source_z, polarity);
        rb.voxels()[rb.lattice().index(source_x, source_y, source_z)].locked = seed::boolean("source.locked", false, "Marker locked", "On pins the marker; Off permits the selected movement protocol to respond.");
        const double packet_x = seed::real("packet.x", mc-separation, -double(N), 2.0*N, "Probe center x", "Moves the probe along x; reset follows the separation from the lattice midpoint. An outside center clips the envelope at the domain.");
        const double packet_y = seed::real("packet.y", mc, 0, N-1, "Probe center y", "Moves the transverse probe envelope along y.");
        const double packet_z = seed::real("packet.z", mc, 0, N-1, "Probe center z", "Moves the transverse probe envelope along z.");
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", width, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, packet_x, packet_y, packet_z, width, seed_wave_0_sigmaTransverse, packet_amplitude, seed_wave_0_direction, seed_wave_0_carrierK, seed_wave_0_phase);
    }
    // ── Moore Seeds ──
    else if (name == "s0-seed-octahedron") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-octahedron
        // Physical Purpose: Seeds an octahedral arrangement of 6 face-neighboring charges.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by 6 positive charges.
        // Discrepancy: None.
        const int centerCharge = seed::choice("geometry.centerPolarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int shellCharge = seed::choice("geometry.shellPolarity", +1, POLARITY,
            "Shell polarity", "Ternary manifestation state shared by all 6 face-neighbor voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        const int off[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, c.ix+off[i][0], c.iy+off[i][1], c.iz+off[i][2], shellCharge);
    }
    else if (name == "s0-seed-cuboctahedron") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-cuboctahedron
        // Physical Purpose: Seeds a cuboctahedral arrangement of 12 edge-neighboring charges.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by 12 positive charges.
        // Discrepancy: None.
        const int centerCharge = seed::choice("geometry.centerPolarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int shellCharge = seed::choice("geometry.shellPolarity", +1, POLARITY,
            "Shell polarity", "Ternary manifestation state shared by all 12 edge-neighbor voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        const int off[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, c.ix+off[i][0], c.iy+off[i][1], c.iz+off[i][2], shellCharge);
    }
    else if (name == "s0-seed-stella-octangula") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-stella-octangula
        // Physical Purpose: Seeds a stella octangula arrangement of 8 corner charges.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by 8 positive charges.
        // Discrepancy: None.
        const int centerCharge = seed::choice("geometry.centerPolarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int shellCharge = seed::choice("geometry.shellPolarity", +1, POLARITY,
            "Shell polarity", "Ternary manifestation state shared by all 8 corner voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        const int off[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, c.ix+off[i][0], c.iy+off[i][1], c.iz+off[i][2], shellCharge);
    }
    else if (name == "s0-seed-moore-cell") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-moore-cell
        // Physical Purpose: Seeds a full 26-neighbor Moore cell.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by 26 positive charges.
        // Discrepancy: None.
        // genesis=false (audit-2 2026-04-28): the 27-site geometric seed
        // should stay a 27-site seed. Mirrors JS s0-seed-moore-cell.
        rb.toggles.genesis = false;
        const int centerCharge = seed::choice("geometry.centerPolarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int shellCharge = seed::choice("geometry.shellPolarity", +1, POLARITY,
            "Shell polarity", "Ternary manifestation state shared by all 26 Moore-neighbor voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        for (int dx = -1; dx <= 1; dx++) for (int dy = -1; dy <= 1; dy++) for (int dz = -1; dz <= 1; dz++) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            IP(rb, c.ix+dx, c.iy+dy, c.iz+dz, shellCharge);
        }
    }
    else if (name == "s0-seed-emergent-ic1") {
        // Scenario ID: s0-seed-emergent-ic1
        // Physical Purpose: Finite axial A=10 genesis-response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: At L=24 the deterministic count is 3 at ticks
        // 100 and 120. The advertised 25-site octahedron is closed negative.
        // FTD-0102 / FTD-0107 ic1 (point injection).
        // Inject 10·K_GENESIS flux at lattice center; under the right
        // toggles (genesis + langevin + gauss_projection + wave_propagation),
        // the dynamics produce the emergent 25-voxel L¹-ball-radius-2
        // octahedral bound state. See:
        //   docs/theory/10_eft_program/ANALYSIS_EMERGENT_SPECTRUM_G1.md
        //   docs/theory/08_structural/EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md
        //   docs/theory/08_structural/EXPLR_OCTAHEDRAL_BOUND_STATES.md
        //
        // This scenario sets the required toggles directly so the scenario
        // is self-contained when invoked from the WASM bridge or tests.
        const double bath_t = 0.005;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 10.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS to set the axial point-injection amplitude A.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic3-collision") {
        // Scenario ID: s0-seed-emergent-ic3-collision
        // Physical Purpose: Finite response to two separated opposite A=5 seeds.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: At L=24 the deterministic count is 2 at ticks
        // 100 and 120; two 2-3-site collision products are not observed.
        // FTD-0102 / FTD-0107 ic3 (two-beam collision).
        // Two opposing flux beams at ±L/4 from centre on the x-axis
        // produce 2 stable bound states of 2-3 voxels each at the
        // collision points. Reproduced 5/5 seeds at L=32 and L=64
        // post-fix (RTX 5090, 2026-04-27).
        const double bath_t = 0.005;
        const int separation_divisor = seed::integer("geometry.separationDivisor", 4, 2, 64,
            "Beam separation divisor", "N/divisor sets each beam's offset from the lattice center along x.");
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 5.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS to set the magnitude of each opposing point injection.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        const int q = N / separation_divisor;
        IF(rb, c.ix - q, c.iy, c.iz, +amplitude_mult * K_GENESIS, 0, 0);
        IF(rb, c.ix + q, c.iy, c.iz, -amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic4-subthreshold") {
        // Scenario ID: s0-seed-emergent-ic4-subthreshold
        // Physical Purpose: Sub-threshold negative control point injection (FTD-0107).
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Zero manifested sites through tick 120 at L=24.
        // FTD-0102 / FTD-0107 ic4 (sub-threshold injection).
        // 0.5·K_GENESIS at lattice centre — below the K_GENESIS gap.
        // Pre-registered Outcome: 0 manifested voxels across 5/5 seeds
        // (negative control demonstrating the genesis threshold).
        const double bath_t = 0.005;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 0.5, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS to set the point-injection amplitude; the pre-registered negative-control reading assumes the default (below the genesis gap).", "×K_GENESIS", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic2-thermal-runaway") {
        // Scenario ID: s0-seed-emergent-ic2-thermal-runaway
        // Physical Purpose: Empty-lattice T=0.05 Langevin/genesis bath probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Zero manifested sites through tick 120 at L=24;
        // the thermal-runaway interpretation is closed for this finite run.
        // FTD-0102 / FTD-0107 ic2 (thermal-driven runaway).
        // No flux injection — only elevated Langevin T = 0.05 (10× the
        // standard ic1/ic3 setting). Demonstrates the unstable-phase
        // regime where pure thermal noise drives runaway genesis.
        // The L=32 seed-4 finite-size escape observed in the post-fix
        // re-measurement lives in this phase-space neighbourhood.
        const double bath_t = 0.05;
        configure_genesis_cluster_terms(rb, bath_t); // 10x ic1
        // No IF call — thermal noise alone drives the dynamics.
    }
    else if (name == "s0-seed-emergent-ic1-diagonal") {
        // Scenario ID: s0-seed-emergent-ic1-diagonal
        // Physical Purpose: Body-diagonal A=10 genesis-response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: One manifested site at ticks 100 and 120 at L=24.
        // FTD-0110 D3g: body-diagonal injection.
        // Same total amplitude as ic1 (10·K_GENESIS) but distributed along
        // (1,1,1)/√3 instead of +x. The 3-fold rotation about the body
        // diagonal is Z_3 (not Z_4); if the cluster-efficiency ¼ comes from
        // the i-cycle Z_4 about the injection axis, then a body-diagonal
        // injection should give k ≈ 1/3 instead of ¼ — and a cluster size
        // of (1/3)·A² ≈ 33 voxels at A=10 (vs 25 for axial).
        // If k stays at ¼, the structural origin is global (N_base, not
        // axis-specific Z_4).
        const double bath_t = 0.005;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 10.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the (1,1,1)/√3 decomposition keeps the total vector magnitude equal to this value times K_GENESIS.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
        const double A = amplitude_mult * K_GENESIS * inv_sqrt3;
        IF(rb, c.ix, c.iy, c.iz, A, A, A);
    }
    else if (name == "s0-seed-emergent-ic1-isotropic") {
        // Scenario ID: s0-seed-emergent-ic1-isotropic
        // Physical Purpose: Six-axis A=10 genesis-response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Eight manifested sites at ticks 100 and 120 at L=24.
        // FTD-0110 D3h: isotropic 6-axis injection at the canonical
        // ic1 amplitude. Decomposes A·K_GENESIS uniformly across the
        // 6 SC face-neighbour directions of the centre voxel; the
        // resulting bound state should be O_h-symmetric under all
        // cube rotations (no injection-direction breaking the +x/−x
        // asymmetry seen in s0-seed-emergent-ic1).
        const double bath_t = 0.005;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 10.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the 6-axis decomposition keeps the summed vector-budget scale equal to this value squared times K_GENESIS squared.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        // Distribute 10·K_GENESIS magnitude across 6 directions: each
        // of the 6 face neighbours of the centre receives a flux pointing
        // outward from the centre with magnitude (10/√6)·K_GENESIS
        // (so |J|² summed across all 6 voxels = 10²·K_GENESIS² as in ic1).
        const double inv_sqrt6 = 1.0 / std::sqrt(6.0);
        const double a = amplitude_mult * K_GENESIS * inv_sqrt6;
        IF(rb, c.ix + 1, c.iy, c.iz, +a, 0, 0);
        IF(rb, c.ix - 1, c.iy, c.iz, -a, 0, 0);
        IF(rb, c.ix, c.iy + 1, c.iz, 0, +a, 0);
        IF(rb, c.ix, c.iy - 1, c.iz, 0, -a, 0);
        IF(rb, c.ix, c.iy, c.iz + 1, 0, 0, +a);
        IF(rb, c.ix, c.iy, c.iz - 1, 0, 0, -a);
    }
    else if (name == "s0-seed-emergent-ic1-viz") {
        // Scenario ID: s0-seed-emergent-ic1-viz
        // Physical Purpose: Axial A=20 zero-temperature response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Deterministic but decaying count, 22 -> 20 from
        // ticks 100 -> 120 at L=24; no static/stable claim.
        // Clean visualisation of the axial ic1 cluster (dashboard demo).
        // Uses A=20·K_GENESIS instead of the campaign A=10. The amplitude was
        // chosen 2026-04-27 when only the CPU path applied the manifestation
        // drain; since FTD-0276 (2026-06-12) `kinetic_drain` is honoured on both
        // backends. Kept for continuity of the pinned counts (LEDGER draft
        // FTD-1044 records the provenance). T=0 disables
        // Langevin thermal driving so the cluster is NOT obscured by
        // background thermal genesis. Run ~200 ticks for clearest view.
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 20.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS to set the axial point-injection amplitude A.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law") {
        // Scenario ID: s0-seed-cluster-law
        // Qualification target: the dashboard's default interactive point only.
        // At L=24, T=0.005, A=10 the selected profile has 3 manifested sites
        // at ticks 100 and 120 with bit-exact replay.  User-selected amplitudes
        // are new experiments; no universal N(A), knee, or power law is implied.
        const double bath_t = 0.005;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 10.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS to set the axial point-injection amplitude A. A different value is a new, unregistered experiment.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-subknee") {
        // Scenario ID: s0-seed-cluster-law-subknee
        // Physical Purpose: Fixed finite-box genesis response at A=12.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Smallest nonzero member of the registered
        // A=12/16/40 ordering, stable from ticks 200 to 220 at L=24.
        // No universal N(A) law or geometric-regime label is inferred.
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 12.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the registered A=12/16/40 ordering assumes the default.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-knee") {
        // Scenario ID: s0-seed-cluster-law-knee
        // Physical Purpose: Fixed finite-box genesis response at A=16.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Middle member of the registered A=12/16/40
        // ordering, stable from ticks 200 to 220 at L=24. No knee is claimed.
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 16.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the registered A=12/16/40 ordering assumes the default.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-cluster-law-superknee") {
        // Scenario ID: s0-seed-cluster-law-superknee
        // Physical Purpose: Fixed finite-box genesis response at A=40.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Largest member of the registered A=12/16/40
        // ordering, stable from ticks 200 to 220 at L=24. No A-squared law is claimed.
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 40.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the registered A=12/16/40 ordering assumes the default.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        IF(rb, c.ix, c.iy, c.iz, amplitude_mult * K_GENESIS, 0, 0);
    }
    else if (name == "s0-seed-emergent-ic1-diagonal-viz") {
        // Scenario ID: s0-seed-emergent-ic1-diagonal-viz
        // Physical Purpose: Body-diagonal A=20 zero-temperature response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Deterministic but decaying count, 22 -> 20 from
        // ticks 100 -> 120 at L=24; no static/stable claim.
        // Clean body-diagonal cluster (D3g shape comparison).
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 20.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the (1,1,1)/√3 decomposition keeps the total vector magnitude equal to this value times K_GENESIS.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        const double inv_sqrt3 = 1.0 / std::sqrt(3.0);
        const double A = amplitude_mult * K_GENESIS * inv_sqrt3;
        IF(rb, c.ix, c.iy, c.iz, A, A, A);
    }
    else if (name == "s0-seed-emergent-ic1-isotropic-viz") {
        // Scenario ID: s0-seed-emergent-ic1-isotropic-viz
        // Physical Purpose: Six-axis A=20 zero-temperature response probe.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Deterministic but decaying count, 20 -> 18 from
        // ticks 100 -> 120 at L=24; no static/stable claim.
        // Clean isotropic 6-axis injection (D3h full O_h symmetry view).
        const double bath_t = 0.0;
        const double amplitude_mult = seed::real("source.amplitudeMultiplier", 20.0, 0.0, 100.0,
            "Injection amplitude multiplier", "Multiplies K_GENESIS; the 6-axis decomposition keeps the summed vector-budget scale equal to this value squared times K_GENESIS squared.", "×K_GENESIS", 0.5);
        const SourceCenter c = read_source_center(N, mc);
        configure_genesis_cluster_terms(rb, bath_t);
        const double inv_sqrt6 = 1.0 / std::sqrt(6.0);
        const double a = amplitude_mult * K_GENESIS * inv_sqrt6;
        IF(rb, c.ix + 1, c.iy, c.iz, +a, 0, 0);
        IF(rb, c.ix - 1, c.iy, c.iz, -a, 0, 0);
        IF(rb, c.ix, c.iy + 1, c.iz, 0, +a, 0);
        IF(rb, c.ix, c.iy - 1, c.iz, 0, -a, 0);
        IF(rb, c.ix, c.iy, c.iz + 1, 0, 0, +a);
        IF(rb, c.ix, c.iy, c.iz - 1, 0, 0, -a);
    }
    // s0-seed-symmetry-regression removed 2026-04-28 (audit removal): engine
    // CI artefact (voxel_uniform() RNG determinism check), not a user-facing
    // physics scenario. Fold into a ctest under engine/tests/ if still needed.

    else if (name == "s0-seed-moore-decomposition") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-moore-decomposition
        // Physical Purpose: Seeds a Moore cell decomposed into shell layers.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by octahedron (+1), cuboctahedron (-1), and stella octangula (+1).
        // Discrepancy: None.
        const int centerCharge = seed::choice("geometry.centerPolarity", -1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int octCharge = seed::choice("geometry.octahedronPolarity", +1, POLARITY,
            "Octahedron shell polarity", "Ternary manifestation state of the 6 face-neighbor voxels.");
        const int cubCharge = seed::choice("geometry.cuboctahedronPolarity", -1, POLARITY,
            "Cuboctahedron shell polarity", "Ternary manifestation state of the 12 edge-neighbor voxels.");
        const int stelCharge = seed::choice("geometry.stellaOctangulaPolarity", +1, POLARITY,
            "Stella octangula shell polarity", "Ternary manifestation state of the 8 corner voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, c.ix+oct[i][0], c.iy+oct[i][1], c.iz+oct[i][2], octCharge);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, c.ix+cub[i][0], c.iy+cub[i][1], c.iz+cub[i][2], cubCharge);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, c.ix+stel[i][0], c.iy+stel[i][1], c.iz+stel[i][2], stelCharge);
    }
    // ── Composite seeds via dp/tri helpers ──
    // Audit-3 2026-04-28: removed electron-l3, neutrino, quark, antiquark.
    // Audit-4 2026-04-28: removed positron, pion, proton-l4, neutron — all
    // now canonical in vacuum.cpp (s0-vacuum-*).
    else if (name == "s0-seed-hydrogen") {
        // Scenario ID: s0-seed-hydrogen
        // Legacy label: hydrogen atom.
        // Qualification target: a prepared locked-nucleus Coulomb candidate.
        // Binding is measured rather than inferred from the atom label.
        configure_prepared_coulomb_candidate_terms(rb);
        const int outer_divisor = seed::integer("geometry.electronOrbitDivisor", 6, 2, 64,
            "Electron orbit divisor", "N/divisor (floored at 4) sets the electron marker's distance from the nucleus along z.");
        const int nucleus_divisor = seed::integer("geometry.nucleusRadiusDivisor", 12, 2, 64,
            "Nucleus triad radius divisor", "N/divisor (floored at 2) sets the 3-quark triad's circumradius.");
        const double electron_dress_sigma = seed::real("constituent0.dressSigma", 2.0, 0.5, 10.0,
            "Electron dressing sigma", "Gaussian falloff width of the electron marker's radial flux dressing.", "lattice units", 0.1);
        const double electron_dress_mult = seed::real("constituent0.dressAmplitudeMultiplier", 1.0, 0.0, 5.0,
            "Electron dressing amplitude multiplier", "Multiplies K_B to set the electron marker's dressing amplitude.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const double nucleusRotationOffset = seed::real("geometry.nucleusRotationOffset", 0.0, -PI, PI,
            "Nucleus triad rotation offset", "Additive angle rotating all three quark vertices about the nucleus center; 0 reproduces the original fixed 120°-spaced layout.", "radians", 0.01);
        const double nucleusDressSigma = seed::real("constituent1.dressSigma", 2.0, 0.1, 10.0,
            "Nucleus dressing sigma", "Gaussian falloff width of each of the 3 nucleus quark markers' dressing (shared by symmetry).", "lattice units", 0.1);
        const double nucleusDressAmplitudeMultiplier = seed::real("constituent1.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Nucleus dressing amplitude multiplier", "Multiplies K_B to set each nucleus quark marker's dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const int oR = std::max(4, N / outer_divisor);
        const int bR = std::max(2, N / nucleus_divisor);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        // B4 (2026-07-27): place the triad + electron before dressing either
        // -- IPF always zeroes flux at its own center, so dressing the triad
        // after placing the electron (or vice versa) could silently discard
        // whichever one's dressing landed on the other's voxel.
        const TriPositions nucleus = tri_place(rb, c.ix, c.iy, c.iz, charges, colors, bR, true, nucleusRotationOffset);
        dp_place(rb, c.ix, c.iy, c.iz + oR, -1, -1, 0, false);
        tri_dress(rb, nucleus, c.iz, charges, nucleusDressSigma, K_B * nucleusDressAmplitudeMultiplier);
        dp_dress(rb, c.ix, c.iy, c.iz + oR, -1, electron_dress_sigma, K_B * electron_dress_mult);
    }
    else if (name == "s0-seed-helium") {
        // Scenario ID: s0-seed-helium
        // Legacy label: helium atom.
        // Qualification target: a prepared locked-nucleus two-electron
        // Coulomb candidate, not an emergent alpha particle or 1s orbital.
        configure_prepared_coulomb_candidate_terms(rb);
        // ⁴He / α-particle (audit 2026-04-28 fix): 2 protons + 2 neutrons
        // at tetrahedral vertices + 2 electrons in 1s² shell. Each nucleon
        // is a 3-quark triad. Mirrors JS s0-seed-helium body.
        const int outer_divisor = seed::integer("geometry.electronOrbitDivisor", 8, 2, 64,
            "Electron orbit divisor", "N/divisor (floored at 3) sets each electron marker's distance from the nucleus along z.");
        const int tet_divisor = seed::integer("geometry.tetrahedronDivisor", 12, 2, 64,
            "Tetrahedron radius divisor", "N/divisor (floored at 2) sets the tetrahedral nucleon-center circumradius.");
        const int quark_divisor = seed::integer("geometry.quarkTriadDivisor", 16, 2, 64,
            "Quark triad radius divisor", "N/divisor (floored at 1) sets each nucleon's 3-quark triad circumradius.");
        const double electron_dress_mult = seed::real("constituent0.dressAmplitudeMultiplier", 0.8, 0.0, 5.0,
            "Electron dressing amplitude multiplier", "Multiplies K_B to set both electron markers' dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const double nucleonRotationOffset = seed::real("geometry.nucleonRotationOffset", 0.0, -PI, PI,
            "Nucleon triad rotation offset", "Additive angle rotating each nucleon's 3 quark vertices about its own center; 0 reproduces the original fixed 120°-spaced layout.", "radians", 0.01);
        const double nucleonDressSigma = seed::real("constituent1.dressSigma", 2.0, 0.1, 10.0,
            "Nucleon quark dressing sigma", "Gaussian falloff width of each nucleon's quark dressing (shared by symmetry).", "lattice units", 0.1);
        const double nucleonDressAmplitudeMultiplier = seed::real("constituent1.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Nucleon quark dressing amplitude multiplier", "Multiplies K_B to set each nucleon's quark dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const int oR = std::max(3, N / outer_divisor);
        const int nR = std::max(2, N / tet_divisor);
        const int bR = std::max(1, N / quark_divisor);
        const int tet[4][3] = {
            { +nR, +nR, +nR },   // proton 1
            { -nR, -nR, +nR },   // proton 2
            { +nR, -nR, -nR },   // neutron 1
            { -nR, +nR, -nR },   // neutron 2
        };
        const int pCharges[3] = { +1, +1, -1 };
        const int nCharges[3] = { +1, -1, -1 };
        const int colors[3]   = { 1, 2, 3 };
        // B4 (2026-07-27): place all 4 nucleon triads + both electrons before
        // dressing any of them -- see s0-seed-hydrogen above for why.
        TriPositions nucleons[4];
        int nucleonCz[4];
        const int* nucleonCharges[4];
        for (int i = 0; i < 4; ++i) {
            const int* charges = (i < 2) ? pCharges : nCharges;
            nucleonCharges[i] = charges;
            nucleonCz[i] = c.iz + tet[i][2];
            nucleons[i] = tri_place(rb, c.ix + tet[i][0], c.iy + tet[i][1], c.iz + tet[i][2],
                                    charges, colors, bR, true, nucleonRotationOffset);
        }
        dp_place(rb, c.ix, c.iy, c.iz + oR, -1, +1, 0, false);
        dp_place(rb, c.ix, c.iy, c.iz - oR, -1, -1, 0, false);
        for (int i = 0; i < 4; ++i) {
            tri_dress(rb, nucleons[i], nucleonCz[i], nucleonCharges[i],
                      nucleonDressSigma, K_B * nucleonDressAmplitudeMultiplier);
        }
        dp_dress(rb, c.ix, c.iy, c.iz + oR, -1, 2, K_B * electron_dress_mult);
        dp_dress(rb, c.ix, c.iy, c.iz - oR, -1, 2, K_B * electron_dress_mult);
    }
    else if (name == "s0-seed-h2-bond-formation") {
        // Scenario ID: s0-seed-h2-bond-formation
        // Legacy label: dynamic H2 bond formation.
        // Qualification target: two prepared locked nuclei plus two central
        // mobile negative markers under Poisson-Coulomb force and movement.
        // No bond is assumed by the initial placement.
        configure_prepared_coulomb_candidate_terms(rb);
        const int bond_divisor = seed::integer("geometry.bondDistanceDivisor", 6, 2, 64,
            "Bond distance divisor", "N/divisor (floored at 4) sets the total nucleus-to-nucleus separation.");
        const int nucleus_divisor = seed::integer("geometry.nucleusRadiusDivisor", 16, 2, 64,
            "Nucleus triad radius divisor", "N/divisor (floored at 1) sets each 3-quark triad's circumradius.");
        const double placement_fraction = seed::real("geometry.nucleusPlacementFraction", 0.7, 0.0, 1.0,
            "Nucleus placement fraction", "Fraction of the half-bond-distance used to randomize each nucleus's x offset from center.", "fraction", 0.01);
        const double marker_dress_mult = seed::real("constituent0.dressAmplitudeMultiplier", 0.8, 0.0, 5.0,
            "Mobile marker dressing amplitude multiplier", "Multiplies K_B to set both central mobile markers' dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const double nucleusRotationOffset = seed::real("geometry.nucleusRotationOffset", 0.0, -PI, PI,
            "Nucleus triad rotation offset", "Additive angle rotating each nucleus's 3 quark vertices about its own center; 0 reproduces the original fixed 120°-spaced layout.", "radians", 0.01);
        const double nucleusDressSigma = seed::real("constituent1.dressSigma", 2.0, 0.1, 10.0,
            "Nucleus quark dressing sigma", "Gaussian falloff width of each nucleus's quark dressing (shared by symmetry).", "lattice units", 0.1);
        const double nucleusDressAmplitudeMultiplier = seed::real("constituent1.dressAmplitudeMultiplier", 0.5, 0.0, 5.0,
            "Nucleus quark dressing amplitude multiplier", "Multiplies K_B to set each nucleus's quark dressing amplitude (shared by symmetry).", "×K_B", 0.05);
        const int bd = std::max(4, N / bond_divisor);
        const int hf = bd / 2;
        const int bR = std::max(1, N / nucleus_divisor);
        const int charges[3] = {+1, +1, -1};
        const int colors[3]  = {1, 2, 3};
        // B4 (2026-07-27): place both nuclei + both mobile markers before
        // dressing any of them -- see s0-seed-hydrogen above for why.
        const TriPositions nucleusL = tri_place(rb, c.ix - RND(hf * placement_fraction), c.iy, c.iz,
                                                charges, colors, bR, true, nucleusRotationOffset);
        const TriPositions nucleusR = tri_place(rb, c.ix + RND(hf * placement_fraction), c.iy, c.iz,
                                                charges, colors, bR, true, nucleusRotationOffset);
        dp_place(rb, c.ix, c.iy, c.iz + 1, -1, -1, 0, false);
        dp_place(rb, c.ix, c.iy, c.iz - 1, -1, +1, 0, false);
        tri_dress(rb, nucleusL, c.iz, charges, nucleusDressSigma, K_B * nucleusDressAmplitudeMultiplier);
        tri_dress(rb, nucleusR, c.iz, charges, nucleusDressSigma, K_B * nucleusDressAmplitudeMultiplier);
        dp_dress(rb, c.ix, c.iy, c.iz + 1, -1, 2, K_B * marker_dress_mult);
        dp_dress(rb, c.ix, c.iy, c.iz - 1, -1, 2, K_B * marker_dress_mult);
    }
    else if (name == "s0-seed-spark-of-life") {
        // Scenario ID: s0-seed-spark-of-life
        // Qualification: prepared locked ring, mobile polarity pairs, unlocked
        // triad, central super-threshold seed, and two sub-threshold pockets
        // under the selected patterned genesis-response stack. There is no
        // chemistry, metabolism, heredity, replication, or autocatalysis rule.
        configure_patterned_genesis_response_terms(rb);
        const int ring_divisor = seed::integer("geometry.ringDivisor", 8, 2, 64,
            "Ring radius divisor", "N/divisor (floored at 5) sets the locked precursor ring's radius.");
        const int ring_sites = seed::integer("geometry.ringSiteCount", 16, 4, 64,
            "Ring site count", "Number of alternating-polarity markers placed around the locked ring.");
        const int precursor_divisor = seed::integer("geometry.precursorDivisor", 4, 1, 64,
            "Precursor ring divisor", "N/divisor sets the incoming precursor pairs' radius (floored at ringR+4).");
        const double precursor_speed_mult = seed::real("source.precursorSpeedMultiplier", 0.12, 0.0, 1.0,
            "Precursor inward speed multiplier", "Multiplies C_SPEED to set the precursor pairs' inward velocity magnitude.", "×C_SPEED", 0.01);
        const double precursor_sigma = seed::real("source.precursorDressSigma", 1.6, 0.1, 10.0,
            "Precursor dressing sigma", "Gaussian falloff width of each precursor marker's dressing.", "lattice units", 0.1);
        const double precursor_dress_mult = seed::real("source.precursorDressAmplitudeMultiplier", 0.7, 0.0, 5.0,
            "Precursor dressing amplitude multiplier", "Multiplies K_B to set each precursor marker's dressing amplitude.", "×K_B", 0.05);
        const int catalyst_divisor = seed::integer("geometry.catalystDivisor", 18, 2, 64,
            "Central catalyst triad radius divisor", "N/divisor (floored at 2) sets the unlocked central triad's circumradius.");
        const double spark_mult = seed::real("source.sparkAmplitudeMultiplier", 6.0, 0.0, 100.0,
            "Central spark amplitude multiplier", "Multiplies K_GENESIS; the 6-axis decomposition keeps the summed vector-budget scale equal to this value times K_GENESIS.", "×K_GENESIS", 0.5);
        const int daughter_divisor = seed::integer("geometry.daughterPocketDivisor", 6, 2, 64,
            "Daughter pocket divisor", "N/divisor sets the two daughter pockets' distance from center (floored at ringR+2).");
        const int daughter_radius = seed::integer("source.daughterPocketRadius", 4, 1, 20,
            "Daughter pocket radius", "Integer voxel radius of each sub-threshold daughter pocket.", "voxels");
        const double daughter_sigma = seed::real("source.daughterPocketSigma", 2.0, 0.1, 20.0,
            "Daughter pocket sigma", "Gaussian falloff width of each daughter pocket's dressing.", "lattice units", 0.1);
        const double daughter_mult = seed::real("source.daughterPocketAmplitudeMultiplier", 0.75, 0.0, 1.0,
            "Daughter pocket amplitude multiplier", "Multiplies K_GENESIS; the value is kept below 1.0 so each pocket stays sub-threshold at setup.", "×K_GENESIS", 0.01);
        const int daughter_z_offset = seed::integer("source.daughterPocketZOffset", 2, 0, 20,
            "Daughter pocket z offset", "Voxel offset of each daughter pocket's plane from the ring's z-plane (one pocket +offset, the other -offset).", "voxels");
        const SourceCenter c = read_source_center(N, mc);

        const int ringR = std::max(5, N / ring_divisor);
        const int ringSites = ring_sites;
        for (int i = 0; i < ringSites; ++i) {
            const double angle = (2.0 * PI * i) / ringSites;
            const int px = RND(c.x + ringR * std::cos(angle));
            const int py = RND(c.y + ringR * std::sin(angle));
            const int state = (i % 2 == 0) ? +1 : -1;
            IPF(rb, px, py, c.iz, state, state, 0);
            LOCK(rb, px, py, c.iz);
        }

        // Four incoming precursor charge pairs, each dressed with
        // sub-threshold flux and nudged inward toward the pore.
        const int precursorR = std::max(ringR + 4, N / precursor_divisor);
        const double precursorSpeed = precursor_speed_mult * C_SPEED;
        for (int k = 0; k < 4; ++k) {
            const double angle = (2.0 * PI * k) / 4.0;
            const double dirX = std::cos(angle);
            const double dirY = std::sin(angle);
            const double tanX = -dirY;
            const double tanY = dirX;
            for (int j = 0; j < 2; ++j) {
                const int side = (j == 0) ? -1 : +1;
                const int state = (j == 0) ? +1 : -1;
                const int px = RND(c.x + precursorR * dirX + side * tanX);
                const int py = RND(c.y + precursorR * dirY + side * tanY);
                dp(rb, px, py, c.iz, state, state, ((k + j) % 3) + 1,
                   precursor_sigma, K_B * precursor_dress_mult, false);
                SET_VEL(rb, px, py, c.iz, -dirX * precursorSpeed,
                        -dirY * precursorSpeed, 0);
            }
        }

        // Central unlocked catalytic triad: color-labelled, not locked, so
        // the ordinary Scale-0 dynamics own its fate.
        const int triR = std::max(2, N / catalyst_divisor);
        const int catalystCharges[3] = {+1, -1, +1};
        const int catalystColors[3] = {1, 2, 3};
        tri(rb, c.ix, c.iy, c.iz, catalystCharges, catalystColors, triR, false);

        // Six-axis deterministic spark. Per-axis amplitude is chosen so
        // the vector-budget scale is spark_mult*K_GENESIS.
        const double spark = spark_mult * K_GENESIS / std::sqrt(6.0);
        IF(rb, c.ix + 1, c.iy, c.iz, +spark, 0, 0);
        IF(rb, c.ix - 1, c.iy, c.iz, -spark, 0, 0);
        IF(rb, c.ix, c.iy + 1, c.iz, 0, +spark, 0);
        IF(rb, c.ix, c.iy - 1, c.iz, 0, -spark, 0);
        IF(rb, c.ix, c.iy, c.iz + 1, 0, 0, +spark);
        IF(rb, c.ix, c.iy, c.iz - 1, 0, 0, -spark);

        // Two daughter pockets stay below K_GENESIS at setup; if they
        // bloom, it is because dynamics feed them.
        auto daughter_pocket = [&](int cx, int cy, int cz, int sign,
                                   int radius, double sigma, double amp) {
            for (int dz = -radius; dz <= radius; ++dz)
            for (int dy = -radius; dy <= radius; ++dy)
            for (int dx = -radius; dx <= radius; ++dx) {
                const int x = cx + dx;
                const int y = cy + dy;
                const int z = cz + dz;
                if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N) continue;
                const int r2i = dx*dx + dy*dy + dz*dz;
                if (r2i == 0 || r2i > radius * radius) continue;
                const double r2 = static_cast<double>(r2i);
                const double r = std::sqrt(r2);
                const double val = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                if (val < 0.001) continue;
                IF(rb, x, y, z,
                   sign * val * dx / r,
                   sign * val * dy / r,
                   sign * val * dz / r);
            }
        };
        const int daughterR = std::max(ringR + 2, N / daughter_divisor);
        const double daughterAmp = daughter_mult * K_GENESIS;
        daughter_pocket(c.ix - daughterR, c.iy, c.iz + daughter_z_offset, +1, daughter_radius, daughter_sigma, daughterAmp);
        daughter_pocket(c.ix + daughterR, c.iy, c.iz - daughter_z_offset, -1, daughter_radius, daughter_sigma, daughterAmp);
    }
    // ── Legacy quark-labelled wave-template cohort ──
    else if (name == "s0-seed-up-quark" || name == "s0-seed-down-quark" ||
             name == "s0-seed-strange-quark" || name == "s0-seed-charm-quark" ||
             name == "s0-seed-bottom-quark" || name == "s0-seed-top-quark" ||
             name == "s0-seed-anti-up-quark" || name == "s0-seed-anti-down-quark" ||
             name == "s0-seed-anti-strange-quark" || name == "s0-seed-anti-charm-quark" ||
             name == "s0-seed-anti-bottom-quark" || name == "s0-seed-anti-top-quark") {
        // Six selected polarity/color labels and amplitude multipliers share
        // one geometry. The selected metadata do not couple to the isolated
        // wave map, so no flavor, fractional charge, mass, or quark identity
        // is represented. The cohort qualifies only amplitude scaling and
        // source-free wave-invariant conservation. The 6 "anti-*" ids are the
        // charge-sign mirror of their particle counterpart (same color,
        // same ampBoost, flipped charge) — reusing the same color label is
        // consistent with FTD's color field being a non-dynamical display
        // label that does not couple to any operator, not a claim about
        // anticolor physics.
        configure_free_wave_terms(rb, false);
        int charge, color;
        double ampBoostDefault;
        if      (name == "s0-seed-up-quark")           { charge = +1; color = 1; ampBoostDefault = 0.5; }
        else if (name == "s0-seed-down-quark")         { charge = -1; color = 2; ampBoostDefault = 0.5; }
        else if (name == "s0-seed-strange-quark")      { charge = -1; color = 3; ampBoostDefault = 0.7; }
        else if (name == "s0-seed-charm-quark")        { charge = +1; color = 1; ampBoostDefault = 1.0; }
        else if (name == "s0-seed-bottom-quark")       { charge = -1; color = 2; ampBoostDefault = 1.4; }
        else if (name == "s0-seed-top-quark")          { charge = +1; color = 3; ampBoostDefault = 2.5; }
        else if (name == "s0-seed-anti-up-quark")      { charge = -1; color = 1; ampBoostDefault = 0.5; }
        else if (name == "s0-seed-anti-down-quark")    { charge = +1; color = 2; ampBoostDefault = 0.5; }
        else if (name == "s0-seed-anti-strange-quark") { charge = +1; color = 3; ampBoostDefault = 0.7; }
        else if (name == "s0-seed-anti-charm-quark")   { charge = -1; color = 1; ampBoostDefault = 1.0; }
        else if (name == "s0-seed-anti-bottom-quark")  { charge = +1; color = 2; ampBoostDefault = 1.4; }
        else                                            { charge = -1; color = 3; ampBoostDefault = 2.5; }
        const double ampBoost = seed::real("source.amplitudeBoost", ampBoostDefault, 0.0, 10.0,
            "Wave-packet amplitude boost", "Multiplies K_B to set this cohort member's dressing amplitude; this is the selected per-label scale, not a flavor-dependent mass.", "×K_B", 0.05);
        const double qSig = seed::real("packet.sigma", 1.5, 0.1, 10.0,
            "Wave-packet sigma", "Gaussian falloff width of the isolated wave packet's dressing.", "lattice units", 0.1);
        const int qR = seed::integer("packet.radius", 4, 1, 20,
            "Wave-packet radius", "Integer voxel radius of the isolated wave packet's dressing.", "voxels");
        const double axis_bias_amount = seed::real("source.colorAxisBias", 0.5, 0.0, 5.0,
            "Color axis bias", "Additive bias applied to the dressing gradient along the axis selected by this marker's color label; the label does not couple to any operator.", "lattice units", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        IPF(rb, c.ix, c.iy, c.iz, charge, (charge > 0) ? +1 : -1, color);
        const double qAmp = K_B * ampBoost;
        for (int dz = -qR; dz <= qR; dz++) for (int dy = -qR; dy <= qR; dy++) for (int dx = -qR; dx <= qR; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > qR * qR) continue;
            double r = std::sqrt(double(r2));
            double g = qAmp * std::exp(-r2 / (2.0 * qSig * qSig));
            if (g < 1e-3) continue;
            int sign = (charge > 0) ? 1 : -1;
            double axisBias[3] = {0, 0, 0};
            axisBias[color - 1] = axis_bias_amount;
            IF(rb, c.ix + dx, c.iy + dy, c.iz + dz,
               sign * g * (dx / r + axisBias[0]),
               sign * g * (dy / r + axisBias[1]),
               sign * g * (dz / r + axisBias[2]));
        }
    }
    // ── Legacy Higgs/gluon-labelled vector templates ──
    // Audit-4 2026-04-28: s0-seed-higgs-boson removed (mirror of s0-vacuum-higgs).
    else if (name == "s0-seed-higgs-field") {
        // Scenario ID: s0-seed-higgs-field
        // Qualification: deterministic volume-filling three-vector background
        // under the source-free wave map. It has no scalar degree of freedom,
        // potential, symmetry breaking, or VEV observable.
        configure_free_wave_terms(rb, false);
        const double vev_mult = seed::real("source.vevAmplitudeMultiplier", 0.3, 0.0, 5.0,
            "Background amplitude multiplier", "Multiplies K_B to set the uniform volume-filling background level; there is no VEV or potential behind this label.", "×K_B", 0.01);
        const double noise_mult = seed::real("source.noiseAmplitudeMultiplier", 0.05, 0.0, 5.0,
            "Background noise multiplier", "Multiplies K_B to set the amplitude of the deterministic spatial sinusoid superposed on the background.", "×K_B", 0.01);
        const double vevAmp = K_B * vev_mult;
        const double noise  = K_B * noise_mult;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double sx = std::sin(0.19*x + 0.23*y + 0.29*z);
            double sy = std::sin(0.37*x + 0.13*y + 0.17*z);
            double sz = std::sin(0.11*x + 0.31*y + 0.41*z);
            IF(rb, x, y, z, vevAmp + noise*sx, vevAmp + noise*sy, vevAmp + noise*sz);
        }
    }
    // Audit-4 2026-04-28: s0-seed-{w-boson, z-boson} removed —
    // mirrors of s0-vacuum-{w-boson, z-boson} which are now canonical.
    else if (name == "s0-seed-gluon") {
        // Scenario ID: s0-seed-gluon
        // Qualification: isolated mixed-polarization native vector packet.
        // No color substrate, gauge connection, self-coupling, or gluon
        // observable is enabled, so the legacy gluon identity is absent.
        configure_free_wave_terms(rb, false);
        const int sigma = seed::integer("packet.sigma", 3, 1, 20,
            "Packet sigma", "Gaussian falloff width (in voxels) of the mixed-polarization packet.", "voxels");
        const double amp_mult = seed::real("source.amplitudeMultiplier", 2.0, 0.0, 10.0,
            "Packet amplitude multiplier", "Multiplies K_B to set the packet's peak amplitude.", "×K_B", 0.05);
        const int start_divisor = seed::integer("geometry.startDivisor", 4, 2, 64,
            "Packet start divisor", "N/divisor (floored at 4) sets the packet center's x position.");
        const int half_width = seed::integer("geometry.halfWidth", 8, 1, 32,
            "Packet x half-width", "Voxel half-width of the packet's x extent.", "voxels");
        const double transverseY = seed::real("source.y", midF, 0, N - 1,
            "Transverse center y", "Lattice y-coordinate of the packet's transverse falloff center.", "lattice units", 1.0);
        const double transverseZ = seed::real("source.z", midF, 0, N - 1,
            "Transverse center z", "Lattice z-coordinate of the packet's transverse falloff center.", "lattice units", 1.0);
        const double gAmp = K_B * amp_mult;
        const int startX = std::max(4, N / start_divisor);
        const int halfR = half_width;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int dx = -halfR; dx <= halfR; dx++) {
            int x = startX + dx;
            if (x < 0 || x >= N) continue;
            double dy = y - transverseY, dz = z - transverseZ;
            double gg = gAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (gg < 1e-6) continue;
            IF(rb, x, y, z, 0, gg, 0);
            IW(rb, x, y, z, gg, 0, 0);
        }
        // P5: this seed puts J in y but W in x -- a component mismatch that left
        // Sum W_x = 432.67, i.e. a permanent uniform E ramp that outgrew the
        // seeded y-channel 20:1 in |J| within five ticks. Project out k=0.
        remove_wave_mean(rb);
    }
    // ── Process demos ──
    else if (name == "s0-seed-beta-decay") {
        // Scenario ID: s0-seed-beta-decay
        // Qualification: a prepared 3-site polarity cohort, a separate
        // negative marker, and a neutral vector packet. The alleged electron
        // and neutrino products are present at t=0, so this is not a decay
        // derivation. Only the selected weak polarity-flip rule is enabled.
        configure_weak_transmutation_probe_terms(rb);
        const int triplet_divisor = seed::integer("geometry.tripletRingDivisor", 10, 2, 64,
            "Triplet ring divisor", "N/divisor (floored at 2) sets the 3-site polarity cohort's ring radius.");
        const int lepton_divisor = seed::integer("geometry.leptonOffsetDivisor", 5, 2, 64,
            "Lepton offset divisor", "N/divisor (floored at 4) sets the separate negative marker's distance from center.");
        const double neutrino_sigma = seed::real("constituent0.neutrinoPacketSigma", 2.0, 0.1, 10.0,
            "Neutrino packet sigma", "Gaussian falloff width of the neutral vector packet.", "lattice units", 0.1);
        const int neutrino_radius = seed::integer("constituent0.neutrinoPacketRadius", 4, 1, 20,
            "Neutrino packet radius", "Integer voxel radius of the neutral vector packet.", "voxels");
        const double neutrino_amp_mult = seed::real("constituent0.neutrinoAmplitudeMultiplier", 0.3, 0.0, 5.0,
            "Neutrino packet amplitude multiplier", "Multiplies K_B to set the neutral vector packet's peak amplitude.", "×K_B", 0.01);
        const SourceCenter c = read_source_center(N, mc);
        const int bdR = std::max(2, N / triplet_divisor);
        for (int k = 0; k < 3; k++) {
            double ang = (2.0 * PI * k) / 3.0;
            int bx = RND(c.x + bdR * std::cos(ang));
            int by = RND(c.y + bdR * std::sin(ang));
            int charge = (k == 0) ? +1 : -1;
            IP(rb, bx, by, c.iz, charge);
        }
        const int leptonR = std::max(4, N / lepton_divisor);
        IP(rb, c.ix, c.iy, c.iz + leptonR, -1);
        const int nuR = neutrino_radius;
        for (int dz2 = -nuR; dz2 <= nuR; dz2++) for (int dy2 = -nuR; dy2 <= nuR; dy2++) for (int dx2 = -nuR; dx2 <= nuR; dx2++) {
            int r22 = dx2*dx2 + dy2*dy2 + dz2*dz2;
            if (r22 > nuR * nuR) continue;
            double g = K_B * neutrino_amp_mult * std::exp(-r22 / (2.0 * neutrino_sigma * neutrino_sigma));
            if (g < 1e-3) continue;
            IF(rb, c.ix+dx2, c.iy-leptonR+dy2, c.iz+dz2, g*0.55, g*0.45, 0);
            IW(rb, c.ix+dx2, c.iy-leptonR+dy2, c.iz+dz2, g*0.55, g*0.45, 0);
        }
    }
    else if (name == "s0-seed-ee-annihilation") {
        // Scenario ID: s0-seed-ee-annihilation
        // Qualification: long-baseline opposite-polarity collision under the
        // production movement rule only. The initial radial dressing is held
        // static. Collision removes the two states and redistributes that
        // pre-existing field; it creates no rest-mass radiation or photons.
        configure_annihilation_terms(rb);
        const int separation_divisor = seed::integer("geometry.separationDivisor", 3, 2, 64,
            "Separation divisor", "N/divisor (floored at 6) sets the total marker-to-marker separation.");
        const double speed_mult = seed::real("source.speedMultiplier", 0.3, 0.0, 1.0,
            "Approach speed multiplier", "Multiplies C_SPEED to set each marker's inward speed.", "×C_SPEED", 0.01);
        const double dress_sigma = seed::real("constituent0.dressSigma", 2.0, 0.1, 10.0,
            "Marker dressing sigma", "Gaussian falloff width of each marker's radial dressing.", "lattice units", 0.1);
        const int dress_radius = seed::integer("constituent0.dressRadius", 4, 1, 20,
            "Marker dressing radius", "Integer voxel radius of each marker's radial dressing.", "voxels");
        const double dress_amp_mult = seed::real("constituent0.dressAmplitudeMultiplier", 1.0, 0.0, 5.0,
            "Marker dressing amplitude multiplier", "Multiplies K_B to set each marker's dressing amplitude.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const int aSep = std::max(6, N / separation_divisor);
        const int half = aSep / 2;
        const double speed = speed_mult * C_SPEED;
        IP(rb, c.ix - half, c.iy, c.iz, -1);
        SET_VEL(rb, c.ix - half, c.iy, c.iz, +speed, 0, 0);
        IP(rb, c.ix + half, c.iy, c.iz, +1);
        SET_VEL(rb, c.ix + half, c.iy, c.iz, -speed, 0, 0);
        const double aSig = dress_sigma;
        const int aR = dress_radius;
        for (int pass = 0; pass < 2; pass++) {
            int cx = (pass == 0) ? c.ix - half : c.ix + half;
            int sign = (pass == 0) ? -1 : +1;
            for (int dz2 = -aR; dz2 <= aR; dz2++) for (int dy2 = -aR; dy2 <= aR; dy2++) for (int dx2 = -aR; dx2 <= aR; dx2++) {
                int r2 = dx2*dx2 + dy2*dy2 + dz2*dz2;
                if (r2 == 0 || r2 > aR * aR) continue;
                double r = std::sqrt(double(r2));
                double g = K_B * dress_amp_mult * std::exp(-r2 / (2.0 * aSig * aSig));
                if (g < 1e-3) continue;
                IF(rb, cx+dx2, c.iy+dy2, c.iz+dz2, sign*g*dx2/r, sign*g*dy2/r, sign*g*dz2/r);
            }
        }
    }
    else if (name == "s0-seed-quark-gluon-plasma") {
        // Scenario ID: s0-seed-quark-gluon-plasma
        // Qualification: eight alternating-polarity/color-labelled markers
        // freely transported through a fixed-seed T=0.02 Langevin vector bath.
        // Color force and confinement are off, so this tests neither QCD nor
        // deconfinement and the labels do not alter the dynamics.
        // bath_t/bath_gamma below are NOT scenario-local ftd::seed bindings, but
        // they are still overridable: dispatch_scenario_seed() (scenarios.cpp)
        // re-reads rb.toggles.langevin_T/langevin_gamma as protocol.bathVariance/
        // protocol.bathDamping after every scenario body runs, so a later global
        // override always wins over these local defaults. Do not add a
        // scenario-local "source.bathTemperature"-style override here — it would
        // be silently clobbered by that global re-read.
        const double bath_t = 0.02;
        const double bath_gamma = 0.05;
        configure_thermal_transport_terms(rb, bath_t, bath_gamma);
        const int marker_offset = seed::integer("geometry.markerOffset", 2, 1, 20,
            "Marker offset", "Voxel offset of each of the 8 markers from center along each axis.", "voxels");
        const double marker_speed_mult = seed::real("source.markerSpeedMultiplier", 0.5, 0.0, 1.0,
            "Marker initial speed multiplier", "Multiplies C_SPEED to set each marker's fixed-seed isotropic initial speed.", "×C_SPEED", 0.01);
        const int pulse_radius = seed::integer("source.pulseRadius", 4, 1, 20,
            "Bath pulse radius", "Integer voxel radius of the fixed-seed random vector-bath ball at center.", "voxels");
        const double pulse_amp_mult = seed::real("source.pulseAmplitudeMultiplier", 3.0, 0.0, 20.0,
            "Bath pulse amplitude multiplier", "Multiplies K_B to set the maximum magnitude drawn per voxel by the fixed-seed random pulse; the per-voxel draw itself is unchanged.", "×K_B", 0.1);
        const SourceCenter c = read_source_center(N, mc);
        const int qOffset = marker_offset;
        const int dirs[2] = {-qOffset, qOffset};
        int quarkIndex = 0;
        for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++) {
            int dx = dirs[i], dy = dirs[j], dz = dirs[k];
            const int charge = (quarkIndex % 2 == 0) ? +1 : -1;
            const int color = (quarkIndex % 3) + 1; // R=1, G=2, B=3
            IPF(rb, c.ix + dx, c.iy + dy, c.iz + dz, charge, (charge > 0) ? +1 : -1, color);

            // Fixed-seed isotropic initial velocity, speed = marker_speed_mult*C_SPEED.
            const double theta = urand() * 2.0 * PI;
            const double phi = std::acos(urand() * 2.0 - 1.0);
            const double speed = marker_speed_mult * C_SPEED;
            SET_VEL(rb, c.ix + dx, c.iy + dy, c.iz + dz,
                    speed * std::sin(phi) * std::cos(theta),
                    speed * std::sin(phi) * std::sin(theta),
                    speed * std::cos(phi));

            quarkIndex++;
        }

        // Fixed-seed random vector-bath initial data in the central ball.
        const int pulseR = pulse_radius;
        for (int dz = -pulseR; dz <= pulseR; dz++)
        for (int dy = -pulseR; dy <= pulseR; dy++)
        for (int dx = -pulseR; dx <= pulseR; dx++) {
            const int r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > pulseR * pulseR) continue;

            const double amp = K_B * pulse_amp_mult * urand();
            const double theta = urand() * 2.0 * PI;
            const double phi = std::acos(urand() * 2.0 - 1.0);

            const double jx = amp * std::sin(phi) * std::cos(theta);
            const double jy = amp * std::sin(phi) * std::sin(theta);
            const double jz = amp * std::cos(phi);

            const double wx = amp * std::sin(phi) * std::cos(theta) * C_SPEED;
            const double wy = amp * std::sin(phi) * std::sin(theta) * C_SPEED;
            const double wz = amp * std::cos(phi) * C_SPEED;

            IF(rb, c.ix + dx, c.iy + dy, c.iz + dz, jx, jy, jz);
            IW(rb, c.ix + dx, c.iy + dy, c.iz + dz, wx, wy, wz);
        }
    }
    else if (name == "s0-seed-gravitational-lensing") {
        // Scenario ID: s0-seed-gravitational-lensing
        // Physical Purpose: Tests the native gravitational optical channel around a latency well.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: A valid transverse packet passes the well; the frozen native operator predicts no bending.
        // Verification: structural-null observatory, not a demonstration of gravitational lensing.
        // Isolated linear-wave null test.  With no gravity-to-wave vertex the
        // radial background and packet must evolve by exact superposition.
        configure_free_wave_terms(rb, false);
        const double mass_radius = seed::real("source.centralMassRadius", 3.0, 0.5, 20.0,
            "Central mass radius scale", "Multiplies K_B in the imposed 1/r² background mg = G_N*(K_B*rs)/r²; larger values deepen the background well.", "lattice units", 0.1);
        const int offset_divisor = seed::integer("geometry.packetOffsetDivisor", 6, 2, 64,
            "Packet transverse offset divisor", "N/divisor (floored at 4) sets the packet's transverse (y) offset from the well.");
        const int sigma_divisor = seed::integer("geometry.packetSigmaDivisor", 12, 2, 64,
            "Packet sigma divisor", "N/divisor (floored at 2) sets the packet's spatial width sigma.");
        const double amp_mult = seed::real("packet.amplitudeMultiplier", 0.5, 0.0, 5.0,
            "Packet amplitude multiplier", "Multiplies K_B to set the injected packet's peak amplitude.", "×K_B", 0.05);
        const int launch_divisor = seed::integer("geometry.packetLaunchDivisor", 4, 2, 64,
            "Packet launch-position divisor", "N/divisor sets the off-axis photon pulse's starting x position.");
        const SourceCenter c = read_source_center(N, midF);
        // Selected inward inverse-square background at the center:
        const double rs = mass_radius;
        IP(rb, c.ix, c.iy, c.iz, +1);
        for (int z = 0; z < N; z++)
        for (int y = 0; y < N; y++)
        for (int x = 0; x < N; x++) {
            const double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            const double r = std::max(std::sqrt(rx * rx + ry * ry + rz * rz), 0.5);
            const double mg = G_N * (K_B * rs) / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }

        // Off-axis photon pulse launched at x0 = N/launch_divisor, propagating in +x:
        const int x0 = N / launch_divisor;
        const int offset = std::max(4, N / offset_divisor);
        const int y0 = c.iy + offset;
        const int z0 = c.iz;

        const int sigma = std::max(2, N / sigma_divisor);
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, x0, y0, z0, sigma, seed_wave_0_sigmaTransverse, K_B * amp_mult, seed_wave_0_direction, 2.0 * PI / (4.0 * sigma), seed_wave_0_phase);
    }
    // ── Level 6: Gauge / Topological ──
    else if (name == "s0-seed-wilson-loop") {
        // Scenario ID: s0-seed-wilson-loop
        // Physical Purpose: Exact oriented square path in the native vector field.
        // This is initial data only: a Wilson observable would require link
        // holonomy and a traced path product, neither of which is computed here.
        configure_static_seed_terms(rb);
        const int loop_divisor = seed::integer("geometry.loopRadiusDivisor", 8, 2, 64,
            "Loop radius divisor", "N/divisor (floored at 3) sets the square path's half-width.");
        const double amp_mult = seed::real("source.amplitudeMultiplier", 1.0, 0.0, 10.0,
            "Loop amplitude multiplier", "Multiplies K_B to set the flux magnitude along each edge of the square path.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const int R = std::max(3, N / loop_divisor);
        const double wAmp = K_B * amp_mult;
        for (int x = c.ix - R; x <= c.ix + R; x++) IF(rb, x, c.iy - R, c.iz,  wAmp, 0, 0);
        for (int y = c.iy - R; y <= c.iy + R; y++) IF(rb, c.ix + R, y, c.iz, 0,  wAmp, 0);
        for (int x = c.ix + R; x >= c.ix - R; x--) IF(rb, x, c.iy + R, c.iz, -wAmp, 0, 0);
        for (int y = c.iy + R; y >= c.iy - R; y--) IF(rb, c.ix - R, y, c.iz, 0, -wAmp, 0);
    }
    else if (name == "s0-seed-flux-tube") {
        // Scenario ID: s0-seed-flux-tube
        // Physical Purpose: Exact Gaussian axial tube with opposite ternary
        // endpoint markers. Confinement and a q-qbar identity are not implied.
        configure_static_seed_terms(rb);
        const int sep_divisor = seed::integer("geometry.tubeSeparationDivisor", 4, 2, 64,
            "Tube separation divisor", "N/divisor (floored at 6) sets the total endpoint-to-endpoint separation.");
        const double tube_sigma = seed::real("source.tubeSigma", 1.5, 0.1, 10.0,
            "Tube sigma", "Gaussian falloff width of the axial tube's transverse profile.", "lattice units", 0.1);
        const double amp_mult = seed::real("source.amplitudeMultiplier", 1.0, 0.0, 10.0,
            "Tube amplitude multiplier", "Multiplies K_B to set the tube's on-axis flux amplitude.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const int ftSep = std::max(6, N / sep_divisor), ftH = ftSep / 2;
        IP(rb, c.ix - ftH, c.iy, c.iz, +1);
        IP(rb, c.ix + ftH, c.iy, c.iz, -1);
        const double ftSig = tube_sigma;
        const double tubeAmp = K_B * amp_mult;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = c.ix - ftH; x <= c.ix + ftH; x++) {
            double dy2 = y - c.iy, dz2 = z - c.iz;
            double p2 = dy2*dy2 + dz2*dz2;
            double g = tubeAmp * std::exp(-p2 / (2.0 * ftSig * ftSig));
            if (g > 0.001) IF(rb, x, y, z, g, 0, 0);
        }
    }
    else if (name == "s0-seed-monopole") {
        // Scenario ID: s0-seed-monopole
        // Physical Purpose: Imposed radial inverse-square vector profile.
        // It is a monopole-shaped ansatz, not evidence for magnetic charge.
        configure_static_seed_terms(rb);
        const double amp_mult = seed::real("source.amplitudeMultiplier", 1.0, 0.0, 10.0,
            "Radial profile amplitude multiplier", "Multiplies the imposed 1/(4πr²) radial profile.", "unitless", 0.05);
        const SourceCenter c = read_source_center(N, (N - 1) / 2.0);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 1e-12) continue;
            double mg = amp_mult / (4.0 * PI * r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, rx / r * mg, ry / r * mg, rz / r * mg);
        }
    }
    else if (name == "s0-seed-instanton") {
        // Scenario ID: s0-seed-instanton
        // Physical Purpose: Exact localized radial 3-vector profile.
        // This is NOT a Yang-Mills instanton: the engine setup has no Euclidean
        // time component, non-Abelian connection, or measured topological charge.
        configure_static_seed_terms(rb);
        const double instanton_size = seed::real("source.instantonSize", 3.0, 0.5, 20.0,
            "Instanton profile size", "Length scale of the imposed radial profile mg = size/(r²+size²).", "lattice units", 0.1);
        const SourceCenter c = read_source_center(N, (N - 1) / 2.0);
        const double iSize = instanton_size;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            double r2 = rx*rx + ry*ry + rz*rz;
            double r = std::sqrt(r2);
            double mg = iSize / (r2 + iSize * iSize);
            if (mg < 1e-6 || r < 0.5) continue;
            IF(rb, x, y, z, mg * rx / r, mg * ry / r, mg * rz / r);
        }
    }
    // ── Level 7: Gravity / Cosmology ──
    else if (name == "s0-seed-schwarzschild") {
        // Scenario ID: s0-seed-schwarzschild
        // Qualification: exact inert inward inverse-square vector ansatz with
        // one central marker.  It is not a Schwarzschild metric or engine
        // gravity solution and contains no horizon or curvature observable.
        configure_static_seed_terms(rb);
        const double schwarzschild_radius = seed::real("source.schwarzschildRadius", 3.0, 0.5, 20.0,
            "Schwarzschild-shaped radius scale", "Multiplies K_B in the imposed 1/r² background mg = G_N*(K_B*rs)/r²; larger values deepen the background well.", "lattice units", 0.1);
        const SourceCenter c = read_source_center(N, (N - 1) / 2.0);
        const double rs = schwarzschild_radius;
        IP(rb, c.ix, c.iy, c.iz, +1);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 0.5) r = 0.5;
            double mg = G_N * (K_B * rs) / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }
    }
    else if (name == "s0-seed-massive-body") {
        configure_mass_latency_terms(rb);
        // Scenario ID: s0-seed-massive-body
        // Physical Purpose: Seeds a massive body using real manifested mass (locked).
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central dense core of locked mass that sources gravity via the Poisson equation.
        // Discrepancy: None.
        // A dense ball of LOCKED rest mass. Gravity is sourced from REAL manifested
        // gravity charge (rho = M_GRAVITATIONAL*|state|) by latency Poisson (enable
        // latency_field), NOT the |J|^2 field-energy proxy. Locked => static body
        // (skipped by movement + evaporation), so it is a stable gravitational source.
        const int body_divisor = seed::integer("geometry.bodyRadiusDivisor", 16, 2, 64,
            "Body radius divisor", "N/divisor (clamped to [1,2]) sets the locked mass ball's radius.");
        const SourceCenter c = read_source_center(N, (N - 1) / 2.0);
        // Small compact body so the latency well is sub-horizon with a visible
        // 1/r tail (a dense large ball saturates to a black hole — that's the
        // schwarzschild scenario). ~33 voxels at L=33 -> latencyMax ~0.5.
        const int R = std::min(2, std::max(1, N / body_divisor));
        const double R2 = static_cast<double>(R) * R;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            if (rx*rx + ry*ry + rz*rz > R2) continue;
            IP(rb, x, y, z, +1);
            LOCK(rb, x, y, z);
        }
    }
    else if (name == "s0-seed-gravitational-wave") {
        // Scenario ID: s0-seed-gravitational-wave
        // Exact n=4 native transverse wave.  The legacy label supplied no
        // tensor, metric, mass-source, or gravity-specific observable; the
        // qualified scenario therefore closes the gravitational-wave identity.
        configure_free_wave_terms(rb, false);
        const int mode_number = seed::integer("packet.modeNumber", 4, 1, 32,
            "Harmonic mode number", "Integer wavenumber n of the injected plane harmonic (k = 2πn/N).");
        const double amplitude = seed::real("packet.amplitude", 0.1, 0.0, 5.0,
            "Harmonic amplitude", "Peak amplitude of the injected plane harmonic.", "lattice units", 0.01);
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        inject_plane_harmonic_x(rb, mode_number, amplitude, seed_wave_0_direction);
    }
    // ── Time-dilation scenarios (2026-06-07) ──
    // Thin reuse mirrors for the Time Observatory panel. Each reproduces an
    // existing gravity seed so the latency well (gravitational clock-slowdown)
    // is real; no new physics. The latency SAMPLER builds dτ/dt from the |J|²
    // flux field, so these reuse FLUX-producing wells (a locked-rest-mass body
    // has zero flux → no proxy latency). The former JS delegating cases are
    // retained only as historical provenance in the archived Scale-0 mirror.
    else if (name == "s0-seed-time-gravity-well" || name == "s0-seed-time-twin-clocks") {
        // Legacy IDs: s0-seed-time-{gravity-well,twin-clocks}.
        // Qualification: exact aliases of the plain native harmonic.  No well,
        // clock, observer, worldline, latency, or proper-time comparison exists.
        // Exact aliases of the plain n=4 transverse wave above. They contain
        // no clock, observer, gravity source, or proper-time comparison.
        configure_free_wave_terms(rb, false);
        const int mode_number = seed::integer("packet.modeNumber", 4, 1, 32,
            "Harmonic mode number", "Integer wavenumber n of the injected plane harmonic (k = 2πn/N).");
        const double amplitude = seed::real("packet.amplitude", 0.1, 0.0, 5.0,
            "Harmonic amplitude", "Peak amplitude of the injected plane harmonic.", "lattice units", 0.01);
        const int seed_wave_0_direction = seed::choice("wave0.direction", +1, {{-1,"-x"},{1,"+x"}}, "Wave 1 direction", "Changes the travel direction of wave ingredient 1 along x.");
        inject_plane_harmonic_x(rb, mode_number, amplitude, seed_wave_0_direction);
    }
    else if (name == "s0-seed-time-horizon") {
        // Scenario ID: s0-seed-time-horizon
        // Physical Purpose: Models deep time dilation near a black hole horizon.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Strong central mass well showing near-zero dτ/dt dilation at the center.
        // Discrepancy: None.
        // Exact alias of the inert Schwarzschild-shaped ansatz.  There is no
        // latency field, clock, horizon condition, or proper-time observable.
        configure_static_seed_terms(rb);
        const double schwarzschild_radius = seed::real("source.schwarzschildRadius", 3.0, 0.5, 20.0,
            "Schwarzschild-shaped radius scale", "Multiplies K_B in the imposed 1/r² background mg = G_N*(K_B*rs)/r²; larger values deepen the background well.", "lattice units", 0.1);
        const SourceCenter c = read_source_center(N, (N - 1) / 2.0);
        const double rs = schwarzschild_radius;
        IP(rb, c.ix, c.iy, c.iz, +1);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - c.x, ry = y - c.y, rz = z - c.z;
            double r = std::sqrt(rx*rx + ry*ry + rz*rz);
            if (r < 0.5) r = 0.5;
            double mg = G_N * (K_B * rs) / (r * r);
            if (mg < 1e-6) continue;
            IF(rb, x, y, z, -mg * rx / r, -mg * ry / r, -mg * rz / r);
        }
    }
    // ── Level 8: Reference frame context / Observer ──
    else if (name == "s0-seed-sloop") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-sloop
        // Physical Purpose: Seeds a self-referential sLoop ring.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Loop of positive charges carrying angular/circulating flux.
        // Discrepancy: None.
        const int loop_divisor = seed::integer("geometry.loopRadiusDivisor", 8, 2, 64,
            "Loop radius divisor", "N/divisor (floored at 3) sets the ring's radius.");
        const int loop_sites = seed::integer("geometry.loopSiteCount", 12, 3, 64,
            "Loop site count", "Number of positive markers placed around the ring.");
        const double amp_mult = seed::real("source.amplitudeMultiplier", 1.0, 0.0, 10.0,
            "Circulating flux amplitude multiplier", "Multiplies K_B to set each ring marker's tangential flux magnitude.", "×K_B", 0.05);
        const SourceCenter c = read_source_center(N, mc);
        const double phase = seed::real("geometry.rotation", 0.0, -PI, PI, "Ring rotation", "Rotates the marker layout and tangential flux around the ring center.", "radians");
        const int polarity = seed::choice("constituent0.polarity", 1, POLARITY, "Ring polarity", "Changes the stored manifestation of every ring marker.");
        const int slR = std::max(3, N / loop_divisor);
        const int slN = loop_sites;
        const double slA = K_B * amp_mult;
        for (int i = 0; i < slN; i++) {
            double a = phase + 2.0 * PI * i / slN;
            int px = RND(c.x + slR * std::cos(a));
            int py = RND(c.y + slR * std::sin(a));
            IP(rb, px, py, c.iz, polarity);
            IF(rb, px, py, c.iz, -std::sin(a) * slA, std::cos(a) * slA, 0);
        }
    }
    else if (name == "s0-seed-observer-cell") {
        configure_static_seed_terms(rb);
        // Scenario ID: s0-seed-observer-cell
        // Physical Purpose: Seeds an observer cell configuration on a 3^3 lattice.
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: Central -1 charge surrounded by shells of +1, -1, and +1 charges.
        // Discrepancy: None.
        const int centerCharge = seed::choice("geometry.centerPolarity", +1, POLARITY,
            "Center polarity", "Ternary manifestation state of the central voxel.");
        const int octCharge = seed::choice("geometry.octahedronPolarity", -1, POLARITY,
            "Octahedron shell polarity", "Ternary manifestation state of the 6 face-neighbor voxels.");
        const int cubCharge = seed::choice("geometry.cuboctahedronPolarity", +1, POLARITY,
            "Cuboctahedron shell polarity", "Ternary manifestation state of the 12 edge-neighbor voxels.");
        const int stelCharge = seed::choice("geometry.stellaOctangulaPolarity", -1, POLARITY,
            "Stella octangula shell polarity", "Ternary manifestation state of the 8 corner voxels.");
        const SourceCenter c = read_source_center(N, mc);
        IP(rb, c.ix, c.iy, c.iz, centerCharge);
        const int oct[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int i = 0; i < 6; i++) IP(rb, c.ix+oct[i][0], c.iy+oct[i][1], c.iz+oct[i][2], octCharge);
        const int cub[12][3] = {
            {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
            {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
            {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
        };
        for (int i = 0; i < 12; i++) IP(rb, c.ix+cub[i][0], c.iy+cub[i][1], c.iz+cub[i][2], cubCharge);
        const int stel[8][3] = {
            {1,1,1},{1,1,-1},{1,-1,1},{1,-1,-1},
            {-1,1,1},{-1,1,-1},{-1,-1,1},{-1,-1,-1}
        };
        for (int i = 0; i < 8; i++) IP(rb, c.ix+stel[i][0], c.iy+stel[i][1], c.iz+stel[i][2], stelCharge);
    }
    else if (name == "s0-seed-de-broglie-clock") {
        // Scenario ID: s0-seed-de-broglie-clock
        // Physical Purpose: Simulates the De Broglie internal compton clock (FTD-0271).
        // Initial Condition Parameters: see the ftd::seed bindings below.
        // Expected Behaviour: A central manifested block oscillates at the Compton frequency.
        // Discrepancy: None.
        // FTD-0271: de Broglie internal clock (single-particle pilot wave).
        // A central manifested block carries a uniform flux J0. When the
        // de_broglie_clock toggle is ON (the de-broglie-clock-panel enables it),
        // the Klein-Gordon mass term -omega0^2*J makes the block's flux
        // oscillate at the rest-frame Compton frequency omega0 -- the internal
        // clock. [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT]: omega0~K_B is
        // IMPOSED (A0: FTD's native flux is massless); Schrodinger + de Broglie
        // are textbook Klein-Gordon, not an FTD prediction. The block interior
        // is the k=0 rest mode (uniform => Laplacian 0), so the centre voxel
        // oscillates at a clean omega0. genesis/damping OFF so the block
        // persists; the panel drives the clock and reads centre |J|(t).
        // Isolate the selected Klein-Gordon operator.  Previously this branch
        // left default gravity and Poisson terms enabled and the dashboard did
        // not enable the clock toggle at all, so the displayed run was not the
        // experiment described by its label.
        for (const auto& spec : TOGGLE_SPECS) rb.toggles.*(spec.field) = false;
        rb.toggles.wave_propagation     = true;
        rb.toggles.de_broglie_clock     = true;
        rb.toggles.omega0               = seed::real("protocol.omega0", 0.30, 0.0, 1.49,
            "Klein-Gordon rest frequency", "Imposed omega0 in the -omega0^2*J mass term; the default kick-drift integrator requires 0 < omega0 <= 1.49 for stability.", "per tick", 0.001);
        const int half = seed::integer("geometry.blockHalfWidth", 3, 0, 20,
            "Manifested block half-width", "Integer half-width of the central manifested cube (a (2*half+1)^3 block).", "voxels");
        const double J0 = seed::real("source.amplitude", 0.08, 0.0, 5.0,
            "Uniform interior flux", "Uniform x-flux J0 carried by the manifested block; the block's k=0 interior mode oscillates at omega0.", "lattice units", 0.001);
        const SourceCenter c = read_source_center(N, mc);
        const int polarity = seed::choice("constituent0.polarity", 1, POLARITY, "Block polarity", "Changes the stored manifestation of every block marker.");
        for (int dx = -half; dx <= half; ++dx)
            for (int dy = -half; dy <= half; ++dy)
                for (int dz = -half; dz <= half; ++dz) {
                    IP(rb, c.ix + dx, c.iy + dy, c.iz + dz, polarity);
                    IF(rb, c.ix + dx, c.iy + dy, c.iz + dz, J0, 0, 0);
                }
    }
    else if (name == "s0-seed-thermal-ignition") {
        // Scenario ID: s0-seed-thermal-ignition
        // Qualification target: the deterministic finite-volume response of
        // an initially empty lattice to the selected Langevin + genesis stack.
        // T=0.03 is an imposed probe point.  No hot voxel is injected and this
        // single profile does not, by itself, establish ignition, a phase
        // transition, condensation, hysteresis, or a thermodynamic limit.
        const double bath_t = 0.03;
        const double bath_gamma = 0.02;
        configure_genesis_cluster_terms(rb, bath_t, bath_gamma);
    }

    else if (name == "s0-seed-ew-phase-transition") {
        // Scenario ID: s0-seed-ew-phase-transition
        // Qualification: empty-lattice response to a uniform additive +x
        // drive D(t)=(sin(0.01t)+1)*0.025 with genesis enabled. D(t)>=0,
        // so this is not a cyclic sweep and cannot demonstrate hysteresis,
        // electroweak symmetry breaking, or a thermodynamic phase transition.
        configure_uniform_genesis_drive_terms(rb);
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
