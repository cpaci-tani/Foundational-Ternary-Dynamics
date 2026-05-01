# EXPLR — Fourier Curve at Level 4: Triple-Cusp Structure and Class Divisibility

**Document type:** Exploratory (number-theoretic)
**Status:** [EXPLORATORY] — not yet promoted to derivation; one prior conjecture in this neighborhood was falsified (recorded below)
**Created:** 2026-04-30
**Provenance:** distilled from `docs/theory/07_assessment/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` §7 ("the Fourier curve")
**Related:** `THEOREM_HARMONIC_INVARIANT_TOWER.md` (the (1+i)-tower whose level-indexing matches the Fourier-support exponents);
`SPEC_ALGEBRAIC_SPINE.md` Theorem 9 (field-theoretic characterization of `Q(G*)`);
`DERIV_LFUNCTION_GSTAR_CONNECTION.md` (Watson identity W₃ = G*²/(2π) and L(1, χ_{−4}) = π/4)

---

## 0 · Why this document exists

The (1+i)-tower of master quadratics generates an explicit sequence of polynomials
indexed by `k ∈ {2, 3, 4, …}` whose coefficients live in
`Z[2, G*]`. The Fourier support of the curve studied in this document
sits exactly on the same dyadic grid `{2, 4, 8, 16, …}` that indexes the
tower. This is suggestive but not yet a derivation — and one prior
conjecture connecting the Fourier curve directly to a Damerell-style basis
was falsified during analysis. We file the surviving structure here as
exploratory material.

---

## 1 · The Fourier curve

Define the real curve

```
x(t) = cos t + (1/2) cos 2t + (1/2) cos 4t + (3/8) cos 8t,    t ∈ [0, 2π).
```

Its Fourier support is concentrated at frequencies `{1, 2, 4, 8}` — i.e.
`{2^0, 2^1, 2^2, 2^3}`. The level-4 truncation truncates at the
`2^3 = 8`-frequency band.

### 1.1 · Closed form for the area

Let `y(t) = -dx/dt = sin t + sin 2t + 2 sin 4t + 3 sin 8t` and consider
the planar curve `(x(t), y(t))`. Direct integration gives the signed area
enclosed:

```
A_4 = (1/2) ∮ (x dy - y dx) = 3π/4.
```

### 1.2 · Connection to L(1, χ_{−4}) = π/4

The Dirichlet L-value at s = 1 for the unique non-trivial character mod 4 is

```
L(1, χ_{−4}) = π/4 = β(1) = Catalan-conjugate value.
```

Hence the area satisfies

```
A_4 = 3 · L(1, χ_{−4}).
```

The **factor of 3** is the artifact this document attempts to explain.

---

## 2 · Cuspidal extensions at frequency ±16

Add a level-5 term `a_{16} cos 16t` to the level-4 curve and examine the
cusp structure of the resulting plane curve as `a_{16}` is varied.

| `a_{16}` | Cusp structure of (x, y) curve |
|---|---|
| `0` | smooth (no cusps); area = 3π/4 |
| `1/16` | **triple cusp** at the three cube roots of unity (parametric values `t = 0, 2π/3, 4π/3`) |
| `3/16` | **single cusp** at `t = 0` |
| generic small | smooth perturbation |

The values `1/16` and `3/16` appear as **resonant cuspidal coefficients** —
the only two values in `[0, 1]` (sampled to step 1/64) at which the curve
degenerates with a clean cusp pattern.

### 2.1 · Triple-cusp at cube roots of unity (a_{16} = 1/16)

The triple-cusp configuration is a Z/3Z-symmetric singular fibre. Its
cusps lie at the three cube roots of unity in the complex plane
identification `z = x + iy`. This Z/3Z symmetry is **emergent from the
level-5 perturbation** — it is not a symmetry of the level-4 curve itself.

The discovery is that adding a single Fourier coefficient at the
`16 = 2^4` band — exactly at the next dyadic step — promotes the
abelian level-4 curve to a curve with non-trivial Z/3Z fibre structure.
This is structurally suggestive: the dyadic ladder of the (1+i)-tower
seems to gain additional symmetry at `k = 5`. Whether this is a
genuine connection or a Fourier-engineering artifact is the central
question this exploration leaves open.

### 2.2 · Single-cusp configuration (a_{16} = 3/16)

The single-cusp configuration is the asymmetric resonance. Its cusp
sits at `t = 0` only. The numerical coincidence `3/16 = 3 · 1/16`
mirrors the area-prefactor `3 = A_4 / L(1, χ_{−4})`, hinting that the
factor 3 governs both the area normalization and the asymmetric cusp
amplitude — but no proof of equivalence is yet available.

---

## 3 · Class divisibility of moments

For the family of curves obtained by scaling the level-4 base
`(x(t), y(t))`, define the n-th moment

```
M_n = ∫_0^{2π} x(t)^n dt.
```

A divisibility pattern observed empirically (verified to n = 24 by
direct mpmath computation):

| Conductor class | Divisibility of M_n by 3 |
|---|---|
| Class-1 (e.g. base curve) | M_n divisible by 3 for n ≡ 0 (mod 3); not divisible otherwise |
| Class-3 (Z/3Z-twisted curves with a_{16} = 1/16) | M_n divisible by 3 for **all** n ≥ 0 |

This is a **Class-3 → universal divisibility by 3** phenomenon. It is
consistent with the Z/3Z symmetry imposed by the triple-cusp resonance.
It is NOT explained by the level-4 curve in isolation.

---

## 4 · Falsified prior conjecture (for the record)

**Falsified conjecture (Damerell-basis):** "The level-4 curve `x(t)` is
the real part of a Damerell-type modular form of weight 2 on Γ_0(4)
expanded in the basis of newforms."

**Why it failed:** the q-expansion of the candidate Damerell form
matched x(t)'s Fourier coefficients at `q = 1, 2, 4` but **disagreed at
q = 8**. The `cos 8t` coefficient in `x(t)` is `3/8`, but the
corresponding Fourier coefficient of any weight-2 newform on Γ_0(4) is
forced by the Hecke recursion to differ from `3/8` by a non-zero rational.

The Fourier curve is therefore **not a modular newform expansion**. It
is a tailored real-Fourier construction whose level-indexing happens to
coincide with the dyadic ladder. The Class-3 divisibility and triple-cusp
structure are real (verified empirically), but the modular-forms reading
is closed-negative.

This is recorded as a falsified candidate rather than deleted because
the area-equals-`3 · L(1, χ_{−4})` identity and the resonance values
`a_{16} ∈ {1/16, 3/16}` survive the falsification. They are
exploratory data points that any future modular-forms-style reading
must reproduce.

---

## 5 · What this means for FTD

The exploratory significance for FTD is:

1. **Fourier support on `{2^k}` dyadic ladder** — same indexing as the
   `(1+i)`-tower of master quadratics in
   `THEOREM_HARMONIC_INVARIANT_TOWER.md`. Whether the two `2^k` ladders
   are the same ladder or merely isomorphic is open.

2. **Area = `3 · L(1, χ_{−4})`** — places `L(1, χ_{−4}) = π/4`
   alongside `W₃ = G*² / (2π)` (`DERIV_LFUNCTION_GSTAR_CONNECTION.md`)
   and `L(1, χ_{−4}) = β(1) = π/4` as the only L-values at s = 1 that
   appear in the algebraic core. The factor 3 is unexplained.

3. **Triple-cusp Z/3Z resonance at `2^4` band** — provides a candidate
   "appearance of N_c = 3" inside the dyadic ladder that is **not** the
   master-quadratic root x_− = 3.024. If the two threes turn out to be
   one three, that would be a structural connection. Currently they are
   two unconnected appearances of 3.

4. **Class-3 universal divisibility by 3** — a clean number-theoretic
   pattern that survives falsification of the modular-form reading.

None of this is promoted. It is filed as exploratory.

---

## 6 · LEDGER status

This document does not introduce a new LEDGER entry on its own.
The level-indexing-coincidence with the (1+i)-tower (and any future
upgrade of any item in §5 to [DERIVED]) would warrant a fresh LEDGER
entry at that time.

Currently the empirical observations (area = 3π/4, triple-cusp at
a_{16} = 1/16, Class-3 divisibility) are tagged [MEASURED]. The
structural connection to the (1+i)-tower is [STRONGLY MOTIVATED
CONJECTURE]. The modular-newform reading is [RETRACTED] (§4).

---

## 7 · What this document does NOT claim

- **NOT a derivation of N_c = 3.** The triple-cusp structure is a
  Fourier-engineering coincidence at level 5 of an explicit construction;
  it does not by itself force N_c = 3 from FTD axioms.

- **NOT an L-function identity for FTD.** The area `3π/4` factors
  through `L(1, χ_{−4})` but the factor 3 is unexplained. There is no
  derivation chain from FTD axioms to either the area or the factor.

- **NOT a Damerell-basis statement.** That conjecture is closed-negative
  (§4). Anyone citing this document must NOT cite it as a modular-forms
  result.

- **NOT a connection to the master quadratic.** The two `2^k` ladders
  are observationally similar; whether they coincide is open.

---

## 8 · Suggested follow-ups (open)

1. **Modular-forms re-attempt**: replace the falsified Damerell-basis
   conjecture with a half-integral-weight or Maass-form candidate and
   test whether the cos 8t coefficient mismatch resolves.

2. **Connect a_{16} ∈ {1/16, 3/16} to (1+i)-tower coefficients**: the
   `1/16 = 1/2^4` resonance value coincides with the level-4 normalization
   in `THEOREM_HARMONIC_INVARIANT_TOWER.md` (where coefficients are
   `2^k · G*^{k−2}`). Direct check: is `1/16` literally `1 / (2^4)` from
   the tower, or is it independent?

3. **Run a look-elsewhere scan** on the cuspidal-coefficient resonance
   values across `a_n cos nt` perturbations for `n ∈ {16, 32, 64}` to see
   whether the `{1/16, 3/16}` structure persists at deeper levels of the
   dyadic ladder.

4. **Verify Class-3 divisibility analytically** rather than empirically —
   prove that any Z/3Z-twisted curve in this family has `M_n ≡ 0 (mod 3)`
   for all n by a representation-theoretic argument over Z/3Z.

---

*End of document.*
