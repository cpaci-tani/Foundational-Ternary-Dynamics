"""FTD-0264: Mechanism beta — parameter-free envelope prediction of the
sub-knee onset (derivation attempt against the FTD-0263 constraint profile).

MODEL (stated before compute). Pre-genesis the engine dynamics is exactly
LINEAR: symplectic-Euler wave update on the 18-pt (2:1) stencil with
alpha = 1/18 (pinned by the FTD-0251 measured dispersion
omega = 2*(1/sqrt(3))*|sin(k/2)|), followed per tick by the Gauss projection
(charge-free => divergence removal). The injected flux is A*K_GENESIS * x-hat
at the center voxel. Define the ENVELOPE e(delta) = max_t |J(delta, t)| per
unit injection — pure lattice geometry, no parameters. Mechanism beta then
predicts (sharp-kinetics, initial-crossing approximation):

    voxel manifests  <=>  A * e(delta) > 1      (K_GENESIS cancels)
    N(A) = #{delta : e(delta) > 1/A}            (rank-ordered spectrum inverse)

Computed in two variants (the engine's exact discrete projector is the one
modeling unknown): (a) no per-tick projection; (b) FFT divergence projection
with central-difference symbol. Known approximations, stated: deterministic
threshold (Boltzmann smearing softens steps), no manifestation back-reaction,
no evaporation — the model predicts the INITIAL crossing set.

FROZEN COMPARISON THRESHOLDS (before compute; targets = FTD-0263 run of
record: elbow knee_N = 14.6; staircase table; onset facts N(1.5)=1,
second voxel joining near A ~ 8.5-9):
  T1 elbow: predicted broken-power elbow N_pred in [9.7, 21.9]  (14.6 x/1.5)
  T2 shape: log10-RMS between predicted N(A) and the measured F-arm table
            over points with 2 <= N_meas <= 25 is <= 0.20
  T3 absolute onset (reported, not verdict-gating): predicted A at N=2
            within a factor 2 of the measured ~8.75; center-only for A > 1.
Verdict map: BETA-SUPPORTED if T1 AND T2 hold in at least one variant;
BETA-PARTIAL if exactly one of T1/T2 holds (best variant); BETA-FAIL if
neither. Priors: PARTIAL 40%, SUPPORTED 30%, FAIL 30%.

Quick-check platform (linear algebra, no engine); the engine remains the
canonical instrument for any follow-up.
"""
import math

import numpy as np

L = 64
T = 110
RMAX = 12          # wrap-safe analysis radius (front returns after ~(L-r)*sqrt(3) ticks)
ALPHA = 1.0 / 18.0

# frozen FTD-0263 F-arm staircase (A, N_mean), canonical axial L=32
MEASURED = [(8.5, 1.80), (9.0, 2.00), (9.5, 3.00), (10.0, 4.00), (10.5, 5.20),
            (11.0, 6.00), (11.5, 7.80), (12.0, 8.40), (12.5, 10.40),
            (13.0, 11.80), (13.5, 13.40), (14.0, 16.40), (14.5, 18.00),
            (15.0, 19.80), (15.5, 20.80), (16.0, 21.60), (17.0, 24.20),
            (18.0, 24.60)]
KNEE_N_MEAS = 14.6
T1_BAND = (9.7, 21.9)
T2_RMS = 0.20


def laplacian18(f):
    """18-point stencil: faces weight 2, edges weight 1, sum-of-weights 24."""
    out = -24.0 * f
    for ax in range(3):
        out += 2.0 * (np.roll(f, 1, axis=ax) + np.roll(f, -1, axis=ax))
    for a1 in range(3):
        for a2 in range(a1 + 1, 3):
            for s1 in (1, -1):
                for s2 in (1, -1):
                    out += np.roll(np.roll(f, s1, axis=a1), s2, axis=a2)
    return out


def project_divfree(Jx, Jy, Jz, K, K2):
    """Remove the longitudinal part: J -> J - K (K.J)/|K|^2 (central-diff symbol)."""
    Fx, Fy, Fz = np.fft.fftn(Jx), np.fft.fftn(Jy), np.fft.fftn(Jz)
    dot = K[0] * Fx + K[1] * Fy + K[2] * Fz
    with np.errstate(invalid="ignore", divide="ignore"):
        coef = np.where(K2 > 1e-15, dot / K2, 0.0)
    Fx -= K[0] * coef; Fy -= K[1] * coef; Fz -= K[2] * coef
    return (np.real(np.fft.ifftn(Fx)), np.real(np.fft.ifftn(Fy)),
            np.real(np.fft.ifftn(Fz)))


def dispersion_selfcheck():
    """Axis mode n=4 at L=64: winding frequency vs 2*(1/sqrt(3))*sin(k/2)."""
    k = 2 * math.pi * 4 / L
    x = np.arange(L)
    f = np.sin(k * x)[:, None, None] * np.ones((1, L, L))
    v = np.zeros_like(f)
    q0 = float((f * np.sin(k * x)[:, None, None]).sum())
    qs = []
    for _ in range(60):
        v += ALPHA * laplacian18(f)
        f = f + v
        qs.append(float((f * np.sin(k * x)[:, None, None]).sum()) / q0)
    # zero-crossing-free phase estimate via arcsin of normalized first step for Symplectic Euler
    om = 2.0 * math.asin(0.5 * math.sqrt(max(0.0, 1.0 - qs[0])))
    om_pred = 2 * (1 / math.sqrt(3)) * abs(math.sin(k / 2))
    return om, om_pred


def run_envelope(project):
    c = L // 2
    Jx = np.zeros((L, L, L)); Jy = np.zeros_like(Jx); Jz = np.zeros_like(Jx)
    Vx = np.zeros_like(Jx); Vy = np.zeros_like(Jx); Vz = np.zeros_like(Jx)
    Jx[c, c, c] = 1.0
    if project:
        kk = 2 * math.pi * np.fft.fftfreq(L)
        KX = np.sin(kk)[:, None, None] * np.ones((1, L, L))
        KY = np.sin(kk)[None, :, None] * np.ones((L, 1, L))
        KZ = np.sin(kk)[None, None, :] * np.ones((L, L, 1))
        K = (KX, KY, KZ); K2 = KX ** 2 + KY ** 2 + KZ ** 2
        Jx, Jy, Jz = project_divfree(Jx, Jy, Jz, K, K2)
    env = np.sqrt(Jx ** 2 + Jy ** 2 + Jz ** 2)
    for _ in range(T):
        Vx += ALPHA * laplacian18(Jx); Vy += ALPHA * laplacian18(Jy)
        Vz += ALPHA * laplacian18(Jz)
        Jx = Jx + Vx; Jy = Jy + Vy; Jz = Jz + Vz
        if project:
            Jx, Jy, Jz = project_divfree(Jx, Jy, Jz, K, K2)
        mag = np.sqrt(Jx ** 2 + Jy ** 2 + Jz ** 2)
        np.maximum(env, mag, out=env)
    # collect e(delta) within RMAX
    co = np.indices((L, L, L))
    d = np.minimum(np.abs(co - c), L - np.abs(co - c))
    r = np.sqrt((d ** 2).sum(axis=0))
    mask = r <= RMAX
    return np.sort(env[mask])[::-1]   # ranked envelope spectrum


def broken_fit(pts):
    def fit_power(p):
        xs = [math.log10(a) for a, n in p]; ys = [math.log10(n) for a, n in p]
        n = len(xs); mx, my = sum(xs) / n, sum(ys) / n
        sxx = sum((x - mx) ** 2 for x in xs)
        sxy = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
        pw = sxy / sxx if sxx else 0.0
        c = 10 ** (my - pw * mx)
        rms = math.sqrt(sum((math.log10(c) + pw * x - y) ** 2
                            for x, y in zip(xs, ys)) / n)
        return c, pw, rms
    best = None
    for ik in range(2, len(pts) - 3):
        lo, hi = pts[:ik + 1], pts[ik + 1:]
        flo, fhi = fit_power(lo), fit_power(hi)
        rms = math.sqrt((flo[2] ** 2 * len(lo) + fhi[2] ** 2 * len(hi)) / len(pts))
        kA = pts[ik][0]; kN = flo[0] * kA ** flo[1]
        if best is None or rms < best[0]:
            best = (rms, kA, kN, flo[1], fhi[1])
    return best


def evaluate(name, spectrum):
    print(f"\n===== variant {name} =====")
    # predicted curve: voxel of rank k joins at A_k = 1/e_(k)
    e = spectrum
    print("top-25 ranked envelopes e_(k) and join amplitudes A_k = 1/e_(k):")
    for k in range(min(25, len(e))):
        print(f"  rank {k+1:3d}: e={e[k]:.5f}  A_join={1/e[k]:7.2f}")
    # N(A) at the measured A grid
    def N_of_A(A):
        return int(np.count_nonzero(e > 1.0 / A))
    # T2 shape over measured points with 2 <= N_meas <= 25
    sel = [(A, Nm) for A, Nm in MEASURED if 2 <= Nm <= 25]
    resid = []
    print("\n   A    | N_meas | N_pred")
    for A, Nm in sel:
        Np = N_of_A(A)
        print(f"{A:6.1f} | {Nm:6.2f} | {Np:5d}")
        if Np >= 1:
            resid.append(math.log10(Nm) - math.log10(Np))
    t2_rms = math.sqrt(sum(r * r for r in resid) / len(resid)) if resid else 9.9
    # T1 elbow of the PREDICTED curve over a dense A grid
    Agrid = [a / 10 for a in range(20, 401, 2)]
    pts = []
    for A in Agrid:
        n = N_of_A(A)
        if n >= 1 and (not pts or n != pts[-1][1]):
            pts.append((A, n))
    pts = [(a, n) for a, n in pts if n >= 1]
    bf = broken_fit(pts) if len(pts) >= 7 else None
    if bf:
        rms, kA, kN, plo, phi = bf
        print(f"\npredicted-curve elbow: knee_A={kA:.1f} knee_N={kN:.1f} "
              f"(p_lo={plo:.2f}, p_hi={phi:.2f})")
    else:
        kN = float("nan")
        print("\npredicted-curve elbow: UNDETERMINED")
    t1 = bf is not None and T1_BAND[0] <= kN <= T1_BAND[1]
    t2 = t2_rms <= T2_RMS
    # T3 absolute-scale checks (reported)
    A2 = 1.0 / e[1] if len(e) > 1 else float("inf")
    print(f"T3 absolute: A_join(rank 2) = {A2:.2f} (measured N=2 near A~8.75); "
          f"center-only for A in (1, {A2:.2f})")
    print(f"T1 elbow in {T1_BAND}: {'PASS' if t1 else 'FAIL'}   "
          f"T2 shape RMS={t2_rms:.3f} (<= {T2_RMS}): {'PASS' if t2 else 'FAIL'}")
    return t1, t2


def main():
    om, om_pred = dispersion_selfcheck()
    print(f"dispersion self-check (axis n=4): omega={om:.4f} vs "
          f"2c*sin(k/2)={om_pred:.4f}  (alpha=1/18 pin)")
    res = {}
    res["a:no-projection"] = evaluate("a: no per-tick projection",
                                      run_envelope(project=False))
    res["b:div-free"] = evaluate("b: per-tick divergence projection",
                                 run_envelope(project=True))
    print("\n================ VERDICT ================")
    best = max(res.items(), key=lambda kv: sum(kv[1]))
    n_hold = sum(best[1])
    if n_hold == 2:
        print(f"BETA-SUPPORTED (variant {best[0]}): the parameter-free envelope "
              "model reproduces the elbow location and the sub-knee shape.")
    elif n_hold == 1:
        print(f"BETA-PARTIAL (variant {best[0]}): one of T1/T2 holds; see above.")
    else:
        print("BETA-FAIL: the envelope model reproduces neither the elbow nor "
              "the shape — the initial-crossing approximation is insufficient "
              "(back-reaction/evaporation/kinetics are load-bearing).")


if __name__ == "__main__":
    main()
