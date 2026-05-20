# Higher-Dimensional Watson Identity

**Status:** [THEOREM] — proved 2026-05-19 as part of the G* paper open-question
closure (Q3 in `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §13).

**Canonical citation:** `PAPER_GSTAR_INTRODUCTION.tex` Theorem 13.2 (generalised
Watson identity).

**Companion artifacts:**
- Proof: `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §13 (Theorem 13.2 with full
  proof).
- Numerical verification (mpmath, 50 digits):
  `scripts/exploration/gstar_open_questions.py`.
- Independent GPU Monte Carlo cross-check (3-4 digits via 2M random walks on
  BCC lattice, RTX 5090 + CuPy in WSL2): `scripts/exploration/gstar_W_montecarlo_gpu.py`.

## The theorem

For integer dimension `D ≥ 3`, let

```
W^(D) := (1/π^D) · ∫₀^π ··· ∫₀^π dk₁ ··· dk_D / (1 − cos k₁ cos k₂ ··· cos k_D).
```

Then

```
W^(D) = _D F_{D-1}(1/2, 1/2, ..., 1/2; 1, 1, ..., 1; 1)
      = Σ_{n=0}^∞ [(1/2)_n / n!]^D
```

where `(1/2)_n = (1/2)(3/2)(5/2)···((2n-1)/2)` is the rising-factorial Pochhammer
symbol with `D` copies of `1/2` and `D − 1` copies of `1`. Convergence at the
hypergeometric argument `z = 1` requires `(D − 2)/2 > 0`, i.e. `D ≥ 3`.

## Proof sketch (full proof in the paper)

1. Expand `1/(1 − ∏ cos k_i) = Σ_n (∏ cos k_i)^n` as a geometric series.
2. Integrate term-by-term using `∫₀^π (cos k)^(2m) dk = π · (1/2)_m / m!`
   (and `∫₀^π (cos k)^n dk = 0` for odd `n`).
3. The resulting series is exactly `_D F_{D-1}(1/2,...,1/2; 1,...,1; 1)`.

## Special cases

| D | Value | Closed form |
|---|---|---|
| 3 | 1.39320392968567686 | `G*²/(2π) = 4η(i)⁴` (Watson 1939, Clausen ³F₂ identity) |
| 4 | 1.11863638716418707 | `_4F_3(1/2^4; 1^3; 1)`, no simpler form known |
| 5 | 1.04682554983350001 | `_5F_4(1/2^5; 1^4; 1)`, no simpler form known |

All values verified to 50 decimal digits via mpmath (see
`scripts/exploration/gstar_open_questions.py`).

## Relationship to the R_n family

The family `R_n = Γ(1/n)/Γ((n-1)/n)` of which `R_4 = G*` is the lemniscatic
member appears in `W^(D)` **only at D = 3**, via Watson's classical reduction:

- `W^(3) = G*²/(2π) = R_4²/(2π)`.

For `D ≥ 4`, the hypergeometric `_DF_{D-1}` involves only half-integer
Pochhammer `(1/2)_n`, and PSLQ search at 30+ digit precision finds no integer
relation between `W^(4)` and any rational power product of
`{G*, π, R_3, R_5, R_6, Γ(1/4)}` with coefficient bound `10^12`. The constants
`R_n` for `n ≠ 4` do not appear in any higher-dimensional Watson generalisation.

## Why this matters for the project

The 3D Watson identity `W_BCC = G*²/(2π)` is one of the canonical pillars of
the FTD algebraic spine
(`docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`, Theorem 4.1). It connects
the body-centred-cubic Moore-sublattice Green's function to the Gauss-lemniscate
ratio `G*`. The generalisation above shows:

1. **The G* appearance is specific to D = 3**. There is no analogous
   `Γ(1/n)`-ratio that captures higher-D BCC Watson integrals; the natural
   closed form for those is the hypergeometric series itself.

2. **The 3D case is privileged**. Among the family `{W^(D) : D ≥ 3}`, only
   `D = 3` admits a clean closed form in terms of a single `Γ`-ratio (specifically
   `G*`). This is consistent with the project's existing thread of D = 3 being
   structurally forced by `|Aut(E)|² = 2^D (D − 1)! = 16` (FTD-0122).

3. **No new `R_n` constants are forced by higher-D physics**. For a discrete
   substrate framework that lives in higher dimensions, the lattice Green's
   function does not introduce new transcendentals beyond the hypergeometric
   `_DF_{D-1}` value, and the family `R_n` remains relevant only via the
   D = 3 case.

## Cross-references

- **Existing project work cited**: `docs/theory/04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md`
  (3D BCC Watson reduction); `docs/theory/09_mathematical/MATH_FAMILY_OF_RACES.md`
  (R_n family for n = 2..6); `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md`
  (canonical statement of Watson identity).
- **Resolves**: Q3 of `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` §10
  (now §13 with the theorem).

## Provenance

- 2026-05-19: derivation completed; numerical verification at 50 digits;
  paper Theorem 13.2 written; this document created.
