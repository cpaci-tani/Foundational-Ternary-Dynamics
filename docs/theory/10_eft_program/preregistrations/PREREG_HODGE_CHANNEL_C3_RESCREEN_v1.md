# PREREG — Re-screen C3 with the native Hodge channel added

**Status:** `[PREREGISTRATION — LOCKED BEFORE EXECUTION]`
**Question:** FTD-0800 screened for `n = 4` using the compact law **alone**, in
which same-polarity pairs are masked to exactly zero. FTD-0575's native
Hodge-derived force gives same-polarity pairs a **nonzero attractive**
interaction on `r ≤ √3`. Does adding that channel produce the self-stress that
all 62 `N = 6` isomorphism classes lacked, and hence an `n = 4` mechanism?
**Parents:** FTD-0575 (`[THEOREM]`, independently re-verified 2026-08-04,
10/10, `verify_hodge_static_pole.py`), FTD-0800, FTD-0789 (the criterion).
**Production impact:** none.

## 1 · Why

FTD-0800 §7.5 closed `N ≤ 6` exhaustively: **no non-degenerate unit-distance
framework has both a self-stress and a flex.** The structural reason given was
that the `±` mask forces the bond graph **bipartite**, and the redundancy that
self-stress needs lives in same-sublattice pairs — exactly the ones
bipartiteness forbids.

**That reason is scoped to the compact law.** The native force is not masked the
same way. Verified real-space kernel, converged to 5 figures over `N = 32→192`:

| r | 0 | 1 | √2 | √3 | 2 | 3 | 4 |
|---|---|---|---|---|---|---|---|
| `K` | +1.2476 | **+0.15529** | **+0.08985** | **+0.04598** | −0.15874 | −0.01533 | −0.00365 |

with `U_cross = −G_C² q_i q_j K(r)` and `G_C² = α`. Same polarity attracts on
`r ≤ √3` (the Moore-26 shell) and repels beyond.

**Note the two channels have opposite polarity structure**, which is the whole
point: the compact law bonds *opposite* polarity and ignores *same*; the Hodge
channel attracts *same* polarity and repels *opposite*. Neutral (`s = 0`) sites
carry `q = 0`, so they have **no** Hodge interaction at all, where the compact
mask gives them `1/2`.

## 2 · Model — both channels, nothing else changed

```text
E = sum_{i<j} [ A(s_i,s_j) V(q_ij)          # compact law, unchanged
                - alpha * s_i * s_j * K(r_ij) ]   # native Hodge channel
A(s_i,s_j) = (1 - s_i s_j)/2 ,  V(q) = -16 eps (q-3/2)^2 (q-3/4) for q < 3/2
alpha = G_C^2 = 0.00729735...,  eps = 0.01
K = inverse Fourier transform of R(k) = 3 sigma^2 / M(k)   (FTD-0575)
```

`K` is evaluated for **continuous** separation by direct trigonometric sum over
the Brillouin-zone grid, not by interpolating lattice values — the kernel is
cubic-anisotropic and depends on direction, not only `|r|`.

## 3 · Two predictions, locked

**P-A — the likely outcome is `n = 2`, not `n = 4`.** The Hodge kernel is
nonzero at *every* separation (it decays but never vanishes), so **every** pair
with `s_i s_j ≠ 0` contributes a rigidity-matrix row. On `N = 6` that is up to
15 rows against `3N − 6 = 12`, so the framework becomes **over-constrained and
generically rigid**. Adding a long-range channel plausibly destroys the flex
rather than supplying the stress. **Registered as the expected outcome.**

**P-B — the zero-tension framework may not apply at all.** FTD-0789's
trichotomy assumes an *unstressed* equilibrium: every bond at its own minimum.
The Hodge kernel has **no minimum** on `r ≤ √3` — it decreases monotonically
from `r = 0`. So a combined-channel equilibrium is generically **pre-stressed**
(`V' ≠ 0`), the Hessian acquires the stress term `2V'(q)·I`, and `n = 2/4/∞`
classification is not licensed. If so, the honest verdict is
`FRAMEWORK_INAPPLICABLE`, not a count.

## 4 · Protocol

For each of the 62 `N = 6` isomorphism classes: re-equilibrate under the
combined energy from the FTD-0800 embedding, require `|∇E| < 1e-9` and a
non-degenerate geometry (`d_min = 0.5`), then compute `B`, `rank(R)`, stress,
flex, and run the FTD-0800 relaxation guard (coordinates relaxed at every
amplitude; amplitudes changing the bond set discarded).

## 5 · Preregistered outcomes

- **`HODGE_N4_FOUND`** — at least one class has `stress > 0`, `flex > 0`, and a
  quartic positive definite on the whole null space, surviving relaxation.
  **C3 realized, and FTD-0800's closure was an artifact of screening with the
  wrong force law.**
- **`HODGE_RIGID`** — the added channel over-constrains and flex vanishes.
  **P-A confirmed;** FTD-0800's `n=∞` verdicts become `n=2` under the fuller
  law, and C3 stays unrealized from the opposite side.
- **`FRAMEWORK_INAPPLICABLE`** — equilibria are pre-stressed and the trichotomy
  does not license a classification. **P-B confirmed.**
- **`HODGE_NO_CHANGE`** — the channel is too weak to alter any verdict.

## 6 · Kill conditions

1. The FTD-0800 controls must still reproduce under the compact law with the
   Hodge channel switched **off** (trimer `n = ∞`, positive control clean `t⁴`).
   Otherwise `SCREEN_INVALID`.
2. `K` must be verified against `verify_hodge_static_pole.py`'s converged
   lattice values before use — same numbers at integer `r`, or the continuous
   evaluator is wrong.
3. Any `n = 4` report must survive relaxation and an `O_h` sweep, per FTD-0800.
4. Verdicts must be stable across null tolerances `1e-6 / 1e-7 / 1e-8`.

## 7 · What this cannot show

- **Whether the two channels legitimately coexist.** The Hodge force is
  field-mediated (eliminate `J`); the compact law is a matter-sector contact
  term. They may both act, or the compact law may be a *replacement* for the
  field channel rather than an addition. **This run assumes addition, which is
  a `[SELECTION]`, and a positive result would be conditional on it.**
- `N = 6` only; `N = 7` untested.
- `n = 4` satisfies **C3 alone** — not C2, not the other ten constraints, and
  per FTD-0784 not the FC-W surd.
