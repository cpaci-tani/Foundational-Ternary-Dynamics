# THEOREM — The lattice Coulomb amplitude `1/(2π)`, derived

**Tag:** `[THEOREM]` on §2 (exact, symbolic) + `[THEOREM — standard]` on §3
(classical lattice-Green's-function asymptotics) + `[MEASURED]` on §4
(finite-`r` corrections).
**Supersedes as the load-bearing content of:** spine Theorem 6 (Phase G),
demoted 2026-08-03 by FTD-0785 to `[NUMERICAL FACT — VALIDATED FIT]`.
**Verifier:** `scripts/proofs/verify_lattice_coulomb_asymptotic.py`
**Production impact:** none.

---

## 0 · What FTD-0785 asked for

The demotion was explicit about the gap:

> **(a)** an exact lattice-Green's-function identity, if one can be exhibited
> with hypotheses, would be `[THEOREM]` — **not shown here**; **(b)** "the
> engine reproduces the lattice Poisson kernel to 0.07% median residual at
> L = 384" is `[MEASURED]`. Only (b) is established.

This document exhibits (a). It is stated about **ℤ³ and a difference
stencil** — no simulator appears in the hypotheses, which was FTD-0785's other
objection ("the Statement is engine-referential — it cannot be posed without
the simulator").

## 1 · Statement

Let `M(k)` be the Fourier symbol of the production 18-point Moore Laplacian
(faces `1/3`, edges `1/6`, centre `−4`):

```
M(k) = 4 − (2/3) Σ_i cos k_i − (2/3) Σ_{i<j} cos k_i cos k_j
```

and let `G` be the associated Green's function on ℤ³,
`G(r) = (2π)^{-3} ∫_BZ e^{i k·r} / M(k) d³k`. Define `α_r(r) := 2|r| G(r)`.

**Then `α_r(r) → 1/(2π)` as `|r| → ∞`,** with zero free parameters and no
fine-structure-constant content.

## 2 · The exact input `[THEOREM]`

Expanding `M` about `k = 0`:

```
M(k) = |k|² − |k|⁴/12 + O(|k|⁶)
```

and — this is the load-bearing part — **the fourth-order term is exactly
isotropic.** Writing `S₂ = Σk_i²`, `S₄ = Σk_i⁴`, `P₂₂ = Σ_{i<j}k_i²k_j²`, the
expansion gives `−(1/12)S₄ − (1/6)P₂₂`, and since `S₂² = S₄ + 2P₂₂` this is
identically `−(1/12)S₂²`. **The `S₄` anisotropy cancels exactly**, leaving a
function of `|k|` alone.

Verified symbolically in sympy: `c₄ + (1/12)S₂² ≡ 0`. Anisotropy first appears
at `O(|k|⁶)`.

Two consequences matter:

- **the `|k|²` coefficient is exactly 1.** This is where "zero free
  parameters" actually lives — it fixes the amplitude, and nothing was fitted
  to obtain it.
- **fourth-order isotropy is a property of the stencil weights**, not an
  approximation. It is why the 18-point stencil was chosen.

## 3 · The asymptotics `[THEOREM — standard]`

For large `|r|` the integral is controlled by the neighbourhood of `k = 0`,
where `1/M = 1/|k|² + 1/12 + O(|k|²)`. The 3-D Fourier transform of `1/|k|²`
is `1/(4π|r|)`; the analytic remainder contributes only contact terms
(derivatives of `δ³`), which vanish for `r ≠ 0`. Hence

```
G(r) = 1/(4π|r|) · (1 + o(1)) ,      α_r(r) = 2|r|G(r) → 1/(2π)
```

This is classical lattice-Green's-function theory (Glasser & Zucker 1980); the
framework-specific input is §2 — that this stencil's symbol has unit `|k|²`
coefficient and isotropic `|k|⁴` correction.

**`1/(2π) = 0.15915494309…` is a pure geometric constant.** No coupling, no
`α`, nothing fitted.

## 4 · Finite-`r` corrections `[MEASURED]` — and what the 0.07% was

Infinite-lattice values obtained by computing `G_L` on `L = 48…160` and
extrapolating linearly in `1/L`:

| direction | `\|r\|` | `α_r` | deviation |
|---|---|---|---|
| axial | 1.000 | 0.150375 | **−5.52%** |
| face | 1.414 | 0.166247 | **+4.46%** |
| body | 1.732 | 0.153876 | **−3.32%** |
| axial | 2.000 | 0.158973 | −0.11% |
| body | 3.464 | 0.158844 | −0.20% |
| axial | 4.000 | 0.159026 | −0.08% |
| face | 5.657 | 0.158939 | −0.14% |

**The 0.07% median residual FTD-0785 objected to is this**: the genuine
finite-`r` lattice correction in the Coulomb tail, not noise and not a fitting
artifact. It is ~5% at `r = 1` with a ~10% directional spread, and falls below
0.5% by `r ≥ 2`.

So the fit was measuring something real. Its error was calling an
**asymptotic** law an exact identity with "zero free parameters at every
finite `L`".

## 5 · A second defect in the original framing `[MEASURED]`

The demoted statement was posed on the **`L³` torus**, as `α_r(r, L)`. That
object does **not** have the stated limit. With the `k = 0` mode removed (the
neutralising background any periodic Poisson solve requires), `G_L` carries a
constant offset, so `α_r = 2|r|G_L` **drifts linearly in `|r|`**:

| | `r = 2` | `r = 8` | `r = 24` |
|---|---|---|---|
| `L = 96` | −6.0% | −23.5% | **−67.4%** |
| `L = 144` | −4.0% | −15.7% | **−46.3%** |

The deviation grows with `r` and shrinks with `L` — a finite-torus artifact.
**`1/(2π)` is recovered only after `L → ∞` extrapolation**, which §4 does.

So the original claim was wrong in two independent ways: asymptotic sold as
exact, and the torus object sold as the infinite-lattice one. Neither was
visible from `R² = 1.0000` at a single `L`.

## 6 · What this does and does not license

**Does:** `α_r → 1/(2π)` is now a theorem about `ℤ³` and a stencil, with the
exact isotropy result of §2 as its framework-specific content. It is
**not** engine-referential and can be checked without a simulator.

**Does not:** it says nothing about `α`. The physical fine-structure constant
enters through the coupling `g_c`, which is `[PARAMETRIC]` — unchanged. The
deflationary reading stands and is *strengthened*: what the Gauss projector
returns is a Green's function whose Coulomb amplitude is the pure geometric
constant `1/(2π)`, so **there is no fine-structure content in it to extract**.
That is consistent with FTD-0792 (the engine runs on a CODATA-matched `ALPHA`;
the derived root feeds nothing).

**Spine count:** unchanged. This does **not** restore Theorem 6 — the demoted
statement was a different, and false, claim. Whether §1–§3 should be admitted
as a *new* numbered spine result is an owner decision, deliberately not taken
here.
