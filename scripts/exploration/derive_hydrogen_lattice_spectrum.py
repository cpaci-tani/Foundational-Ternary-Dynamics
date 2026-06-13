#!/usr/bin/env python3
"""
derive_hydrogen_lattice_spectrum.py -- FTD-0278 Leg 1 (operator spectroscopy)

DOES THE CLOCKED FLUX FIELD, BOUND IN THE ENGINE'S EXACT LATTICE COULOMB
POTENTIAL, PRODUCE A RYDBERG LADDER E_n ~ -R/n^2?

THE CONSTRUCTION (every ingredient engine-exact or registered-imposed):
  * 18-pt O_h Laplacian L18 (engine phase_read stencil; THEOREM-grade symbol
    M(k), verified to machine precision -- gate G-1).
  * Lattice Coulomb potential V(r) = -q_eff * phiG(r): phiG is the PERIODIC
    lattice Green's function of L18 (the engine's own Gauss/Poisson solution
    for a unit point charge; OT-1.4 / Phase-G THEOREM), mean-free, with the
    far-field offset subtracted (declared) so V -> 0 away from the charge.
  * de Broglie clock omega0 [IMPOSED, FTD-0271] + scalar-potential coupling
    omega_eff^2(r) = omega0^2 + 2*omega0*V(r) [IMPOSED -- the same structural
    move the engine makes for gravity: latency modulates the local clock rate].

THE OPERATOR. The engine wave equation with the coupled clock is
    d^2 J/dt^2 = c^2 Lap18 J - (omega0^2 + 2 omega0 V) J,
so eigenmodes satisfy  omega_n = sqrt(omega0^2 + a_n)  where a_n are the
eigenvalues of the SECOND-ORDER operator
    A = -c^2 L18 + 2 omega0 V(r)            (sparse symmetric).
Bound states: a_n < 0 (omega_n < omega0). The envelope energy is
    E_n = omega_n - omega0  ~  a_n / (2 omega0)   (non-relativistic limit),
i.e. exactly the Schrodinger Hamiltonian H = -(c^2/(2 omega0)) L18 + V.

SCHRODINGER-LIMIT PREDICTIONS (continuum; the lattice corrections are what the
falsifier bands must absorb): effective mass m = omega0/c^2;
    a0 = 4*pi/(m q)  =>  q(a0) = 4*pi*c^2/(omega0*a0)
    R  = m q^2/(2 (4 pi)^2),   E_n = -R/n^2,   R ~ q^2 => R(q) ~ q^4 at fixed a0-scaling.

STABILITY BOUND (tachyon guard): omega_eff^2(0) > 0 requires
    2*omega0*q*|phiG(0)| < omega0^2   =>   q < omega0 / (2 |phiG(0)|).
With |phiG(0)| ~ 0.22 (18-pt lattice) and omega0 = 1.0: q < ~2.27.

[EPISTEMIC: development/quick-check tier until the pre-registration lock. The
 result class is [CONDITIONAL -- DERIVED-GIVEN-IMPOSED-INPUT] + lattice
 correctness; KG-in-Coulomb is textbook; NEVER 'FTD derives QM'.]

Usage:
  python derive_hydrogen_lattice_spectrum.py --dev          # development survey
  python derive_hydrogen_lattice_spectrum.py --record --out results.csv
"""

import argparse
import csv
import math
import sys

import numpy as np
import scipy.sparse as sp
import scipy.sparse.linalg as spla

C2 = 1.0 / 3.0
C = math.sqrt(C2)
OMEGA0 = 1.0          # [IMPOSED] clock scalar for this arc (omega0*dt<2 ok)

FACE = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
EDGE = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_L18(L, periodic=True):
    """Sparse 18-pt O_h Laplacian on L^3 (engine stencil)."""
    N = L * L * L
    idx = lambda x, y, z: (x * L + y) * L + z
    rows, cols, vals = [], [], []
    for x in range(L):
        for y in range(L):
            for z in range(L):
                i = idx(x, y, z)
                rows.append(i); cols.append(i); vals.append(-4.0)
                for off, w in ((FACE, 1.0 / 3.0), (EDGE, 1.0 / 6.0)):
                    for dx, dy, dz in off:
                        nx, ny, nz = x + dx, y + dy, z + dz
                        if periodic:
                            nx %= L; ny %= L; nz %= L
                        elif not (0 <= nx < L and 0 <= ny < L and 0 <= nz < L):
                            continue
                        rows.append(i); cols.append(idx(nx, ny, nz)); vals.append(w)
    return sp.csr_matrix((vals, (rows, cols)), shape=(N, N))


def symbol_M(kx, ky, kz):
    cx, cy, cz = math.cos(kx), math.cos(ky), math.cos(kz)
    return (2.0 / 3.0) * (cx + cy + cz) + (2.0 / 3.0) * (cx * cy + cy * cz + cz * cx) - 4.0


def validate_symbol(L=8):
    """G-1: periodic eigenvalues match the closed-form symbol (machine precision)."""
    A = build_L18(L, periodic=True)
    worst = 0.0
    idx = lambda x, y, z: (x * L + y) * L + z
    v = np.zeros(L * L * L, dtype=complex)
    for (nx, ny, nz) in [(0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1), (2, 1, 0)]:
        kx, ky, kz = 2 * math.pi * nx / L, 2 * math.pi * ny / L, 2 * math.pi * nz / L
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    v[idx(x, y, z)] = np.exp(1j * (kx * x + ky * y + kz * z))
        eig = (np.vdot(v, A @ v) / np.vdot(v, v)).real
        worst = max(worst, abs(eig - symbol_M(kx, ky, kz)))
    return worst


def green_potential(L):
    """Periodic lattice Green's function phiG of L18 for a +1 point charge at the
    origin: Lap18 phiG = delta - 1/N (mean-free torus). Engine-exact (the FFT
    Gauss solve). Returns phiG as an (L,L,L) array (phiG(0) < 0)."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    cx = np.cos(k)
    CX = cx[:, None, None]; CY = cx[None, :, None]; CZ = cx[None, None, :]
    M = (2.0 / 3.0) * (CX + CY + CZ) + (2.0 / 3.0) * (CX * CY + CY * CZ + CZ * CX) - 4.0
    src = np.zeros((L, L, L)); src[0, 0, 0] = 1.0
    inv = np.zeros_like(M)
    nz = np.abs(M) > 1e-12
    inv[nz] = 1.0 / M[nz]
    inv[0, 0, 0] = 0.0
    phi = np.fft.ifftn(np.fft.fftn(src) * inv).real
    return phi


def coulomb_well(L, q):
    """Engine-convention attractive well: V = +q * phiG with phiG the MEAN-FREE
    periodic lattice Green's function (the FFT zero-mode kill IS the engine's
    convention — no further offset). V(0) < 0 (attractive core), small positive
    plateau far away (mean-free on the torus). Offset conventions shift the whole
    spectrum by a constant; all falsifier observables are GAPS (offset-invariant)."""
    phi = green_potential(L)        # already mean-free (zero mode removed)
    V = q * phi
    return V, 0.0, float(phi[0, 0, 0])


def continuum_well(L, q):
    """REFERENCE (diagnostic, not FTD): periodized continuum Coulomb
    V = -q/(4 pi r_mi) (minimum-image), core regularized to the lattice
    Green's self-value, then MEAN-FREED — the identical convention as
    coulomb_well, so the lattice-vs-1/r comparison is offset-honest.
    Separates 'lattice Green vs 1/r' effects from torus truncation (same L)."""
    idx = np.indices((L, L, L))
    half = L // 2
    d = ((idx + half) % L) - half
    r = np.sqrt((d[0] ** 2 + d[1] ** 2 + d[2] ** 2).astype(float))
    r[0, 0, 0] = 1.0   # placeholder
    V = -q / (4.0 * math.pi * r)
    V[0, 0, 0] = q * float(green_potential(L)[0, 0, 0])
    V -= V.mean()      # engine convention: mean-free
    return V


def spectrum(L, q, omega0=OMEGA0, k_eigs=10, potential="lattice"):
    """Eigenvalues a_n of A = -c^2 L18 + 2 omega0 V; returns (a_n sorted,
    omega_n, E_n = omega_n - omega0, V0, n_bound).
    potential = 'lattice' (engine Green's function) | 'continuum' (1/r reference)."""
    A_lap = build_L18(L, periodic=True)
    if potential == "lattice":
        V, offset, phi0 = coulomb_well(L, q)
    else:
        V = continuum_well(L, q)
    Vdiag = sp.diags(2.0 * omega0 * V.reshape(-1))
    A = (-C2) * A_lap + Vdiag
    # lowest (most negative) eigenvalues
    vals = spla.eigsh(A, k=k_eigs, which='SA', return_eigenvectors=False)
    a = np.sort(vals)
    omega2 = omega0 ** 2 + a
    if np.any(omega2 <= 0):
        return None  # tachyonic — q too deep for this omega0
    omega = np.sqrt(omega2)
    E = omega - omega0
    n_bound = int(np.sum(a < -1e-12))
    return a, omega, E, float(V.min()), n_bound


def schrod_prediction(q, omega0=OMEGA0):
    """Continuum Schrodinger-limit a0 and Rydberg for V = -q/(4 pi r),
    m = omega0/c^2."""
    m = omega0 / C2
    a0 = 4.0 * math.pi / (m * q)
    R = m * q * q / (2.0 * (4.0 * math.pi) ** 2)
    return a0, R


def dev_survey(Ls, qs, omega0=OMEGA0):
    print("=" * 72)
    print("FTD-0278 Leg 1 -- DEVELOPMENT survey (no verdicts)")
    print(f"omega0 = {omega0}  (tachyon guard: q < omega0/(2|phiG(0)|))")
    print("=" * 72)
    werr = validate_symbol(8)
    print(f"[G-1] |eig(L18) - M(k)| = {werr:.2e}  "
          f"({'PASS' if werr < 1e-10 else 'FAIL'})")
    for q in qs:
        a0, R = schrod_prediction(q, omega0)
        print(f"\n--- q = {q:.4f}  (Schrodinger-limit a0 = {a0:.2f}, R = {R:.5f}, "
              f"E1 = {-R:.5f}, E2 = {-R/4:.5f}) ---")
        for L in Ls:
            res = spectrum(L, q, omega0)
            if res is None:
                print(f"  L={L:3d}: TACHYONIC (q too deep)")
                continue
            a, omega, E, vmin, nb = res
            Estr = " ".join(f"{e:+.5f}" for e in E[:6])
            print(f"  L={L:3d}: V_min={vmin:+.4f}  E_n: {Estr}")
            # GAP observables (offset-invariant): n=2 multiplet = states 2..5
            # (A1g + T1u on O_h); Rydberg prediction: gap12 = (3/4) R.
            gap12 = float(np.mean(E[1:5])) - E[0]
            print(f"          gap12 = {gap12:+.5f}  vs (3/4)R = {0.75*R:+.5f}  "
                  f"ratio = {gap12/(0.75*R):.3f}")
            # continuum-1/r reference at the same L (identical mean-free
            # convention; torus truncation identical)
            resc = spectrum(L, q, omega0, potential="continuum")
            if resc is not None:
                ac, omc, Ec, vminc, nbc = resc
                gap12c = float(np.mean(Ec[1:5])) - Ec[0]
                print(f"          [1/r ref] gap12 = {gap12c:+.5f}  "
                      f"lattice/ref gap ratio = {gap12/gap12c:.4f}")


def box_ground_dirichlet(L):
    """Free Dirichlet box ground mode (the FTD-0270 control): returns
    (omega1_massless, E_env_kg) at the record omega0."""
    A = build_L18(L, periodic=False)
    vals = spla.eigsh(A, k=4, which='LA', return_eigenvectors=False)
    M1 = min(float(np.max(vals)), -1e-12)
    omega1 = C * math.sqrt(-M1)
    e_env = math.sqrt(C2 * (-M1) + OMEGA0_RECORD ** 2) - OMEGA0_RECORD
    return omega1, e_env


# ---- FROZEN RECORD PROTOCOL (locked by PREREG_HYDROGEN_LATTICE_SPECTRUM_v1) --
OMEGA0_RECORD = 1.5
RECORD_QS = [1.1170, 0.9308, 0.6981]      # a0 = 2.5, 3.0, 4.0 at omega0=1.5
RECORD_LS = [48, 64]
CTRL_LS = [12, 16, 20, 24, 32]            # FTD-0270 control grid
FA_TOL = 0.05         # F-A: |lattice/ref gap12 ratio - 1| <= 0.05, all cells
FB_MAX_A4 = 1.40      # F-B: ratio(a0=4, L=64) in (1.0, 1.40) + monotone in a0
FC_T1U_TOL = 0.05     # F-C: T1u internal spread <= 5% of gap12
FC_SPLIT_TOL = 0.50   # F-C: A1g-T1u splitting <= 50% of gap12
FE_S_BAND = (0.8, 1.2)   # F-E: massless Dirichlet s in FTD-0270 band


def record_run(out_csv=None):
    """The frozen run of record. Mechanical; no parameter choices here."""
    print("=" * 72)
    print("FTD-0278 Leg 1 -- RUN OF RECORD (frozen protocol)")
    print(f"omega0={OMEGA0_RECORD}  qs={RECORD_QS}  Ls={RECORD_LS}")
    print("=" * 72)
    rows = []
    results = {}

    # G-1 operator correctness
    werr = validate_symbol(8)
    g1 = werr < 1e-10
    print(f"[G-1] |eig(L18)-M(k)| = {werr:.2e}  -> {'PASS' if g1 else 'FAIL'}")
    rows.append(("G1", "symbol_err", werr, ""))

    # F-E causal control: massless Dirichlet box scaling (FTD-0270)
    omgs = []
    for L in CTRL_LS:
        o, _ = box_ground_dirichlet(L)
        omgs.append(o)
    lx = np.log(np.array(CTRL_LS, float)); ly = np.log(np.array(omgs))
    s_massless = -float(np.polyfit(lx, ly, 1)[0])
    fe = FE_S_BAND[0] <= s_massless <= FE_S_BAND[1]
    print(f"[F-E] massless Dirichlet s = {s_massless:.3f} "
          f"(band {FE_S_BAND}) -> {'PASS' if fe else 'FAIL'}")
    rows.append(("FE", "s_massless", s_massless, "PASS" if fe else "FAIL"))

    # spectroscopy grid
    fa_ratios, fb_by_a0, fc_flags = [], {}, []
    m = OMEGA0_RECORD / C2
    for q in RECORD_QS:
        a0 = 4.0 * math.pi / (m * q)
        R = m * q * q / (2.0 * (4.0 * math.pi) ** 2)
        for L in RECORD_LS:
            res = spectrum(L, q, OMEGA0_RECORD)
            resc = spectrum(L, q, OMEGA0_RECORD, potential="continuum")
            if res is None or resc is None:
                print(f"  q={q} L={L}: TACHYONIC -> grid cell INVALID")
                rows.append((f"q{q}_L{L}", "tachyonic", 1.0, "INVALID"))
                continue
            a, omega, E, vmin, _ = res
            ac, omc, Ec, _, _ = resc
            gap12 = float(np.mean(E[1:5])) - float(E[0])
            gap12c = float(np.mean(Ec[1:5])) - float(Ec[0])
            ratio_lr = gap12 / gap12c
            ratio_ryd = gap12 / (0.75 * R)
            t1u_spread = float(np.max(E[2:5]) - np.min(E[2:5]))
            split_a1g_t1u = float(abs(E[1] - np.mean(E[2:5])))
            fa_ratios.append(ratio_lr)
            if L == max(RECORD_LS):
                fb_by_a0[round(a0, 1)] = ratio_ryd
            fc_ok = (t1u_spread <= FC_T1U_TOL * gap12
                     and split_a1g_t1u <= FC_SPLIT_TOL * gap12)
            fc_flags.append(fc_ok)
            print(f"  q={q:.4f} (a0={a0:.1f}) L={L}: gap12={gap12:+.5f} "
                  f"lat/ref={ratio_lr:.4f} ryd-ratio={ratio_ryd:.3f} "
                  f"T1u-spread={t1u_spread/gap12:.3f} split={split_a1g_t1u/gap12:.3f} "
                  f"{'ok' if fc_ok else 'FC-FAIL'}")
            rows.append((f"q{q}_L{L}", "gap12", gap12, ""))
            rows.append((f"q{q}_L{L}", "ratio_lat_ref", ratio_lr, ""))
            rows.append((f"q{q}_L{L}", "ratio_rydberg", ratio_ryd, ""))
            for n, e in enumerate(E[:8]):
                rows.append((f"q{q}_L{L}", f"E_{n}", float(e), "lattice"))
            for n, e in enumerate(Ec[:8]):
                rows.append((f"q{q}_L{L}", f"Eref_{n}", float(e), "continuum"))

    # frozen verdicts
    fa = all(abs(r - 1.0) <= FA_TOL for r in fa_ratios) and len(fa_ratios) == 6
    a0s = sorted(fb_by_a0)
    fb = (len(a0s) == 3
          and fb_by_a0[a0s[0]] > fb_by_a0[a0s[1]] > fb_by_a0[a0s[2]]
          and 1.0 < fb_by_a0[a0s[2]] < FB_MAX_A4)
    fc = all(fc_flags) and len(fc_flags) == 6
    print("-" * 72)
    print(f"F-A (engine potential Coulombic, |ratio-1|<={FA_TOL}): "
          f"{'PASS' if fa else 'FAIL'}  ratios={['%.4f' % r for r in fa_ratios]}")
    print(f"F-B (Rydberg approach, monotone in a0 + ratio(a0=4)<{FB_MAX_A4}): "
          f"{'PASS' if fb else 'FAIL'}  {dict((k, round(v,3)) for k,v in fb_by_a0.items())}")
    print(f"F-C (O_h multiplet structure): {'PASS' if fc else 'FAIL'}")
    print(f"F-E (causal control): {'PASS' if fe else 'FAIL'}")
    if g1 and fa and fb and fc and fe:
        verdict = "HYDROGEN-CONFIRMED"
    elif g1 and fa and fc:
        verdict = "PARTIAL"
    else:
        verdict = "CLOSED-NEGATIVE"
    print("=" * 72)
    print(f"FTD-0278 Leg 1 VERDICT: {verdict}")
    print("=" * 72)

    if out_csv:
        with open(out_csv, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["cell", "quantity", "value", "note"])
            w.writerows(rows)
            w.writerow(["VERDICT", verdict, "", ""])
        print(f"# wrote {out_csv}")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dev", action="store_true")
    ap.add_argument("--record", action="store_true")
    ap.add_argument("--Ls", default="24,32,48")
    ap.add_argument("--qs", default=None,
                    help="comma list; default derived from a0 targets 4,6,8")
    ap.add_argument("--omega0", type=float, default=OMEGA0)
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    if args.record:
        return record_run(args.out)

    Ls = [int(s) for s in args.Ls.split(",")]
    if args.qs:
        qs = [float(s) for s in args.qs.split(",")]
    else:
        m = args.omega0 / C2
        qs = [4.0 * math.pi / (m * a0) for a0 in (4.0, 6.0, 8.0)]

    if args.dev:
        dev_survey(Ls, qs, args.omega0)
        return 0

    print("use --dev or --record", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
