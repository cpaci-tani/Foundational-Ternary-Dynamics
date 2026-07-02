r"""proof_phase_j_zero_modes.py — FTD-0350: Phase-J ultralocality beyond L=2.

Settles the L >= 4 scope of spine Theorem 7 (SPEC_ALGEBRAIC_SPINE.md §7).

Companion doc: docs/theory/09_mathematical/ANALYSIS_PHASE_J_ZERO_MODES_v1.md

BACKGROUND
    Prior status: ultralocality (S_E depends on s only through Sigma s^2)
    was [THEOREM at L=2], numerical at L=3, and OPEN/ambiguous at L>=4
    because the matched centered-difference stencil acquires zero modes
    (e.g. k=(0,0,pi) at L=4) and the naive masked-Parseval scan
    (proof_phase_j_general_L.py) showed a ~3-28% placement-dependent
    spread on random neutral configs.

WHAT THIS SCRIPT PROVES / VERIFIES (exact linear algebra, no Monte Carlo)
    Lemma (matched-stencil ultralocality, all L). For ANY translation-
    invariant first-difference stencil D = (D_1,D_2,D_3) used
    consistently in BOTH the Gauss divergence (div J = Sigma_i D_i J_i)
    and the kinetic norm (K[J] = Sigma_{ij} ||D_i J_j||^2):

      (a) div J = s is solvable  <=>  s_hat(k) = 0 for all k in
          Ker(D) = {k : D(k) = 0};
      (b) for realizable s:  min { K[J] : div J = s } = Sigma_x s(x)^2,
          exactly, at every L  (per-mode Cauchy-Schwarz: the |D(k)|^2
          in the kinetic weight cancels the |D(k)|^-2 forced by the
          constraint — mode-independently);
      (c) hence S_E[s] = (c^2/2 + g_c) * Sigma s^2 on the realizable
          space: ULTRALOCAL AT EVERY L.

    Stencil instances:
      * centered half-difference (the FTD-0090 matched analysis
        stencil): Ker = {0} for odd L; Ker = {0,pi}^3 (8 modes) for
        even L. Realizable space at even L = all 8 parity-sublattice
        charges vanish.
      * forward difference (D^T D = the engine's 7-point Laplacian):
        Ker = {0} at EVERY L. Realizable space = all charge-neutral
        configs; no exclusions; reproduces the historical L=2 value
        S_E = 7/3 for a unit dipole (g_c=1, c^2=1/3) by an honest
        lattice computation (the original partition_function_L2.py
        asserted the kinetic value from the continuum identity).

    The old L>=4 "spread" is a PROVEN masking artifact: the masked
    pseudo-action equals  Sigma s^2 - (1/N) Sigma_{k in Ker\{0}}
    |s_hat(k)|^2  identically (Test 5), i.e. the spread is exactly the
    kernel content of configurations that are NOT Gauss-realizable
    under the centered stencil (no J satisfies div J = s; under the
    spec's lambda_G -> infinity [AXIOM] they carry infinite action).

    Finite lambda_G (soft constraint) is also computed exactly (Test 6):
    on the realizable space S_eff = A(lambda_G) * Sigma s^2 for EVERY
    lambda_G > 0, with A = lambda_G - (2 lambda_G - g_c)^2 /
    (4 (c^2/2 + lambda_G)) -> c^2/2 + g_c. Off the realizable space the
    only extra term is lambda_G * (kernel content) — the constraint-
    violation norm itself.

VERDICT
    THEOREM at all L >= 2 (matched stencil, Gauss-realizable state
    space, lambda_G -> infinity per SPEC_FTD_LAGRANGIAN §3.3 [AXIOM]).
    The L >= 4 ambiguity is closed as a masking artifact, not a
    structural failure. See the companion doc for tag conditions.

Usage:
    python scripts/proofs/proof_phase_j_zero_modes.py
"""

from __future__ import annotations

import sys

import numpy as np

try:  # Windows console safety
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:  # pragma: no cover
    pass

C2 = 1.0 / 3.0   # c^2 = 1/D (CFL), matches partition_function_L2.py
G_COUP = 1.0     # g_c = 1 benchmark value (theorem is g_c-independent)

RESULTS: list[tuple[str, bool]] = []


def check(name: str, ok: bool, detail: str = "") -> None:
    RESULTS.append((name, bool(ok)))
    mark = "PASS" if ok else "FAIL"
    print(f"    [{mark}] {name}" + (f"  ({detail})" if detail else ""))


# ---------------------------------------------------------------------
# Real-space operators (dense; N = L^3 <= 512 here)
# ---------------------------------------------------------------------
def shift_matrix(L: int, axis: int, step: int) -> np.ndarray:
    """Matrix of (S f)(x) = f(x + step * e_axis) on the periodic torus."""
    N = L ** 3
    idx = np.arange(N).reshape(L, L, L)
    cols = np.roll(idx, -step, axis=axis).ravel()
    M = np.zeros((N, N))
    M[np.arange(N), cols] = 1.0
    return M


def build_stencil(L: int, kind: str) -> list[np.ndarray]:
    """Return [D_1, D_2, D_3] real-space matrices."""
    ops = []
    for ax in range(3):
        if kind == "centered":
            D = 0.5 * (shift_matrix(L, ax, +1) - shift_matrix(L, ax, -1))
        elif kind == "forward":
            D = shift_matrix(L, ax, +1) - np.eye(L ** 3)
        else:
            raise ValueError(kind)
        ops.append(D)
    return ops


def build_B_G(Ds: list[np.ndarray]) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """B (N x 3N) divergence, A = Sum D_i^T D_i, G = I_3 (x) A."""
    N = Ds[0].shape[0]
    B = np.hstack(Ds)                      # div J = B @ [J1; J2; J3]
    A = sum(D.T @ D for D in Ds)
    G = np.zeros((3 * N, 3 * N))
    for j in range(3):
        G[j * N:(j + 1) * N, j * N:(j + 1) * N] = A
    return B, A, G


def symbol_sq(L: int, kind: str) -> np.ndarray:
    """|D(k)|^2 = Sum_i |D_i(k)|^2 on the L^3 mode grid."""
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    kx, ky, kz = np.meshgrid(k, k, k, indexing="ij")
    if kind == "centered":
        return np.sin(kx) ** 2 + np.sin(ky) ** 2 + np.sin(kz) ** 2
    if kind == "forward":
        return 2.0 * ((1 - np.cos(kx)) + (1 - np.cos(ky)) + (1 - np.cos(kz)))
    raise ValueError(kind)


def kernel_mask(L: int, kind: str, tol: float = 1e-12) -> np.ndarray:
    return symbol_sq(L, kind) < tol


def kernel_content(s: np.ndarray, kind: str, include_k0: bool = True) -> float:
    """(1/N) Sum_{k in Ker} |s_hat(k)|^2  (optionally excluding k=0)."""
    L = s.shape[0]
    m = kernel_mask(L, kind).copy()
    if not include_k0:
        m[0, 0, 0] = False
    s_hat = np.fft.fftn(s)
    return float(np.sum(np.abs(s_hat[m]) ** 2) / L ** 3)


# ---------------------------------------------------------------------
# Constrained minimum via generic KKT (independent of Fourier reasoning)
# ---------------------------------------------------------------------
def kmin_kkt(s_flat: np.ndarray, B: np.ndarray, G: np.ndarray) -> tuple[float, float]:
    """min { J^T G J : B J = s }  via least-squares on the KKT system.

    Returns (K_min, kkt_residual). Any KKT point of this convex problem
    is a global minimizer; the value is unique even when the minimizer
    is not (flat kernel-mode directions of J carry zero kinetic weight).
    """
    N = B.shape[0]
    n = B.shape[1]
    KKT = np.zeros((n + N, n + N))
    KKT[:n, :n] = 2.0 * G
    KKT[:n, n:] = B.T
    KKT[n:, :n] = B
    rhs = np.concatenate([np.zeros(n), s_flat])
    sol, *_ = np.linalg.lstsq(KKT, rhs, rcond=None)
    J = sol[:n]
    resid = float(np.linalg.norm(KKT @ sol - rhs))
    return float(J @ G @ J), resid


def feasibility_residual_sq(s_flat: np.ndarray, B: np.ndarray) -> float:
    """min_J ||B J - s||^2 (0 iff the Gauss constraint is solvable)."""
    J, *_ = np.linalg.lstsq(B, s_flat, rcond=None)
    r = B @ J - s_flat
    return float(r @ r)


# ---------------------------------------------------------------------
# Config generators
# ---------------------------------------------------------------------
def random_neutral(L: int, n_charges: int, rng: np.random.Generator) -> np.ndarray:
    half = n_charges // 2
    flat = np.zeros(L ** 3, dtype=np.int8)
    idx = rng.choice(L ** 3, size=n_charges, replace=False)
    flat[idx[:half]] = +1
    flat[idx[half:]] = -1
    return flat.reshape((L, L, L))


def parity_class_sites(L: int, parity: tuple[int, int, int]) -> np.ndarray:
    """Flat indices of sites x with x mod 2 == parity (even L)."""
    coords = np.indices((L, L, L)).reshape(3, -1).T
    sel = np.all(coords % 2 == np.array(parity), axis=1)
    return np.nonzero(sel)[0]


def random_realizable_even_L(L: int, pairs_per_class: dict,
                             rng: np.random.Generator) -> np.ndarray:
    """Ternary config with ZERO charge in every parity class (=> centered-
    stencil realizable at even L)."""
    flat = np.zeros(L ** 3, dtype=np.int8)
    for parity, n_pairs in pairs_per_class.items():
        sites = parity_class_sites(L, parity)
        pick = rng.choice(sites, size=2 * n_pairs, replace=False)
        flat[pick[:n_pairs]] = +1
        flat[pick[n_pairs:]] = -1
    return flat.reshape((L, L, L))


def parity_class_charges(s: np.ndarray) -> np.ndarray:
    """The 8 sublattice charges Q_sigma = Sum_{x = sigma mod 2} s(x)."""
    L = s.shape[0]
    flat = s.ravel()
    out = []
    for px in (0, 1):
        for py in (0, 1):
            for pz in (0, 1):
                out.append(flat[parity_class_sites(L, (px, py, pz))].sum())
    return np.array(out, dtype=float)


# ---------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------
def test1_kernel_characterization() -> None:
    print("\nTest 1: kernel characterization  Ker(D) = {k : D(k) = 0}")
    print("  centered: {0} for odd L, {0,pi}^3 (8 modes) for even L;")
    print("  forward:  {0} at every L.  Fourier symbol vs real-space ker(A).")
    for L in (2, 3, 4, 5, 6):
        for kind, expect in (("centered", 8 if L % 2 == 0 else 1),
                             ("forward", 1)):
            if kind == "centered" and L == 2:
                expect = 8  # {0,pi}^3 = ALL modes at L=2
            n_sym = int(kernel_mask(L, kind).sum())
            Ds = build_stencil(L, kind)
            _, A, _ = build_B_G(Ds)
            eig = np.linalg.eigvalsh(A)
            n_real = int(np.sum(eig < 1e-10))
            check(f"L={L} {kind}: |Ker|={n_sym} (expect {expect}), "
                  f"dim ker(A)={n_real}",
                  n_sym == expect and n_real == expect)
    # Explicit L=4 centered kernel mode list
    L = 4
    m = kernel_mask(L, "centered")
    modes = sorted((i, j, k) for i in range(L) for j in range(L)
                   for k in range(L) if m[i, j, k])
    expect_modes = sorted((a, b, c) for a in (0, 2) for b in (0, 2)
                          for c in (0, 2))  # index 2 <-> k = pi
    check("L=4 centered kernel modes are exactly {0,pi}^3",
          modes == expect_modes, f"modes(idx)={modes}")


def test2_solvability_criterion() -> None:
    print("\nTest 2: solvability  <=>  zero kernel amplitudes")
    print("  min_J ||div J - s||^2  ==  (1/N) Sum_{k in Ker} |s_hat(k)|^2")
    rng = np.random.default_rng(7)
    for L, kind in ((3, "centered"), (4, "centered"), (6, "centered"),
                    (4, "forward")):
        Ds = build_stencil(L, kind)
        B, _, _ = build_B_G(Ds)
        max_err = 0.0
        for _ in range(10):
            s = random_neutral(L, max(2, 2 * (L ** 3 // 8)), rng)
            r2 = feasibility_residual_sq(s.ravel().astype(float), B)
            kc = kernel_content(s, kind, include_k0=True)
            max_err = max(max_err, abs(r2 - kc))
        check(f"L={L} {kind}: residual^2 == kernel content "
              f"(10 random neutral configs)", max_err < 1e-9,
              f"max |diff| = {max_err:.2e}")
    # parity-class equivalence at L=4 centered
    L = 4
    rng2 = np.random.default_rng(8)
    ok = True
    for _ in range(20):
        s = random_neutral(L, 16, rng2)
        kc = kernel_content(s, "centered", include_k0=True)
        q = parity_class_charges(s)
        # (1/N) Sum_{k*} |s_hat(k*)|^2 = (8/N) Sum_sigma Q_sigma^2
        ok &= abs(kc - 8.0 * float(q @ q) / L ** 3) < 1e-9
    check("L=4 centered: kernel content == (8/N) * Sum_sigma Q_sigma^2 "
          "(parity-class charges)", ok)
    # the nearest-neighbour dipole is NOT centered-realizable at L=4
    s = np.zeros((4, 4, 4), dtype=np.int8)
    s[0, 0, 0] = +1
    s[1, 0, 0] = -1   # adjacent site: different parity class
    Ds = build_stencil(4, "centered")
    B, _, _ = build_B_G(Ds)
    r2 = feasibility_residual_sq(s.ravel().astype(float), B)
    check("L=4 centered: nearest-neighbour dipole is NOT Gauss-realizable",
          abs(r2 - 0.25) < 1e-10, f"residual^2 = {r2:.6f} (= 1/4 exactly)")


def test3_ultralocality_on_realizable_space() -> None:
    print("\nTest 3: MAIN THEOREM — K_min == Sigma s^2 on realizable configs")
    print("  (real-space KKT, no Fourier; equal Sigma s^2 => equal S_E)")

    # --- L=4 centered: hand-built witnesses, equal Sigma s^2 = 2 ---
    L = 4
    Ds = build_stencil(L, "centered")
    B, _, G = build_B_G(Ds)
    witnesses = {}
    cfg = {}
    cfg["A: pair in class(0,0,0), sep 2"] = [((0, 0, 0), +1), ((2, 0, 0), -1)]
    cfg["B: pair in class(0,0,0), sep 2*sqrt3"] = [((0, 0, 0), +1),
                                                   ((2, 2, 2), -1)]
    cfg["C: pair in class(1,1,1)"] = [((1, 1, 1), +1), ((3, 3, 3), -1)]
    for name, placement in cfg.items():
        s = np.zeros((L, L, L))
        for pos, q in placement:
            s[pos] = q
        r2 = feasibility_residual_sq(s.ravel(), B)
        K, kkt_res = kmin_kkt(s.ravel(), B, G)
        witnesses[name] = K
        SE = (C2 / 2.0) * K + G_COUP * float(np.sum(s * s))
        check(f"L=4 centered {name}: realizable, K_min = {K:.12f}",
              r2 < 1e-18 and abs(K - 2.0) < 1e-9 and kkt_res < 1e-8,
              f"S_E = {SE:.10f} (= 7/3 = {7/3:.10f})")
    spread = max(witnesses.values()) - min(witnesses.values())
    check("L=4 centered: equal Sigma s^2 => equal S_E across placements",
          spread < 1e-9, f"spread = {spread:.2e}")

    # --- L=4 centered: random parity-realizable, Sigma s^2 = 8 ---
    rng = np.random.default_rng(11)
    vals = []
    for _ in range(6):
        s = random_realizable_even_L(
            L, {(0, 0, 0): 2, (1, 1, 1): 2}, rng).astype(float)
        K, kkt_res = kmin_kkt(s.ravel(), B, G)
        vals.append(K)
        if kkt_res > 1e-8:
            vals.append(np.inf)
    check("L=4 centered: 6 random realizable configs (Sigma s^2 = 8): "
          "K_min == 8",
          max(abs(v - 8.0) for v in vals) < 1e-8,
          f"max |K-8| = {max(abs(v - 8.0) for v in vals):.2e}")

    # --- L=5 centered: kernel = {0}, ALL neutral configs realizable ---
    L = 5
    Ds5 = build_stencil(L, "centered")
    B5, _, G5 = build_B_G(Ds5)
    errs = []
    for _ in range(6):
        s = random_neutral(L, 30, rng).astype(float)
        K, _ = kmin_kkt(s.ravel(), B5, G5)
        errs.append(abs(K - float(np.sum(s * s))))
    check("L=5 centered (odd): 6 random neutral configs: K_min == Sigma s^2",
          max(errs) < 1e-8, f"max err = {max(errs):.2e}")

    # --- L=6 centered: parity-realizable ---
    L = 6
    Ds6 = build_stencil(L, "centered")
    B6, _, G6m = build_B_G(Ds6)
    errs = []
    for _ in range(4):
        s = random_realizable_even_L(
            L, {(0, 0, 0): 3, (0, 1, 0): 2, (1, 1, 1): 3}, rng).astype(float)
        K, _ = kmin_kkt(s.ravel(), B6, G6m)
        errs.append(abs(K - float(np.sum(s * s))))
    check("L=6 centered: 4 random realizable configs: K_min == Sigma s^2",
          max(errs) < 1e-7, f"max err = {max(errs):.2e}")

    # --- L=4 forward: NO exclusions — any neutral config, incl. the NN dipole
    L = 4
    Dsf = build_stencil(L, "forward")
    Bf, _, Gf = build_B_G(Dsf)
    s = np.zeros((L, L, L))
    s[0, 0, 0] = +1
    s[1, 0, 0] = -1
    r2 = feasibility_residual_sq(s.ravel(), Bf)
    K, _ = kmin_kkt(s.ravel(), Bf, Gf)
    SE = (C2 / 2.0) * K + G_COUP * 2.0
    check("L=4 forward: NN dipole realizable, K_min == 2, S_E == 7/3",
          r2 < 1e-18 and abs(K - 2.0) < 1e-9 and abs(SE - 7.0 / 3.0) < 1e-9,
          f"S_E = {SE:.10f}")
    errs = []
    for _ in range(6):
        s = random_neutral(L, 16, rng).astype(float)
        K, _ = kmin_kkt(s.ravel(), Bf, Gf)
        errs.append(abs(K - float(np.sum(s * s))))
    check("L=4 forward: 6 random neutral configs: K_min == Sigma s^2",
          max(errs) < 1e-8, f"max err = {max(errs):.2e}")


def test4_form_is_identity_on_physical_subspace() -> None:
    print("\nTest 4: effective quadratic form == Identity on the physical")
    print("  subspace (L=4 centered; dim = 64 - 8 = 56), incl. polarization")
    L = 4
    N = L ** 3
    Ds = build_stencil(L, "centered")
    B, _, G = build_B_G(Ds)
    # projector onto physical subspace: kill the 8 kernel channels
    chis = []
    for px in (0, 2):
        for py in (0, 2):
            for pz in (0, 2):
                kvec = np.array([px, py, pz]) * (np.pi / 2.0)
                coords = np.indices((L, L, L)).reshape(3, -1)
                chi = np.cos(kvec @ coords)  # = (+-1)^..., real
                chis.append(chi / np.linalg.norm(chi))
    P = np.eye(N) - sum(np.outer(c, c) for c in chis)
    rng = np.random.default_rng(23)
    diag_errs, polar_errs = [], []
    for _ in range(10):
        u = P @ rng.standard_normal(N)
        v = P @ rng.standard_normal(N)
        Ku, ru = kmin_kkt(u, B, G)
        Kv, rv = kmin_kkt(v, B, G)
        Kuv, ruv = kmin_kkt(u + v, B, G)
        diag_errs.append(abs(Ku - float(u @ u)))
        diag_errs.append(abs(Kv - float(v @ v)))
        polar_errs.append(abs((Kuv - Ku - Kv) - 2.0 * float(u @ v)))
        if max(ru, rv, ruv) > 1e-6:
            diag_errs.append(np.inf)
    check("Q(v) == ||v||^2 for 20 random physical-subspace vectors",
          max(diag_errs) < 1e-7, f"max err = {max(diag_errs):.2e}")
    check("polarization: Q(u+v)-Q(u)-Q(v) == 2<u,v> (10 pairs) — form is "
          "IDENTITY, not just diagonal-matching",
          max(polar_errs) < 1e-7, f"max err = {max(polar_errs):.2e}")


def test5_artifact_accounting() -> None:
    print("\nTest 5: the old 3-28% spread is EXACTLY the dropped kernel")
    print("  content:  K_masked == Sigma s^2 - (1/N) Sum_{Ker\\{0}} "
          "|s_hat|^2")
    print("  (same seed-42 configs as proof_phase_j_general_L.py Test 2)")
    rng = np.random.default_rng(42)   # reproduce the old scan exactly
    for L in (3, 4, 6, 8):
        n_charges = max(2, 2 * (L ** 3 // 8))
        masked_vals, ids = [], []
        for _ in range(8):
            s = random_neutral(L, n_charges, rng).astype(float)
            # old masked-Parseval computation (matched stencil):
            s_hat = np.fft.fftn(s)
            sig2 = symbol_sq(L, "centered")
            m = sig2 > 1e-12
            masked = float(np.sum(np.abs(s_hat[m]) ** 2) / L ** 3)
            masked_vals.append(masked)
            kc = kernel_content(s, "centered", include_k0=False)
            ids.append(abs(masked - (float(np.sum(s * s)) - kc)))
        spread = max(masked_vals) - min(masked_vals)
        rel = spread / np.mean(masked_vals)
        check(f"L={L}: masked identity holds for all 8 configs "
              f"(old spread {rel * 100:.1f}%)",
              max(ids) < 1e-9, f"max |diff| = {max(ids):.2e}")


def test6_finite_lambda_G() -> None:
    print("\nTest 6: finite lambda_G (soft constraint), exact effective "
          "action")
    print("  S_min == A(lam)*(Sigma s^2 - kappa) + lam*kappa,  kappa = "
          "kernel content;")
    print("  A(lam) = lam - (2 lam - g_c)^2 / (4 (c^2/2 + lam))  ->  "
          "c^2/2 + g_c")
    L = 4
    Ds = build_stencil(L, "centered")
    B, _, G = build_B_G(Ds)
    rng = np.random.default_rng(31)
    s_real = random_realizable_even_L(L, {(0, 0, 0): 2, (1, 0, 1): 2},
                                      rng).astype(float)
    s_nonreal = np.zeros((L, L, L))
    s_nonreal[0, 0, 0] = +1
    s_nonreal[1, 0, 0] = -1   # NN dipole, kappa = 1/4
    for lam in (1.0, 10.0, 1000.0):
        A_pred = lam - (2 * lam - G_COUP) ** 2 / (4 * (C2 / 2 + lam))
        for tag, s in (("realizable", s_real), ("non-realizable", s_nonreal)):
            sf = s.ravel()
            # minimize (c^2/2) J^T G J + g_c s^T B J + lam ||B J - s||^2
            M = C2 * G + 2.0 * lam * (B.T @ B)
            rhs = (2.0 * lam - G_COUP) * (B.T @ sf)
            J, *_ = np.linalg.lstsq(M, rhs, rcond=None)
            val = (0.5 * C2 * float(J @ G @ J)
                   + G_COUP * float(sf @ (B @ J))
                   + lam * float(np.sum((B @ J - sf) ** 2)))
            kap = kernel_content(s, "centered", include_k0=True)
            pred = A_pred * (float(sf @ sf) - kap) + lam * kap
            check(f"lam={lam:g}, {tag}: S_min == prediction",
                  abs(val - pred) < 1e-7 * max(1.0, abs(pred)),
                  f"S_min = {val:.8f}, pred = {pred:.8f}, "
                  f"A = {A_pred:.6f}")
    check("A(lam) -> c^2/2 + g_c = 7/6 as lam -> inf",
          abs((1e9 - (2e9 - G_COUP) ** 2 / (4 * (C2 / 2 + 1e9)))
              - (C2 / 2 + G_COUP)) < 1e-6)


def test7_L2_provenance_repair() -> None:
    print("\nTest 7: L=2 provenance repair")
    print("  centered at L=2: D == 0 (all modes in Ker) => constraint")
    print("  unsolvable for s != 0; forward at L=2: ALL 1107 neutral")
    print("  configs realizable with K_min == Sigma s^2 (the identity the")
    print("  original partition_function_L2.py asserted from the continuum).")
    import itertools
    L, N = 2, 8
    # centered degeneracy
    Dsc = build_stencil(L, "centered")
    Bc = np.hstack(Dsc)
    check("L=2 centered: divergence operator is identically zero",
          np.max(np.abs(Bc)) < 1e-15)
    # forward: full enumeration of the 1107 neutral configs
    Dsf = build_stencil(L, "forward")
    Bf, _, Gf = build_B_G(Dsf)
    n_neutral, max_err, worst_kkt = 0, 0.0, 0.0
    for c in itertools.product((-1, 0, 1), repeat=N):
        if sum(c) != 0:
            continue
        n_neutral += 1
        s = np.array(c, dtype=float)
        K, res = kmin_kkt(s, Bf, Gf)
        max_err = max(max_err, abs(K - float(s @ s)))
        worst_kkt = max(worst_kkt, res)
    check(f"L=2 forward: all {n_neutral} neutral configs have "
          "K_min == Sigma s^2",
          n_neutral == 1107 and max_err < 1e-8 and worst_kkt < 1e-8,
          f"max err = {max_err:.2e}")
    # historical value: unit dipole S_E = (c^2/2 + g_c) * 2 = 7/3 = 2.333
    s = np.zeros(N)
    s[0], s[1] = +1, -1
    K, _ = kmin_kkt(s, Bf, Gf)
    SE = (C2 / 2) * K + G_COUP * 2.0
    check("L=2 forward: dipole S_E == 7/3 == 2.3333 (historical value, "
          "now honestly computed on the lattice)",
          abs(SE - 7.0 / 3.0) < 1e-9, f"S_E = {SE:.10f}")


def test8_mismatched_stencil_remark() -> None:
    print("\nTest 8 (remark): the MISMATCHED engine pairing (G6 Laplacian +")
    print("  centered gradient, FTD-0090) is accidentally ultralocal at L=3")
    print("  (single frequency shell: sin^2/q-ratio constant = 1/4) but")
    print("  genuinely non-ultralocal at L>=4 — a real property of the")
    print("  mismatch, NOT of the matched action.")
    for L, want_const in ((3, True), (4, False)):
        sig2 = symbol_sq(L, "centered")   # sin^2 numerator
        q2 = symbol_sq(L, "forward")      # 2(1-cos) = G6 symbol
        m = q2 > 1e-12
        ratio = sig2[m] / q2[m]
        # drop modes where numerator vanishes but denominator doesn't:
        # those contribute factor 0 (also mode-dependent unless absent)
        is_const = (np.max(ratio) - np.min(ratio)) < 1e-12
        check(f"L={L}: mode factor sin^2/q^2 constant = {is_const} "
              f"(expected {want_const})", is_const == want_const,
              f"range [{np.min(ratio):.4f}, {np.max(ratio):.4f}]")


def main() -> int:
    print("=" * 72)
    print("proof_phase_j_zero_modes.py — FTD-0350")
    print("Phase-J ultralocality beyond L=2: zero modes, correct measure,")
    print("verdict. Companion: ANALYSIS_PHASE_J_ZERO_MODES_v1.md")
    print("=" * 72)

    test1_kernel_characterization()
    test2_solvability_criterion()
    test3_ultralocality_on_realizable_space()
    test4_form_is_identity_on_physical_subspace()
    test5_artifact_accounting()
    test6_finite_lambda_G()
    test7_L2_provenance_repair()
    test8_mismatched_stencil_remark()

    n_pass = sum(1 for _, ok in RESULTS if ok)
    n_tot = len(RESULTS)
    print()
    print("=" * 72)
    print(f"Summary: {n_pass}/{n_tot} checks passed")
    for name, ok in RESULTS:
        if not ok:
            print(f"  FAILED: {name}")
    print("=" * 72)
    print()
    print("VERDICT (FTD-0350):")
    print("  THEOREM at every L >= 2: on the Gauss-realizable state space")
    print("  (s_hat = 0 on Ker D), with the constraint enforced exactly")
    print("  (lambda_G -> inf, the spec's own [AXIOM]), the classical")
    print("  Euclidean action is S_E = (c^2/2 + g_c) * Sigma s^2 — placement-")
    print("  independent — for ANY consistent first-difference stencil D.")
    print("  The old L>=4 spread is a PROVEN masking artifact (Test 5):")
    print("  exactly the kernel content of non-realizable configurations.")
    print("  Conditionality: stencil-consistency (FTD-0090 matched-stencil")
    print("  discipline) is the only live [SELECTION]; it moves the DOMAIN")
    print("  (which s are realizable), never the ultralocality ON it.")
    return 0 if n_pass == n_tot else 1


if __name__ == "__main__":
    sys.exit(main())
