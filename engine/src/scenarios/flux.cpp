// ==========================================================================
//  engine/src/scenarios/flux.cpp
//
//  Group: flux-* (22 scenarios)
//  Canonical seed implementation; the former JS mirror is archived.
//
//  Split out of engine/src/scenarios.cpp (ticket S1). Every scenario body
//  is byte-identical to the pre-split source — see _helpers.h for the
//  shared IF/IW/IP/IPF/SET_VEL/LOCK/SET_SPIN/FLR/CEL/RND primitives
//  and docs/scenarios.h for the group-function contract.
//
//  Universal scenario seeding (2026-09-16): every meaningful hardcoded
//  construction literal below is now bound through ftd::seed::{real,
//  integer,choice} (declared in ftd/scenario_seed.h). With no
//  ftd::seed::Context installed, record() returns each default_value
//  unchanged and never touches the RNG, so the legacy no-context dispatch
//  path is byte-identical to the pre-seeding source. Grouping convention:
//    source.*   — absolute placement coordinates of a packet/marker
//    geometry.* — radii, offsets, box half-widths, Gaussian variances,
//                 amplitude cutoffs, counts
//    packet.*   — field amplitudes and component ratios of seeded
//                 flux/wave-velocity vectors
//    kinematics.* — imposed marker velocities
//    field.*    — imposed external field strengths (e.g. cyclotron B_z)
//  Fixed-law constants (K_B, K_GENESIS, K_MANIFEST, C_SPEED) and
//  structural counts that define a scenario's declared identity (exact
//  adjacency in flux-annihilation; exactly-3/6/24 marker layouts) are left
//  hardcoded per FTD-0371/SCOPE_CONSUMPTION_PROGRAM discipline: exposing an
//  amplitude or width is not a physics claim, but redefining a scenario's
//  named structure would be. Integer/real bounds are deliberately widened
//  past each literal's natural N-proportional range (see the per-property
//  std::max(...) floors below) so a default construction never fails
//  Property validation at small lattice sizes.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>
#include <stdexcept>

namespace ftd {

// Bring detail::urand() into scope for the stochastic scenarios below.
using detail::urand;

bool setup_flux_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("flux-", 0) != 0) return false;
    const int    N     = rb.lattice().size();
    const int    mid   = N / 2;
    const double midF  = (N - 1) * 0.5;
    const double sigma = N / 10.0;
    const double amp   = K_B * 2.0;

    if (name == "flux-pulse") {
        // Scenario ID: flux-pulse
        // Physical Purpose: Boundary-response probe for the frozen native wave
        // map. This is a transverse field packet, not a particle or EM claim.
        // Initial Condition: finite discrete-curl packet traveling +x from L/3.
        // Expected Behaviour: periodic propagation, Neumann-shell reflection,
        // or complete face-record excision under Dispersal. The last law has
        // exact storage-level no-return semantics; it is not an all-angle
        // reflection-free radiation theorem.
        configure_free_wave_terms(rb, false);
        const double px0 = seed::real("source.x", N / 3.0, 0.0, N - 1.0,
            "Packet start x", "Lattice x-coordinate where the transverse packet is centered before the first tick.");
        const double py0 = seed::real("source.y", midF, 0.0, N - 1.0,
            "Packet start y", "Lattice y-coordinate of the packet center.");
        const double pz0 = seed::real("source.z", midF, 0.0, N - 1.0,
            "Packet start z", "Lattice z-coordinate of the packet center.");
        const double sx = seed::real("packet.sigma", std::max(3.0, N / 16.0), 1.0, std::max(N * 0.5, 64.0),
            "Packet width", "Gaussian width (lattice sites) of the packet, used both along the direction of travel and transversely.");
        const double pAmp = seed::real("packet.amplitude", K_B * 0.5, 0.0, K_B * 40.0,
            "Packet amplitude", "Peak amplitude of the seeded curl-potential building the divergence-free transverse packet.");
        const int pDir = seed::choice("packet.direction", 1, {{1.0, "+x"}, {-1.0, "-x"}},
            "Propagation direction", "Sign of the packet's seeded travel direction along x.");
        const double pCarrierK = seed::real("packet.carrierK", 2.0 * PI / (4.0 * sx), 0.0, PI,
            "Carrier wavenumber", "Spatial wavenumber of the cosine carrier riding inside the packet envelope; defaults to a value coupled to the width above.");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sx, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, px0, py0, pz0, sx, seed_wave_0_sigmaTransverse, pAmp, pDir, pCarrierK, seed_wave_0_phase);
    }
    else if (name == "flux-dipole") {
        // Scenario ID: flux-dipole
        // Physical Purpose: Antisymmetric pair of Gaussian vector-wave blobs.
        // This tests native parity preservation, not an electric/magnetic dipole.
        configure_free_wave_terms(rb, false);
        const int off = seed::integer("geometry.offset", N / 4, 0, std::max(N, 64),
            "Lobe half-separation", "Half the distance (lattice sites) between the two opposite-polarity Gaussian lobes.");
        const int boxR = seed::integer("geometry.boxRadius", 4, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic region around each lobe center that receives Gaussian dressing.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 9.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2, lattice sites^2) of the Gaussian envelope dressing each lobe.");
        const double baseAmp = seed::real("packet.amplitude", amp, 0.0, amp * 40.0,
            "Lobe peak amplitude", "Peak flux-and-wave amplitude at the center of each Gaussian lobe.");
        const double ratioY = seed::real("packet.componentRatioY", 0.5, -2.0, 2.0,
            "y-to-x component ratio", "Ratio of the y-component to the x-component of the seeded flux/wave vector at each site.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Pair center x", "Lattice x-coordinate about which the two opposite-polarity lobes are symmetrically offset.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Pair center y", "Lattice y-coordinate of both lobes.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Pair center z", "Lattice z-coordinate of both lobes.");
        const int pLx = FLR(cx) - off, pRx = CEL(cx) + off;
        const int yLo = FLR(cy) - boxR, yHi = CEL(cy) + boxR;
        const int zLo = FLR(cz) - boxR, zHi = CEL(cz) + boxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int dx = -boxR; dx <= boxR; dx++) {
            double dy = y - cy, dz = z - cz;
            double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) {
                IF(rb, pLx + dx, y, z,  val,  val * ratioY, 0);
                IW(rb, pLx + dx, y, z,  val,  val * ratioY, 0);
                IF(rb, pRx + dx, y, z, -val, -val * ratioY, 0);
                IW(rb, pRx + dx, y, z, -val, -val * ratioY, 0);
            }
        }
    }
    else if (name == "flux-standing") {
        // Scenario ID: flux-standing
        // Physical Purpose: Reflection-even, zero-initial-momentum broadband
        // wave pair. It is a standing-wave proxy, not a pure eigenmode.
        configure_free_wave_terms(rb, false);
        const int off = seed::integer("geometry.offset", N / 3, 0, std::max(N, 64),
            "Lobe half-separation", "Half the distance (lattice sites) between the two same-polarity Gaussian lobes.");
        const int boxR = seed::integer("geometry.boxRadius", 4, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic region around each lobe center that receives Gaussian dressing.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 9.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2, lattice sites^2) of the Gaussian envelope dressing each lobe.");
        const double baseAmp = seed::real("packet.amplitude", amp, 0.0, amp * 40.0,
            "Lobe peak amplitude", "Peak flux amplitude at the center of each Gaussian lobe.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Pair center x", "Lattice x-coordinate about which the two same-polarity lobes are symmetrically offset.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Pair center y", "Lattice y-coordinate of both lobes.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Pair center z", "Lattice z-coordinate of both lobes.");
        const int pLx = FLR(cx) - off, pRx = CEL(cx) + off;
        const int yLo = FLR(cy) - boxR, yHi = CEL(cy) + boxR;
        const int zLo = FLR(cz) - boxR, zHi = CEL(cz) + boxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int dx = -boxR; dx <= boxR; dx++) {
            double dy = y - cy, dz = z - cz;
            double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) {
                IF(rb, pLx + dx, y, z, val, 0, 0);
                IF(rb, pRx + dx, y, z, val, 0, 0);
            }
        }
    }
    else if (name == "flux-soliton") {
        // Scenario ID: flux-soliton
        // Physical Purpose: Measures dispersion of a high-amplitude localized packet.
        // Initial Condition Parameters: Divergence-free Gaussian packet; genesis disabled.
        // Expected Behaviour: Packet centroid propagates while its width records lattice dispersion.
        // Verification: dispersion diagnostic; the frozen wave sector has no soliton nonlinearity.
        configure_free_wave_terms(rb);
        const double sx0 = seed::real("source.x", midF, 0.0, N - 1.0,
            "Packet start x", "Lattice x-coordinate of the packet center.");
        const double sy0 = seed::real("source.y", midF, 0.0, N - 1.0,
            "Packet start y", "Lattice y-coordinate of the packet center.");
        const double sz0 = seed::real("source.z", midF, 0.0, N - 1.0,
            "Packet start z", "Lattice z-coordinate of the packet center.");
        const double sSigma = seed::real("packet.sigma", 2.0, 0.5, std::max(N * 0.5, 64.0),
            "Packet width", "Gaussian width (lattice sites) used for both the longitudinal and transverse extent of the packet.");
        const double sAmp = seed::real("packet.amplitude", amp * 2.0, 0.0, amp * 40.0,
            "Packet amplitude", "Peak amplitude of the seeded curl-potential packet.");
        const int sDir = seed::choice("packet.direction", 1, {{1.0, "+x"}, {-1.0, "-x"}},
            "Propagation direction", "Sign of the packet's seeded travel direction along x.");
        const double seed_wave_0_carrierK = seed::real("wave0.carrierK", 0.0, 0.0, PI, "Wave 1 carrier wavenumber", "Sets wave 1 carrier wavenumber of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians per cell");
        const double seed_wave_0_phase = seed::real("wave0.phase", 0.0, -PI, PI, "Wave 1 carrier phase", "Sets wave 1 carrier phase of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "radians");
        const double seed_wave_0_sigmaTransverse = seed::real("wave0.sigmaTransverse", sSigma, 1.0, std::max(64.0, double(N)), "Wave 1 transverse width", "Sets wave 1 transverse width of wave ingredient 1; increasing it changes the prepared profile before the first tick.", "cells");
        inject_transverse_packet_x(rb, sx0, sy0, sz0, sSigma, seed_wave_0_sigmaTransverse, sAmp, sDir, seed_wave_0_carrierK, seed_wave_0_phase);
    }
    else if (name == "flux-cascade") {
        // Scenario ID: flux-cascade
        // Physical Purpose: One-tick supercritical Gaussian genesis response.
        // Initial Condition Parameters: Highly concentrated flux pulse at the center (amplitude = K_GENESIS * 3.0).
        // Expected Behaviour: Deterministic fixed-seed cohort of independent
        // single-site genesis events. No branching, cascade, or pair process.
        configure_genesis_gate_terms(rb);
        const int boxR = seed::integer("geometry.boxRadius", 3, 0, std::max(N, 64),
            "Pulse box half-width", "Half-width (lattice sites) of the cubic region receiving the concentrated genesis pulse.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 4.0, 0.25, std::max(double(N) * N, 256.0),
            "Pulse Gaussian variance", "Variance (sigma^2) of the Gaussian envelope shaping the genesis pulse.");
        const double bigAmp = seed::real("packet.amplitude", K_GENESIS * 3.0, 0.0, K_GENESIS * 60.0,
            "Pulse peak amplitude", "Peak flux/wave amplitude at the center of the one-tick genesis pulse.");
        const double ratioZ = seed::real("packet.componentRatioZ", 0.5, -2.0, 2.0,
            "z-to-x component ratio", "Ratio of the z-component to the x-component of the seeded flux/wave vector.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Pulse center x", "Lattice x-coordinate of the concentrated genesis pulse.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Pulse center y", "Lattice y-coordinate of the concentrated genesis pulse.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Pulse center z", "Lattice z-coordinate of the concentrated genesis pulse.");
        const int xLo = FLR(cx) - boxR, xHi = CEL(cx) + boxR;
        const int yLo = FLR(cy) - boxR, yHi = CEL(cy) + boxR;
        const int zLo = FLR(cz) - boxR, zHi = CEL(cz) + boxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int x = xLo; x <= xHi; x++) {
            double dx = x - cx, dy = y - cy, dz = z - cz;
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) { IF(rb, x, y, z, val, 0, val * ratioZ); IW(rb, x, y, z, val, 0, val * ratioZ); }
        }
        // B2 (2026-07-27): SCOPED OUT of the remove_wave_mean fix, deliberately.
        // This is a one-tick genesis-threshold probe (test_scenario_behavior.cpp
        // ticks it once); the defect remove_wave_mean guards against is a
        // PERMANENT drift that accumulates over many ticks and cannot manifest
        // within one tick. Measured cost of applying the fix here anyway: it
        // shifts which borderline voxels cross K_GENESIS on that single tick
        // (an unprincipled side effect on the exact deterministic threshold
        // count, not a benefit), so it is intentionally left unprojected. If
        // this scenario is ever changed to tick continuously in normal use,
        // revisit.
    }
    else if (name == "flux-annihilation") {
        // Scenario ID: flux-annihilation
        // Exact probe of the native opposite-state collision rule. A + state
        // crosses one face into an adjacent stationary - state after two ticks.
        // The rule removes both states and spreads only their PRE-EXISTING flux
        // over their respective six face neighbours. It contains no
        // rest-mass-to-flux or outgoing-wave mechanism. The single-voxel
        // adjacency IS the rule under test, so it is not exposed as a knob.
        configure_annihilation_terms(rb);
        const int mc = N / 2;
        const double collideAmp = seed::real("packet.amplitude", K_B, 0.0, K_B * 40.0,
            "Pre-collision flux magnitude", "Magnitude of the opposite-signed y-flux seeded on each of the two colliding states before the first tick.");
        const double collideVel = seed::real("kinematics.velocity", C_SPEED, 0.0, 1.0,
            "Approach speed", "Speed (lattice units per tick) given to the upstream + state toward its stationary - neighbor.");
        IP(rb, mc - 1, mc, mc, +1);
        IP(rb, mc,     mc, mc, -1);
        IF(rb, mc - 1, mc, mc, 0.0, +collideAmp, 0.0);
        IF(rb, mc,     mc, mc, 0.0, -collideAmp, 0.0);
        SET_VEL(rb, mc - 1, mc, mc, collideVel, 0.0, 0.0);
    }
    else if (name == "flux-pair-production") {
        // Scenario ID: flux-pair-production
        // One-tick cohort probe of the separate selected polarity-pair
        // transition. Each isolated source has p=1/2 exactly under the compiled
        // hazard. Accepted events place -1 upstream and +1 downstream, assign a
        // shared pair id, and leave pairwise state and vector-flux sums zero.
        // This qualifies the engine rule, not physical Schwinger production.
        configure_pair_production_terms(rb);
        // pair_amp is constructed to sit at exactly the p=1/2 hazard (see the
        // purpose note above); it is fixed-law-derived, not an editable slider.
        const double pair_amp = K_GENESIS + K_MANIFEST * std::log(2.0);
        const int stride = seed::integer("geometry.stride", 3, 1, std::max(N, 64),
            "Source grid stride", "Lattice spacing between successive isolated pair-production source sites along each axis.");
        const int margin = seed::integer("geometry.margin", 2, 0, std::max(N, 64),
            "Boundary margin", "Number of lattice sites kept empty next to each face before the first/last source site.");
        for (int z = margin; z < N - margin; z += stride)
        for (int y = margin; y < N - margin; y += stride)
        for (int x = margin; x + 1 < N - margin; x += stride)
            IF(rb, x, y, z, pair_amp, 0.0, 0.0);
    }
    else if (name == "flux-interference") {
        // Scenario ID: flux-interference
        // Physical Purpose: Four-lobe reflection-symmetric broadband wave field.
        // This qualifies native parity preservation, not a detector fringe law.
        configure_free_wave_terms(rb, false);
        const int q = seed::integer("geometry.offset", N / 4, 0, std::max(N, 64),
            "Source half-separation", "Half the distance (lattice sites) each of the four sources sits from the lattice center along x and z.");
        const int boxR = seed::integer("geometry.boxRadius", 4, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic dressing region around each of the four sources.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 6.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian envelope dressing each source.");
        const double baseAmp = seed::real("packet.amplitude", amp * 1.5, 0.0, amp * 60.0,
            "Source peak amplitude", "Peak x-flux/wave amplitude seeded at the center of each of the four broadband sources.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Cluster center x", "Lattice x-coordinate about which the four sources are symmetrically arranged.");
        const int cy = seed::integer("source.y", RND(midF), 0, N - 1,
            "Cluster center y", "Lattice y-coordinate shared by all four sources.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Cluster center z", "Lattice z-coordinate about which the four sources are symmetrically arranged.");
        const int qL = FLR(cx) - q, qR = CEL(cx) + q;
        const int qzL = FLR(cz) - q, qzR = CEL(cz) + q;
        const int sources[4][3] = { {qL, cy, qzL}, {qR, cy, qzL}, {qL, cy, qzR}, {qR, cy, qzR} };
        for (int s = 0; s < 4; s++) {
            int sx = sources[s][0], sy = sources[s][1], sz = sources[s][2];
            for (int dz = -boxR; dz <= boxR; dz++) for (int dy = -boxR; dy <= boxR; dy++) for (int dx = -boxR; dx <= boxR; dx++) {
                double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
                if (val > cutoff) { IF(rb, sx + dx, sy + dy, sz + dz, val, 0, 0); IW(rb, sx + dx, sy + dy, sz + dz, val, 0, 0); }
            }
        }
        // P5: W = J with a strictly single-signed lobe left Sum W_x = 1162.76,
        // a permanent uniform E ramp. Project out the conserved k=0 mode.
        remove_wave_mean(rb);
    }
    else if (name == "flux-vortex") {
        // Scenario ID: flux-vortex
        // Physical Purpose: Exact discrete helical-ring vector ansatz.
        // It does not demonstrate spin, quantization, or persistent rotation.
        configure_static_seed_terms(rb);
        const int vRadius = seed::integer("geometry.radius", N / 5, 0, std::max(N, 64),
            "Ring radius", "Radius (lattice sites) of the helical vector ring in the x-z plane.");
        const double tangAmp = seed::real("packet.tangentialAmplitude", amp * 2.0, 0.0, amp * 80.0,
            "Tangential amplitude", "Peak magnitude of the in-plane (x,z) tangential flux vector at each ring point.");
        const double axialAmp = seed::real("packet.axialAmplitude", amp * 0.5, -amp * 80.0, amp * 80.0,
            "Axial amplitude", "Peak magnitude of the y-directed flux component at each ring point.");
        const int nV = seed::integer("geometry.ringPointCount", 24, 3, std::max(N * 4, 64),
            "Ring point count", "Number of discrete points sampled around the helical vector ring.");
        const double ringRotation = seed::real("geometry.rotationOffset", 0.0, -2.0 * PI, 2.0 * PI,
            "Ring rotation offset", "Angle (radians) added to every sampled ring point's angular position.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Ring center x", "Lattice x-coordinate of the helical vector ring's center in the x-z plane.");
        const int mc = seed::integer("source.y", RND(midF), 0, N - 1,
            "Ring center y", "Lattice y-coordinate of the ring's axial (helix) center.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Ring center z", "Lattice z-coordinate of the helical vector ring's center in the x-z plane.");
        for (int i = 0; i < nV; i++) {
            double angle = (2.0 * PI * i) / nV + ringRotation;
            int rx = RND(cx + vRadius * std::cos(angle));
            int rz = RND(cz + vRadius * std::sin(angle));
            double tX = -std::sin(angle) * tangAmp;
            double tZ =  std::cos(angle) * tangAmp;
            double tY =  axialAmp;
            IF(rb, rx, mc,     rz, tX,        tY,        tZ);
            IF(rb, rx, mc + 1, rz, tX * 0.5,  tY * 0.5,  tZ * 0.5);
            IF(rb, rx, mc - 1, rz, tX * 0.5, -tY * 0.5,  tZ * 0.5);
        }
    }
    else if (name == "flux-dual-substrate") {
        // Scenario ID: flux-dual-substrate
        // Physical Purpose: Mirror-polarized Gaussian wave pair.
        // The dual_substrate term is deliberately OFF: this legacy setup never
        // represented two fields and provides no evidence for a dual ontology.
        configure_free_wave_terms(rb, false);
        const int off = seed::integer("geometry.offset", N / 4, 0, std::max(N, 64),
            "Lobe half-separation", "Half the distance (lattice sites) between the two mirror-polarized Gaussian lobes.");
        const int boxR = seed::integer("geometry.boxRadius", 5, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic region around each lobe center that receives Gaussian dressing.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 8.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian envelope dressing each lobe.");
        const double baseAmp = seed::real("packet.amplitude", amp * 1.5, 0.0, amp * 60.0,
            "Lobe peak amplitude", "Peak flux/wave amplitude at the center of each Gaussian lobe.");
        const double ratioY = seed::real("packet.componentRatioY", 0.5, -2.0, 2.0,
            "y-to-x component ratio", "Ratio of the y-component to the x-component of the seeded flux/wave vector.");
        const double ratioZ = seed::real("packet.componentRatioZ", 0.3, -2.0, 2.0,
            "z-to-x component ratio", "Ratio of the z-component to the x-component of the seeded flux/wave vector.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Pair center x", "Lattice x-coordinate about which the two mirror-polarized lobes are symmetrically offset.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Pair center y", "Lattice y-coordinate of both lobes.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Pair center z", "Lattice z-coordinate of both lobes.");
        const int pLx = FLR(cx) - off, pRx = CEL(cx) + off;
        const int yLo = FLR(cy) - boxR, yHi = CEL(cy) + boxR;
        const int zLo = FLR(cz) - boxR, zHi = CEL(cz) + boxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int dx = -boxR; dx <= boxR; dx++) {
            double dy = y - cy, dz = z - cz;
            double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) {
                IF(rb, pLx + dx, y, z, val,  val * ratioY, -val * ratioZ); IW(rb, pLx + dx, y, z, val,  val * ratioY, -val * ratioZ);
                IF(rb, pRx + dx, y, z, val, -val * ratioY,  val * ratioZ); IW(rb, pRx + dx, y, z, val, -val * ratioY,  val * ratioZ);
            }
        }
        // P5: W_x is single-signed across both lobes, leaving Sum W_x ~ 935-970
        // and a uniform ramp that overtook the seeded peak by ~6.5 ticks at
        // L=17. Project out the conserved k=0 mode.
        remove_wave_mean(rb);
    }
    else if (name == "flux-random-genesis") {
        // Scenario ID: flux-random-genesis
        // Physical Purpose: One-tick fixed-seed random-patch genesis response.
        // Initial Condition Parameters: 8 randomly distributed high-amplitude flux patches exceeding the genesis threshold.
        // Expected Behaviour: Exact replay of single-site genesis outcomes;
        // pair production, wave propagation, and later reactions are disabled.
        configure_genesis_gate_terms(rb);
        const int nPatches = seed::integer("geometry.patchCount", 8, 1, 64,
            "Patch count", "Number of independent random high-amplitude genesis patches seeded this tick.");
        const double thresholdMult = seed::real("packet.thresholdMultiplier", 2.5, 0.0, 40.0,
            "Threshold multiplier", "Multiplier on K_GENESIS giving the nominal peak amplitude of each patch before jitter.");
        const int margin = seed::integer("geometry.margin", 4, 1, std::max(N, 64),
            "Placement margin", "Lattice sites kept clear next to each face when drawing a random patch center.");
        const int patchRadius = seed::integer("geometry.patchRadius", 2, 0, std::max(N, 64),
            "Patch half-width", "Half-width (lattice sites) of the cubic dressing region around each patch center.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 3.0, 0.25, std::max(double(N) * N, 256.0),
            "Patch Gaussian variance", "Variance (sigma^2) of the Gaussian envelope dressing each patch.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double jitterBase = seed::real("packet.jitterBase", 0.8, 0.0, 4.0,
            "Amplitude jitter base", "Lower fraction of the threshold amplitude every patch is guaranteed.");
        const double jitterRange = seed::real("packet.jitterRange", 0.8, 0.0, 4.0,
            "Amplitude jitter range", "Additional random fraction of the threshold amplitude added per patch, drawn uniformly.");
        const double threshold = K_GENESIS * thresholdMult;
        const int span = std::max(1, N - 2 * margin);
        for (int p = 0; p < nPatches; p++) {
            int cx = int(urand() * span) + margin;
            int cy = int(urand() * span) + margin;
            int cz = int(urand() * span) + margin;
            double pAmp = threshold * (jitterBase + urand() * jitterRange);
            for (int dz = -patchRadius; dz <= patchRadius; dz++) for (int dy = -patchRadius; dy <= patchRadius; dy++) for (int dx = -patchRadius; dx <= patchRadius; dx++) {
                double val = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
                if (val > cutoff) {
                    double sx = (urand() - 0.5) * val;
                    double sy = (urand() - 0.5) * val;
                    double sz = (urand() - 0.5) * val;
                    IF(rb, cx + dx, cy + dy, cz + dz, sx, sy, sz);
                    IW(rb, cx + dx, cy + dy, cz + dz, sx, sy, sz);
                }
            }
        }
    }
    else if (name == "flux-genesis-between-gates") {
        // Scenario ID: flux-genesis-between-gates
        // Physical Purpose: Empirical discriminator for the FTD-0388 genesis-gate adoption.
        //   K_GENESIS = N_c·K_MANIFEST = 3·W_SC = 1.5163860591519780 (adopted 2026-07-17);
        //   the pre-adoption gate was 3·K_B = 1.533. Three uniform-|J| initial
        //   cohorts straddle both gates for the first production decision.
        // Initial Condition Parameters: band amplitudes |J| = 1.5160 (below both gates),
        //   1.5250 (between the gates), 1.5340 (above both). These three defaults are
        //   compile-time-checked to straddle K_GENESIS below; the bands are exposed as
        //   independently editable amplitudes for operators who want a different probe,
        //   at the cost of the static invariant applying only to the shipped defaults.
        // Expected Behaviour: on tick one the hazards are 0 / 0.0168973 /
        //   0.034247 per site; the upper/lower nonzero hazard ratio is 2.0268.
        //   The field is exact only at the
        //   initial decision: accepted genesis drains local flux and the same
        //   master rule also permits evaporation, so this is not a sustained
        //   frozen-field or branching-cascade claim.
        static_assert(1.5160 < K_GENESIS && K_GENESIS < 1.5250,
                      "flux-genesis-between-gates: bands no longer straddle the FTD-0388 "
                      "genesis gate — re-band this scenario (and its JS twin) consciously");
        configure_genesis_gate_terms(rb);
        const double bandLow = seed::real("packet.bandAmplitudeLow", 1.5160, 0.0, K_GENESIS * 8.0,
            "Low band amplitude", "|J| amplitude of the below-both-gates band; the default sits below the FTD-0388 genesis gate.");
        const double bandMid = seed::real("packet.bandAmplitudeMid", 1.5250, 0.0, K_GENESIS * 8.0,
            "Mid band amplitude", "|J| amplitude of the between-gates band; the default sits between the pre- and post-adoption gates.");
        const double bandHigh = seed::real("packet.bandAmplitudeHigh", 1.5340, 0.0, K_GENESIS * 8.0,
            "High band amplitude", "|J| amplitude of the above-both-gates band; the default sits above the FTD-0388 genesis gate.");
        const double bandAmp[3] = { bandLow, bandMid, bandHigh };
        const int x1 = 1 + (N - 2) / 3, x2 = 1 + 2 * (N - 2) / 3;
        for (int x = 1; x < N - 1; x++) {
            if (x == x1 || x == x2) continue;   // 1-plane visual separators between bands
            const int b = (x < x1) ? 0 : (x < x2) ? 1 : 2;
            for (int z = 1; z < N - 1; z++) for (int y = 1; y < N - 1; y++)
                IF(rb, x, y, z, bandAmp[b], 0, 0);
        }
    }
    // ── QCD scenarios ──
    else if (name == "flux-meson") {
        // Scenario ID: flux-meson
        // Physical Purpose: Exact free-transport wiring probe for two opposite
        // ternary states with counter-directed y velocities. The Gaussian field
        // blobs are inert dressing; no color, confinement, or meson identity.
        configure_free_movement_terms(rb);
        const int mOff = seed::integer("geometry.offset", std::max(2, N / 8), 0, std::max(N, 64),
            "Constituent half-separation", "Half the distance (lattice sites) between the two counter-moving markers.");
        const int mDress = seed::integer("geometry.dressRadius", std::max(2, N / 10), 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic field-dressing region around each marker.");
        const double mesonVel = seed::real("kinematics.velocity", 0.05, -1.0, 1.0,
            "Marker speed", "Speed (lattice units per tick) given to each marker along y, with opposite sign on the two markers.");
        const double mesonAmp = seed::real("packet.amplitude", K_B * 1.5, 0.0, K_B * 40.0,
            "Dressing amplitude", "Peak x-flux amplitude at the center of each marker's inert dressing field.");
        const double mSigma2 = seed::real("geometry.sigmaSquared", std::max(0.25, double(mDress) * mDress), 0.25, std::max(double(N) * N, 4096.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian dressing envelope; defaults to the dressing box radius squared.");
        const double mCutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Pair center x", "Lattice x-coordinate about which the two counter-moving markers are symmetrically offset.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Pair center y", "Lattice y-coordinate shared by both markers and the dressing region.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Pair center z", "Lattice z-coordinate shared by both markers and the dressing region.");
        const int mL = FLR(cx) - mOff, mR = CEL(cx) + mOff;
        const int mcY = RND(cy), mcZ = RND(cz);
        IP(rb, mL, mcY, mcZ,  1);
        SET_VEL(rb, mL, mcY, mcZ, 0, mesonVel, 0);
        IP(rb, mR, mcY, mcZ, -1);
        SET_VEL(rb, mR, mcY, mcZ, 0, -mesonVel, 0);
        const int myLo = FLR(cy) - mDress, myHi = CEL(cy) + mDress;
        const int mzLo = FLR(cz) - mDress, mzHi = CEL(cz) + mDress;
        for (int z = mzLo; z <= mzHi; z++) for (int y = myLo; y <= myHi; y++) for (int dx = -mDress; dx <= mDress; dx++) {
            double dy = y - cy, dz = z - cz;
            double val = mesonAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * mSigma2));
            if (val > mCutoff) {
                IF(rb, mL + dx, y, z,  val, 0, 0);
                IF(rb, mR + dx, y, z, -val, 0, 0);
            }
        }
    }
    else if (name == "flux-string-breaking") {
        // Scenario ID: flux-string-breaking
        // Physical Purpose: Outward opposite-polarity free-transport control.
        // Initial Condition: +/- states with vx=-/+0.3 plus inert central J/W.
        // Expected Behaviour: Separation increases with exactly two states.
        // No string, color, confinement, or pair-production term is active.
        configure_free_movement_terms(rb);
        const int sbOff = seed::integer("geometry.offset", std::max(2, N / 10), 0, std::max(N, 64),
            "Constituent half-separation", "Half the distance (lattice sites) between the two outward-moving opposite-polarity markers.");
        const int sbDress = seed::integer("geometry.dressRadius", std::max(2, N / 8), 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic field-dressing region around the center.");
        const double sbVel = seed::real("kinematics.velocity", 0.3, 0.0, 1.0,
            "Marker speed", "Speed (lattice units per tick) each marker is given moving outward along x.");
        const double sbAmp = seed::real("packet.amplitude", K_B * 3.0, 0.0, K_B * 80.0,
            "Dressing amplitude", "Peak amplitude of the inert central field dressing.");
        const double sbRatioY = seed::real("packet.componentRatioY", 0.3, -2.0, 2.0,
            "y-to-x component ratio", "Ratio of the y-component to the x-component of the dressing field.");
        // NOTE: the original literal denominator is 2.0*sbDress, not sbDress
        // squared — a historical deviation from this file's usual Gaussian
        // pattern, preserved verbatim rather than silently "corrected".
        const double sbVariance = seed::real("geometry.varianceParameter", std::max(0.25, double(sbDress)), 0.25, std::max(double(N), 64.0),
            "Dressing Gaussian variance parameter", "Enters the dressing Gaussian as 2*value in the denominator (not squared); defaults to the dressing box radius.");
        const double sbCutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const int sbL = FLR(midF) - sbOff, sbR = CEL(midF) + sbOff;
        const int mc = RND(midF);
        IP(rb, sbL, mc, mc,  1);
        SET_VEL(rb, sbL, mc, mc, -sbVel, 0, 0);
        IP(rb, sbR, mc, mc, -1);
        SET_VEL(rb, sbR, mc, mc,  sbVel, 0, 0);
        const int sbLo = FLR(midF) - sbDress, sbHi = CEL(midF) + sbDress;
        for (int z = sbLo; z <= sbHi; z++) for (int y = sbLo; y <= sbHi; y++) for (int x = sbLo; x <= sbHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = sbAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sbVariance));
            if (val > sbCutoff) { IF(rb, x, y, z, val, val * sbRatioY, 0); IW(rb, x, y, z, val, val * sbRatioY, 0); }
        }
        // B2 (2026-07-27): single-signed lobe (val is always positive), the
        // "inert central J/W" comment above notwithstanding -- Sum(wave_vel)
        // does not cancel. Project out k=0.
        remove_wave_mean(rb);
    }
    else if (name == "flux-baryon") {
        // Scenario ID: flux-baryon
        // Physical Purpose: Threefold tangential free-transport control with
        // one stationary opposite-polarity marker and inert field dressing.
        // Expected Behaviour: Seeded remainders produce deterministic lattice
        // translations. No binding, color, quark, or baryon identity is active.
        configure_free_movement_terms(rb);
        const int bR = seed::integer("geometry.radius", N / 6, 0, std::max(N, 64),
            "Constituent ring radius", "Radius (lattice sites) of the three-fold marker ring in the x-z plane.");
        const double bVel = seed::real("kinematics.velocity", 0.04, 0.0, 1.0,
            "Tangential speed", "Speed (lattice units per tick) each ring marker is given tangent to the ring.");
        const double bSeaRatio = seed::real("geometry.seaOffsetRatio", 0.5, 0.0, 4.0,
            "Sea-marker offset ratio", "Fraction of the ring radius used to offset the single opposite-polarity marker from the ring center.");
        const int bBoxR = seed::integer("geometry.boxRadius", 3, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic field-dressing region around the ring center.");
        const double bSigma2 = seed::real("geometry.sigmaSquared", 4.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian dressing envelope.");
        const double bAmp = seed::real("packet.amplitude", amp * 0.5, 0.0, amp * 40.0,
            "Dressing amplitude", "Peak x-flux amplitude at the center of the dressing region.");
        const double bRatioZ = seed::real("packet.componentRatioZ", 0.3, -2.0, 2.0,
            "z-to-x component ratio", "Ratio of the z-component to the x-component of the dressing field.");
        const double bCutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Ring center x", "Lattice x-coordinate of the three-fold marker ring's center in the x-z plane.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Ring center y", "Lattice y-coordinate shared by the ring, sea marker, and dressing region.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Ring center z", "Lattice z-coordinate of the three-fold marker ring's center in the x-z plane.");
        const int mc = RND(cy);
        for (int k = 0; k < 3; k++) {
            double angle = (2.0 * PI * k) / 3.0;
            int bx = RND(cx + bR * std::cos(angle));
            int bz = RND(cz + bR * std::sin(angle));
            IP(rb, bx, mc, bz, 1);
            SET_VEL(rb, bx, mc, bz, -bVel * std::sin(angle), 0, bVel * std::cos(angle));
        }
        int bSea = std::max(1, int(bR * bSeaRatio));
        IP(rb, RND(cx) + bSea, mc + bSea, RND(cz), -1);
        const int xLo = FLR(cx) - bBoxR, xHi = CEL(cx) + bBoxR;
        const int yLo = FLR(cy) - bBoxR, yHi = CEL(cy) + bBoxR;
        const int zLo = FLR(cz) - bBoxR, zHi = CEL(cz) + bBoxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int x = xLo; x <= xHi; x++) {
            double dx = x - cx, dy = y - cy, dz = z - cz;
            double val = bAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * bSigma2));
            if (val > bCutoff) IF(rb, x, y, z, val, 0, val * bRatioZ);
        }
    }
    else if (name == "flux-nested-standing") {
        // Scenario ID: flux-nested-standing
        // Physical Purpose: Orthogonal reflection-even broadband wave pairs.
        // These are not pure standing eigenmodes.
        configure_free_wave_terms(rb, false);
        const int offX = seed::integer("geometry.offsetX", N / 3, 0, std::max(N, 64),
            "x-axis lobe half-separation", "Half the distance (lattice sites) between the two x-aligned Gaussian lobes.");
        const int offZ = seed::integer("geometry.offsetZ", N / 4, 0, std::max(N, 64),
            "z-axis lobe half-separation", "Half the distance (lattice sites) between the two z-aligned Gaussian lobes.");
        const int boxR = seed::integer("geometry.boxRadius", 4, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic region around each lobe center that receives Gaussian dressing.");
        const double sigma2 = seed::real("geometry.sigmaSquared", 9.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian envelope dressing each lobe.");
        const double baseAmp = seed::real("packet.amplitude", amp, 0.0, amp * 40.0,
            "Lobe peak amplitude", "Peak flux amplitude at the center of each Gaussian lobe.");
        const double cutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double cx = seed::real("source.x", midF, 0.0, N - 1.0,
            "Cluster center x", "Lattice x-coordinate shared by both lobe pairs.");
        const double cy = seed::real("source.y", midF, 0.0, N - 1.0,
            "Cluster center y", "Lattice y-coordinate shared by both lobe pairs.");
        const double cz = seed::real("source.z", midF, 0.0, N - 1.0,
            "Cluster center z", "Lattice z-coordinate shared by both lobe pairs.");
        const int xL = FLR(cx) - offX, xR = CEL(cx) + offX;
        const int zL = FLR(cz) - offZ, zR = CEL(cz) + offZ;
        const int yLo = FLR(cy) - boxR, yHi = CEL(cy) + boxR;
        const int zLo = FLR(cz) - boxR, zHi = CEL(cz) + boxR;
        const int xLo = FLR(cx) - boxR, xHi = CEL(cx) + boxR;
        for (int z = zLo; z <= zHi; z++) for (int y = yLo; y <= yHi; y++) for (int dx = -boxR; dx <= boxR; dx++) {
            double dy = y - cy, dz = z - cz;
            double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) {
                IF(rb, xL + dx, y, z, val, 0, 0);
                IF(rb, xR + dx, y, z, val, 0, 0);
            }
        }
        for (int x = xLo; x <= xHi; x++) for (int y = yLo; y <= yHi; y++) for (int dz = -boxR; dz <= boxR; dz++) {
            double dx = x - cx, dy = y - cy;
            double val = baseAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma2));
            if (val > cutoff) {
                IF(rb, x, y, zL + dz, 0, 0, val);
                IF(rb, x, y, zR + dz, 0, 0, val);
            }
        }
    }
    // ── Experiment scenarios (from test suite) ──
    else if (name == "flux-cyclotron") {
        // Scenario ID: flux-cyclotron
        // Physical Purpose: Native magnetic-curvature test in an imposed,
        // uniform-curl vector potential. This qualifies the engine's selected
        // F=alpha*s*(v x curl J) rule, not emergence of electromagnetism.
        // Initial Condition: B_z=1 in the central periodic patch, chosen so
        // alpha*B*dt < 0.01; one + state at the centre with v=(0.12,0,0).
        // Expected Behaviour: Velocity bends toward -y. Finite-tick speed
        // drift is measured rather than presumed absent.
        configure_lorentz_orbit_terms(rb);
        const double imposed_bz = seed::real("field.imposedBz", 1.0, -20.0, 20.0,
            "Imposed B_z", "Magnitude of the uniform curl-vector-potential field imposed on the lattice; the documented small-angle bound is alpha*B*dt < 0.01.");
        const double cycloVel = seed::real("kinematics.velocity", 0.12, -1.0, 1.0,
            "Initial speed", "Initial speed (lattice units per tick) of the central marker along x.");
        const double axisX = seed::real("source.x", midF, 0.0, N - 1.0,
            "Field axis x", "Lattice x-coordinate of the imposed uniform-curl field's rotation axis.");
        const double axisY = seed::real("source.y", midF, 0.0, N - 1.0,
            "Field axis y", "Lattice y-coordinate of the imposed uniform-curl field's rotation axis.");
        const int markerZ = seed::integer("source.z", mid, 0, N - 1,
            "Marker z position", "Lattice z-coordinate of the central marker.");
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double dx = x - axisX, dy = y - axisY;
            IF(rb, x, y, z, -0.5 * imposed_bz * dy,
                                  0.5 * imposed_bz * dx, 0.0);
        }
        const int markerX = RND(axisX), markerY = RND(axisY);
        IP(rb, markerX, markerY, markerZ, +1);
        SET_VEL(rb, markerX, markerY, markerZ, cycloVel, 0.0, 0.0);
    }
    else if (name == "flux-screening") {
        // Scenario ID: flux-screening
        // Physical Purpose: Exact prepared octahedral polarity-shell geometry.
        // Initial Condition: one central + state, six face-orbit - states, and
        // a separately imposed compact radial 1/r dressing.
        // Expected Behaviour: Inert initial data. The net ternary polarity is
        // -5, so this does not demonstrate neutralization or screening. The
        // six-fold octahedral marker layout that fixes that net polarity is
        // not exposed as a count.
        configure_static_seed_terms(rb);
        const int shellR = seed::integer("geometry.shellRadius", N / 5, 0, std::max(N, 64),
            "Shell radius", "Distance (lattice sites) from the center to each of the six face-orbit markers.");
        const double dressRatio = seed::real("geometry.dressRatio", 0.8, 0.0, 4.0,
            "Dressing radius ratio", "Fraction of the shell radius used as the radius of the imposed 1/r radial dressing.");
        const double scAmp = seed::real("packet.amplitude", amp * 0.5, 0.0, amp * 40.0,
            "Dressing prefactor", "Numerator of the imposed 1/r radial-flux dressing.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Center x", "Lattice x-coordinate of the central marker, the six-fold shell, and the radial dressing.");
        const int cy = seed::integer("source.y", mid, 0, N - 1,
            "Center y", "Lattice y-coordinate of the central marker, the six-fold shell, and the radial dressing.");
        const int cz = seed::integer("source.z", mid, 0, N - 1,
            "Center z", "Lattice z-coordinate of the central marker, the six-fold shell, and the radial dressing.");
        IP(rb, cx, cy, cz, 1);
        const int scOff[6][3] = {
            {shellR,0,0},{-shellR,0,0},{0,shellR,0},{0,-shellR,0},{0,0,shellR},{0,0,-shellR}
        };
        for (int s = 0; s < 6; s++) IP(rb, cx + scOff[s][0], cy + scOff[s][1], cz + scOff[s][2], -1);
        const int scDress = std::max(3, int(shellR * dressRatio));
        const int scDress2 = scDress * scDress;
        for (int dz = -scDress; dz <= scDress; dz++) for (int dy = -scDress; dy <= scDress; dy++) for (int dx = -scDress; dx <= scDress; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > scDress2) continue;
            double r = std::sqrt(double(r2));
            double val = scAmp / r;
            IF(rb, cx + dx, cy + dy, cz + dz, val * dx / r, val * dy / r, val * dz / r);
        }
    }
    else if (name == "flux-triad") {
        // Scenario ID: flux-triad
        // Physical Purpose: Exact prepared threefold polarity/flux seed.
        // Initial Condition: three + states at rounded 120-degree positions,
        // with independently imposed inward-directed local flux dressing.
        // Expected Behaviour: Inert initial data. No binding or stability
        // mechanism is active, and no baryon/gauge identity is inferred.
        configure_static_seed_terms(rb);
        const int tR = seed::integer("geometry.radius", N / 6, 0, std::max(N, 64),
            "Constituent ring radius", "Radius (lattice sites) of the three-fold marker ring in the x-z plane.");
        const int tBoxR = seed::integer("geometry.boxRadius", 3, 0, std::max(N, 64),
            "Dressing box half-width", "Half-width (lattice sites) of the cubic dressing region around each marker.");
        const double tSigma2 = seed::real("geometry.sigmaSquared", 4.0, 0.25, std::max(double(N) * N, 256.0),
            "Dressing Gaussian variance", "Variance (sigma^2) of the Gaussian dressing envelope around each marker.");
        const double tAmp = seed::real("packet.amplitude", amp * 0.5, 0.0, amp * 40.0,
            "Dressing amplitude", "Peak amplitude of the inward-directed dressing flux at each marker.");
        const double tCutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        const double rotationOffset = seed::real("geometry.rotationOffset", 0.0, -2.0 * PI, 2.0 * PI,
            "Ring rotation offset", "Angle (radians) added to all three 120-degree-spaced marker positions.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Ring center x", "Lattice x-coordinate of the three-fold marker ring's center in the x-z plane.");
        const int cy = seed::integer("source.y", mid, 0, N - 1,
            "Ring center y", "Lattice y-coordinate shared by the ring and dressing region.");
        const int cz = seed::integer("source.z", mid, 0, N - 1,
            "Ring center z", "Lattice z-coordinate of the three-fold marker ring's center in the x-z plane.");
        const double triAng[3] = { 0, 2 * PI / 3, 4 * PI / 3 };
        for (int t = 0; t < 3; t++) {
            double angle = triAng[t] + rotationOffset;
            int px = cx + RND(tR * std::cos(angle));
            int pz = cz + RND(tR * std::sin(angle));
            IP(rb, px, cy, pz, 1);
            for (int dx = -tBoxR; dx <= tBoxR; dx++) for (int dy = -tBoxR; dy <= tBoxR; dy++) for (int dz = -tBoxR; dz <= tBoxR; dz++) {
                double val = tAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * tSigma2));
                if (val > tCutoff) {
                    double toCX = (cx - (px + dx));
                    double toCZ = (cz - (pz + dz));
                    double dist = std::sqrt(toCX * toCX + toCZ * toCZ);
                    if (dist < 1.0) dist = 1.0;
                    IF(rb, px + dx, cy + dy, pz + dz, val * toCX / dist, 0, val * toCZ / dist);
                }
            }
        }
    }
    else if (name == "flux-thermalization") {
        // Scenario ID: flux-thermalization
        // Physical Purpose: Deterministic localized random-wave mixing probe.
        // Initial Condition: fixed-seed compact random J/W patch at L/4.
        // Expected Behaviour: Native linear propagation moves energy outside
        // the initial support while preserving the exact modified Hamiltonian.
        // This is dephasing/spreading, not thermodynamic thermalization.
        configure_free_wave_terms(rb, false);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const int cornerX = seed::integer("geometry.cornerX", N / 4, 0, std::max(N, 64),
            "Patch center x", "Lattice x-coordinate of the random-mixing patch center.");
        const int cornerY = seed::integer("geometry.cornerY", N / 4, 0, std::max(N, 64),
            "Patch center y", "Lattice y-coordinate of the random-mixing patch center.");
        const int cornerZ = seed::integer("geometry.cornerZ", N / 4, 0, std::max(N, 64),
            "Patch center z", "Lattice z-coordinate of the random-mixing patch center.");
        const int thermBoxR = seed::integer("geometry.boxRadius", 4, 0, std::max(N, 64),
            "Patch half-width", "Half-width (lattice sites) of the cubic patch that receives random flux/wave vectors.");
        const double thermSigma2 = seed::real("geometry.sigmaSquared", 6.0, 0.25, std::max(double(N) * N, 256.0),
            "Patch Gaussian variance", "Variance (sigma^2) of the Gaussian envelope scaling the random vectors.");
        const double thermAmp = seed::real("packet.amplitude", amp * 3.0, 0.0, amp * 80.0,
            "Patch peak amplitude", "Peak magnitude of the random flux/wave vector at the patch center.");
        const double thermCutoff = seed::real("geometry.cutoff", 0.001, 0.0, 1.0,
            "Amplitude cutoff", "Sites where the Gaussian envelope falls below this magnitude are left unwritten.");
        for (int dz = -thermBoxR; dz <= thermBoxR; dz++) for (int dy = -thermBoxR; dy <= thermBoxR; dy++) for (int dx = -thermBoxR; dx <= thermBoxR; dx++) {
            double val = thermAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * thermSigma2));
            if (val > thermCutoff) {
                double rx = (urand() - 0.5) * 2;
                double ry = (urand() - 0.5) * 2;
                double rz2 = (urand() - 0.5) * 2;
                double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
                if (rLen < 1e-12) rLen = 1;
                IF(rb, cornerX + dx, cornerY + dy, cornerZ + dz,
                   val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                IW(rb, cornerX + dx, cornerY + dy, cornerZ + dz,
                   val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
            }
        }
        // B2 (2026-07-27): random per-voxel direction, no antisymmetric
        // structure -- Sum(wave_vel) does not cancel. Project out k=0.
        remove_wave_mean(rb);
    }
    else if (name == "flux-vacuum-foam") {
        // Scenario ID: flux-vacuum-foam
        // Physical Purpose: Finite deterministic random-wave-ball probe.
        // Initial Condition: fixed-seed random J/W vectors in a central ball.
        // Expected Behaviour: Source-free native wave evolution with exact
        // replay and modified-H conservation. There is no ongoing noise,
        // quantum-vacuum mechanism, or spacetime-foam interpretation.
        configure_free_wave_terms(rb, false);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const int foamR = seed::integer("geometry.radius", N / 3, 1, std::max(N, 64),
            "Foam ball radius", "Radius (lattice sites) of the spherical region seeded with random flux/wave vectors.");
        const double foamBase = seed::real("packet.amplitudeBase", K_B * 0.9, 0.0, K_B * 40.0,
            "Amplitude base", "Guaranteed minimum peak magnitude of the random vector at each seeded site.");
        const double foamVar = seed::real("packet.amplitudeVariance", K_B * 0.4, 0.0, K_B * 40.0,
            "Amplitude variance", "Additional random amplitude added on top of the base, uniform in [0, this value).");
        const double envelopeRatio = seed::real("geometry.envelopeRatio", 0.5, 0.05, 4.0,
            "Envelope width ratio", "Fraction of the ball radius squared used as the Gaussian envelope's variance.");
        const int cx = seed::integer("source.x", mid, 0, N - 1,
            "Ball center x", "Lattice x-coordinate of the spherical random-vector ball.");
        const int cy = seed::integer("source.y", mid, 0, N - 1,
            "Ball center y", "Lattice y-coordinate of the spherical random-vector ball.");
        const int cz = seed::integer("source.z", mid, 0, N - 1,
            "Ball center z", "Lattice z-coordinate of the spherical random-vector ball.");
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double dx = x - cx, dy = y - cy, dz = z - cz;
            double r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > foamR * foamR) continue;
            double envelope = std::exp(-r2 / (2.0 * foamR * foamR * envelopeRatio));
            double val = (foamBase + foamVar * urand()) * envelope;
            double rx = (urand() - 0.5) * 2;
            double ry = (urand() - 0.5) * 2;
            double rz2 = (urand() - 0.5) * 2;
            double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
            if (rLen < 1e-12) rLen = 1;
            IF(rb, x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
            IW(rb, x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
        }
        // B2 (2026-07-27): random per-voxel direction, no antisymmetric
        // structure -- Sum(wave_vel) does not cancel. Project out k=0.
        remove_wave_mean(rb);
    }
    else if (name == "flux-zero-point") {
        // Scenario ID: flux-zero-point
        // Physical Purpose: Finite periodic random-wave-bath invariant probe.
        // Initial Condition: deterministic low-amplitude random J and W.
        // Expected Behaviour: exact conservation of the kick-drift modified
        // Hamiltonian with zero manifested sites. This is not quantum vacuum
        // energy, a ground state, or a derivation of 1/2 hbar omega.
        configure_free_wave_terms(rb, false);
        rb.toggles.flux_boundary = FluxBoundaryMode::Periodic;
        const double zpeAmp = seed::real("packet.amplitude", K_B * 0.3, 0.0, K_B * 40.0,
            "Bath amplitude", "Half-width of the uniform random distribution each flux/wave-velocity component is drawn from.");
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = (urand() - 0.5) * zpeAmp;
            double jy = (urand() - 0.5) * zpeAmp;
            double jz = (urand() - 0.5) * zpeAmp;
            IF(rb, x, y, z, jx, jy, jz);
            IW(rb, x, y, z, jx, jy, jz);
        }
        // B2 (2026-07-27): random per-voxel direction, no antisymmetric
        // structure -- Sum(wave_vel) does not cancel. Project out k=0.
        remove_wave_mean(rb);
    }
    else {
        return false;
    }
    return true;
}

}  // namespace ftd
