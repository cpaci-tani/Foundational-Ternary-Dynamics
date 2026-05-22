/**
 * @file campaign_graviton_tt_correlator.cpp
 * @brief Frontier 4, Step 4a-ii — emergent transverse-traceless (spin-2) pole.
 *
 * Implements the measurement of Test 4a-ii of the hash-locked pre-registration
 *   docs/theory/10_eft_program/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md
 * (tag `preregister-graviton-substrate-mode-v2`).
 *
 * THE PRE-REGISTRATION IS CANONICAL. This file is a measurement instrument
 * built to its spec AFTER lock; it makes no claim, promotes no tag, and does
 * NOT alter any engine physics. New file + CMake registration only.
 *
 * ─────────────────────────────────────────────────────────────────────────
 * WHAT IS MEASURED (prereg §5)
 * ─────────────────────────────────────────────────────────────────────────
 * Step 4a-i (engine code audit) already established that the linear vacuum
 * spectrum of the flux 3-vector J is exactly spin-0 (1 longitudinal) ⊕ spin-1
 * (2 transverse) — a 3-vector has no room for the 5-component spin-2 rep. So a
 * graviton, if FTD has one, must be EMERGENT: a collective pole of the
 * *interacting* substrate in the transverse-traceless rank-2 channel.
 *
 * This campaign measures the connected, TT-projected two-point function of a
 * rank-2 composite operator O_ij built from J, for the TWO pre-registered
 * probe operators (§5; fixed, no post-hoc operator scanning). Both are genuine
 * symmetric-traceless rank-2 BILINEARS of J — neither is k-reducible, so both
 * carry real transverse-traceless content:
 *
 *   (i′) flux-quadrupole:  O_ij = J_i J_j − (1/3) δ_ij |J|²
 *        A direct J⊗J bilinear — no derivatives. In Fourier
 *        O_ij(k) = Σ_q J_i(q) J_j(k−q) − trace: index i on J(q), index j on
 *        J(k−q), independent momenta — the TT projection is generically
 *        non-zero.
 *   (ii) stress:           O_ij = symmetric-traceless part of the Noether stress
 *          T_ij = (∂_i J_a)(∂_j J_a) − δ_ij L,
 *          L = ½|J̇|² − ½C²|∇J|²
 *        (DERIV_RELATIVITY_DERIVATION.md §14.4 Theorem 14.2 — a derived
 *        rank-2 J-bilinear, the ∂J⊗∂J construction).
 *
 * The two operators are independent constructions (J⊗J vs ∂J⊗∂J); a result on
 * both is a genuine cross-check (prereg §5).
 *
 * PROVENANCE: PREREG v1 §5 declared operator (i) as the strain-rate operator
 * O_ij = ½(∂_iJ_j+∂_jJ_i) − ⅓δ_ij(∂·J). It was found analytically TT-degenerate
 * — a strain tensor is the symmetrized gradient of a VECTOR field, carries a
 * free k index in every Fourier term, and the TT projector annihilates any
 * tensor with a free k index (Λ·O^strain ≡ 0). It carried no helicity-±2
 * content and was never a valid spin-2 probe. PREREG v2 §5 drops it and
 * replaces it with the flux-quadrupole bilinear (i′) above. Operator (ii) is
 * unchanged across v1→v2.
 *
 * Per tick, O_ij(x) is FFT'd to O_ij(k); the 3D transverse-traceless projector
 *   P_ij(k)        = δ_ij − k̂_i k̂_j
 *   Λ_ij,lm(k)     = ½(P_il P_jm + P_im P_jl) − ½ P_ij P_lm
 *   O^TT_ij(k)     = Λ_ij,lm(k) O_lm(k)
 * isolates exactly the 2-dimensional helicity-±2 (spin-2) subspace in 3D —
 * THAT is the operational helicity criterion (prereg §5/§7: the polarization
 * count is NOT the criterion).
 *
 * The connected correlator
 *   C_TT(k,t) = ⟨ O^TT_ij(k,t) · O^TT_ij(−k,0)* ⟩_c   (sum i,j; connected)
 * is accumulated for small-|k| wavevectors along [100],[110],[111] over a
 * post-equilibration window. Small-|k| is the gapless test.
 *
 * CONTROL (mandatory self-validation, prereg §5): the spin-1 sector — the
 * connected transverse-vector correlator of J itself. From Step 4a-i this is a
 * known propagating mode with ω(k) = 2C|sin(k/2)|, C = C_WAVE = 1/√3 (the
 * engine's exact leapfrog dispersion, verified to <0.1% by campaign_dispersion).
 * If the harness cannot cleanly recover this spin-1 pole, the correlator
 * machinery is broken. The control also confirms any spin-2 pole sits at a
 * DIFFERENT ω (separability, prereg §5).
 *
 * OUTPUT: per (k, operator) — dominant ω(k), Γ(k), a pole-vs-continuum
 * diagnostic (spectral prominence), the connected variance C(0), a
 * has_signal flag, and the spin-1 control ω(k). NO Outcome A/B verdict is
 * hardcoded; the verdict is applied afterward against the locked prereg
 * (§6 outcome table, §7 exclusions). A noisy result is Indeterminate, not B.
 *
 * NOTE on the two operators (i′)/(ii): both are genuine rank-2 bilinears of J
 * with a non-vanishing TT projection, so both carry real helicity-±2 content.
 * The flux-quadrupole O_ij = J_iJ_j − ⅓δ_ij|J|² is a J⊗J product: in Fourier
 * its index i sits on J(q) and index j on J(k−q) with independent momenta, so
 * Λ_ij,lm does NOT annihilate it (unlike a strain tensor, whose every Fourier
 * term carries a free k index). The stress O_ij = [(∂_iJ_a)(∂_jJ_a)]_TT is a
 * ∂J⊗∂J product and is likewise not k-reducible. Both pre-registered operators
 * are measured and emitted (prereg v2 §5 fixes both); the analyst weighs them
 * against §6/§7. The has_signal=0 / "no signal" gate in extract_pole remains —
 * it guards against fitting floating-point noise in any genuinely empty
 * channel — but for these two operators a non-zero TT signal is expected.
 *
 * ─────────────────────────────────────────────────────────────────────────
 * EXACT TOGGLE SET (PREREG v2 §8 — enumerated there and mirrored here)
 * ─────────────────────────────────────────────────────────────────────────
 * PREREG v2 §8 enumerates the toggle/solver configuration explicitly (the v1
 * §8 "hash-reference" obligation is discharged inside the v2 registration
 * itself). Prereg v2 §8 specifies, for Test 4a-ii: the logic-first six rules
 * PLUS the nonlinear flux coupling PLUS selective_damping, NO phenomenological
 * toggles beyond those. SPEC_ENGINE.md §1 defines the six rules and their
 * toggles. The campaign sets exactly the following on a freshly-constructed
 * RenderBridge (after toggles.disable_all()):
 *
 *   ON  — wave_propagation   Rule 1: 18-pt Laplacian flux wave equation
 *   ON  — coupling           Rule 2: nonlinear state↔flux coupling g_c·∇s
 *                                    + g_c·∇×(s·v)  ← "the nonlinear flux coupling"
 *   ON  — gauss_projection   Rule 3: enforce ∇·J = s each tick
 *   ON  — genesis            Rule 4: manifestation + evaporation (interacting
 *                                    substrate — REQUIRED active for 4a-ii)
 *   ON  — forces             Rule 5: field-mediated force master toggle
 *   ON  — gravity            Rule 5: F = G_N·∇ρ  (a derived substrate force)
 *   ON  — poisson_coulomb    Rule 5: Poisson Coulomb potential φ_C
 *   ON  — lorentz_force      Rule 5: F = α·s·(v×B), B = ∇×J
 *   ON  — movement           Rule 6: particle position integration + collision
 *   ON  — damping            Rule 1 (write phase): flux dissipation
 *   ON  — selective_damping  damp ONLY near manifested particles ⇒ the vacuum
 *                            EM/flux sector is LOSSLESS — required so the
 *                            measured propagator is not artificially broadened
 *
 *   OFF — everything else, in particular all phenomenological extensions:
 *         dual_substrate, weak_transmutation, color_forces, strong_force,
 *         larmor_radiation, triad_binding, pair_production, exchange_force,
 *         latency_field, evaporation(alone), emergent_forces, langevin,
 *         exact_dual_gauss, confinement, strict_validation.
 *
 * RATIONALE for the two judgement calls (flagged in the report):
 *  - damping + selective_damping are ON: damping is one of SPEC_ENGINE §1's
 *    "core ON" toggles (part of Rule 1's write phase, not a phenomenological
 *    extension). selective_damping ON makes the VACUUM lossless, which is what
 *    a clean gapless-pole measurement needs; turning damping fully OFF is a
 *    non-default configuration and is itself a parameter change the prereg's
 *    §7 anti-gaming clause discourages.
 *  - dual_substrate and weak_transmutation are OFF even though the engine's
 *    enable_all() default profile turns them ON: SPEC_ENGINE.md §1 lists
 *    dual_substrate under "Toggle-gated extensions" and weak_transmutation
 *    under "What was removed" (phenomenology). Prereg §8 says "NO
 *    phenomenological toggles beyond [the six rules]", so both are OFF. The
 *    campaign therefore runs the single-substrate genesis path.
 *
 * Determinism: FTD is deterministic. The broadband J perturbation is a fixed
 * sum of plane waves with a hardcoded integer seed (kPerturbSeed) and a fixed
 * amplitude (kPerturbAmplitude); genesis RNG is seeded from the same seed.
 * The whole campaign is exactly reproducible.
 *
 * ─────────────────────────────────────────────────────────────────────────
 * CLI
 * ─────────────────────────────────────────────────────────────────────────
 *   campaign_graviton_tt_correlator [--L=N] [--equil=N] [--window=N]
 *                                   [--seed=N] [--amp=F] [--output-dir=PATH]
 *
 *   Default (smoke):  L=16, equil=200, window=512.
 *   Canonical runs (launched separately): L ∈ {32,64,128}, longer window.
 *
 * Output: CSV to stdout (one row per k-point per operator/sector); optional
 * meta.json + correlator dumps when --output-dir is given. stderr carries
 * human-readable progress (per project rule: long runs stream live).
 *
 * Epistemic status: [PRE-REGISTERED MEASUREMENT INSTRUMENT]. This file
 * produces measurement data only. The smoke run's numbers are NOT
 * publishable; the canonical L∈{32,64,128} run is the measurement.
 */

#define _USE_MATH_DEFINES
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "ftd/constants.h"
#include "ftd/correlations.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include "ftd/spectral.h"
#include "ftd/spectrum_extraction.h"

namespace fs = std::filesystem;

namespace {

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

// ───────────────────────────────────────────────────────────────────────────
// Pre-registered fixed parameters (deterministic; hash-referenced).
// ───────────────────────────────────────────────────────────────────────────

// Broadband initial J perturbation: a fixed sum of plane waves. FTD is
// deterministic, so the "pseudo-random field" is realized as a fixed,
// reproducible superposition with hardcoded seed + amplitude. Each plane
// wave gets a deterministic phase/direction from a tiny LCG seeded by
// kPerturbSeed.
constexpr unsigned int kPerturbSeed      = 0x4A21B7u;  // fixed RNG seed
constexpr double       kPerturbAmplitude = 0.02;       // small-amplitude (per-mode)
constexpr int          kPerturbModesPerAxis = 4;       // modes n=1..4 per axis

// Equilibration + measurement window defaults (smoke). Overridable via CLI.
constexpr int kDefaultEquil  = 200;
constexpr int kDefaultWindow = 512;
constexpr int kDefaultL      = 16;

// Number of small-|k| points sampled along each high-symmetry direction.
constexpr int kNumKPoints = 4;  // n = 1,2,3,4

// Lattice signal speed — the engine's wave speed (constants.h: C_WAVE = 1/√3).
// Spin-1 control pole prediction (prereg §5): ω(k) = 2·C·|sin(k/2)|.
const double kC = ftd::C_WAVE;

// ───────────────────────────────────────────────────────────────────────────
// Small deterministic LCG — used ONLY to lay down the fixed broadband
// perturbation. Not engine physics. Numerical Recipes constants.
// ───────────────────────────────────────────────────────────────────────────
struct Lcg {
    std::uint64_t s;
    explicit Lcg(std::uint64_t seed) : s(seed ? seed : 0x9E3779B97F4A7C15ull) {}
    std::uint32_t next() {
        s = s * 6364136223846793005ull + 1442695040888963407ull;
        return static_cast<std::uint32_t>(s >> 32);
    }
    double uniform() { return next() / 4294967296.0; }                 // [0,1)
    double sym()     { return 2.0 * uniform() - 1.0; }                 // [-1,1)
};

// ───────────────────────────────────────────────────────────────────────────
// 3D FFT of a real scalar field laid out as lattice index (x*L*L + y*L + z).
// Returns a complex L^3 array in the same index order. Built on the radix-2
// fft_1d from ftd/spectral.h; L must be a power of two (16/32/64/128 all are).
// ───────────────────────────────────────────────────────────────────────────
void fft3d(std::vector<std::complex<double>>& data, int L, bool inverse) {
    using C = std::complex<double>;
    std::vector<C> line(L);
    // Transform along z (stride 1).
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y) {
            int base = (x * L + y) * L;
            for (int z = 0; z < L; ++z) line[z] = data[base + z];
            ftd::fft_1d(line, inverse);
            for (int z = 0; z < L; ++z) data[base + z] = line[z];
        }
    // Transform along y (stride L).
    for (int x = 0; x < L; ++x)
        for (int z = 0; z < L; ++z) {
            for (int y = 0; y < L; ++y) line[y] = data[(x * L + y) * L + z];
            ftd::fft_1d(line, inverse);
            for (int y = 0; y < L; ++y) data[(x * L + y) * L + z] = line[y];
        }
    // Transform along x (stride L*L).
    for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            for (int x = 0; x < L; ++x) line[x] = data[(x * L + y) * L + z];
            ftd::fft_1d(line, inverse);
            for (int x = 0; x < L; ++x) data[(x * L + y) * L + z] = line[x];
        }
}

// ───────────────────────────────────────────────────────────────────────────
// k-space geometry.
//
// Lattice momentum component for integer Fourier index n on an L-lattice:
//   k_α = 2π·n_α / L  (in [-π, π) after folding n > L/2 → n − L).
// We sample small-|k| points along the high-symmetry directions [100],[110],
// [111] at n = 1..kNumKPoints. The probe lives at Fourier index (n·d) where
// d ∈ {(1,0,0),(1,1,0),(1,1,1)}.
// ───────────────────────────────────────────────────────────────────────────
struct Direction {
    const char* name;
    int dx, dy, dz;
};
const std::array<Direction, 3> kDirections = {{
    {"[100]", 1, 0, 0},
    {"[110]", 1, 1, 0},
    {"[111]", 1, 1, 1},
}};

// Continuum wavevector for integer index (nx,ny,nz). Folds to [-π,π).
struct KVec {
    double kx, ky, kz;
    double mag() const { return std::sqrt(kx * kx + ky * ky + kz * kz); }
};
KVec k_of_index(int nx, int ny, int nz, int L) {
    auto fold = [L](int n) {
        int m = ((n % L) + L) % L;
        if (m > L / 2) m -= L;
        return 2.0 * M_PI * m / L;
    };
    return {fold(nx), fold(ny), fold(nz)};
}

// ───────────────────────────────────────────────────────────────────────────
// The 3D transverse-traceless projector applied to a symmetric rank-2 tensor.
//
//   P_ij(k)    = δ_ij − k̂_i k̂_j
//   Λ_ij,lm(k) = ½(P_il P_jm + P_im P_jl) − ½ P_ij P_lm
//   O^TT_ij    = Λ_ij,lm O_lm
//
// In 3D, Λ projects onto the 2-dimensional helicity-±2 subspace exactly —
// this is the operational spin-2 criterion (prereg §5). At k = 0, k̂ is
// undefined and the TT projection is not defined; the k=0 mode is excluded
// from the measurement (it is the DC / mean mode anyway).
//
// Input/output O are 3x3 complex matrices in row-major [i*3+j].
// ───────────────────────────────────────────────────────────────────────────
void apply_tt_projector(const std::array<std::complex<double>, 9>& O,
                        double kx, double ky, double kz,
                        std::array<std::complex<double>, 9>& O_tt) {
    const double kmag = std::sqrt(kx * kx + ky * ky + kz * kz);
    if (kmag < 1e-12) {
        O_tt.fill(std::complex<double>(0.0, 0.0));
        return;
    }
    const double khx = kx / kmag, khy = ky / kmag, khz = kz / kmag;
    // P_ij = δ_ij − k̂_i k̂_j   (real, symmetric).
    double P[3][3];
    const double kh[3] = {khx, khy, khz};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            P[i][j] = (i == j ? 1.0 : 0.0) - kh[i] * kh[j];
    // O^TT_ij = ½(P_il P_jm + P_im P_jl) O_lm − ½ P_ij (P_lm O_lm).
    // First the trace-like contraction P_lm O_lm.
    std::complex<double> P_dot_O(0.0, 0.0);
    for (int l = 0; l < 3; ++l)
        for (int m = 0; m < 3; ++m)
            P_dot_O += P[l][m] * O[l * 3 + m];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            std::complex<double> acc(0.0, 0.0);
            for (int l = 0; l < 3; ++l)
                for (int m = 0; m < 3; ++m) {
                    const double w = 0.5 * (P[i][l] * P[j][m] + P[i][m] * P[j][l]);
                    acc += w * O[l * 3 + m];
                }
            acc -= 0.5 * P[i][j] * P_dot_O;
            O_tt[i * 3 + j] = acc;
        }
    }
}

// ───────────────────────────────────────────────────────────────────────────
// Per-tick rank-2 operator fields.
//
// We need, for each lattice site, the symmetric-traceless rank-2 tensor O_ij
// for BOTH pre-registered probe operators (PREREG v2 §5). Each is stored as 6
// real scalar fields (xx,yy,zz,xy,xz,yz) — symmetric, so 6 independent
// components; the trace is removed analytically.
//
// (i′) flux-quadrupole:  O_ij = J_i J_j − (1/3)δ_ij|J|².
//      A direct J⊗J bilinear of the flux vector — no spatial derivatives.
//      Symmetric and traceless by construction; carries genuine helicity-±2
//      content (not k-reducible). Replaces v1's strain-rate operator, which
//      was analytically TT-degenerate.
//
// (ii) stress:  T_ij = (∂_i J_a)(∂_j J_a) − δ_ij L,
//      L = ½|J̇|² − ½C²|∇J|².  J̇ is the engine's wave_vel (the leapfrog
//      momentum: each tick flux += wave_vel, so wave_vel is exactly the
//      per-tick backward difference of flux — i.e. J̇ in tick units).
//      O_ij = symmetric-traceless part of T_ij. T_ij is already symmetric;
//      removing its trace gives the spin-2-projectable composite.
// ───────────────────────────────────────────────────────────────────────────
struct Rank2Field {
    // 6 symmetric components per site, traceless-projected.
    std::vector<double> xx, yy, zz, xy, xz, yz;
    void resize(std::size_t n) {
        xx.assign(n, 0.0); yy.assign(n, 0.0); zz.assign(n, 0.0);
        xy.assign(n, 0.0); xz.assign(n, 0.0); yz.assign(n, 0.0);
    }
};

// Central-difference partial derivatives of the three flux components at a
// site. Returns dJ[a][b] = ∂_b J_a  (a = flux component, b = spatial axis).
// Uses the engine's 6-face neighbor ordering: [+x,-x,+y,-y,+z,-z].
inline void flux_jacobian(const std::vector<ftd::Voxel>& vox,
                          const ftd::Lattice& lat, int idx,
                          double dJ[3][3]) {
    const auto& n = lat.neighbors_6(idx);
    const ftd::Vec3& Jpx = vox[n[0]].flux; const ftd::Vec3& Jmx = vox[n[1]].flux;
    const ftd::Vec3& Jpy = vox[n[2]].flux; const ftd::Vec3& Jmy = vox[n[3]].flux;
    const ftd::Vec3& Jpz = vox[n[4]].flux; const ftd::Vec3& Jmz = vox[n[5]].flux;
    // ∂_x
    dJ[0][0] = (Jpx.x - Jmx.x) * 0.5; dJ[1][0] = (Jpx.y - Jmx.y) * 0.5; dJ[2][0] = (Jpx.z - Jmx.z) * 0.5;
    // ∂_y
    dJ[0][1] = (Jpy.x - Jmy.x) * 0.5; dJ[1][1] = (Jpy.y - Jmy.y) * 0.5; dJ[2][1] = (Jpy.z - Jmy.z) * 0.5;
    // ∂_z
    dJ[0][2] = (Jpz.x - Jmz.x) * 0.5; dJ[1][2] = (Jpz.y - Jmz.y) * 0.5; dJ[2][2] = (Jpz.z - Jmz.z) * 0.5;
}

// Build the flux-quadrupole operator O_ij = J_i J_j − (1/3)δ_ij|J|²
// (PREREG v2 §5, operator (i′)). A direct J⊗J bilinear of the flux vector —
// no spatial derivatives. Symmetric and traceless by construction; a genuine
// rank-2 composite carrying helicity-±2 content (not k-reducible). Replaces
// PREREG v1's strain-rate operator, which was analytically TT-degenerate.
void compute_flux_quadrupole(const ftd::RenderBridge& rb, Rank2Field& out) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int L = lat.size();
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    out.resize(N);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int idx = lat.index(x, y, z);
                const ftd::Vec3& J = vox[idx].flux;
                // |J|² = Jx² + Jy² + Jz².
                const double j2 = J.x * J.x + J.y * J.y + J.z * J.z;
                const double tr3 = j2 / 3.0;  // (1/3)|J|²
                // O_ij = J_i J_j − (1/3)δ_ij|J|².
                out.xx[idx] = J.x * J.x - tr3;
                out.yy[idx] = J.y * J.y - tr3;
                out.zz[idx] = J.z * J.z - tr3;
                out.xy[idx] = J.x * J.y;
                out.xz[idx] = J.x * J.z;
                out.yz[idx] = J.y * J.z;
            }
}

// Build the stress operator: symmetric-traceless part of the Noether stress
//   T_ij = (∂_i J_a)(∂_j J_a) − δ_ij L,  L = ½|J̇|² − ½C²|∇J|².
// The δ_ij L piece is pure-trace; removing the full trace of T_ij drops it
// automatically, so O_ij = [(∂_iJ_a)(∂_jJ_a)]_traceless. We compute it that
// way (numerically identical, and avoids needing L explicitly) but the
// docstring records the derived T_ij for provenance.
void compute_stress(const ftd::RenderBridge& rb, Rank2Field& out) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int L = lat.size();
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    out.resize(N);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int idx = lat.index(x, y, z);
                double dJ[3][3];
                flux_jacobian(vox, lat, idx, dJ);
                // S_ij = (∂_i J_a)(∂_j J_a) = Σ_a dJ[a][i]·dJ[a][j].
                auto Sij = [&](int i, int j) {
                    return dJ[0][i] * dJ[0][j]
                         + dJ[1][i] * dJ[1][j]
                         + dJ[2][i] * dJ[2][j];
                };
                const double sxx = Sij(0, 0);
                const double syy = Sij(1, 1);
                const double szz = Sij(2, 2);
                const double sxy = Sij(0, 1);
                const double sxz = Sij(0, 2);
                const double syz = Sij(1, 2);
                const double tr3 = (sxx + syy + szz) / 3.0;
                out.xx[idx] = sxx - tr3;
                out.yy[idx] = syy - tr3;
                out.zz[idx] = szz - tr3;
                out.xy[idx] = sxy;
                out.xz[idx] = sxz;
                out.yz[idx] = syz;
            }
}

// ───────────────────────────────────────────────────────────────────────────
// A k-space probe: holds the time series of the TT-projected scalar amplitude
// at one wavevector, for one operator. The connected correlator and its
// Prony spectrum are extracted at the end.
// ───────────────────────────────────────────────────────────────────────────
struct ProbeSeries {
    // The 6 independent components of O^TT_ij(k,t), complex, one per tick.
    std::vector<std::array<std::complex<double>, 6>> samples;  // [t][component]
    void clear() { samples.clear(); }
    int size() const { return static_cast<int>(samples.size()); }
};

// Pull the symmetric rank-2 tensor O_ij(k) at a given Fourier index out of 6
// FFT'd component grids, then TT-project it. Returns the 6 independent
// components of O^TT_ij(k).
std::array<std::complex<double>, 6> tt_components_at_k(
    const std::array<std::vector<std::complex<double>>, 6>& Fk,
    int nx, int ny, int nz, int L,
    double* untraced_power = nullptr) {
    auto gidx = [L](int a, int b, int c) {
        auto w = [L](int v) { return ((v % L) + L) % L; };
        return (w(a) * L + w(b)) * L + w(c);
    };
    const int g = gidx(nx, ny, nz);
    // Assemble the full symmetric 3x3 complex matrix.
    std::array<std::complex<double>, 9> O;
    const std::complex<double> Oxx = Fk[0][g];
    const std::complex<double> Oyy = Fk[1][g];
    const std::complex<double> Ozz = Fk[2][g];
    const std::complex<double> Oxy = Fk[3][g];
    const std::complex<double> Oxz = Fk[4][g];
    const std::complex<double> Oyz = Fk[5][g];
    O[0] = Oxx; O[1] = Oxy; O[2] = Oxz;
    O[3] = Oxy; O[4] = Oyy; O[5] = Oyz;
    O[6] = Oxz; O[7] = Oyz; O[8] = Ozz;
    // Reference scale: the FULL (un-TT-projected) rank-2 power Σ_ij |O_ij|²
    // at this k. The caller compares the TT-projected variance against a tiny
    // fraction of this to distinguish an analytic-zero channel from a real
    // (possibly small) signal — see extract_pole's noise_floor argument.
    if (untraced_power) {
        const double w6[6] = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0};
        const std::complex<double> comp[6] = {Oxx, Oyy, Ozz, Oxy, Oxz, Oyz};
        double p = 0.0;
        for (int c = 0; c < 6; ++c) p += w6[c] * std::norm(comp[c]);
        *untraced_power += p;  // accumulated across ticks by the caller
    }
    const KVec k = k_of_index(nx, ny, nz, L);
    std::array<std::complex<double>, 9> Ott;
    apply_tt_projector(O, k.kx, k.ky, k.kz, Ott);
    return {Ott[0], Ott[4], Ott[8], Ott[1], Ott[2], Ott[5]};  // xx,yy,zz,xy,xz,yz
}

// ───────────────────────────────────────────────────────────────────────────
// Connected scalar correlator C_TT(k,τ) = ⟨ O^TT_ij(k,t+τ) O^TT_ij(k,t)* ⟩_c.
//
// "Connected" = subtract the time-mean ⟨O^TT⟩ before correlating. The sum over
// (i,j) is the rank-2 contraction; for a symmetric traceless tensor stored as
// 6 components, Σ_ij A_ij B_ij* = Σ_diag A B* + 2 Σ_offdiag A B* (the off-
// diagonal pairs xy,xz,yz each appear twice in the full 3x3 sum).
//
// The correlator is real and even in τ by construction (autocorrelation of a
// stationary signal). Returns C[τ] for τ = 0..max_tau-1.
// ───────────────────────────────────────────────────────────────────────────
std::vector<double> connected_tt_correlator(const ProbeSeries& ps, int max_tau) {
    const int T = ps.size();
    if (max_tau < 0 || max_tau > T / 2) max_tau = T / 2;
    if (T < 4) return {};
    // Time-mean per component (connected subtraction).
    std::array<std::complex<double>, 6> mean;
    mean.fill(std::complex<double>(0.0, 0.0));
    for (const auto& s : ps.samples)
        for (int c = 0; c < 6; ++c) mean[c] += s[c];
    for (int c = 0; c < 6; ++c) mean[c] /= static_cast<double>(T);
    // Component weights for the Σ_ij contraction (diag ×1, offdiag ×2).
    const double w[6] = {1.0, 1.0, 1.0, 2.0, 2.0, 2.0};
    std::vector<double> C(max_tau, 0.0);
    for (int tau = 0; tau < max_tau; ++tau) {
        const int count = T - tau;
        double acc = 0.0;
        for (int t = 0; t < count; ++t) {
            std::complex<double> dot(0.0, 0.0);
            for (int c = 0; c < 6; ++c) {
                const std::complex<double> a = ps.samples[t + tau][c] - mean[c];
                const std::complex<double> b = ps.samples[t][c] - mean[c];
                dot += w[c] * a * std::conj(b);
            }
            acc += dot.real();  // imag part averages to ~0 for a real correlator
        }
        C[tau] = acc / count;
    }
    return C;
}

// Connected transverse-vector (spin-1) correlator of J itself — the CONTROL.
// At wavevector k, the transverse part of J(k) is J_T = (δ − k̂k̂)·J. The
// connected autocorrelator of |J_T|² components recovers the spin-1 pole at
// ω(k) = 2C|sin(k/2)|. We store the 2 transverse components and contract.
std::vector<double> connected_transverse_vector_correlator(
    const std::vector<std::array<std::complex<double>, 3>>& Jk_series,
    double kx, double ky, double kz, int max_tau) {
    const int T = static_cast<int>(Jk_series.size());
    if (max_tau < 0 || max_tau > T / 2) max_tau = T / 2;
    if (T < 4) return {};
    const double kmag = std::sqrt(kx * kx + ky * ky + kz * kz);
    // Build the transverse projector P = δ − k̂k̂.
    double P[3][3];
    if (kmag < 1e-12) {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) P[i][j] = (i == j) ? 1.0 : 0.0;
    } else {
        const double kh[3] = {kx / kmag, ky / kmag, kz / kmag};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                P[i][j] = (i == j ? 1.0 : 0.0) - kh[i] * kh[j];
    }
    // Project each sample to its transverse part.
    std::vector<std::array<std::complex<double>, 3>> JT(T);
    for (int t = 0; t < T; ++t) {
        for (int i = 0; i < 3; ++i) {
            std::complex<double> acc(0.0, 0.0);
            for (int j = 0; j < 3; ++j) acc += P[i][j] * Jk_series[t][j];
            JT[t][i] = acc;
        }
    }
    // Connected subtraction.
    std::array<std::complex<double>, 3> mean;
    mean.fill(std::complex<double>(0.0, 0.0));
    for (const auto& s : JT)
        for (int i = 0; i < 3; ++i) mean[i] += s[i];
    for (int i = 0; i < 3; ++i) mean[i] /= static_cast<double>(T);
    std::vector<double> C(max_tau, 0.0);
    for (int tau = 0; tau < max_tau; ++tau) {
        const int count = T - tau;
        double acc = 0.0;
        for (int t = 0; t < count; ++t) {
            std::complex<double> dot(0.0, 0.0);
            for (int i = 0; i < 3; ++i) {
                const std::complex<double> a = JT[t + tau][i] - mean[i];
                const std::complex<double> b = JT[t][i] - mean[i];
                dot += a * std::conj(b);
            }
            acc += dot.real();
        }
        C[tau] = acc / count;
    }
    return C;
}

// ───────────────────────────────────────────────────────────────────────────
// Pole extraction from a connected correlator.
//
// A clean propagating pole means C(τ) ≈ A·cos(ω τ)·e^{−Γ τ}. We extract:
//   - ω from the dominant peak of the temporal power spectrum (FFT of C(τ)),
//   - Γ from the decay of the oscillation envelope (log-fit of successive
//     extrema magnitudes),
//   - a pole-vs-continuum diagnostic: the spectral peak's prominence —
//     (peak power) / (median off-peak power). A single sharp pole has a
//     prominence ≫ 1; a broad two-particle continuum has prominence ~ O(1).
//
// The Prony two-state extractor (spectrum_extraction.h) is run in parallel as
// a cross-check — for an oscillatory correlator it generically returns
// "complex roots" (which is itself the signature of an oscillation rather
// than a pure decay), so its `valid` flag is reported but the FFT-peak ω is
// the primary estimate. We also report the GEVP-free Prony failure string so
// the analyst can see why.
// ───────────────────────────────────────────────────────────────────────────
struct PoleFit {
    double omega        = 0.0;   // dominant angular frequency (rad/tick)
    double gamma        = 0.0;   // decay rate (1/tick); 0 if no clean envelope
    double prominence   = 0.0;   // peak/median spectral power — pole sharpness
    double peak_power   = 0.0;
    double signal_power = 0.0;   // C(0) — the connected variance of the channel
    bool   prony_valid  = false;
    const char* prony_note = "";
    bool   resolved     = false; // true if a real signal was found (above noise floor)
    bool   has_signal   = false; // true if C(0) is meaningfully above the noise floor
};

// extract_pole — fit (ω, Γ, prominence) to a connected correlator C(τ).
//
// `noise_floor` is an absolute power scale: a channel whose zero-lag variance
// C(0) sits below noise_floor is treated as carrying NO signal. Without this
// gate a genuinely empty channel reports a meaningless prominence built from
// floating-point noise. Both PREREG v2 probe operators — the flux-quadrupole
// J⊗J and the stress ∂J⊗∂J — are genuine rank-2 bilinears with non-vanishing
// TT projections, so they are expected to clear the floor; the gate is kept as
// a generic safeguard (it is what surfaced the v1 strain-rate operator's
// analytic TT-degeneracy before that operator was dropped in PREREG v2).
PoleFit extract_pole(const std::vector<double>& C, double noise_floor) {
    PoleFit fit;
    const int N = static_cast<int>(C.size());
    if (N < 8) { fit.prony_note = "correlator too short"; return fit; }
    fit.signal_power = (N > 0) ? std::abs(C[0]) : 0.0;
    fit.has_signal = (fit.signal_power > noise_floor);
    if (!fit.has_signal) {
        // Channel is at the floating-point noise floor — no propagating
        // structure to extract. Report it honestly rather than fitting noise.
        fit.prony_note = "no signal (C(0) below noise floor — analytic-zero channel)";
        return fit;
    }

    // --- ω from the temporal power spectrum of C(τ). ---
    auto psd = ftd::power_spectrum(C);  // |FFT|²/N, bins 0..Nfft/2
    int nfft = 1;
    while (nfft < N) nfft <<= 1;
    // Find the dominant non-DC bin.
    int peak_bin = 1;
    double peak_pw = 0.0;
    for (int b = 1; b < static_cast<int>(psd.size()); ++b) {
        if (psd[b] > peak_pw) { peak_pw = psd[b]; peak_bin = b; }
    }
    fit.omega = 2.0 * M_PI * peak_bin / nfft;
    fit.peak_power = peak_pw;

    // --- pole-vs-continuum: spectral prominence = peak / median(off-peak). ---
    std::vector<double> offpeak;
    offpeak.reserve(psd.size());
    for (int b = 1; b < static_cast<int>(psd.size()); ++b) {
        if (std::abs(b - peak_bin) <= 1) continue;  // exclude the peak ± 1 bin
        offpeak.push_back(psd[b]);
    }
    double median = 0.0;
    if (!offpeak.empty()) {
        std::sort(offpeak.begin(), offpeak.end());
        median = offpeak[offpeak.size() / 2];
    }
    fit.prominence = (median > 1e-300) ? (peak_pw / median) : 0.0;
    fit.resolved = (peak_pw > 0.0);

    // --- Γ from the oscillation envelope: collect |extrema| of C(τ) and
    //     fit log|extremum| vs τ by least squares. A propagating pole gives
    //     a clean negative slope (= −Γ); a non-decaying or noisy correlator
    //     gives slope ≈ 0 or a bad fit. ---
    std::vector<std::pair<int, double>> extrema;
    for (int t = 1; t < N - 1; ++t) {
        const double a = C[t - 1], b = C[t], c = C[t + 1];
        if ((b > a && b > c) || (b < a && b < c)) {
            const double mag = std::abs(b);
            if (mag > 1e-300) extrema.emplace_back(t, std::log(mag));
        }
    }
    if (extrema.size() >= 3) {
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        const double n = static_cast<double>(extrema.size());
        for (auto& e : extrema) {
            sx += e.first; sy += e.second;
            sxx += static_cast<double>(e.first) * e.first;
            sxy += static_cast<double>(e.first) * e.second;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-300) {
            const double slope = (n * sxy - sx * sy) / denom;
            fit.gamma = (slope < 0.0) ? -slope : 0.0;  // decay ⇒ slope<0
        }
    }

    // --- Prony cross-check (reports validity + note; not the primary ω). ---
    ftd::TwoStateSpectrum prony = ftd::extract_two_state_prony(C, /*tau0=*/2);
    fit.prony_valid = prony.valid;
    fit.prony_note  = prony.valid ? "two-state real roots"
                                  : (prony.failure_reason ? prony.failure_reason : "");
    return fit;
}

// ───────────────────────────────────────────────────────────────────────────
// Lay down the fixed broadband J perturbation on a vacuum lattice.
//
// J(x) = amp · Σ_modes  ê_mode · sin(k_mode · x + φ_mode)
// where k_mode runs over n=1..kPerturbModesPerAxis along each of the 3 axes,
// and ê_mode / φ_mode are deterministic from the kPerturbSeed LCG. This
// excites a broad band of small-|k| modes simultaneously (prereg §5: "a fixed
// sum of plane waves across many k"). Amplitude kept small (kPerturbAmplitude)
// so the substrate stays in the weakly-interacting regime.
// ───────────────────────────────────────────────────────────────────────────
void seed_broadband_perturbation(ftd::RenderBridge& rb, int L,
                                 unsigned int seed, double amp) {
    Lcg lcg(seed);
    struct Mode { double kx, ky, kz, ex, ey, ez, phase; };
    std::vector<Mode> modes;
    // Axis-aligned plane waves, n = 1..kPerturbModesPerAxis on each axis.
    for (int axis = 0; axis < 3; ++axis) {
        for (int n = 1; n <= kPerturbModesPerAxis; ++n) {
            const double k = 2.0 * M_PI * n / L;
            Mode m{};
            m.kx = (axis == 0) ? k : 0.0;
            m.ky = (axis == 1) ? k : 0.0;
            m.kz = (axis == 2) ? k : 0.0;
            // Random unit polarization vector (deterministic).
            double ex = lcg.sym(), ey = lcg.sym(), ez = lcg.sym();
            double norm = std::sqrt(ex * ex + ey * ey + ez * ez);
            if (norm < 1e-9) { ex = 1.0; ey = ez = 0.0; norm = 1.0; }
            m.ex = ex / norm; m.ey = ey / norm; m.ez = ez / norm;
            m.phase = 2.0 * M_PI * lcg.uniform();
            modes.push_back(m);
        }
    }
    // Also a handful of body-diagonal modes so [110]/[111] channels are seeded.
    for (int n = 1; n <= kPerturbModesPerAxis; ++n) {
        const double k = 2.0 * M_PI * n / L;
        // [110]
        {
            Mode m{}; m.kx = k; m.ky = k; m.kz = 0.0;
            double ex = lcg.sym(), ey = lcg.sym(), ez = lcg.sym();
            double norm = std::sqrt(ex * ex + ey * ey + ez * ez);
            if (norm < 1e-9) { ex = 0.0; ey = 0.0; ez = 1.0; norm = 1.0; }
            m.ex = ex / norm; m.ey = ey / norm; m.ez = ez / norm;
            m.phase = 2.0 * M_PI * lcg.uniform();
            modes.push_back(m);
        }
        // [111]
        {
            Mode m{}; m.kx = k; m.ky = k; m.kz = k;
            double ex = lcg.sym(), ey = lcg.sym(), ez = lcg.sym();
            double norm = std::sqrt(ex * ex + ey * ey + ez * ez);
            if (norm < 1e-9) { ex = 1.0; ey = -1.0; ez = 0.0; norm = std::sqrt(2.0); }
            m.ex = ex / norm; m.ey = ey / norm; m.ez = ez / norm;
            m.phase = 2.0 * M_PI * lcg.uniform();
            modes.push_back(m);
        }
    }
    // Write the superposition into every voxel's flux.
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                ftd::Vec3 J{0.0, 0.0, 0.0};
                for (const auto& m : modes) {
                    const double ph = m.kx * x + m.ky * y + m.kz * z + m.phase;
                    const double s = std::sin(ph);
                    J.x += m.ex * s;
                    J.y += m.ey * s;
                    J.z += m.ez * s;
                }
                J.x *= amp; J.y *= amp; J.z *= amp;
                rb.inject_flux(x, y, z, J);
            }
}

// ───────────────────────────────────────────────────────────────────────────
// CSV emission.
// ───────────────────────────────────────────────────────────────────────────
void emit_csv_header() {
    std::printf("sector,operator,direction,n_index,L,kx,ky,kz,kmag,"
                "omega,gamma,prominence,peak_power,signal_power,has_signal,"
                "resolved,prony_valid,"
                "control_omega_spin1,control_omega_predicted,prony_note\n");
}

struct ResultRow {
    const char* sector;     // "spin2" or "spin1_control"
    const char* op_name;    // "flux_quadrupole" / "stress" / "transverse_J"
    const char* direction;
    int n_index;
    int L;
    KVec k;
    PoleFit fit;
    double control_omega          = 0.0;  // measured spin-1 ω at this k
    double control_omega_predicted = 0.0; // 2C|sin(k/2)|
};

void emit_csv_row(const ResultRow& r) {
    std::printf("%s,%s,%s,%d,%d,%.6f,%.6f,%.6f,%.6f,"
                "%.6e,%.6e,%.6e,%.6e,%.6e,%d,%d,%d,%.6e,%.6e,\"%s\"\n",
                r.sector, r.op_name, r.direction, r.n_index, r.L,
                r.k.kx, r.k.ky, r.k.kz, r.k.mag(),
                r.fit.omega, r.fit.gamma, r.fit.prominence, r.fit.peak_power,
                r.fit.signal_power, r.fit.has_signal ? 1 : 0,
                r.fit.resolved ? 1 : 0, r.fit.prony_valid ? 1 : 0,
                r.control_omega, r.control_omega_predicted,
                r.fit.prony_note ? r.fit.prony_note : "");
    std::fflush(stdout);
}

}  // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════
int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    int L          = kDefaultL;
    int equil      = kDefaultEquil;
    int window     = kDefaultWindow;
    unsigned int seed = kPerturbSeed;
    double amp     = kPerturbAmplitude;
    std::string output_dir;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L      = std::atoi(a.c_str() + 4);
        else if (a.rfind("--equil=", 0) == 0)      equil  = std::atoi(a.c_str() + 8);
        else if (a.rfind("--window=", 0) == 0)     window = std::atoi(a.c_str() + 9);
        else if (a.rfind("--seed=", 0) == 0)       seed   = static_cast<unsigned int>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a.rfind("--amp=", 0) == 0)        amp    = std::atof(a.c_str() + 6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    // L must be a power of two for the radix-2 FFT.
    {
        int p = 1; while (p < L) p <<= 1;
        if (p != L) {
            std::cerr << "[FATAL] L=" << L << " is not a power of two; "
                         "the radix-2 FFT in spectral.h requires L ∈ {16,32,64,128,...}\n";
            return 2;
        }
    }

    std::cerr << "================================================================\n";
    std::cerr << "  Frontier 4 / Step 4a-ii — emergent TT (spin-2) pole campaign\n";
    std::cerr << "  PREREG: docs/theory/10_eft_program/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md\n";
    std::cerr << "          tag preregister-graviton-substrate-mode-v2\n";
    std::cerr << "================================================================\n";
    std::cerr << "  L=" << L << "  equilibration=" << equil
              << "  measurement window=" << window << " ticks\n";
    std::cerr << "  perturbation: fixed broadband plane-wave sum, seed=0x"
              << std::hex << seed << std::dec << ", amplitude=" << amp << "\n";
    std::cerr << "  C_WAVE = " << kC
              << "  (spin-1 control pole: ω = 2C|sin(k/2)|)\n";
    std::cerr << "  Probe operators (prereg v2 §5): (i') flux-quadrupole  (ii) stress\n";
    std::cerr << "  NO Outcome verdict is emitted — measurement data only.\n\n";

    // ── Build the interacting substrate with the prereg §8 toggle set. ──
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();           // start from a clean slate
    // The six rules (SPEC_ENGINE.md §1) + the nonlinear flux coupling:
    rb.toggles.wave_propagation = true; // Rule 1
    rb.toggles.coupling         = true; // Rule 2 — the nonlinear state↔flux coupling
    rb.toggles.gauss_projection = true; // Rule 3
    rb.toggles.genesis          = true; // Rule 4 — interacting substrate
    rb.toggles.forces           = true; // Rule 5
    rb.toggles.gravity          = true; // Rule 5
    rb.toggles.poisson_coulomb  = true; // Rule 5
    rb.toggles.lorentz_force    = true; // Rule 5
    rb.toggles.movement         = true; // Rule 6
    rb.toggles.damping          = true; // Rule 1 write-phase dissipation (core ON)
    rb.toggles.selective_damping = true;// vacuum lossless ⇒ clean propagator
    // Everything phenomenological stays OFF (disable_all() left it OFF):
    //   dual_substrate, weak_transmutation, color_forces, strong_force,
    //   larmor_radiation, triad_binding, pair_production, exchange_force,
    //   latency_field, langevin, emergent_forces, evaporation, ...

    std::string toggle_err;
    if (!rb.toggles.validate(&toggle_err)) {
        std::cerr << "[FATAL] toggle validation failed:\n" << toggle_err;
        return 2;
    }
    // Genesis RNG: seed from the same fixed seed for full reproducibility.
    rb.seed_rng(seed);
    // Tighten the Poisson solve a little — the default 6 SOR iters are tuned
    // for interactive frame rates; a measurement wants the Gauss constraint
    // converged. (This is a solver-accuracy knob, NOT a physics toggle.)
    rb.set_sor_iterations(20);

    // Echo the resolved toggle state to stderr for the hash-reference record.
    std::cerr << "  Resolved toggle set (prereg §8 hash-reference):\n";
    auto echo = [&](const char* n, bool v) {
        std::cerr << "    " << (v ? "[ON ] " : "[off] ") << n << "\n";
    };
    echo("wave_propagation", rb.toggles.wave_propagation);
    echo("coupling",         rb.toggles.coupling);
    echo("gauss_projection", rb.toggles.gauss_projection);
    echo("genesis",          rb.toggles.genesis);
    echo("forces",           rb.toggles.forces);
    echo("gravity",          rb.toggles.gravity);
    echo("poisson_coulomb",  rb.toggles.poisson_coulomb);
    echo("lorentz_force",    rb.toggles.lorentz_force);
    echo("movement",         rb.toggles.movement);
    echo("damping",          rb.toggles.damping);
    echo("selective_damping",rb.toggles.selective_damping);
    echo("dual_substrate (OFF=single-substrate path)", rb.toggles.dual_substrate);
    echo("weak_transmutation", rb.toggles.weak_transmutation);
    std::cerr << "\n";

    // ── Initial state: vacuum + fixed broadband J perturbation. ──
    seed_broadband_perturbation(rb, L, seed, amp);
    std::cerr << "  Broadband perturbation seeded. Equilibrating " << equil
              << " ticks...\n";

    // ── Equilibration. ──
    for (int t = 0; t < equil; ++t) {
        rb.run(1);
        if ((t + 1) % 50 == 0 || t + 1 == equil)
            std::cerr << "    equilibration tick " << (t + 1) << "/" << equil << "\n";
    }

    // ── Measurement window. ──
    // For each k-point along each direction we accumulate a ProbeSeries for
    // both rank-2 operators, plus a Jk series for the spin-1 control.
    struct KSlot {
        Direction dir;
        int n;                // 1..kNumKPoints
        int nx, ny, nz;       // Fourier index = n·direction
        KVec k;
        ProbeSeries quad;     // operator (i') flux-quadrupole
        ProbeSeries stress;   // operator (ii)
        std::vector<std::array<std::complex<double>, 3>> Jk;  // control
        // Time-summed un-TT-projected rank-2 power for each operator — the
        // reference scale that distinguishes an analytic-zero TT channel from
        // a real small signal (see extract_pole noise_floor).
        double quad_untraced_power   = 0.0;
        double stress_untraced_power = 0.0;
    };
    std::vector<KSlot> slots;
    for (const auto& d : kDirections)
        for (int n = 1; n <= kNumKPoints; ++n) {
            KSlot s;
            s.dir = d; s.n = n;
            s.nx = n * d.dx; s.ny = n * d.dy; s.nz = n * d.dz;
            s.k  = k_of_index(s.nx, s.ny, s.nz, L);
            slots.push_back(std::move(s));
        }

    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    std::cerr << "  Measuring " << window << " ticks over " << slots.size()
              << " k-points (3 directions × " << kNumKPoints << ")...\n";

    Rank2Field quad_field, stress_field_;
    std::array<std::vector<std::complex<double>>, 6> Fquad, Fstress;
    for (int c = 0; c < 6; ++c) { Fquad[c].resize(N); Fstress[c].resize(N); }
    std::array<std::vector<std::complex<double>>, 3> Fjx;  // FFT of Jx,Jy,Jz
    for (int c = 0; c < 3; ++c) Fjx[c].resize(N);

    for (int t = 0; t < window; ++t) {
        rb.run(1);
        const auto& vox = rb.voxels();
        const auto& lat = rb.lattice();

        // --- operator (i'): flux-quadrupole ---
        compute_flux_quadrupole(rb, quad_field);
        // --- operator (ii): stress ---
        compute_stress(rb, stress_field_);

        // FFT each of the 6 components of both operators.
        auto load_and_fft = [&](const std::vector<double>& src,
                                std::vector<std::complex<double>>& dst) {
            for (std::size_t i = 0; i < N; ++i) dst[i] = std::complex<double>(src[i], 0.0);
            fft3d(dst, L, /*inverse=*/false);
        };
        load_and_fft(quad_field.xx, Fquad[0]);
        load_and_fft(quad_field.yy, Fquad[1]);
        load_and_fft(quad_field.zz, Fquad[2]);
        load_and_fft(quad_field.xy, Fquad[3]);
        load_and_fft(quad_field.xz, Fquad[4]);
        load_and_fft(quad_field.yz, Fquad[5]);
        load_and_fft(stress_field_.xx, Fstress[0]);
        load_and_fft(stress_field_.yy, Fstress[1]);
        load_and_fft(stress_field_.zz, Fstress[2]);
        load_and_fft(stress_field_.xy, Fstress[3]);
        load_and_fft(stress_field_.xz, Fstress[4]);
        load_and_fft(stress_field_.yz, Fstress[5]);

        // FFT the raw flux components for the spin-1 control.
        for (int comp = 0; comp < 3; ++comp) {
            for (int x = 0; x < L; ++x)
                for (int y = 0; y < L; ++y)
                    for (int z = 0; z < L; ++z) {
                        const int idx = lat.index(x, y, z);
                        const ftd::Vec3& J = vox[idx].flux;
                        const double v = (comp == 0) ? J.x : (comp == 1) ? J.y : J.z;
                        Fjx[comp][idx] = std::complex<double>(v, 0.0);
                    }
            fft3d(Fjx[comp], L, /*inverse=*/false);
        }

        // Sample every k-slot.
        for (auto& s : slots) {
            // TT-projected rank-2 components for each operator. The 5th arg
            // accumulates the un-projected rank-2 power (reference scale).
            s.quad.samples.push_back(
                tt_components_at_k(Fquad, s.nx, s.ny, s.nz, L, &s.quad_untraced_power));
            s.stress.samples.push_back(
                tt_components_at_k(Fstress, s.nx, s.ny, s.nz, L, &s.stress_untraced_power));
            // Raw J(k) for the control.
            auto gidx = [L](int a, int b, int c) {
                auto w = [L](int v) { return ((v % L) + L) % L; };
                return (w(a) * L + w(b)) * L + w(c);
            };
            const int g = gidx(s.nx, s.ny, s.nz);
            s.Jk.push_back({Fjx[0][g], Fjx[1][g], Fjx[2][g]});
        }

        if ((t + 1) % 64 == 0 || t + 1 == window)
            std::cerr << "    measurement tick " << (t + 1) << "/" << window << "\n";
    }

    // ── Analysis: per k-slot, extract poles for both operators + control. ──
    std::cerr << "\n  Extracting poles (FFT-peak ω, envelope Γ, prominence)...\n";
    emit_csv_header();
    std::vector<ResultRow> rows;
    const int max_tau = window / 2;

    // Track control recovery quality for the smoke-test acceptance gate.
    int control_total = 0, control_recovered = 0;

    for (auto& s : slots) {
        const double kmag = s.k.mag();
        const double predicted_spin1 = 2.0 * kC * std::abs(std::sin(kmag / 2.0));

        // Per-operator noise floor: 1e-20 × the mean un-TT-projected rank-2
        // power per tick. A TT-projected channel whose connected variance
        // C(0) sits below this is an analytic-zero channel, not a small
        // signal. The control always carries a real signal, so its floor is
        // a negligible absolute constant.
        const double inv_w = 1.0 / static_cast<double>(window);
        const double quad_floor   = 1e-20 * s.quad_untraced_power * inv_w;
        const double stress_floor = 1e-20 * s.stress_untraced_power * inv_w;
        const double control_floor = 1e-300;

        // --- spin-1 control ---
        auto Cctrl = connected_transverse_vector_correlator(
            s.Jk, s.k.kx, s.k.ky, s.k.kz, max_tau);
        PoleFit ctrl_fit = extract_pole(Cctrl, control_floor);
        ResultRow rc;
        rc.sector = "spin1_control"; rc.op_name = "transverse_J";
        rc.direction = s.dir.name; rc.n_index = s.n; rc.L = L; rc.k = s.k;
        rc.fit = ctrl_fit;
        rc.control_omega = ctrl_fit.omega;
        rc.control_omega_predicted = predicted_spin1;
        rows.push_back(rc);
        emit_csv_row(rc);

        // Control acceptance: measured ω within 20% of 2C|sin(k/2)|.
        ++control_total;
        const bool ctrl_ok =
            ctrl_fit.resolved && predicted_spin1 > 1e-6 &&
            std::abs(ctrl_fit.omega - predicted_spin1) / predicted_spin1 < 0.20;
        if (ctrl_ok) ++control_recovered;

        // --- spin-2: operator (i') flux-quadrupole ---
        auto Cquad = connected_tt_correlator(s.quad, max_tau);
        PoleFit quad_fit = extract_pole(Cquad, quad_floor);
        ResultRow r1;
        r1.sector = "spin2"; r1.op_name = "flux_quadrupole";
        r1.direction = s.dir.name; r1.n_index = s.n; r1.L = L; r1.k = s.k;
        r1.fit = quad_fit;
        r1.control_omega = ctrl_fit.omega;
        r1.control_omega_predicted = predicted_spin1;
        rows.push_back(r1);
        emit_csv_row(r1);

        // --- spin-2: operator (ii) stress ---
        auto Cstress = connected_tt_correlator(s.stress, max_tau);
        PoleFit stress_fit = extract_pole(Cstress, stress_floor);
        ResultRow r2;
        r2.sector = "spin2"; r2.op_name = "stress";
        r2.direction = s.dir.name; r2.n_index = s.n; r2.L = L; r2.k = s.k;
        r2.fit = stress_fit;
        r2.control_omega = ctrl_fit.omega;
        r2.control_omega_predicted = predicted_spin1;
        rows.push_back(r2);
        emit_csv_row(r2);

        auto chan = [](const PoleFit& f) -> std::string {
            if (!f.has_signal) return std::string("[no signal]");
            char buf[96];
            std::snprintf(buf, sizeof(buf), "ω=%.4f prom=%.3g", f.omega, f.prominence);
            return std::string(buf);
        };
        std::cerr << "    " << s.dir.name << " n=" << s.n
                  << "  |k|=" << kmag
                  << " | spin1 ctrl ω=" << ctrl_fit.omega
                  << " (pred " << predicted_spin1 << ")"
                  << (ctrl_ok ? " OK" : " --")
                  << " | quad " << chan(quad_fit)
                  << " | stress " << chan(stress_fit) << "\n";
    }

    // ── Smoke-test summary + control gate. ──
    std::cerr << "\n  ── Summary ──────────────────────────────────────────────\n";
    std::cerr << "  spin-1 control: " << control_recovered << "/" << control_total
              << " k-points recovered ω within 20% of 2C|sin(k/2)|.\n";
    std::cerr << "  (The control must recover cleanly or the correlator\n";
    std::cerr << "   machinery is suspect — prereg §5 self-validation.)\n";
    std::cerr << "  spin-2 sectors: measurement data emitted as CSV above.\n";
    std::cerr << "  Verdict (Outcome A / B / Indeterminate) is NOT applied here —\n";
    std::cerr << "  it is decided afterward against PREREG §6 / §7.\n";

    // ── Optional artifacts. ──
    if (!output_dir.empty()) {
        std::error_code ec;
        fs::create_directories(output_dir, ec);
        std::ofstream csv(fs::path(output_dir) / "tt_correlator.csv");
        if (csv) {
            csv << "sector,operator,direction,n_index,L,kx,ky,kz,kmag,"
                   "omega,gamma,prominence,peak_power,signal_power,has_signal,"
                   "resolved,prony_valid,"
                   "control_omega_spin1,control_omega_predicted,prony_note\n";
            for (const auto& r : rows) {
                csv << r.sector << "," << r.op_name << "," << r.direction << ","
                    << r.n_index << "," << r.L << ","
                    << r.k.kx << "," << r.k.ky << "," << r.k.kz << "," << r.k.mag()
                    << "," << r.fit.omega << "," << r.fit.gamma << ","
                    << r.fit.prominence << "," << r.fit.peak_power << ","
                    << r.fit.signal_power << "," << (r.fit.has_signal ? 1 : 0) << ","
                    << (r.fit.resolved ? 1 : 0) << "," << (r.fit.prony_valid ? 1 : 0)
                    << "," << r.control_omega << "," << r.control_omega_predicted
                    << ",\"" << (r.fit.prony_note ? r.fit.prony_note : "") << "\"\n";
            }
        }
        std::ofstream meta(fs::path(output_dir) / "meta.json");
        if (meta) {
            meta << "{\n";
            meta << "  \"campaign\": \"graviton_tt_correlator\",\n";
            meta << "  \"prereg\": \"docs/theory/10_eft_program/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md\",\n";
            meta << "  \"prereg_tag\": \"preregister-graviton-substrate-mode-v2\",\n";
            meta << "  \"step\": \"Frontier 4 / Step 4a-ii\",\n";
            meta << "  \"L\": " << L << ",\n";
            meta << "  \"equilibration_ticks\": " << equil << ",\n";
            meta << "  \"measurement_window\": " << window << ",\n";
            meta << "  \"perturbation_seed\": " << seed << ",\n";
            meta << "  \"perturbation_amplitude\": " << amp << ",\n";
            meta << "  \"C_wave\": " << kC << ",\n";
            meta << "  \"control_recovered\": " << control_recovered << ",\n";
            meta << "  \"control_total\": " << control_total << ",\n";
            meta << "  \"note\": \"measurement data only — no Outcome verdict\"\n";
            meta << "}\n";
        }
        std::cerr << "  artifacts → " << output_dir << "\n";
    }

    // Exit code: 0 if the run completed and the control was recovered at the
    // majority of k-points (correlator machinery sane). A failed control is a
    // harness problem, not a physics result — surface it as a non-zero exit so
    // CTest flags it, WITHOUT implying anything about the spin-2 question.
    if (control_total > 0 && control_recovered * 2 < control_total) {
        std::cerr << "\n  [WARN] spin-1 control recovered at fewer than half the\n";
        std::cerr << "         k-points — correlator machinery needs review before\n";
        std::cerr << "         the canonical run. (Not a statement about spin-2.)\n";
        return 1;
    }
    std::cerr << "\n  [OK] Campaign completed; spin-1 control recovered.\n";
    return 0;
}
