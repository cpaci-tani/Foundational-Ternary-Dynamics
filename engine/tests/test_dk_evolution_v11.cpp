/**
 * @file test_dk_evolution_v11.cpp
 * @brief M1 v1.1 (FTD-0379 scope extension) — corrected-operator, free-scale
 *        Dirac-Kähler evolution re-test.
 *
 * Pre-registered in PREREG_VERTEX_DK_CLOSURE_v1_1.md — locked before this
 * runner produced any output. Closes the two instrument defects the
 * adversarial math review established in M1 v1:
 *
 *   (1) v1 executed FTD-0089 §A1.3's literal codifferential convention,
 *       which is provably NOT the Dirac-Kähler operator (delta != d*, so
 *       D = d - delta is not skew-adjoint and D² != -Hodge-Laplacian).
 *       v1.1 uses the true adjoints (uniform <d phi, psi> = <phi, delta psi>),
 *       asserted numerically along with skew-adjointness of D and
 *       D² = -Delta componentwise.
 *   (2) v1 locked unit coefficient on (d - delta). v1.1 fits
 *       dPhi/dt = a (d - delta) Phi - m Phi with free (a, m), per-grade
 *       weighted (1/sum||dphi_k/dt||²) so amplitude-degree inhomogeneity
 *       cannot let one grade dominate the joint objective.
 *
 * Same protocol, seeds, pairs, configs, fit window as v1.
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

struct Lat {
    int L;
    int idx(int x, int y, int z) const { return (x * L + y) * L + z; }
    int wp(int c) const { return (c + 1) % L; }
    int wm(int c) const { return (c + L - 1) % L; }
    int n3() const { return L * L * L; }
};

using Field  = std::vector<double>;
using Field3 = std::array<std::vector<double>, 3>;

Field  mkF (const Lat& g) { return Field (g.n3(), 0.0); }
Field3 mkF3(const Lat& g) { return { Field(g.n3(),0.0), Field(g.n3(),0.0), Field(g.n3(),0.0) }; }

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
// d operators: identical to v1. delta operators: TRUE adjoints (v1.1).
Field3 d0(const Lat& g, const Field& f) {
    Field3 out = mkF3(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        for (int a = 0; a < 3; ++a) out[a][s] = f[shift_p(g,x,y,z,a)] - f[s];
    }
    return out;
}
Field3 d1(const Lat& g, const Field3& V) {
    Field3 out = mkF3(g);
    static const int I[3] = {0, 0, 1}, J[3] = {1, 2, 2};
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        for (int p = 0; p < 3; ++p) {
            const int i = I[p], j = J[p];
            out[p][s] = (V[j][shift_p(g,x,y,z,i)] - V[j][s])
                      - (V[i][shift_p(g,x,y,z,j)] - V[i][s]);
        }
    }
    return out;
}
Field d2(const Lat& g, const Field3& P) {
    Field out = mkF(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        auto gp = [&](const Field& f, int a) { return f[shift_p(g,x,y,z,a)] - f[s]; };
        out[s] = gp(P[2], 0) - gp(P[1], 1) + gp(P[0], 2);
    }
    return out;
}
// delta1 = (d0)^T : <d0 f, W> = <f, delta1 W>  =>  delta1 W = -div^- W
Field delta1(const Lat& g, const Field3& V) {
    Field out = mkF(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        double acc = 0.0;
        for (int a = 0; a < 3; ++a) acc += V[a][s] - V[a][shift_m(g,x,y,z,a)];
        out[s] = -acc;
    }
    return out;
}
// delta2 = (d1)^T : unchanged from v1 (already the true adjoint)
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
// delta3 = (d2)^T : sign-flipped relative to v1
Field3 delta3(const Lat& g, const Field& T) {
    Field3 out = mkF3(g);
    for (int x = 0; x < g.L; ++x) for (int y = 0; y < g.L; ++y) for (int z = 0; z < g.L; ++z) {
        const int s = g.idx(x,y,z);
        auto gm = [&](int a) { return T[s] - T[shift_m(g,x,y,z,a)]; };
        out[0][s] = -gm(2);
        out[1][s] =  gm(1);
        out[2][s] = -gm(0);
    }
    return out;
}
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

// multi-grade field and the full DK operator D = d - delta (true adjoints)
struct MG {
    Field  g0, g3;
    Field3 g1, g2;
};
MG mkMG(const Lat& g) { return MG{ mkF(g), mkF(g), mkF3(g), mkF3(g) }; }

MG applyD(const Lat& g, const MG& p) {
    MG out = mkMG(g);
    Field  dl1 = delta1(g, p.g1);
    Field3 ds  = d0(g, p.g0);
    Field3 dl2 = delta2(g, p.g2);
    Field3 dv  = d1(g, p.g1);
    Field3 dl3 = delta3(g, p.g3);
    Field  dp  = d2(g, p.g2);
    for (int s = 0; s < g.n3(); ++s) {
        out.g0[s] = -dl1[s];
        for (int a = 0; a < 3; ++a) {
            out.g1[a][s] = ds[a][s] - dl2[a][s];
            out.g2[a][s] = dv[a][s] - dl3[a][s];
        }
        out.g3[s] = dp[s];
    }
    return out;
}
double dotMG(const MG& a, const MG& b) {
    return dotF(a.g0,b.g0) + dotF3(a.g1,b.g1) + dotF3(a.g2,b.g2) + dotF(a.g3,b.g3);
}

bool self_checks(int L) {
    Lat g{L};
    unsigned st = 0xC0FFEE01u;
    auto rnd = [&st]() {
        st = st * 1664525u + 1013904223u;
        return (double)(st >> 8) / (double)(1u << 24) - 0.5;
    };
    MG A = mkMG(g), B = mkMG(g);
    Field f = mkF(g), u = mkF(g);
    Field3 W = mkF3(g), Q = mkF3(g), V = mkF3(g), P = mkF3(g);
    for (int s = 0; s < g.n3(); ++s) {
        f[s] = rnd(); u[s] = rnd();
        A.g0[s] = rnd(); A.g3[s] = rnd(); B.g0[s] = rnd(); B.g3[s] = rnd();
        for (int a = 0; a < 3; ++a) {
            W[a][s]=rnd(); Q[a][s]=rnd(); V[a][s]=rnd(); P[a][s]=rnd();
            A.g1[a][s]=rnd(); A.g2[a][s]=rnd(); B.g1[a][s]=rnd(); B.g2[a][s]=rnd();
        }
    }
    bool ok = true;
    auto rel_check = [&ok](const char* name, double lhs, double rhs, double scale) {
        const double d = std::abs(lhs - rhs) / std::max(scale, 1e-30);
        const bool pass = d < 1e-12;
        std::printf("    [%s] %-34s rel.err = %.2e\n", pass ? "PASS" : "FAIL", name, d);
        if (!pass) ok = false;
    };
    // UNIFORM adjointness (the true-DK requirement)
    {
        const double l = dotF3(d0(g,f), W), r = dotF(f, delta1(g,W));
        rel_check("<d0 f, W> = +<f, delta1 W>", l, r, std::abs(l)+std::abs(r));
    }
    {
        const double l = dotF3(d1(g,V), Q), r = dotF3(V, delta2(g,Q));
        rel_check("<d1 V, Q> = +<V, delta2 Q>", l, r, std::abs(l)+std::abs(r));
    }
    {
        const double l = dotF(d2(g,P), u), r = dotF3(P, delta3(g,u));
        rel_check("<d2 P, u> = +<P, delta3 u>", l, r, std::abs(l)+std::abs(r));
    }
    // skew-adjointness of D — the Dirac-ness certificate
    {
        const double l = dotMG(applyD(g,A), B), r = -dotMG(A, applyD(g,B));
        rel_check("<D A, B> = -<A, D B>", l, r, std::abs(l)+std::abs(r));
    }
    // D^2 = -Hodge-Laplacian = +componentwise second-difference operator
    // (Delta_H = dd* + d*d is POSITIVE semidefinite; the componentwise
    // lattice lap is NEGATIVE semidefinite; on the flat torus
    // Delta_H = -lap, so D^2 = -Delta_H = +lap. First lock's gate had the
    // sign inverted; corrected pre-measurement, 2026-07-10 — no dynamics
    // output was observed before the correction.)
    {
        MG DD = applyD(g, applyD(g, A));
        Field l0 = lap(g, A.g0), l3 = lap(g, A.g3);
        double num = 0, den = 0;
        for (int s = 0; s < g.n3(); ++s) {
            num += (DD.g0[s] - l0[s]) * (DD.g0[s] - l0[s]);
            num += (DD.g3[s] - l3[s]) * (DD.g3[s] - l3[s]);
            den += l0[s]*l0[s] + l3[s]*l3[s];
        }
        for (int a = 0; a < 3; ++a) {
            Field l1 = lap(g, A.g1[a]), l2 = lap(g, A.g2[a]);
            for (int s = 0; s < g.n3(); ++s) {
                num += (DD.g1[a][s] - l1[s]) * (DD.g1[a][s] - l1[s]);
                num += (DD.g2[a][s] - l2[s]) * (DD.g2[a][s] - l2[s]);
                den += l1[s]*l1[s] + l2[s]*l2[s];
            }
        }
        const double d = std::sqrt(num) / std::sqrt(std::max(den, 1e-30));
        const bool pass = d < 1e-12;
        std::printf("    [%s] %-34s rel.norm = %.2e\n", pass ? "PASS" : "FAIL",
                    "D^2 = +lap = -Hodge (all grades)", d);
        if (!pass) ok = false;
    }
    return ok;
}

// ------------------------------------------------------------ grade fields
struct Grades { Field S, T; Field3 V, P; };
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
            gr.P[p][s] = comp(fl, i) * comp(vox[shift_p(g,x,y,z,i)].flux, j)
                       - comp(vox[shift_p(g,x,y,z,j)].flux, i) * comp(fl, j);
        }
    }
    return gr;
}
MG toMG(const Lat& g, const Grades& gr) {
    MG m = mkMG(g);
    m.g0 = gr.S; m.g3 = gr.T; m.g1 = gr.V; m.g2 = gr.P;
    return m;
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
void enable_config_M(ftd::TermToggles& t) {
    t.disable_all();
    t.wave_propagation  = true;
    t.gauss_projection  = true;
}
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
    hist.push_back(compute_grades(rb.voxels(), g));
    for (int t = 1; t <= n_ticks; ++t) {
        rb.run(1);
        hist.push_back(compute_grades(rb.voxels(), g));
    }
    return hist;
}

// ------------------------------------------ two-parameter weighted DK fit
// rows: y = a*u + m*w   (y = dPhi/dt, u = D Phi_mid, w = -Phi_mid)
struct DKAccum2 {
    // per grade: 0=S,1=V,2=P,3=T
    double Suu[4]={0,0,0,0}, Sww[4]={0,0,0,0}, Suw[4]={0,0,0,0};
    double Syu[4]={0,0,0,0}, Syw[4]={0,0,0,0}, Syy[4]={0,0,0,0};
    void add(int k, double y, double u, double w) {
        Suu[k]+=u*u; Sww[k]+=w*w; Suw[k]+=u*w;
        Syu[k]+=y*u; Syw[k]+=y*w; Syy[k]+=y*y;
    }
    // weighted joint solve, weights w_k = 1 / max(Syy_k, floor)
    void solve(double& a, double& m) const {
        double A11=0,A12=0,A22=0,B1=0,B2=0;
        for (int k = 0; k < 4; ++k) {
            const double wk = 1.0 / std::max(Syy[k], 1e-30);
            A11 += wk*Suu[k]; A12 += wk*Suw[k]; A22 += wk*Sww[k];
            B1  += wk*Syu[k]; B2  += wk*Syw[k];
        }
        const double det = A11*A22 - A12*A12;
        if (std::abs(det) < 1e-30) { a = m = 0.0; return; }
        a = ( A22*B1 - A12*B2) / det;
        m = ( A11*B2 - A12*B1) / det;
    }
    double rho_grade(int k, double a, double m) const {
        const double r2 = Syy[k] - 2.0*a*Syu[k] - 2.0*m*Syw[k]
                        + a*a*Suu[k] + m*m*Sww[k] + 2.0*a*m*Suw[k];
        return std::sqrt(std::max(r2, 0.0)) / std::sqrt(std::max(Syy[k], 1e-30));
    }
    double rho_all(double a, double m) const {
        double num = 0, den = 0;
        for (int k = 0; k < 4; ++k) {
            const double r2 = Syy[k] - 2.0*a*Syu[k] - 2.0*m*Syw[k]
                            + a*a*Suu[k] + m*m*Sww[k] + 2.0*a*m*Suw[k];
            num += std::max(r2, 0.0); den += Syy[k];
        }
        return std::sqrt(num) / std::sqrt(std::max(den, 1e-30));
    }
};
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
                    int t_lo, int t_hi, DKAccum2& dk, KGAccum kg[4]) {
    for (int t = t_lo; t < t_hi; ++t) {
        const Grades& a = hist[t];
        const Grades& b = hist[t + 1];
        Grades mid{ mkF(g), mkF(g), mkF3(g), mkF3(g) };
        for (int s = 0; s < g.n3(); ++s) {
            mid.S[s] = 0.5 * (a.S[s] + b.S[s]);
            mid.T[s] = 0.5 * (a.T[s] + b.T[s]);
            for (int c = 0; c < 3; ++c) {
                mid.V[c][s] = 0.5 * (a.V[c][s] + b.V[c][s]);
                mid.P[c][s] = 0.5 * (a.P[c][s] + b.P[c][s]);
            }
        }
        const MG D = applyD(g, toMG(g, mid));
        for (int s = 0; s < g.n3(); ++s) {
            dk.add(0, b.S[s] - a.S[s], D.g0[s], -mid.S[s]);
            for (int c = 0; c < 3; ++c) {
                dk.add(1, b.V[c][s] - a.V[c][s], D.g1[c][s], -mid.V[c][s]);
                dk.add(2, b.P[c][s] - a.P[c][s], D.g2[c][s], -mid.P[c][s]);
            }
            dk.add(3, b.T[s] - a.T[s], D.g3[s], -mid.T[s]);
        }
    }
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
    std::printf("  M1 v1.1 (FTD-0379 ext) - corrected-operator, free-scale DK test\n");
    std::printf("  PREREG_VERTEX_DK_CLOSURE_v1_1.md - locked before first run\n");
    std::printf("================================================================\n\n");

    std::printf("--- Harness validity gates (true-DK operator, L=8) ---\n");
    if (!self_checks(8)) {
        std::printf("\n  HARNESS INVALID - operator self-checks failed. Aborting.\n");
        return 1;
    }
    std::printf("\n");

    const int    L = 8;
    const double A = 10.0;
    const int    N_TICKS = 30, T_LO = 4, T_HI = 28;
    const std::array<unsigned, 8> seeds = {
        0xF4170517u, 0xF4170518u, 0xF4170519u, 0xF417051Au,
        0xF417051Bu, 0xF417051Cu, 0xF417051Du, 0xF417051Eu
    };
    const std::array<std::pair<int,int>, 3> pairs = {{ {0,1}, {0,2}, {1,2} }};
    const char* pair_name[3]  = { "(x,y)", "(x,z)", "(y,z)" };
    const char* grade_name[4] = { "0 (scalar S)", "1 (vector V)",
                                  "2 (bivector P)", "3 (pseudo T)" };
    const Lat g{L};

    struct CfgRes {
        double a[3], m[3];
        double rho_k[4], rho_all;
        double rho_kg[4];
    };
    CfgRes res[2];

    for (int cfg = 0; cfg < 2; ++cfg) {
        const bool config_n = (cfg == 0);
        std::printf("===== CONFIG-%s =====\n",
                    config_n ? "N (full non-local)" : "M (wave+gauss control)");
        double pn[4] = {0,0,0,0}, pd[4] = {0,0,0,0};
        double kn[4] = {0,0,0,0}, kd[4] = {0,0,0,0};
        for (int p = 0; p < 3; ++p) {
            DKAccum2 dk;
            KGAccum kg[4];
            for (unsigned seed : seeds) {
                auto hist = run_record(L, A, seed, pairs[p].first, pairs[p].second,
                                       config_n, N_TICKS);
                accumulate_run(g, hist, T_LO, T_HI, dk, kg);
            }
            double a, m; dk.solve(a, m);
            res[cfg].a[p] = a; res[cfg].m[p] = m;
            std::printf("  Pair %s:  a* = %+9.5f   m* = %+9.5f\n", pair_name[p], a, m);
            std::printf("    grade         rho_DK(a*,m*)   rho_KG\n");
            for (int k = 0; k < 4; ++k) {
                const double rdk = dk.rho_grade(k, a, m);
                const double rkg = kg[k].rho();
                std::printf("    %-13s %10.4f    %8.4f\n", grade_name[k], rdk, rkg);
                const double r2 = rdk * rdk * std::max(dk.Syy[k], 1e-30);
                pn[k] += r2; pd[k] += std::max(dk.Syy[k], 1e-30);
                kn[k] += rkg * rkg * std::max(kg[k].Syy, 1e-30);
                kd[k] += std::max(kg[k].Syy, 1e-30);
            }
            std::printf("\n");
        }
        double num_all = 0, den_all = 0;
        for (int k = 0; k < 4; ++k) {
            res[cfg].rho_k[k]  = std::sqrt(pn[k]) / std::sqrt(pd[k]);
            res[cfg].rho_kg[k] = std::sqrt(kn[k]) / std::sqrt(kd[k]);
            num_all += pn[k]; den_all += pd[k];
        }
        res[cfg].rho_all = std::sqrt(num_all) / std::sqrt(den_all);
        std::printf("  Pooled: rho_all = %.4f\n", res[cfg].rho_all);
        std::printf("    grade         rho_DK    rho_KG   form\n");
        for (int k = 0; k < 4; ++k) {
            const double rdk = res[cfg].rho_k[k], rkg = res[cfg].rho_kg[k];
            const char* form = (rdk + 0.10 <= rkg) ? "DIRAC-FORM"
                             : (rkg + 0.10 <= rdk) ? "KG-FORM" : "TIE";
            std::printf("    %-13s %8.4f  %8.4f  %s\n", grade_name[k], rdk, rkg, form);
        }
        std::printf("\n");
    }

    {
        const double rdk = res[1].rho_k[1], rkg = res[1].rho_kg[1];
        const bool dirac_won = (rdk + 0.10 <= rkg);
        std::printf("--- Sanity anchor: CONFIG-M grade 1 -> %s (rho_DK %.4f vs rho_KG %.4f)\n",
                    dirac_won ? "DIRAC-FORM ** HARNESS SUSPECT **" : "KG-FORM or TIE (holds)",
                    rdk, rkg);
    }

    const CfgRes& R = res[0];
    double a_lo = R.a[0], a_hi = R.a[0], a_absmean = 0;
    for (int p = 0; p < 3; ++p) {
        a_lo = std::min(a_lo, R.a[p]); a_hi = std::max(a_hi, R.a[p]);
        a_absmean += std::abs(R.a[p]) / 3.0;
    }
    const double a_spread = a_absmean > 1e-12 ? (a_hi - a_lo) / a_absmean : 1e9;

    bool all_k_ok = true, any_k_tight = false, all_k_static = true;
    for (int k = 0; k < 4; ++k) {
        if (R.rho_k[k] >= 0.25) all_k_ok = false;
        if (R.rho_k[k] <  0.15) any_k_tight = true;
        if (R.rho_k[k] <  0.50) all_k_static = false;
    }

    std::printf("\n================================================================\n");
    std::printf("  M1 v1.1 Verdict (PREREG v1_1 S2, CONFIG-N)\n");
    std::printf("================================================================\n");
    std::printf("  rho_all = %.4f;  per-grade rho = %.4f / %.4f / %.4f / %.4f\n",
                R.rho_all, R.rho_k[0], R.rho_k[1], R.rho_k[2], R.rho_k[3]);
    std::printf("  a* per pair = %+.5f / %+.5f / %+.5f  (spread %.1f%%);  m* = %+.5f / %+.5f / %+.5f\n",
                R.a[0], R.a[1], R.a[2], 100.0 * a_spread, R.m[0], R.m[1], R.m[2]);

    const char* verdict;
    if (R.rho_all < 0.15 && all_k_ok && a_spread < 0.30) verdict = "DK-DYNAMICAL";
    else if (any_k_tight)                                 verdict = "DK-PARTIAL";
    else if (all_k_static)                                verdict = "DK-STATIC-ONLY";
    else                                                  verdict = "UNDETERMINED";
    std::printf("\n  VERDICT: %s\n", verdict);
    std::printf("================================================================\n");
    return 0;
}
