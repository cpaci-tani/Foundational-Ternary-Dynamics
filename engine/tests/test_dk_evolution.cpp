/**
 * @file test_dk_evolution.cpp
 * @brief M1 (FTD-0379) — Dirac-Kähler evolution test.
 *
 * Executes the verification path of DERIV_DIRAC_KAHLER_IDENTIFICATION.md
 * §A1.5 (FTD-0089), never previously run. Pre-registered in
 * PREREG_VERTEX_DK_CLOSURE_v1.md §2 — expectations locked before this
 * runner produced any output.
 *
 * Question: does the engine's evolution satisfy the discrete Dirac-Kähler
 * equation  ∂_t Φ = (d − δ)Φ − mΦ  on the local grade fields
 *
 *   φ⁰(x) = S(x)   = |J(x)|²
 *   φ¹_i(x) = V_i(x) = J_i(x)
 *   φ²_ij(x) = P_ij(x) = J_i(x)J_j(x+e_i) − J_i(x+e_j)J_j(x)   (i<j)
 *   φ³(x) = T(x)   = J_x(x)J_y(x)J_z(x)
 *
 * with a single fitted effective mass m — against the second-order
 * Klein-Gordon comparator  ∂_t²φ = c²Δφ − μ²φ  fitted per grade?
 *
 * Outcomes (pre-registered): DK-DYNAMICAL / DK-PARTIAL / DK-STATIC-ONLY /
 * UNDETERMINED. This runner prints the measured residuals and the verdict
 * against the locked criteria; it adjusts nothing to match expectations.
 *
 * Harness validity gates (run before any dynamics; exit 1 on failure):
 *   - signed adjointness  <d0 f, W> = −<f, δ1 W>,  <d1 V, Q> = +<V, δ2 Q>,
 *     <d2 P, u> = −<P, δ3 u>   (relative 1e-12)
 *   - nilpotency  d1∘d0 = 0, d2∘d1 = 0, δ1∘δ2 = 0, δ2∘δ3 = 0
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>
#include <algorithm>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace {

// ---------------------------------------------------------------- lattice
struct Lat {
    int L;
    int idx(int x, int y, int z) const { return (x * L + y) * L + z; }
    int wp(int c) const { return (c + 1) % L; }        // +1 wrapped
    int wm(int c) const { return (c + L - 1) % L; }    // -1 wrapped
    int n3() const { return L * L * L; }
};

using Field  = std::vector<double>;               // scalar field, L^3
using Field3 = std::array<std::vector<double>, 3>; // 3-component field

Field  mkF (const Lat& g) { return Field (g.n3(), 0.0); }
Field3 mkF3(const Lat& g) { return { Field(g.n3(),0.0), Field(g.n3(),0.0), Field(g.n3(),0.0) }; }

// coordinate shift by +1 along axis a
inline int shift_p(const Lat& g, int x, int y, int z, int a) {
    return a == 0 ? g.idx(g.wp(x), y, z)
         : a == 1 ? g.idx(x, g.wp(y), z)
                  : g.idx(x, y, g.wp(z));
}
inline int shift_m(const Lat& g, int x, int y, int z, int a) {
    return a == 0 ? g.idx(g.wm(x), y, z)
         : a == 1 ? g.idx(x, g.wm(y), z)
                  : g.idx(x, y, g.wm(z));
}

// ------------------------------------------------- discrete exterior calc
// Plaquette component order everywhere: 0 = xy, 1 = xz, 2 = yz.
// (d0 f)_i = grad+_i f
Field3 d0(const Lat& g, const Field& f) {
    Field3 out = mkF3(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        for (int a = 0; a < 3; ++a) out[a][s] = f[shift_p(g,x,y,z,a)] - f[s];
    }
    return out;
}
// (δ1 V) = sum_i grad-_i V_i   (FTD-0089 divergence convention)
Field delta1(const Lat& g, const Field3& V) {
    Field out = mkF(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        double acc = 0.0;
        for (int a = 0; a < 3; ++a) acc += V[a][s] - V[a][shift_m(g,x,y,z,a)];
        out[s] = acc;
    }
    return out;
}
// (d1 V)_ij = grad+_i V_j - grad+_j V_i,  (i,j) in {(x,y),(x,z),(y,z)}
Field3 d1(const Lat& g, const Field3& V) {
    Field3 out = mkF3(g);
    static const int I[3] = {0, 0, 1}, J[3] = {1, 2, 2};
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        for (int p = 0; p < 3; ++p) {
            const int i = I[p], j = J[p];
            const double dpi = V[j][shift_p(g,x,y,z,i)] - V[j][s];
            const double dpj = V[i][shift_p(g,x,y,z,j)] - V[i][s];
            out[p][s] = dpi - dpj;
        }
    }
    return out;
}
// (δ2 P)_x = grad-_y P_xy + grad-_z P_xz
// (δ2 P)_y = -grad-_x P_xy + grad-_z P_yz
// (δ2 P)_z = -grad-_x P_xz - grad-_y P_yz
Field3 delta2(const Lat& g, const Field3& P) {
    Field3 out = mkF3(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        auto gm = [&](const Field& f, int a) { return f[s] - f[shift_m(g,x,y,z,a)]; };
        out[0][s] =  gm(P[0], 1) + gm(P[1], 2);
        out[1][s] = -gm(P[0], 0) + gm(P[2], 2);
        out[2][s] = -gm(P[1], 0) - gm(P[2], 1);
    }
    return out;
}
// (d2 P) = grad+_x P_yz - grad+_y P_xz + grad+_z P_xy
Field d2(const Lat& g, const Field3& P) {
    Field out = mkF(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        auto gp = [&](const Field& f, int a) { return f[shift_p(g,x,y,z,a)] - f[s]; };
        out[s] = gp(P[2], 0) - gp(P[1], 1) + gp(P[0], 2);
    }
    return out;
}
// (δ3 T)_xy = grad-_z T,  (δ3 T)_xz = -grad-_y T,  (δ3 T)_yz = grad-_x T
Field3 delta3(const Lat& g, const Field& T) {
    Field3 out = mkF3(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        auto gm = [&](int a) { return T[s] - T[shift_m(g,x,y,z,a)]; };
        out[0][s] =  gm(2);
        out[1][s] = -gm(1);
        out[2][s] =  gm(0);
    }
    return out;
}
// 7-point lattice Laplacian (for the KG comparator)
Field lap(const Lat& g, const Field& f) {
    Field out = mkF(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        double acc = -6.0 * f[s];
        for (int a = 0; a < 3; ++a)
            acc += f[shift_p(g,x,y,z,a)] + f[shift_m(g,x,y,z,a)];
        out[s] = acc;
    }
    return out;
}

double dotF (const Field& a,  const Field& b)  {
    double s = 0.0; for (size_t i = 0; i < a.size(); ++i) s += a[i] * b[i]; return s;
}
double dotF3(const Field3& a, const Field3& b) {
    return dotF(a[0],b[0]) + dotF(a[1],b[1]) + dotF(a[2],b[2]);
}
double norm2F (const Field& a)  { return dotF(a,a); }
double norm2F3(const Field3& a) { return dotF3(a,a); }

// ------------------------------------------------- harness validity gates
bool self_checks(int L) {
    Lat g{L};
    // deterministic pseudorandom fields (LCG), no engine involvement
    unsigned st = 0xC0FFEE01u;
    auto rnd = [&st]() {
        st = st * 1664525u + 1013904223u;
        return (double)(st >> 8) / (double)(1u << 24) - 0.5;
    };
    Field  f = mkF(g), u = mkF(g);
    Field3 W = mkF3(g), Q = mkF3(g), V = mkF3(g), P = mkF3(g);
    for (int s = 0; s < g.n3(); ++s) {
        f[s] = rnd(); u[s] = rnd();
        for (int a = 0; a < 3; ++a) { W[a][s]=rnd(); Q[a][s]=rnd(); V[a][s]=rnd(); P[a][s]=rnd(); }
    }
    bool ok = true;
    auto rel_check = [&ok](const char* name, double lhs, double rhs, double scale) {
        const double d = std::abs(lhs - rhs) / std::max(scale, 1e-30);
        const bool pass = d < 1e-12;
        std::printf("    [%s] %-28s rel.err = %.2e\n", pass ? "PASS" : "FAIL", name, d);
        if (!pass) ok = false;
    };
    // signed adjointness (per PREREG §2.2)
    {
        const double lhs = dotF3(d0(g,f), W);
        const double rhs = -dotF(f, delta1(g,W));
        rel_check("<d0 f, W> = -<f, d1* W>", lhs, rhs, std::abs(lhs) + std::abs(rhs));
    }
    {
        const double lhs = dotF3(d1(g,V), Q);
        const double rhs = dotF3(V, delta2(g,Q));
        rel_check("<d1 V, Q> = +<V, d2* Q>", lhs, rhs, std::abs(lhs) + std::abs(rhs));
    }
    {
        const double lhs = dotF(d2(g,P), u);
        const double rhs = -dotF3(P, delta3(g,u));
        rel_check("<d2 P, u> = -<P, d3* u>", lhs, rhs, std::abs(lhs) + std::abs(rhs));
    }
    // nilpotency
    auto near0 = [&ok](const char* name, double n2, double ref) {
        const double d = std::sqrt(n2) / std::max(std::sqrt(ref), 1e-30);
        const bool pass = d < 1e-12;
        std::printf("    [%s] %-28s rel.norm = %.2e\n", pass ? "PASS" : "FAIL", name, d);
        if (!pass) ok = false;
    };
    near0("d1(d0 f) = 0",      norm2F3(d1(g, d0(g,f))),        norm2F(f));
    near0("d2(d1 V) = 0",      norm2F (d2(g, d1(g,V))),        norm2F3(V));
    near0("delta1(delta2 Q)=0",norm2F (delta1(g, delta2(g,Q))),norm2F3(Q));
    near0("delta2(delta3 u)=0",norm2F3(delta2(g, delta3(g,u))),norm2F(u));
    return ok;
}

// ------------------------------------------------------------ grade fields
struct Grades {
    Field  S, T;
    Field3 V, P;   // P order: xy, xz, yz
};

Grades compute_grades(const std::vector<ftd::Voxel>& vox, const Lat& g) {
    Grades gr{ mkF(g), mkF(g), mkF3(g), mkF3(g) };
    auto comp = [](const ftd::Vec3& v, int a) -> double {
        return a == 0 ? v.x : a == 1 ? v.y : v.z;
    };
    static const int I[3] = {0, 0, 1}, J[3] = {1, 2, 2};
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        const auto& fl = vox[s].flux;
        gr.S[s] = fl.x*fl.x + fl.y*fl.y + fl.z*fl.z;
        gr.V[0][s] = fl.x; gr.V[1][s] = fl.y; gr.V[2][s] = fl.z;
        gr.T[s] = fl.x * fl.y * fl.z;
        for (int p = 0; p < 3; ++p) {
            const int i = I[p], j = J[p];
            const double Ji_x   = comp(fl, i);
            const double Jj_x   = comp(fl, j);
            const double Jj_xpi = comp(vox[shift_p(g,x,y,z,i)].flux, j);
            const double Ji_xpj = comp(vox[shift_p(g,x,y,z,j)].flux, i);
            gr.P[p][s] = Ji_x * Jj_xpi - Ji_xpj * Jj_x;
        }
    }
    return gr;
}

// (d - delta)Phi, all 8 component rows, from grade fields
struct DKRhs {
    Field  row0;    // -delta1 V              (grade 0)
    Field3 row1;    // d0 S - delta2 P        (grade 1)
    Field3 row2;    // d1 V - delta3 T        (grade 2)
    Field  row3;    // d2 P                   (grade 3)
};
DKRhs dk_spatial(const Lat& g, const Grades& gr) {
    DKRhs r{ mkF(g), mkF3(g), mkF3(g), mkF(g) };
    Field  dv  = delta1(g, gr.V);
    Field3 ds  = d0(g, gr.S);
    Field3 dp2 = delta2(g, gr.P);
    Field3 dv1 = d1(g, gr.V);
    Field3 dt3 = delta3(g, gr.T);
    Field  dp  = d2(g, gr.P);
    for (int s = 0; s < g.n3(); ++s) {
        r.row0[s] = -dv[s];
        for (int a = 0; a < 3; ++a) {
            r.row1[a][s] = ds[a][s] - dp2[a][s];
            r.row2[a][s] = dv1[a][s] - dt3[a][s];
        }
        r.row3[s] = dp[s];
    }
    return r;
}

// -------------------------------------------------------------- protocols
inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}
void inject_wh_mode(ftd::RenderBridge& rb, int v_mask, int axis, double A) {
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const double s = static_cast<double>(chi(v_mask, x, y, z));
        ftd::Vec3 dF{0, 0, 0};
        if (axis == 0) dF.x = A * s;
        if (axis == 1) dF.y = A * s;
        if (axis == 2) dF.z = A * s;
        rb.inject_flux_add(x, y, z, dF);
    }
}
// CONFIG-N: the FTD-0088 full non-local set (test_clifford_multigrade.cpp)
void enable_config_N(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation  = true;
    t.gauss_projection  = true;
    t.genesis           = true;
    t.movement          = true;
    t.forces            = true;
    t.emergent_forces   = true;
    t.pair_production   = true;
    t.weak_transmutation= true;
    t.exchange_force    = true;
    t.strong_force      = true;
    t.triad_binding     = true;
    t.color_forces      = true;
}
// CONFIG-M: minimal linear control
void enable_config_M(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation  = true;
    t.gauss_projection  = true;
}

// history of grade fields, tick 0..N_TICKS
std::vector<Grades> run_record(int L, double A, unsigned seed,
                               int axis_f, int axis_g, bool config_n,
                               int n_ticks) {
    ftd::RenderBridge rb(L);
    if (config_n) enable_config_N(rb.toggles);
    else          enable_config_M(rb.toggles);
    rb.seed_rng(seed);
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    inject_wh_mode(rb, w1_mask[axis_f], axis_f, A);
    rb.run(1);
    inject_wh_mode(rb, w1_mask[axis_g], axis_g, A);
    rb.run(1);
    Lat g{L};
    std::vector<Grades> hist;
    hist.reserve(n_ticks + 1);
    hist.push_back(compute_grades(rb.voxels(), g));   // t = 0
    for (int t = 1; t <= n_ticks; ++t) {
        rb.run(1);
        hist.push_back(compute_grades(rb.voxels(), g));
    }
    return hist;
}

// -------------------------------------------------------- fit accumulators
// First-order DK fit: residual rows  r = (dPhi_dt - Drhs) + m * Phi_bar
// accumulate per grade: Saa, Sab, Sbb, and D = sum ||dPhi_dt||^2
struct DKAccum {
    double Saa[4] = {0,0,0,0};
    double Sab[4] = {0,0,0,0};
    double Sbb[4] = {0,0,0,0};
    double Den[4] = {0,0,0,0};
    void add_row(int k, double a, double b, double lhs) {
        Saa[k] += a * a; Sab[k] += a * b; Sbb[k] += b * b; Den[k] += lhs * lhs;
    }
    double mstar() const {
        double sab = 0, sbb = 0;
        for (int k = 0; k < 4; ++k) { sab += Sab[k]; sbb += Sbb[k]; }
        return sbb > 1e-30 ? -sab / sbb : 0.0;
    }
    double rho_grade(int k, double m) const {
        const double num = std::max(Saa[k] + 2.0*m*Sab[k] + m*m*Sbb[k], 0.0);
        return std::sqrt(num) / std::sqrt(std::max(Den[k], 1e-30));
    }
    double rho_all(double m) const {
        double num = 0, den = 0;
        for (int k = 0; k < 4; ++k) {
            num += std::max(Saa[k] + 2.0*m*Sab[k] + m*m*Sbb[k], 0.0);
            den += Den[k];
        }
        return std::sqrt(num) / std::sqrt(std::max(den, 1e-30));
    }
    bool degenerate(int k) const { return Den[k] < 1e-20; }
};

// KG fit per grade: y = p*u + q*w,  y = second time difference,
// u = Laplacian(phi), w = -phi
struct KGAccum {
    double Suu=0, Sww=0, Suw=0, Syu=0, Syw=0, Syy=0;
    void add(double y, double u, double w) {
        Suu += u*u; Sww += w*w; Suw += u*w; Syu += y*u; Syw += y*w; Syy += y*y;
    }
    void solve(double& p, double& q) const {
        const double det = Suu * Sww - Suw * Suw;
        if (std::abs(det) < 1e-30) { p = q = 0.0; return; }
        p = ( Sww * Syu - Suw * Syw) / det;
        q = ( Suu * Syw - Suw * Syu) / det;
    }
    double rho() const {
        double p, q; solve(p, q);
        const double r2 = Syy - 2.0*p*Syu - 2.0*q*Syw
                        + p*p*Suu + q*q*Sww + 2.0*p*q*Suw;
        return std::sqrt(std::max(r2, 0.0)) / std::sqrt(std::max(Syy, 1e-30));
    }
};

void accumulate_run(const Lat& g, const std::vector<Grades>& hist,
                    int t_lo, int t_hi /*exclusive for pairs*/,
                    DKAccum& dk, KGAccum kg[4]) {
    // first-order rows on tick pairs (t, t+1)
    for (int t = t_lo; t < t_hi; ++t) {
        const Grades& a = hist[t];
        const Grades& b = hist[t + 1];
        // midpoint fields
        Grades mid{ mkF(g), mkF(g), mkF3(g), mkF3(g) };
        for (int s = 0; s < g.n3(); ++s) {
            mid.S[s] = 0.5 * (a.S[s] + b.S[s]);
            mid.T[s] = 0.5 * (a.T[s] + b.T[s]);
            for (int c = 0; c < 3; ++c) {
                mid.V[c][s] = 0.5 * (a.V[c][s] + b.V[c][s]);
                mid.P[c][s] = 0.5 * (a.P[c][s] + b.P[c][s]);
            }
        }
        const DKRhs rhs = dk_spatial(g, mid);
        for (int s = 0; s < g.n3(); ++s) {
            {   const double lhs = b.S[s] - a.S[s];
                dk.add_row(0, lhs - rhs.row0[s], mid.S[s], lhs); }
            for (int c = 0; c < 3; ++c) {
                const double lhsV = b.V[c][s] - a.V[c][s];
                dk.add_row(1, lhsV - rhs.row1[c][s], mid.V[c][s], lhsV);
                const double lhsP = b.P[c][s] - a.P[c][s];
                dk.add_row(2, lhsP - rhs.row2[c][s], mid.P[c][s], lhsP);
            }
            {   const double lhs = b.T[s] - a.T[s];
                dk.add_row(3, lhs - rhs.row3[s], mid.T[s], lhs); }
        }
    }
    // second-order KG rows on tick triples (t-1, t, t+1)
    for (int t = t_lo + 1; t < t_hi; ++t) {
        const Grades& m0 = hist[t - 1];
        const Grades& m1 = hist[t];
        const Grades& m2 = hist[t + 1];
        const Field lapS = lap(g, m1.S);
        const Field lapT = lap(g, m1.T);
        Field3 lapV, lapP;
        for (int c = 0; c < 3; ++c) { lapV[c] = lap(g, m1.V[c]); lapP[c] = lap(g, m1.P[c]); }
        for (int s = 0; s < g.n3(); ++s) {
            kg[0].add(m2.S[s] - 2.0*m1.S[s] + m0.S[s], lapS[s], -m1.S[s]);
            for (int c = 0; c < 3; ++c) {
                kg[1].add(m2.V[c][s] - 2.0*m1.V[c][s] + m0.V[c][s], lapV[c][s], -m1.V[c][s]);
                kg[2].add(m2.P[c][s] - 2.0*m1.P[c][s] + m0.P[c][s], lapP[c][s], -m1.P[c][s]);
            }
            kg[3].add(m2.T[s] - 2.0*m1.T[s] + m0.T[s], lapT[s], -m1.T[s]);
        }
    }
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  M1 (FTD-0379) - Dirac-Kahler evolution test\n");
    std::printf("  PREREG_VERTEX_DK_CLOSURE_v1.md S2 - locked before first run\n");
    std::printf("================================================================\n\n");

    // ---------------------------------------------- harness validity gates
    std::printf("--- Harness validity gates (L=8 pseudorandom fields) ---\n");
    if (!self_checks(8)) {
        std::printf("\n  HARNESS INVALID - operator self-checks failed. Aborting.\n");
        return 1;
    }
    std::printf("\n");

    const int    L       = 8;
    const double A       = 10.0;
    const int    N_TICKS = 30;
    const int    T_LO    = 4;    // fit window pairs (t, t+1), t = 4..27
    const int    T_HI    = 28;
    const std::array<unsigned, 8> seeds = {
        0xF4170517u, 0xF4170518u, 0xF4170519u, 0xF417051Au,
        0xF417051Bu, 0xF417051Cu, 0xF417051Du, 0xF417051Eu
    };
    const std::array<std::pair<int,int>, 3> pairs = {{ {0,1}, {0,2}, {1,2} }};
    const char* pair_name[3]  = { "(x,y)", "(x,z)", "(y,z)" };
    const char* grade_name[4] = { "0 (scalar S)", "1 (vector V)",
                                  "2 (bivector P)", "3 (pseudo T)" };
    const Lat g{L};

    std::printf("Protocol: L=%d, A=%.0f, 2-injection + %d ticks, fit window t=[%d,%d)\n",
                L, A, N_TICKS, T_LO, T_HI);
    std::printf("Seeds: %zu (FTD-0088 set). Configs: N (full non-local), M (wave+gauss).\n\n",
                seeds.size());

    struct ConfigResult {
        double mstar[3];          // per pair
        double rho_k_pooled[4];   // pooled across pairs at per-pair m*
        double rho_all_pooled;
        double rho_k_m0_pooled[4];
        double rho_kg_pooled[4];
        bool   degenerate_all;
    };
    ConfigResult results[2];

    for (int cfg = 0; cfg < 2; ++cfg) {
        const bool config_n = (cfg == 0);
        std::printf("===== CONFIG-%s =====\n", config_n ? "N (full non-local)" : "M (wave+gauss control)");

        double pooled_num[4] = {0,0,0,0}, pooled_den[4] = {0,0,0,0};
        double pooled_num_m0[4] = {0,0,0,0};
        double kg_num[4] = {0,0,0,0}, kg_den[4] = {0,0,0,0};
        bool degen = true;

        for (int p = 0; p < 3; ++p) {
            DKAccum dk;
            KGAccum kg[4];
            for (unsigned seed : seeds) {
                auto hist = run_record(L, A, seed,
                                       pairs[p].first, pairs[p].second,
                                       config_n, N_TICKS);
                accumulate_run(g, hist, T_LO, T_HI, dk, kg);
            }
            const double m = dk.mstar();
            results[cfg].mstar[p] = m;
            std::printf("  Pair %s:  m* = %+9.5f\n", pair_name[p], m);
            std::printf("    grade      rho(m*)   rho(m=0)   rho_KG    KG params (c^2, mu^2)\n");
            for (int k = 0; k < 4; ++k) {
                double c2, mu2; kg[k].solve(c2, mu2);
                const double r_dk  = dk.rho_grade(k, m);
                const double r_dk0 = dk.rho_grade(k, 0.0);
                const double r_kg  = kg[k].rho();
                std::printf("    %-12s %8.4f  %8.4f  %8.4f   (%+.4f, %+.4f)%s\n",
                            grade_name[k], r_dk, r_dk0, r_kg, c2, mu2,
                            dk.degenerate(k) ? "  [degenerate]" : "");
                // pool (numerators at this pair's m*)
                pooled_num[k]    += std::max(dk.Saa[k] + 2*m*dk.Sab[k] + m*m*dk.Sbb[k], 0.0);
                pooled_num_m0[k] += dk.Saa[k];
                pooled_den[k]    += dk.Den[k];
                kg_num[k] += kg[k].rho() * kg[k].rho() * std::max(kg[k].Syy, 1e-30);
                kg_den[k] += std::max(kg[k].Syy, 1e-30);
                if (!dk.degenerate(k)) degen = false;
            }
            std::printf("\n");
        }

        double num_all = 0, den_all = 0;
        for (int k = 0; k < 4; ++k) {
            results[cfg].rho_k_pooled[k]    = std::sqrt(pooled_num[k])    / std::sqrt(std::max(pooled_den[k], 1e-30));
            results[cfg].rho_k_m0_pooled[k] = std::sqrt(pooled_num_m0[k]) / std::sqrt(std::max(pooled_den[k], 1e-30));
            results[cfg].rho_kg_pooled[k]   = std::sqrt(kg_num[k])        / std::sqrt(std::max(kg_den[k], 1e-30));
            num_all += pooled_num[k]; den_all += pooled_den[k];
        }
        results[cfg].rho_all_pooled = std::sqrt(num_all) / std::sqrt(std::max(den_all, 1e-30));
        results[cfg].degenerate_all = degen;

        std::printf("  Pooled (%s):  rho_all = %.4f\n",
                    config_n ? "CONFIG-N" : "CONFIG-M", results[cfg].rho_all_pooled);
        std::printf("    grade      rho_DK(m*)  rho_DK(0)  rho_KG    form\n");
        for (int k = 0; k < 4; ++k) {
            const double rdk = results[cfg].rho_k_pooled[k];
            const double rkg = results[cfg].rho_kg_pooled[k];
            const char* form = (rdk + 0.10 <= rkg) ? "DIRAC-FORM"
                             : (rkg + 0.10 <= rdk) ? "KG-FORM" : "TIE";
            std::printf("    %-12s %9.4f  %9.4f  %8.4f  %s\n",
                        grade_name[k], rdk, results[cfg].rho_k_m0_pooled[k], rkg, form);
        }
        std::printf("\n");
    }

    // ------------------------------------------------------ sanity anchor
    {
        const double rdk = results[1].rho_k_pooled[1];
        const double rkg = results[1].rho_kg_pooled[1];
        // anchor: CONFIG-M grade 1 must be KG-FORM or TIE (not DIRAC-FORM)
        const bool dirac_won = (rdk + 0.10 <= rkg);
        std::printf("--- Pre-registered sanity anchor ---\n");
        std::printf("  CONFIG-M grade 1: rho_DK = %.4f, rho_KG = %.4f -> %s\n",
                    rdk, rkg,
                    dirac_won ? "DIRAC-FORM  ** HARNESS SUSPECT (prereg S2.5) **"
                              : "KG-FORM or TIE (anchor holds)");
    }

    // ---------------------------------------------------------- verdict
    const ConfigResult& R = results[0];   // CONFIG-N is primary
    double m_lo = R.mstar[0], m_hi = R.mstar[0], m_absmean = 0;
    for (int p = 0; p < 3; ++p) {
        m_lo = std::min(m_lo, R.mstar[p]);
        m_hi = std::max(m_hi, R.mstar[p]);
        m_absmean += std::abs(R.mstar[p]) / 3.0;
    }
    const double m_spread = m_absmean > 1e-12 ? (m_hi - m_lo) / m_absmean : 1e9;

    bool all_k_ok = true, any_k_tight = false, all_k_static = true;
    for (int k = 0; k < 4; ++k) {
        if (R.rho_k_pooled[k] >= 0.25) all_k_ok = false;
        if (R.rho_k_pooled[k] <  0.15) any_k_tight = true;
        if (R.rho_k_pooled[k] <  0.50) all_k_static = false;
    }

    std::printf("\n================================================================\n");
    std::printf("  M1 Verdict (pre-registered criteria, PREREG S2.5, CONFIG-N)\n");
    std::printf("================================================================\n");
    std::printf("  rho_all = %.4f;  per-grade rho = %.4f / %.4f / %.4f / %.4f\n",
                R.rho_all_pooled, R.rho_k_pooled[0], R.rho_k_pooled[1],
                R.rho_k_pooled[2], R.rho_k_pooled[3]);
    std::printf("  m* per pair = %+.5f / %+.5f / %+.5f  (relative spread %.1f%%)\n",
                R.mstar[0], R.mstar[1], R.mstar[2], 100.0 * m_spread);

    const char* verdict;
    if (R.rho_all_pooled < 0.15 && all_k_ok && m_spread < 0.30) {
        verdict = "DK-DYNAMICAL";
        std::printf("\n  ==> DK-DYNAMICAL: the engine's evolution satisfies the discrete\n");
        std::printf("      Dirac-Kahler equation at the tested protocol. [MEASURED]\n");
    } else if (any_k_tight) {
        verdict = "DK-PARTIAL";
        std::printf("\n  ==> DK-PARTIAL: at least one grade equation holds (rho < 0.15)\n");
        std::printf("      while the joint system does not. See per-grade table.\n");
    } else if (all_k_static || R.degenerate_all) {
        verdict = "DK-STATIC-ONLY";
        std::printf("\n  ==> DK-STATIC-ONLY: no grade equation captures the evolution\n");
        std::printf("      (all rho >= 0.50). FTD-0089 A1 is a kinematic identification\n");
        std::printf("      only; the dynamical DK hypothesis at this protocol is closed\n");
        std::printf("      negative.\n");
    } else {
        verdict = "UNDETERMINED";
        std::printf("\n  ==> UNDETERMINED: residuals fall between the pre-registered\n");
        std::printf("      bands. Reported as measured; no tag movement.\n");
    }
    std::printf("\n  VERDICT: %s\n", verdict);
    std::printf("================================================================\n");
    return 0;
}
