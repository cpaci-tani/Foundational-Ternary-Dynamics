/**
 * @file campaign_gnc_qij.cpp
 * @brief FTD-0349 §9 / FTD-0354 — the GNC-w discriminator: the member-site
 *        gradient quadratic form Q_ij measured on constructed, locked,
 *        Gauss-dressed engine clusters.
 *
 * Frozen instrument for PREREG_GNC_QIJ_v1 (LEDGER row minted by the controller
 * on verdict). Pre-registration:
 *   docs/theory/03_derivations/foundational_mechanics/PREREG_GNC_QIJ_v1.md
 *
 * THE QUESTION (FTD-0349 §9). The FTD-0349 collective-coordinate reduction of
 * cluster inertia N·M_REST is [DERIVED conditional on GNC], the Gradient-
 * Normalization Condition. Nothing in the action or the Gauss constraint
 * forces GNC; whether real engine cluster profiles realize it is [OPEN]. The
 * read-only diagnostic (FTD-0349 Eq. Q):
 *
 *   Q_ij = (1 / (N·K_B²)) · Σ_{x∈support} Σ_a (Δ⁺_i J_a)(x)·(Δ⁺_j J_a)(x)
 *
 * with FORWARD differences Δ⁺_i J_a(x) = J_a(x+e_i) − J_a(x) (periodic wrap),
 * summed over TWO stated supports reported separately:
 *   (m) the cluster support = the N member sites;
 *   (s) the dressing shell  = non-member sites within Chebyshev (Moore)
 *       distance ≤ 3 of any member.
 * Plus an all-site diagnostic sum (FTD-0349 Eq. 4 exact trace identity:
 * for the minimal Coulomb dressing, Σ_allsites Σ|Δ⁺J|² = N·q²·(1−N/L³)).
 *
 * SCOPE LIMIT (FTD-0354 §4.4 — say it in the instrument): Q_ij gates the
 * SUMMED form GNC-w ONLY. GNC-w is necessary-not-sufficient for pointwise
 * GNC-s (two anisotropic sites can sum isotropic); a Q ≈ δ result confirms
 * GNC-w without licensing any inference to the affine texture family
 * (FTD-0354 §5.2 — lattice GNC-s folds exist, GNC realization need not look
 * affine). The full γ_FTD resummation needs GNC-s, which this instrument
 * cannot adjudicate.
 *
 * FROZEN PREDICTIONS TO DISCRIMINATE (FTD-0349 §4/§5/§9):
 *   Minimal-Coulomb dressing: all-site raw trace ≈ N·q²(1−N/L³) exactly
 *     (q = 1 lattice charge unit); member-summed per-axis Q_ii ≈ 0.39–0.46
 *     × q²/K_B²·(1/3)·... i.e. member Q_trace ≈ 1.2–1.4, N-DRIFTING upward,
 *     with ~87% M_xx vs M_yy anisotropy for the 1×1 rod (FTD-0349 T5d/T5e).
 *   GNC-w: Q_ij = δ_ij on the member support — trace = 3 (raw Σ = 3N·K_B²),
 *     isotropic for EVERY shape, N-flat.
 *
 * CONFIG (constructed clusters; CPU deterministic, seed 42):
 *   Toggles ON : wave_propagation, coupling, gauss_projection, damping
 *                (selective_damping OFF — vacuum damps too, so transients
 *                relax and the dressed profile equilibrates).
 *   Toggles OFF: genesis (cluster stays exactly the constructed N members),
 *                forces, movement, dual_substrate, langevin, everything else.
 *   Golden-neutral: this campaign only calls public API (inject_particle,
 *   voxel_at, run, voxels), never touches default toggles, and runs default-
 *   OFF research toggles nowhere. Golden hash 0xb604d81a3d79366e unaffected.
 *
 * GEOMETRIES (frozen; offsets placed centered in the box):
 *   cube   : edge k ∈ {2,3,4}                       → N ∈ {8,27,64}
 *   rod    : 8×1×1, 27×1×1, 16×2×2                  → N ∈ {8,27,64}
 *   lshape : arms 4+4 (1×1), 14+13 (1×1), 32+32 (2×2) → N ∈ {8,27,64}
 *
 * EQUILIBRATION (frozen gate E1): compare consecutive 64-tick window means of
 * S_m(t) = Σ_members Σ|Δ⁺J|²; converged when relative change < 1e-6; cap
 * --max-ticks (sweep default 20000). Not converged ⇒ row INVALID.
 * MEASUREMENT: time-averaged Q over the final 256 ticks (one sample per tick).
 *
 * GATES (pre-reg §3): E1 equilibration; E2 Gauss residual
 * (energy_audit().max_gauss_error < 1e-6 at measurement start); E3 cluster
 * integrity (manifested count == N, every member state=+1 & locked at
 * measurement end); E4 determinism (config #1 re-run bit-identical Q).
 *
 * OUTPUT: stdout ROW lines + CSV under --output-dir
 * (default engine/results/gnc_qij/). THIS RUNNER REPORTS MEASUREMENTS; IT
 * DOES NOT ADJUDICATE THE VERDICT (outcome table lives in the pre-reg; the
 * verdict is written in a separate ANALYSIS doc). Zero promotions: FTD-0110 /
 * FTD-0250 move only via the pre-registered outcomes.
 *
 * CLI:
 *   (no args)          SMOKE: L=16, cube N=8, cap 600 ticks (CI sanity; NOT a
 *                      measurement).
 *   --sweep            CANONICAL: L∈{32,48} × {cube,rod,lshape} × N∈{8,27,64}
 *                      + determinism re-run of config #1.
 *   --L=N (repeatable) lattice sizes override
 *   --N=K (repeatable) cluster sizes override (must be in {8,27,64})
 *   --geometry=S (rep.) geometry override: cube|rod|lshape
 *   --max-ticks=N      equilibration cap        (default 20000 sweep / 600 smoke)
 *   --meas-window=N    measurement window       (default 256)
 *   --sor-iters=N      SOR iterations per tick  (default 150)
 *   --seed=N           RNG seed                 (default 42; no RNG is consumed
 *                      in this config — the seed is pinned for provenance)
 *   --output-dir=PATH  CSV directory            (default engine/results/gnc_qij/)
 *   --tag=S            CSV tag                  (default smoke / v1)
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace fs = std::filesystem;

namespace {

// ---------------------------------------------------------------------------
// Symmetric 3x3 accumulator for the gradient quadratic form.
// ---------------------------------------------------------------------------
struct Sym3 {
    // xx, yy, zz, xy, xz, yz
    double m[6] = {0, 0, 0, 0, 0, 0};

    void accumulate(const ftd::Vec3 d[3]) {
        // d[i] = forward difference of J along axis i (a 3-vector over a).
        m[0] += d[0].dot(d[0]);
        m[1] += d[1].dot(d[1]);
        m[2] += d[2].dot(d[2]);
        m[3] += d[0].dot(d[1]);
        m[4] += d[0].dot(d[2]);
        m[5] += d[1].dot(d[2]);
    }
    Sym3& operator+=(const Sym3& o) {
        for (int k = 0; k < 6; ++k) m[k] += o.m[k];
        return *this;
    }
    Sym3 scaled(double s) const {
        Sym3 r;
        for (int k = 0; k < 6; ++k) r.m[k] = m[k] * s;
        return r;
    }
    double trace() const { return m[0] + m[1] + m[2]; }
    double max_offdiag() const {
        return std::max({std::fabs(m[3]), std::fabs(m[4]), std::fabs(m[5])});
    }
    // Eigenvalues of the symmetric matrix (analytic; Smith's trigonometric method).
    void eigenvalues(double& l1, double& l2, double& l3) const {
        const double a11 = m[0], a22 = m[1], a33 = m[2];
        const double a12 = m[3], a13 = m[4], a23 = m[5];
        const double p1 = a12 * a12 + a13 * a13 + a23 * a23;
        if (p1 < 1e-300) {
            l1 = std::max({a11, a22, a33});
            l3 = std::min({a11, a22, a33});
            l2 = a11 + a22 + a33 - l1 - l3;
            return;
        }
        const double q = (a11 + a22 + a33) / 3.0;
        const double p2 = (a11 - q) * (a11 - q) + (a22 - q) * (a22 - q)
                        + (a33 - q) * (a33 - q) + 2.0 * p1;
        const double p = std::sqrt(p2 / 6.0);
        // B = (A - qI)/p ; r = det(B)/2
        const double b11 = (a11 - q) / p, b22 = (a22 - q) / p, b33 = (a33 - q) / p;
        const double b12 = a12 / p, b13 = a13 / p, b23 = a23 / p;
        double detB = b11 * (b22 * b33 - b23 * b23)
                    - b12 * (b12 * b33 - b23 * b13)
                    + b13 * (b12 * b23 - b22 * b13);
        double r = detB / 2.0;
        r = std::max(-1.0, std::min(1.0, r));
        const double phi = std::acos(r) / 3.0;
        l1 = q + 2.0 * p * std::cos(phi);
        l3 = q + 2.0 * p * std::cos(phi + 2.0 * ftd::PI / 3.0);
        l2 = 3.0 * q - l1 - l3;
    }
};

// ---------------------------------------------------------------------------
// Cluster geometry — frozen offset tables (see header comment / pre-reg §2).
// Offsets are relative; the builder centers the bounding box in the lattice.
// ---------------------------------------------------------------------------
using Offset = std::tuple<int, int, int>;

std::vector<Offset> geometry_offsets(const std::string& geom, int N, bool& ok) {
    std::vector<Offset> out;
    ok = true;
    if (geom == "cube") {
        int k = (N == 8) ? 2 : (N == 27) ? 3 : (N == 64) ? 4 : -1;
        if (k < 0) { ok = false; return out; }
        for (int x = 0; x < k; ++x)
            for (int y = 0; y < k; ++y)
                for (int z = 0; z < k; ++z) out.emplace_back(x, y, z);
    } else if (geom == "rod") {
        int len = 0, a = 1;
        if      (N == 8)  { len = 8;  a = 1; }
        else if (N == 27) { len = 27; a = 1; }
        else if (N == 64) { len = 16; a = 2; }
        else { ok = false; return out; }
        for (int x = 0; x < len; ++x)
            for (int y = 0; y < a; ++y)
                for (int z = 0; z < a; ++z) out.emplace_back(x, y, z);
    } else if (geom == "lshape") {
        std::set<Offset> s;
        if (N == 8) {
            for (int x = 0; x < 4; ++x) s.insert({x, 0, 0});       // arm_x: 4
            for (int y = 1; y < 5; ++y) s.insert({0, y, 0});       // arm_y: 4
        } else if (N == 27) {
            for (int x = 0; x < 14; ++x) s.insert({x, 0, 0});      // arm_x: 14
            for (int y = 1; y < 14; ++y) s.insert({0, y, 0});      // arm_y: 13
        } else if (N == 64) {
            // 2x2-thick arms, non-overlapping: 8*4 + 8*4 = 64.
            for (int x = 0; x < 8; ++x)
                for (int y = 0; y < 2; ++y)
                    for (int z = 0; z < 2; ++z) s.insert({x, y, z});
            for (int y = 2; y < 10; ++y)
                for (int x = 0; x < 2; ++x)
                    for (int z = 0; z < 2; ++z) s.insert({x, y, z});
        } else { ok = false; return out; }
        out.assign(s.begin(), s.end());
    } else {
        ok = false;
    }
    if (static_cast<int>(out.size()) != N) ok = false;
    return out;
}

// ---------------------------------------------------------------------------
// Build the locked +1 cluster centered in the lattice; return flat indices.
// ---------------------------------------------------------------------------
std::vector<int> build_cluster(ftd::RenderBridge& rb, const std::vector<Offset>& offs) {
    const int L = rb.lattice().size();
    int maxx = 0, maxy = 0, maxz = 0;
    for (const auto& o : offs) {
        maxx = std::max(maxx, std::get<0>(o));
        maxy = std::max(maxy, std::get<1>(o));
        maxz = std::max(maxz, std::get<2>(o));
    }
    const int ox = (L - 1 - maxx) / 2;
    const int oy = (L - 1 - maxy) / 2;
    const int oz = (L - 1 - maxz) / 2;

    std::vector<int> members;
    members.reserve(offs.size());
    for (const auto& o : offs) {
        const int x = ox + std::get<0>(o);
        const int y = oy + std::get<1>(o);
        const int z = oz + std::get<2>(o);
        rb.inject_particle(x, y, z, +1, ftd::Vec3{0.0, 0.0, 0.0});
        ftd::Voxel& v = rb.voxel_at(x, y, z);
        v.locked = true;                    // FTD-0349 cluster predicate
        v.velocity = ftd::Vec3{0.0, 0.0, 0.0};
        members.push_back(rb.lattice().index(x, y, z));
    }
    return members;
}

// Shell support: non-member sites within Chebyshev (Moore) distance <= R of
// any member. Frozen at R = 3 (pre-reg §2).
std::vector<int> shell_support(const ftd::RenderBridge& rb,
                               const std::vector<int>& members, int R) {
    const ftd::Lattice& lat = rb.lattice();
    const int L = lat.size();
    std::set<int> mem(members.begin(), members.end());
    std::set<int> shell;
    for (int mi : members) {
        // Decompose flat index (convention: idx = x*L*L + y*L + z).
        const int x = mi / (L * L), y = (mi / L) % L, z = mi % L;
        for (int dx = -R; dx <= R; ++dx)
            for (int dy = -R; dy <= R; ++dy)
                for (int dz = -R; dz <= R; ++dz) {
                    const int xx = ((x + dx) % L + L) % L;
                    const int yy = ((y + dy) % L + L) % L;
                    const int zz = ((z + dz) % L + L) % L;
                    const int idx = lat.index(xx, yy, zz);
                    if (!mem.count(idx)) shell.insert(idx);
                }
    }
    return std::vector<int>(shell.begin(), shell.end());
}

// ---------------------------------------------------------------------------
// Forward-difference gradient quadratic form over a support (READ-ONLY).
//   G_ij = Σ_{x∈support} Σ_a (Δ⁺_i J_a)(x)·(Δ⁺_j J_a)(x)
// ---------------------------------------------------------------------------
Sym3 gradient_form(const ftd::RenderBridge& rb, const std::vector<int>& support) {
    const ftd::Lattice& lat = rb.lattice();
    const int L = lat.size();
    const auto& vox = rb.voxels();   // const overload — no dirty flags
    Sym3 g;
    for (int idx : support) {
        const int x = idx / (L * L), y = (idx / L) % L, z = idx % L;
        const ftd::Vec3& j0 = vox[idx].flux;
        ftd::Vec3 d[3];
        d[0] = vox[lat.index((x + 1) % L, y, z)].flux - j0;
        d[1] = vox[lat.index(x, (y + 1) % L, z)].flux - j0;
        d[2] = vox[lat.index(x, y, (z + 1) % L)].flux - j0;
        g.accumulate(d);
    }
    return g;
}

// All-site raw gradient sum (trace channel only is used, but full form is cheap).
Sym3 gradient_form_allsites(const ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    std::vector<int> all(static_cast<size_t>(L) * L * L);
    for (size_t i = 0; i < all.size(); ++i) all[i] = static_cast<int>(i);
    return gradient_form(rb, all);
}

int manifested_count(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    int n = 0;
    for (const auto& v : vox) if (v.state != 0) ++n;
    return n;
}

// ---------------------------------------------------------------------------
// Frozen engine configuration (pre-reg §2). No RNG is consumed (genesis OFF,
// langevin OFF); the seed is pinned for provenance.
// ---------------------------------------------------------------------------
void configure(ftd::RenderBridge& rb, int sor_iters, std::uint32_t seed) {
    rb.force_cpu();
    rb.set_sor_iterations(sor_iters);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation  = true;
    rb.toggles.coupling          = true;
    rb.toggles.gauss_projection  = true;
    rb.toggles.damping           = true;   // vacuum damps too (selective OFF):
    rb.toggles.selective_damping = false;  // transients relax to the dressed profile
    rb.toggles.genesis           = false;  // cluster integrity: exactly N members
    rb.toggles.dual_substrate    = false;
    rb.toggles.forces            = false;
    rb.toggles.movement          = false;
    rb.toggles.langevin          = false;
    rb.seed_rng(seed);
}

// ---------------------------------------------------------------------------
// One configuration: build, equilibrate, measure.
// ---------------------------------------------------------------------------
struct RunResult {
    int    L = 0;
    std::string geometry;
    int    N = 0;
    bool   geom_ok      = false;
    bool   converged    = false;   // gate E1
    int    ticks_equil  = 0;
    double gauss_max_err = 0.0;    // gate E2 quantity
    bool   integrity_ok = false;   // gate E3
    // Q matrices (time-averaged over the measurement window), per support.
    Sym3   Q_member;               // normalized by 1/(N·K_B²)
    Sym3   Q_shell;
    double Q_all_trace  = 0.0;     // all-site trace, normalized by 1/(N·K_B²)
    double raw_all_trace = 0.0;    // all-site raw Σ|Δ⁺J|² (Eq. 4 identity channel)
    double coulomb_pred_raw = 0.0; // N·q²(1−N/L³), q=1 (Eq. 4 prediction)
    int    shell_count  = 0;
};

RunResult run_config(int L, const std::string& geom, int N, int max_ticks,
                     int meas_window, int sor_iters, std::uint32_t seed) {
    RunResult r;
    r.L = L; r.geometry = geom; r.N = N;

    bool ok = false;
    const auto offs = geometry_offsets(geom, N, ok);
    r.geom_ok = ok;
    if (!ok) return r;

    ftd::RenderBridge rb(L);
    configure(rb, sor_iters, seed);
    const std::vector<int> members = build_cluster(rb, offs);
    const std::vector<int> shell = shell_support(rb, members, 3);
    r.shell_count = static_cast<int>(shell.size());

    // ---- Equilibration (gate E1): consecutive 64-tick window means of the
    // member-support raw gradient sum; converged when rel change < 1e-6. ----
    const int WIN = 64;
    double prev_mean = -1.0;
    int t = 0;
    while (t < max_ticks) {
        double acc = 0.0;
        for (int k = 0; k < WIN && t < max_ticks; ++k, ++t) {
            rb.run(1);
            acc += gradient_form(rb, members).trace();
        }
        const double mean = acc / WIN;
        if (prev_mean > 0.0) {
            const double rel = std::fabs(mean - prev_mean) / std::max(mean, 1e-300);
            if (rel < 1e-6) { r.converged = true; break; }
        }
        prev_mean = mean;
    }
    r.ticks_equil = t;

    // ---- Gate E2 quantity: Gauss residual at measurement start. ----
    r.gauss_max_err = rb.energy_audit().max_gauss_error;

    // ---- Measurement: time-averaged Q over meas_window ticks. ----
    Sym3 acc_m, acc_s;
    double acc_all = 0.0;
    for (int k = 0; k < meas_window; ++k) {
        acc_m += gradient_form(rb, members);
        acc_s += gradient_form(rb, shell);
        acc_all += gradient_form_allsites(rb).trace();
        rb.run(1);
    }
    const double norm = 1.0 / (static_cast<double>(meas_window)
                               * static_cast<double>(N) * ftd::K_B * ftd::K_B);
    r.Q_member = acc_m.scaled(norm);
    r.Q_shell  = acc_s.scaled(norm);
    r.raw_all_trace = acc_all / meas_window;
    r.Q_all_trace = r.raw_all_trace / (static_cast<double>(N) * ftd::K_B * ftd::K_B);
    const double L3 = static_cast<double>(L) * L * L;
    r.coulomb_pred_raw = static_cast<double>(N) * (1.0 - static_cast<double>(N) / L3);

    // ---- Gate E3: cluster integrity. ----
    r.integrity_ok = (manifested_count(rb) == N);
    if (r.integrity_ok) {
        const auto& vox = rb.voxels();
        for (int mi : members) {
            if (vox[mi].state != +1 || !vox[mi].locked) { r.integrity_ok = false; break; }
        }
    }
    return r;
}

void print_and_log(std::FILE* f, const RunResult& r) {
    auto aniso = [](const Sym3& q) {
        const double tr = q.trace();
        return (tr > 1e-300) ? q.max_offdiag() / tr : 0.0;
    };
    auto spread = [](const Sym3& q) {
        double l1, l2, l3;
        q.eigenvalues(l1, l2, l3);
        const double mean = q.trace() / 3.0;
        return (std::fabs(mean) > 1e-300) ? (l1 - l3) / mean : 0.0;
    };
    // ROW,<L>,<geom>,<N>,<converged>,<ticks>,<gauss_err>,<integrity>,
    //   member: qxx,qyy,qzz,qxy,qxz,qyz,trace,aniso,spread
    //   shell : qxx,qyy,qzz,qxy,qxz,qyz,trace,aniso,spread
    //   all   : Q_all_trace, raw_all_trace, coulomb_pred_raw, shell_count
    const Sym3& m = r.Q_member;
    const Sym3& s = r.Q_shell;
    char line[1024];
    std::snprintf(line, sizeof(line),
        "%d,%s,%d,%d,%d,%.6e,%d,"
        "%.8e,%.8e,%.8e,%.8e,%.8e,%.8e,%.8e,%.6f,%.6f,"
        "%.8e,%.8e,%.8e,%.8e,%.8e,%.8e,%.8e,%.6f,%.6f,"
        "%.8e,%.8e,%.8e,%d",
        r.L, r.geometry.c_str(), r.N, r.converged ? 1 : 0, r.ticks_equil,
        r.gauss_max_err, r.integrity_ok ? 1 : 0,
        m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5], m.trace(), aniso(m), spread(m),
        s.m[0], s.m[1], s.m[2], s.m[3], s.m[4], s.m[5], s.trace(), aniso(s), spread(s),
        r.Q_all_trace, r.raw_all_trace, r.coulomb_pred_raw, r.shell_count);
    std::printf("ROW,%s\n", line);
    std::fflush(stdout);
    if (f) { std::fprintf(f, "%s\n", line); std::fflush(f); }
}

} // namespace

int main(int argc, char** argv) {
    std::vector<int>         Ls;
    std::vector<int>         Ns;
    std::vector<std::string> geoms;
    int  max_ticks   = -1;
    int  meas_window = 256;
    int  sor_iters   = 150;
    std::uint32_t seed = 42;
    bool sweep = false;
    std::string output_dir = "engine/results/gnc_qij/";
    std::string tag;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)           Ls.push_back(std::atoi(a.c_str() + 4));
        else if (a.rfind("--N=", 0) == 0)           Ns.push_back(std::atoi(a.c_str() + 4));
        else if (a.rfind("--geometry=", 0) == 0)    geoms.push_back(a.substr(11));
        else if (a.rfind("--max-ticks=", 0) == 0)   max_ticks = std::atoi(a.c_str() + 12);
        else if (a.rfind("--meas-window=", 0) == 0) meas_window = std::atoi(a.c_str() + 14);
        else if (a.rfind("--sor-iters=", 0) == 0)   sor_iters = std::atoi(a.c_str() + 12);
        else if (a.rfind("--seed=", 0) == 0)        seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a == "--sweep")                    sweep = true;
        else if (a.rfind("--output-dir=", 0) == 0)  output_dir = a.substr(13);
        else if (a.rfind("--tag=", 0) == 0)         tag = a.substr(6);
    }

    if (sweep) {
        if (Ls.empty())    Ls = {32, 48};
        if (Ns.empty())    Ns = {8, 27, 64};
        if (geoms.empty()) geoms = {"cube", "rod", "lshape"};
        if (max_ticks < 0) max_ticks = 20000;
        if (tag.empty())   tag = "v1";
    } else {
        if (Ls.empty())    Ls = {16};
        if (Ns.empty())    Ns = {8};
        if (geoms.empty()) geoms = {"cube"};
        if (max_ticks < 0) max_ticks = 600;
        if (tag.empty())   tag = "smoke";
        if (meas_window > 64) meas_window = 64;   // keep the CI smoke fast
    }

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("gnc_qij_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }

    std::printf("# campaign_gnc_qij (FTD-0349 §9 Q_ij discriminator / PREREG_GNC_QIJ_v1)\n");
    std::printf("# Q_ij gates SUMMED GNC-w ONLY (FTD-0354 §4.4); necessary-not-sufficient for GNC-s.\n");
    std::printf("# config: CPU, seed=%u, sor_iters=%d, meas_window=%d, max_ticks=%d%s\n",
                seed, sor_iters, meas_window, max_ticks,
                sweep ? "  [--sweep: canonical measurement]" : "  [SMOKE -- not a measurement]");
    std::printf("# toggles ON: wave_propagation,coupling,gauss_projection,damping(selective OFF) ; "
                "OFF: genesis,forces,movement,dual_substrate,langevin\n");
    std::printf("# golden-neutral (public API only); golden hash 0xb604d81a3d79366e unaffected\n");
    std::printf("# K_B=%.6f  q=1 (lattice charge unit)\n", ftd::K_B);
    const char* cols =
        "L,geometry,N,converged,ticks_equil,gauss_max_err,integrity,"
        "m_qxx,m_qyy,m_qzz,m_qxy,m_qxz,m_qyz,m_trace,m_aniso,m_eigspread,"
        "s_qxx,s_qyy,s_qzz,s_qxy,s_qxz,s_qyz,s_trace,s_aniso,s_eigspread,"
        "all_Q_trace,all_raw_trace,coulomb_pred_raw,shell_count";
    std::printf("# columns: %s\n", cols);
    std::fflush(stdout);

    std::fprintf(f, "# campaign_gnc_qij FTD-0349/FTD-0354 seed=%u sor_iters=%d meas_window=%d%s\n",
                 seed, sor_iters, meas_window, sweep ? " SWEEP" : " SMOKE");
    std::fprintf(f, "%s\n", cols);

    std::vector<RunResult> results;
    for (int L : Ls) {
        for (const std::string& g : geoms) {
            for (int N : Ns) {
                RunResult r = run_config(L, g, N, max_ticks, meas_window, sor_iters, seed);
                if (!r.geom_ok) {
                    std::printf("# SKIP: geometry %s N=%d not defined\n", g.c_str(), N);
                    continue;
                }
                print_and_log(f, r);
                results.push_back(r);
            }
        }
    }

    // ---- Gate E4 (determinism): re-run config #1, bit-compare Q. ----
    if (!results.empty()) {
        const RunResult& a = results.front();
        RunResult b = run_config(a.L, a.geometry, a.N, max_ticks, meas_window,
                                 sor_iters, seed);
        bool identical = (std::memcmp(a.Q_member.m, b.Q_member.m, sizeof(a.Q_member.m)) == 0)
                      && (std::memcmp(a.Q_shell.m, b.Q_shell.m, sizeof(a.Q_shell.m)) == 0);
        std::printf("GATE_E4_DETERMINISM,%s\n", identical ? "PASS" : "FAIL");
        std::fprintf(f, "# GATE_E4_DETERMINISM,%s\n", identical ? "PASS" : "FAIL");
    }

    std::fclose(f);
    std::printf("DONE -> %s\n", out_csv.string().c_str());
    return 0;
}
