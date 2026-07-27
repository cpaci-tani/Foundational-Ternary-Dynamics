// ==========================================================================
//  engine/src/scenarios/flux.cpp
//
//  Group: flux-* (22 scenarios)
//  JS source: engine/web/js/bridge/scenarios/flux-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1). Every scenario body
//  is byte-identical to the pre-split source — see _helpers.h for the
//  shared IF/IW/IP/IPF/SET_VEL/LOCK/SET_SPIN/FLR/CEL/RND primitives
//  and docs/scenarios.h for the group-function contract.
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

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
        // Expected Behaviour: periodic propagation or Neumann-shell reflection.
        // The selected one-shell loss mode is only an imposed attenuation law;
        // it failed the 75%-removal qualification and is not an absorber claim.
        configure_free_wave_terms(rb, false);
        const double sx = std::max(3.0, N / 16.0);
        inject_transverse_packet_x(rb, N / 3.0, midF, midF,
                                   sx, sx, K_B * 0.5, +1,
                                   2.0 * PI / (4.0 * sx));
    }
    else if (name == "flux-dipole") {
        // Scenario ID: flux-dipole
        // Physical Purpose: Antisymmetric pair of Gaussian vector-wave blobs.
        // This tests native parity preservation, not an electric/magnetic dipole.
        configure_free_wave_terms(rb, false);
        const int off = N / 4;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, pLx + dx, y, z,  val,  val * 0.5, 0);
                IW(rb, pLx + dx, y, z,  val,  val * 0.5, 0);
                IF(rb, pRx + dx, y, z, -val, -val * 0.5, 0);
                IW(rb, pRx + dx, y, z, -val, -val * 0.5, 0);
            }
        }
    }
    else if (name == "flux-standing") {
        // Scenario ID: flux-standing
        // Physical Purpose: Reflection-even, zero-initial-momentum broadband
        // wave pair. It is a standing-wave proxy, not a pure eigenmode.
        configure_free_wave_terms(rb, false);
        const int off = N / 3;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
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
        inject_transverse_packet_x(rb, midF, midF, midF, 2.0, 2.0,
                                   amp * 2.0, +1);
    }
    else if (name == "flux-cascade") {
        // Scenario ID: flux-cascade
        // Physical Purpose: One-tick supercritical Gaussian genesis response.
        // Initial Condition Parameters: Highly concentrated flux pulse at the center (amplitude = K_GENESIS * 3.0).
        // Expected Behaviour: Deterministic fixed-seed cohort of independent
        // single-site genesis events. No branching, cascade, or pair process.
        configure_genesis_gate_terms(rb);
        const double bigAmp = K_GENESIS * 3.0;
        const int cLo = FLR(midF) - 3, cHi = CEL(midF) + 3;
        for (int z = cLo; z <= cHi; z++) for (int y = cLo; y <= cHi; y++) for (int x = cLo; x <= cHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = bigAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
            if (val > 0.001) { IF(rb, x, y, z, val, 0, val * 0.5); IW(rb, x, y, z, val, 0, val * 0.5); }
        }
    }
    else if (name == "flux-annihilation") {
        // Scenario ID: flux-annihilation
        // Exact probe of the native opposite-state collision rule. A + state
        // crosses one face into an adjacent stationary - state after two ticks.
        // The rule removes both states and spreads only their PRE-EXISTING flux
        // over their respective six face neighbours. It contains no
        // rest-mass-to-flux or outgoing-wave mechanism.
        configure_annihilation_terms(rb);
        const int mc = N / 2;
        IP(rb, mc - 1, mc, mc, +1);
        IP(rb, mc,     mc, mc, -1);
        IF(rb, mc - 1, mc, mc, 0.0, +K_B, 0.0);
        IF(rb, mc,     mc, mc, 0.0, -K_B, 0.0);
        SET_VEL(rb, mc - 1, mc, mc, C_SPEED, 0.0, 0.0);
    }
    else if (name == "flux-pair-production") {
        // Scenario ID: flux-pair-production
        // One-tick cohort probe of the separate selected polarity-pair
        // transition. Each isolated source has p=1/2 exactly under the compiled
        // hazard. Accepted events place -1 upstream and +1 downstream, assign a
        // shared pair id, and leave pairwise state and vector-flux sums zero.
        // This qualifies the engine rule, not physical Schwinger production.
        configure_pair_production_terms(rb);
        const double pair_amp = K_GENESIS + K_MANIFEST * std::log(2.0);
        for (int z = 2; z < N - 2; z += 3)
        for (int y = 2; y < N - 2; y += 3)
        for (int x = 2; x + 1 < N - 2; x += 3)
            IF(rb, x, y, z, pair_amp, 0.0, 0.0);
    }
    else if (name == "flux-interference") {
        // Scenario ID: flux-interference
        // Physical Purpose: Four-lobe reflection-symmetric broadband wave field.
        // This qualifies native parity preservation, not a detector fringe law.
        configure_free_wave_terms(rb, false);
        const int q = N / 4;
        const int qL = FLR(midF) - q, qR = CEL(midF) + q;
        const int mc = RND(midF);
        const int sources[4][3] = { {qL, mc, qL}, {qR, mc, qL}, {qL, mc, qR}, {qR, mc, qR} };
        for (int s = 0; s < 4; s++) {
            int sx = sources[s][0], sy = sources[s][1], sz = sources[s][2];
            for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double val = amp * 1.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
                if (val > 0.001) { IF(rb, sx + dx, sy + dy, sz + dz, val, 0, 0); IW(rb, sx + dx, sy + dy, sz + dz, val, 0, 0); }
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
        const int vRadius = N / 5;
        const int nV = 24;
        const int mc = RND(midF);
        for (int i = 0; i < nV; i++) {
            double angle = (2.0 * PI * i) / nV;
            int rx = RND(midF + vRadius * std::cos(angle));
            int rz = RND(midF + vRadius * std::sin(angle));
            double tX = -std::sin(angle) * amp * 2.0;
            double tZ =  std::cos(angle) * amp * 2.0;
            double tY =  amp * 0.5;
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
        const int off = N / 4;
        const int pLx = FLR(midF) - off, pRx = CEL(midF) + off;
        const int yzLo = FLR(midF) - 5, yzHi = CEL(midF) + 5;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -5; dx <= 5; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * 1.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 8.0));
            if (val > 0.001) {
                IF(rb, pLx + dx, y, z, val,  val * 0.5, -val * 0.3); IW(rb, pLx + dx, y, z, val,  val * 0.5, -val * 0.3);
                IF(rb, pRx + dx, y, z, val, -val * 0.5,  val * 0.3); IW(rb, pRx + dx, y, z, val, -val * 0.5,  val * 0.3);
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
        const int nPatches = 8;
        const double threshold = K_GENESIS * 2.5;
        for (int p = 0; p < nPatches; p++) {
            int cx = int(urand() * (N - 8)) + 4;
            int cy = int(urand() * (N - 8)) + 4;
            int cz = int(urand() * (N - 8)) + 4;
            double pAmp = threshold * (0.8 + urand() * 0.8);
            for (int dz = -2; dz <= 2; dz++) for (int dy = -2; dy <= 2; dy++) for (int dx = -2; dx <= 2; dx++) {
                double val = pAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 3.0));
                if (val > 0.001) {
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
        //   1.5250 (between the gates), 1.5340 (above both).
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
        const double bandAmp[3] = { 1.5160, 1.5250, 1.5340 };
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
        const int mOff = std::max(2, N / 8);
        const int mDress = std::max(2, N / 10);
        const int mL = FLR(midF) - mOff, mR = CEL(midF) + mOff;
        const int mc = RND(midF);
        IP(rb, mL, mc, mc,  1);
        SET_VEL(rb, mL, mc, mc, 0, 0.05, 0);
        IP(rb, mR, mc, mc, -1);
        SET_VEL(rb, mR, mc, mc, 0, -0.05, 0);
        const double mesonAmp = K_B * 1.5;
        const double mSigma2 = mDress * mDress;
        const int myzLo = FLR(midF) - mDress, myzHi = CEL(midF) + mDress;
        for (int z = myzLo; z <= myzHi; z++) for (int y = myzLo; y <= myzHi; y++) for (int dx = -mDress; dx <= mDress; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = mesonAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2 * mSigma2));
            if (val > 0.001) {
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
        const int sbOff = std::max(2, N / 10);
        const int sbDress = std::max(2, N / 8);
        const int sbL = FLR(midF) - sbOff, sbR = CEL(midF) + sbOff;
        const int mc = RND(midF);
        IP(rb, sbL, mc, mc,  1);
        SET_VEL(rb, sbL, mc, mc, -0.3, 0, 0);
        IP(rb, sbR, mc, mc, -1);
        SET_VEL(rb, sbR, mc, mc,  0.3, 0, 0);
        const double sbAmp = K_B * 3.0;
        const int sbLo = FLR(midF) - sbDress, sbHi = CEL(midF) + sbDress;
        for (int z = sbLo; z <= sbHi; z++) for (int y = sbLo; y <= sbHi; y++) for (int x = sbLo; x <= sbHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = sbAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sbDress));
            if (val > 0.001) { IF(rb, x, y, z, val, val * 0.3, 0); IW(rb, x, y, z, val, val * 0.3, 0); }
        }
    }
    else if (name == "flux-baryon") {
        // Scenario ID: flux-baryon
        // Physical Purpose: Threefold tangential free-transport control with
        // one stationary opposite-polarity marker and inert field dressing.
        // Expected Behaviour: Seeded remainders produce deterministic lattice
        // translations. No binding, color, quark, or baryon identity is active.
        configure_free_movement_terms(rb);
        const int bR = N / 6;
        const int mc = RND(midF);
        for (int k = 0; k < 3; k++) {
            double angle = (2.0 * PI * k) / 3.0;
            int bx = RND(midF + bR * std::cos(angle));
            int bz = RND(midF + bR * std::sin(angle));
            IP(rb, bx, mc, bz, 1);
            SET_VEL(rb, bx, mc, bz, -0.04 * std::sin(angle), 0, 0.04 * std::cos(angle));
        }
        int bSea = std::max(1, bR / 2);
        IP(rb, mc + bSea, mc + bSea, mc, -1);
        const int bLo = FLR(midF) - 3, bHi = CEL(midF) + 3;
        for (int z = bLo; z <= bHi; z++) for (int y = bLo; y <= bHi; y++) for (int x = bLo; x <= bHi; x++) {
            double dx = x - midF, dy = y - midF, dz = z - midF;
            double val = amp * 0.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
            if (val > 0.001) IF(rb, x, y, z, val, 0, val * 0.3);
        }
    }
    else if (name == "flux-nested-standing") {
        // Scenario ID: flux-nested-standing
        // Physical Purpose: Orthogonal reflection-even broadband wave pairs.
        // These are not pure standing eigenmodes.
        configure_free_wave_terms(rb, false);
        const int offX = N / 3, offZ = N / 4;
        const int xL = FLR(midF) - offX, xR = CEL(midF) + offX;
        const int zL = FLR(midF) - offZ, zR = CEL(midF) + offZ;
        const int yzLo = FLR(midF) - 4, yzHi = CEL(midF) + 4;
        for (int z = yzLo; z <= yzHi; z++) for (int y = yzLo; y <= yzHi; y++) for (int dx = -4; dx <= 4; dx++) {
            double dy = y - midF, dz = z - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
                IF(rb, xL + dx, y, z, val, 0, 0);
                IF(rb, xR + dx, y, z, val, 0, 0);
            }
        }
        for (int x = yzLo; x <= yzHi; x++) for (int y = yzLo; y <= yzHi; y++) for (int dz = -4; dz <= 4; dz++) {
            double dx = x - midF, dy = y - midF;
            double val = amp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 9.0));
            if (val > 0.001) {
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
        const double imposed_bz = 1.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            const double cx = x - midF, cy = y - midF;
            IF(rb, x, y, z, -0.5 * imposed_bz * cy,
                                  0.5 * imposed_bz * cx, 0.0);
        }
        IP(rb, mid, mid, mid, +1);
        SET_VEL(rb, mid, mid, mid, 0.12, 0.0, 0.0);
    }
    else if (name == "flux-screening") {
        // Scenario ID: flux-screening
        // Physical Purpose: Exact prepared octahedral polarity-shell geometry.
        // Initial Condition: one central + state, six face-orbit - states, and
        // a separately imposed compact radial 1/r dressing.
        // Expected Behaviour: Inert initial data. The net ternary polarity is
        // -5, so this does not demonstrate neutralization or screening.
        configure_static_seed_terms(rb);
        const int shellR = N / 5;
        IP(rb, mid, mid, mid, 1);
        const int scOff[6][3] = {
            {shellR,0,0},{-shellR,0,0},{0,shellR,0},{0,-shellR,0},{0,0,shellR},{0,0,-shellR}
        };
        for (int s = 0; s < 6; s++) IP(rb, mid + scOff[s][0], mid + scOff[s][1], mid + scOff[s][2], -1);
        const int scDress = std::max(3, int(shellR * 0.8));
        const int scDress2 = scDress * scDress;
        for (int dz = -scDress; dz <= scDress; dz++) for (int dy = -scDress; dy <= scDress; dy++) for (int dx = -scDress; dx <= scDress; dx++) {
            int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > scDress2) continue;
            double r = std::sqrt(double(r2));
            double val = amp * 0.5 / r;
            IF(rb, mid + dx, mid + dy, mid + dz, val * dx / r, val * dy / r, val * dz / r);
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
        const int tR = N / 6;
        const double triAng[3] = { 0, 2 * PI / 3, 4 * PI / 3 };
        for (int t = 0; t < 3; t++) {
            double angle = triAng[t];
            int px = mid + RND(tR * std::cos(angle));
            int pz = mid + RND(tR * std::sin(angle));
            IP(rb, px, mid, pz, 1);
            for (int dx = -3; dx <= 3; dx++) for (int dy = -3; dy <= 3; dy++) for (int dz = -3; dz <= 3; dz++) {
                double val = amp * 0.5 * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 4.0));
                if (val > 0.001) {
                    double toCX = (mid - (px + dx));
                    double toCZ = (mid - (pz + dz));
                    double dist = std::sqrt(toCX * toCX + toCZ * toCZ);
                    if (dist < 1.0) dist = 1.0;
                    IF(rb, px + dx, mid + dy, pz + dz, val * toCX / dist, 0, val * toCZ / dist);
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
        const int corner = N / 4;
        const double thermAmp = amp * 3.0;
        for (int dz = -4; dz <= 4; dz++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
            double val = thermAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * 6.0));
            if (val > 0.001) {
                double rx = (urand() - 0.5) * 2;
                double ry = (urand() - 0.5) * 2;
                double rz2 = (urand() - 0.5) * 2;
                double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
                if (rLen < 1e-12) rLen = 1;
                IF(rb, corner + dx, corner + dy, corner + dz,
                   val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
                IW(rb, corner + dx, corner + dy, corner + dz,
                   val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
            }
        }
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
        const int foamR = N / 3;
        const double foamBase = K_B * 0.9, foamVar = K_B * 0.4;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double dx = x - mid, dy = y - mid, dz = z - mid;
            double r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > foamR * foamR) continue;
            double envelope = std::exp(-r2 / (2.0 * foamR * foamR * 0.5));
            double val = (foamBase + foamVar * urand()) * envelope;
            double rx = (urand() - 0.5) * 2;
            double ry = (urand() - 0.5) * 2;
            double rz2 = (urand() - 0.5) * 2;
            double rLen = std::sqrt(rx * rx + ry * ry + rz2 * rz2);
            if (rLen < 1e-12) rLen = 1;
            IF(rb, x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
            IW(rb, x, y, z, val * rx / rLen, val * ry / rLen, val * rz2 / rLen);
        }
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
        const double zpeAmp = K_B * 0.3;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = (urand() - 0.5) * zpeAmp;
            double jy = (urand() - 0.5) * zpeAmp;
            double jz = (urand() - 0.5) * zpeAmp;
            IF(rb, x, y, z, jx, jy, jz);
            IW(rb, x, y, z, jx, jy, jz);
        }
    }
    return true;
}

}  // namespace ftd
