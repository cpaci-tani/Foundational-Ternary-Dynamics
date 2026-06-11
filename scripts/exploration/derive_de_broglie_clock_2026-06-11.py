#!/usr/bin/env python3
"""
derive_de_broglie_clock_2026-06-11.py  --  FTD-0271

Test whether giving the FTD flux a Klein-Gordon REST-MASS CLOCK turns the
substrate into a single-particle pilot-wave (de Broglie) theory. Frozen under
PREREG_DE_BROGLIE_CLOCK_v1.

THE SETUP. FTD's native flux is MASSLESS (FTD-0270 [MEASURED-BOUNDARY]: linear
dispersion omega ~ k, finite-size exponent s=0.94, no de Broglie wavelength
r=0). The A0 audit confirmed there is NO restoring term in the flux dynamics
(delta_j = c^2*Lap + G_C*grad_s only) -- so a rest-mass clock omega_0 ~ M_REST
is an IMPOSED input, not forced by the action.

ADD the Klein-Gordon mass term:  d^2 J/dt^2 = c^2*Lap18(J) - omega_0^2 * J.
Dispersion becomes  omega^2 = c^2*(-M(k)) + omega_0^2  (massive).

TWO consequences to MEASURE (the rest is textbook KG -- see the honesty note):
  B2 ENVELOPE: the energy above rest, E_env = omega - omega_0 ~ c^2(-M)/(2 omega_0)
     ~ k^2 (QUADRATIC) -- the SCHRODINGER sector FTD-0270 said was the only door.
     Box finite-size exponent of E_env should flip from s=0.94 (massless) to s~2.
  B3 de BROGLIE: a moving KG wave packet has v_group = c^2 k / omega; in the NR
     limit (omega_0 >> c k) v ~ c^2 k / omega_0, so lambda = 2pi/k ~ 1/v (de Broglie).
     Sweep the packet carrier; fit lambda ~ v^-r; expect r~1 (vs FTD-0270 r=0).

GATE G-2 (causal control): with omega_0 = 0 this MUST reproduce FTD-0270
(s ~ 0.94, r ~ 0). If not, the harness is broken -- STOP.

[HONESTY -- load-bearing]: Schrodinger and de Broglie are ANALYTIC consequences
of the KG mass term (1924/1926 textbook). Adding -omega_0^2 J and reading them
back is CIRCULAR. Confirming them on the lattice is [DERIVED-lattice-correctness],
NOT an FTD discovery. The native rest frequency is ZERO; omega_0 ~ M_REST is
[IMPOSED/SELECTION]. Claim ceiling: "FTD is a single-particle pilot-wave theory
GIVEN an imposed rest-mass clock." The non-circular content is elsewhere (A5:
can FTD's own proper-time source omega_0; E: does the wave guide the particle).
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

FACE = [(1, 0, 0), (-1, 0, 0), (0, 1, 0), (0, -1, 0), (0, 0, 1), (0, 0, -1)]
EDGE = [(1, 1, 0), (1, -1, 0), (-1, 1, 0), (-1, -1, 0),
        (1, 0, 1), (1, 0, -1), (-1, 0, 1), (-1, 0, -1),
        (0, 1, 1), (0, 1, -1), (0, -1, 1), (0, -1, -1)]


def build_L18(L, periodic):
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


def box_ground(L):
    """Ground Dirichlet box mode: returns (-M1)>=0, the lattice 'k^2' analog."""
    A = build_L18(L, periodic=False)
    M1 = float(np.max(spla.eigsh(A, k=4, which='LA', return_eigenvectors=False)))
    return -min(M1, -1e-12)


def fit_powerlaw(Ls, ys):
    lx, ly = np.log(np.array(Ls, float)), np.log(np.array(ys, float))
    A = np.vstack([lx, np.ones(len(lx))]).T
    coef, *_ = np.linalg.lstsq(A, ly, rcond=None)
    s = -coef[0]
    yhat = A @ coef
    sxx = np.sum((lx - lx.mean()) ** 2)
    se = math.sqrt(np.sum((ly - yhat) ** 2) / max(1, len(lx) - 2) / sxx) if sxx > 0 else float('nan')
    return s, se


def de_broglie_packet(omega0, k0list, L=512, ticks=None, dt=0.4):
    """1D Klein-Gordon packet launcher. Initialize a right-moving carrier
    J = env(x)*cos(k0 x), Jv = env(x)*omega*sin(k0 x), evolve with the KG leapfrog
    Jv += dt*(c^2 J'' - omega0^2 J); J += dt*Jv. Measure group velocity (envelope
    centroid drift) and carrier wavelength (FFT). Returns [(v_group, lambda, k0)]."""
    out = []
    x = np.arange(L)
    for k0 in k0list:
        omega = math.sqrt(C2 * k0 * k0 + omega0 * omega0)
        sig = 28.0
        x0 = L * 0.35
        env = np.exp(-0.5 * ((x - x0) / sig) ** 2)
        J = env * np.cos(k0 * x)
        Jv = env * omega * np.sin(k0 * x)   # right-moving carrier
        nt = ticks if ticks else int(0.45 * L / max(1e-6, C2 * k0 / omega) / dt)
        nt = min(nt, 1600)

        def centroid(field):
            a = field * field
            tot = a.sum()
            return (x * a).sum() / tot if tot > 1e-12 else x0
        c_start = centroid(J)
        for _ in range(nt):
            lap = np.roll(J, 1) + np.roll(J, -1) - 2.0 * J
            Jv += dt * (C2 * lap - omega0 * omega0 * J)
            J += dt * Jv
        c_end = centroid(J)
        v_group = (c_end - c_start) / (nt * dt)
        sp_amp = np.abs(np.fft.rfft(J - J.mean()))
        kdom = int(np.argmax(sp_amp[1:]) + 1)
        lam = L / kdom if kdom > 0 else float('nan')
        out.append((abs(v_group), lam, k0, omega))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--omega0", type=float, default=0.5, help="rest-mass clock frequency")
    ap.add_argument("--box-Ls", default="12,16,20,24,32")
    ap.add_argument("--out", default=None)
    args = ap.parse_args()
    w0 = args.omega0
    Ls = [int(s) for s in args.box_Ls.split(",")]

    print("=" * 70)
    print(f"FTD-0271 -- de Broglie internal clock (omega_0={w0})  [GIVEN imposed mass]")
    print("=" * 70)
    werr = validate_symbol(8)
    print(f"\n[G-1 / B1] operator: |eig(L18)-M(k)| = {werr:.2e}  "
          f"({'PASS' if werr < 1e-10 else 'FAIL'})  (KG: omega^2=c^2(-M)+omega_0^2)")

    # --- B2: envelope exponent (massless control G-2 + massive Schrodinger) ---
    print(f"\n[B2] box ground mode vs L  (massless control + KG envelope):")
    print(f"  {'L':>4} {'omega1_massless':>16} {'E_env=omega-omega0':>20}")
    mless, env = [], []
    for L in Ls:
        mk = box_ground(L)                     # (-M1) ~ k^2
        o_massless = C * math.sqrt(mk)         # FTD-0270 massless frequency
        o_kg = math.sqrt(C2 * mk + w0 * w0)    # KG frequency
        e_env = o_kg - w0                       # energy above rest = Schrodinger envelope
        mless.append(o_massless); env.append(e_env)
        print(f"  {L:>4} {o_massless:>16.6f} {e_env:>20.8f}")
    s_massless, se0 = fit_powerlaw(Ls, mless)
    s_env, se1 = fit_powerlaw(Ls, env)
    print(f"\n  massless control  omega1 ~ L^-{s_massless:.3f} +/- {se0:.3f}   "
          f"(G-2: must be ~0.94, linear)")
    print(f"  KG envelope       E_env  ~ L^-{s_env:.3f} +/- {se1:.3f}   "
          f"(SCHRODINGER => s~2, quadratic)")

    # --- B3: de Broglie packet ---------------------------------------------
    print(f"\n[B3] de Broglie: moving KG packet, lambda vs group velocity:")
    print(f"  {'k0':>6} {'v_group':>10} {'lambda':>10} {'omega':>8}")
    pk = de_broglie_packet(w0, [0.10, 0.16, 0.24, 0.34, 0.46])
    vv, ll = [], []
    for v, lam, k0, om in pk:
        print(f"  {k0:>6.3f} {v:>10.4f} {lam:>10.3f} {om:>8.4f}")
        if not math.isnan(lam) and v > 1e-4:
            vv.append(v); ll.append(lam)
    if len(vv) >= 3:
        r = -np.polyfit(np.log(vv), np.log(ll), 1)[0]
        print(f"  de Broglie exponent  lambda ~ v^-{r:.3f}   (de Broglie => r~1 ; massless => r~0)")
    else:
        r = float('nan'); print("  no clean packet wavelength (NULL)")

    # --- verdict ------------------------------------------------------------
    print("\n" + "=" * 70)
    print("VERDICT  [CONDITIONAL on the imposed rest-mass clock]")
    print("=" * 70)
    g2 = abs(s_massless - 1.0) < 0.2
    d1 = "SCHRODINGER-ENVELOPE-CONFIRMED" if 1.7 <= s_env <= 2.3 else (
         "NULL" if abs(s_env - 1.0) < 0.25 else "AMBIGUOUS")
    d2 = "DE-BROGLIE-CONFIRMED" if (not math.isnan(r) and abs(r - 1.0) < 0.25) else (
         "DE-BROGLIE-FAILED" if (math.isnan(r) or r < 0.3) else "AMBIGUOUS")
    print(f"  G-2 control (massless reproduces FTD-0270): {'PASS' if g2 else 'FAIL'} (s={s_massless:.2f})")
    print(f"  D1 envelope = {d1}  (s_env={s_env:.2f})")
    print(f"  D2 de Broglie = {d2}  (r={r if not math.isnan(r) else float('nan'):.2f})")
    if g2 and d1.startswith("SCHRODINGER") and d2 == "DE-BROGLIE-CONFIRMED":
        print("  => GIVEN the rest-mass clock, FTD yields de Broglie matter waves +")
        print("     a single-particle SCHRODINGER envelope. [CONDITIONAL --")
        print("     DERIVED-GIVEN-IMPOSED-INPUT]. The clock is imposed (A0); whether")
        print("     FTD's own proper-time sources it (A5) and whether it guides the")
        print("     cluster (E) is the non-circular content, tested separately.")
    elif not g2:
        print("  => G-2 FAIL: harness broken (massless control != FTD-0270). STOP.")
    else:
        print("  => partial/NULL; see exponents.")

    if args.out:
        with open(args.out, "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["kind", "x", "value"])
            for L, o in zip(Ls, mless): w.writerow(["massless_omega1", L, o])
            for L, e in zip(Ls, env): w.writerow(["kg_envelope", L, e])
            for v, lam, k0, om in pk: w.writerow(["debroglie", v, lam])
            w.writerow(["fit", "s_massless", s_massless])
            w.writerow(["fit", "s_env", s_env])
            w.writerow(["fit", "r_debroglie", r])
        print(f"\n# wrote {args.out}")


if __name__ == "__main__":
    sys.exit(main())
