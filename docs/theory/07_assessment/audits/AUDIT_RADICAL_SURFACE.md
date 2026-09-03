# Audit — the radical surface of the constants chain, the spine, and the dimensional map

**Claim id:** FTD-1026 (drafted, unbooked)
**Verdict:** `[MEASURED]` — 16 radical sites classified; one unpriced branch adoption of record
**Tags moved:** none.
**Verifier:** `scripts/proofs/proof_radical_audit.py` (12/12 arithmetic claims)
**Date:** 2026-09-03

---

## 1. The principle

Addition, subtraction, multiplication and division are field operations: they close on
the objects they act on. A square root is not. It is an algebraic extension — it adjoins
a root and requires a **branch** (`±`) to be selected. Under the type-priority principle
(FTD-0339) a branch is a *type*, and a type adopted without warrant is an unpriced
import.

This yields a test that can be applied uniformly:

> A radical is legitimate iff its radicand is independently known to be a square of
> something primitive, with the branch fixed by an independent condition — positivity,
> geometry, orientation, or a metric signature.

The test is not a dimensional one. Half-powers of units are well defined (dimensions form
a group over ℚ) and occur legitimately: noise spectral density carries `V/√Hz`, and in
Gaussian units charge has dimension `g^{1/2}cm^{3/2}s^{-1}` — the convention in which
`α = e²/4π` is written. The cost of a radical is the branch, not the unit.

## 2. Classes

| class | warrant |
|---|---|
| `W-GAUSS` | radicand is a square by the Gaussian/Fubini argument; branch by positivity |
| `W-PYTH` | radicand is a sum of squares; branch by geometry or metric signature |
| `W-AMPL` | radicand is an intensity whose amplitude is the primitive |
| `W-ALG` | adjoining the root *is* the object (field extensions) |
| `V-REP` | the radical is representational and cancels in the quantity actually used |
| `C-COND` | a square only under a stated assumption; priced by that assumption |
| `U` | unpriced branch adoption |

## 3. Scope

The canonical constants chain (`scripts/constants.py`, `engine/include/ftd/constants.h`,
`ontic.h`), the nine results of `SPEC_ALGEBRAIC_SPINE.md`, and the dimensional map. The
~133 `[PARAMETRIC]` rows are out of scope; where one is touched (`m_e`) only its *radical*
is classified, never its formula.

## 4. Ledger

| site | expression | class |
|---|---|---|
| `constants.py` `G_STAR` | `Γ(1/4)²/(√2·Γ(1/2)²)` | `V-REP` |
| `constants.py` `GAMMA_HALF` | `Γ(1/2) = √π` | `W-GAUSS` |
| `constants.py` `VARPI` | `Γ(1/4)²/(2√2·Γ(1/2))` | `W-GAUSS` |
| `constants.py` `M_ELECTRON_DERIVED` | `√(2π)` factor | `W-GAUSS` |
| `constants.py` `PHI` | `(1+√5)/2` | `W-PYTH` |
| `constants.py` `C_SPEED` | `1/√3` | `W-PYTH` |
| `constants.py` `FTD_TICK_S` | `t_P/√3` | `W-PYTH` |
| `constants.h` `WZ_MIXING_ANGLE_COS` | `√(1−sin²θ_W)` | `W-PYTH` |
| `constants.py` `gamma_FTD` | `1/√(1−v²−L²)` | `W-PYTH` |
| `constants.py` `G_C` | `√α` | `W-AMPL` |
| spine Thm 3 | `ℚ(√−d)` | `W-ALG` |
| spine Heegner note | `e^{π√163}` | `W-ALG` |
| `constants.py` `SQRT_GSTAR` | `√G*` | **`C-COND`** |
| spine Thm 2 roots | `8G*² ± √(64G*⁴−16G*³)` | **`U`** |
| FTD-0784 surd | `δ = √(G*(4G*−1))` | **`U`** |
| spine `α_tree` corollary | `1/(2G*) − √(4G*−1)/(4G*^{3/2})` | **`U`** |

Tally: 12 warranted or representational, 1 conditional, 3 unpriced.

## 5. Finding

**Result 5.1.** Every unpriced or conditional radical in the audited surface has `G*`
under the root sign. Every radical whose radicand does **not** contain `G*` is warranted:
`π` and `2π` by Fubini on the Gaussian (branch fixed by positivity of the integrand,
verified `I² = π` to 25 digits); `2, 3, 5` as sums of squares; `1−sin²θ_W` and `1−v²` by
Pythagoras and by the metric signature; `α` by the amplitude-primitive reading that
`constants.h` already encodes (`G_C` defined first, `ALPHA_EFT = G_C²` under a
`static_assert`); `−d` because the extension is the object.

**Result 5.2.** `G*` itself is radical-free. `Γ(1/4)/Γ(3/4)` contains no root; the `√2`
in the equivalent form `Γ(1/4)²/(π√2)` is representational, arising only on conversion by
the reflection formula. Independently, the `√(2π)` carried by each zeta-regularised
quarter-sector determinant `det_ζ D_a = √(2π)/Γ(a)` **cancels exactly in the ratio**,
leaving `G*` clean. The principle predicts this: the physically meaningful object is the
one the radical drops out of.

**Result 5.3.** The three `U` rows are not independent. They are one adoption seen three
times, verified to 18 digits: `√(64G*⁴−16G*³) = 4G*·δ`, and the `α_tree` closed form
equals `1/x₊` exactly.

So the radical surface is clean everywhere except where `G*` sits beneath the root — which
is exactly where the master quadratic's physical content lives.

## 6. Price

**One unpriced branch adoption of record**, radicand `G*(4G*−1)`, cost **1 adopted bit**
(the `±`) plus the type that fixes it. Falsifier: exhibit a primitive whose square is
`G*(4G*−1)`, or show the branch is forced by an independent condition. Until then it is
an import in the sense of `SPEC_IMPORT_LEDGER.md`, not a derivation.

**One conditional**, `√G*`, priced by the reflection-gluing assumption `Z_closed = a·ā`.
The source text already tags this conditional; the audit changes nothing but locates it.

### 6.1 Retirement attempt on the branch bit — `[CLOSED NEGATIVE]`

The falsifier of §6 was fired along its second clause: *show the branch is forced by an
independent condition*. Three candidate conditions were tested and all fail.

**Positivity — closed.** The move that legitimately fixes `a = √G*` at the gluing step is
reflection positivity forcing `a ≥ 0`. It has nothing to bite on here: `y₊ = 0.978410`
and `y₋ = 0.021590` are **both strictly positive**, so positivity excludes neither branch.

**The reflection involution — insufficient.** Under `σ: a ↔ 1−a` the Euler product
`Γ(1/4)Γ(3/4) = π√2` is invariant while the ratio maps `G* → 1/G*`, so `G*` carries the
sign representation of `μ₂`. The *same* involution acts on the branch equation
`y(1−y) = 1/(16G*)`, with `y ↔ 1−y` swapping the branches exactly (`1−y₊ = y₋`, verified
to 18 digits). But an orientation *on* an involution is precisely one bit; naming it does
not derive it. The involution also fails to survive the scaling `x = 16G*²y` — at `1/G*`
the x-roots are `{1.380, 0.448}`, unrelated to `{137.036, 3.024}` — so `16G*²` is not
σ-equivariant, independently locating the same temporal/spatial seam that
`AUDIT_MQ_STEP9_RESPONSE_FUNCTION.md` closes.

**P5's named non-injective expiry — closed, and closed generally.** Two obstructions.

*No channel.* The v3 constitution contains no transfer operator, no reciprocal two-branch
structure, and no FQCR; its only references to the master quadratic and to `α` are
disclaimers (§1.4 declines to have `D=3` forced by the master quadratic; `α` is listed
among the *recovery targets*, not the primitives). The selected first expiry map acts on
eight `(normal,hand)` presentations of a phase-2 Hodge packet absorbed into an SC reserve.
It has no contact with the branch allocations.

*Wrong shape even granting a channel.* The map preserves phase, polarity, one work token
and the carrier line, and expires the signed arrival endpoint, the perpendicular normal
and the handed frame — three `ℤ/2`'s dying, one (`polarity`) surviving. Either the branch
is an expiring `ℤ/2`, in which case expiry **destroys** the distinction and `x₊` and `x₋`
become indistinguishable, so `x₊ = 1/α` is not statable; or it is the surviving `ℤ/2`, in
which case expiry does not act on it and supplies no selection. Neither horn retires the
bit.

**Result 6.1 (no-go).** No expiry map of any kind can retire this bit. Non-injectivity
produces a **quotient**; selecting a branch is a **section** of that quotient; a section
of a two-to-one quotient carries exactly one bit, and a quotient does not come equipped
with one. Irreversibility and orientation are different structures: an arrow of time
states that the dynamics is not invertible, and says nothing about which element of an
internal `ℤ/2` is physical. The branch bit is therefore irreducible to P5, and the
retirement route is closed at the level of the construction rather than of this
particular reference law.

### 6.2 The branch-free formulation already exists

The bit is avoidable rather than retirable. Spine Theorem 8 states
`1/Y₊ + 1/Y₋ = 1` with `Y_± := x_±/G*`, and those reciprocals **are** the branch
allocations: `1/Y₋ = y₊` and `1/Y₊ = y₋`, both exact (residual tracks working precision:
`4.1e-25` at 25 dps, `2.7e-51` at 50 dps, `1.9e-80` at 80 dps). Theorem 8 is symmetric in
the two roots and therefore **carries no branch bit at all**.

The boundary is consequently exact and forced:

| formulation | unpriced radicals | the α claim reads |
|---|---|---|
| symmetric (Thm 8; coefficients `16G*²`, `16G*³`) | **0** | **55.71 ppm** |
| branch (`x₊` alone) | **1 adopted bit** | **1.2572 ppm** |

One or the other. The 1.26 ppm headline is purchasable only with the bit, and no expiry
map sells it.

## 7. Two non-identifications, recorded to prevent rediscovery

- `√G* = 1.72007997464904` against `√3 = 1.7320508`: **0.696% apart. This is not an
  identification** and must not be pursued as one.
- No named FTD primitive (`π`, `√π`, `ϖ`, `2`, `√2`, `√3`, `φ`, `e`, `4/3`, `16/9`) is
  within `1e-6` of `√G*`; the nearest is the `√3` near-miss above.

## 8. Consequence for how the master quadratic is stated

The polynomial's **coefficients** are radical-free — `x₊+x₋ = 16G*²` and `x₊x₋ = 16G*³`
are ring operations on `G*` alone. Only the individual roots require the branch. A
radical-free statement of the physical claim therefore goes through the symmetric
functions, and doing so is less flattering than the usual headline: if `x₊ = 1/α` exactly,
the small root is over-determined, and

$$x_-\big|_{\text{sum}} = 3.024136197494505, \qquad
  x_-\big|_{\text{product}} = 3.023967718055364,$$

a disagreement of **55.71 ppm**, against the **1.2572 ppm** conventionally quoted on the
large root — an amplification of `44.32 = x₊/x₋`. Both figures are correct and answer
different questions; the 1.26 ppm is the residual measured on the branch where it is most
compressed. When the claim is presented as being about the *polynomial* rather than about
one of its roots, 55.7 ppm is the figure that should be quoted.

## 9. Reproduction

```
python scripts/proofs/proof_radical_audit.py
```

Every arithmetic claim is computed in-run; none is transcribed.
