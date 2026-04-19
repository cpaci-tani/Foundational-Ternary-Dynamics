# AUDIT — The Master Quadratic (Phase I)

**Tag:** [AUDIT] — independent numerical + epistemic review of the claim that
`x² − 16G*²x + 16G*³ = 0` derives the fine-structure constant.
**Trigger:** after Phase G+H resolved the engine-side V(r) measurement as
geometric Coulomb, the master quadratic is the only remaining α-derivation
claim in FTD. User requested a full audit.
**Date:** 2026-04-19
**Scope:** the polynomial itself, the claim `x₊ = 1/α`, the 7-term precision
series, and the status of each supporting document.
**Verdict (short):** the master quadratic is **mathematically real and
uniquely tight among a large scan of candidate polynomials**, but its
identification as α's derivation rests on selection choices (which curve?
which polynomial form? which root?) that are honestly flagged in the
source material but overstated in summary headings. Tree-level result is
**[STRONGLY MOTIVATED CONJECTURE]**. The 7-term precision series is a
**post-hoc fit** and should be demoted from any "< 0.001 ppt derivation"
framing. The dual-prediction property (x+ matches 1/α, x- matches N_c=3
simultaneously from the same polynomial) is the strongest structural
evidence.

---

## 1 · What the master quadratic actually is

Pure algebra:

```
  G* = Γ(1/4)/Γ(3/4) = 2.958675119188638892...   (lemniscatic constant)
  x² − 16 G*² x + 16 G*³ = 0

  x₊ = 137.036171458155...     (larger root)
  x₋ = 3.023963916339...       (smaller root)

  Identification claimed by FTD:
    x₊ ≡ 1/α        (measured 1/α = 137.035999177, 1.26 ppm discrepancy)
    x₋ ≡ N_c        (N_c = 3, 0.80% discrepancy)
```

All of the algebra above is rigorous to arbitrary precision. The proof
script `scripts/proofs/proof_motivic_master_quadratic.py` verifies the
polynomial identity `x₊² − 16G*²x₊ + 16G*³ = 2.5e-47` at 50-digit
precision — pure mathematics, nothing to audit there.

The audit question is **physical**: why does this polynomial, and why
do its two roots happen to equal 1/α and N_c?

## 2 · Rigidity test — is this a coincidence?

Run `scripts/proofs/audit_master_quadratic_rigidity.py`.

### 2.1 · Coefficient scan

Polynomials of the form `x² − a·G*^p·x + b·G*^q = 0` for integer
`a, b ∈ [1, 64]` and `p, q ∈ [1, 4]`: **59,611 valid polynomials scanned**.
How many have a positive root within various precisions of 1/α?

| Tolerance | Hits |
|---|--:|
| 1 ppm | **0** |
| 10 ppm | **2** (master + one competitor) |
| 100 ppm | 3 |
| 1000 ppm | 25 |
| 1% | 306 |

**The master quadratic at 1.26 ppm is the single most precise polynomial
in the scan.** The nearest competitor (`a=49, b=42, p=1, q=3`) lands at
9.04 ppm — 7× worse. So the master quadratic's precision is real: random
coefficient choices would hit 1.26 ppm only by accident 1 time in roughly
60,000, and the observed frequency matches.

### 2.2 · G* sensitivity

Perturbing G* by a fractional amount δ shifts x+ by ≈ 2δ (log derivative
2.02). So the 1.26 ppm precision in x+ requires G* accurate to 0.6 ppm.
G* = Γ(1/4)/Γ(3/4) is exact, so this is met automatically — but it
emphasizes that the 1.26 ppm is not from "G* was chosen to fit α."
G* is canonical; the precision is in the coefficient structure.

### 2.3 · Alternative constants

Replacing G* with other canonical constants (π, e, φ, √2+√3, Γ(1/3),
Euler γ, etc.) in the master-quadratic shape gives roots far from 137:

| Constant | Value | x+ | rel err vs 1/α |
|---|--:|--:|--:|
| **G* = Γ(1/4)/Γ(3/4)** | 2.9587 | **137.036** | **1.26e-06** |
| π | 3.1416 | 154.71 | 1.3e-01 |
| e | 2.7183 | 115.44 | 1.6e-01 |
| sqrt(2)+sqrt(3) | 3.1463 | 155.17 | 1.3e-01 |
| golden ratio φ | 1.6180 | 40.20 | 7.1e-01 |
| Γ(1/3) | 2.6789 | 112.08 | 1.8e-01 |

None of the standard constants produce 1/α to better than ~10%. G*
really is special in this polynomial shape.

### 2.4 · Naive integer polynomial

The simplest integer polynomial `x² − 140 x + 411 = 0` gives x+ = 137.0000
(263 ppm error — "137 to three digits from two 3-digit integers"). The
best integer (a, b) with a ≤ 200, b ≤ 500: `x² − 140 x + 406 = 0` gives
x+ = 137.0373 (9.5 ppm). So even a brute-force integer fit gets within
10 ppm. **The master quadratic at 1.26 ppm is ~8× better than the best
brute-force integer fit**, which is significant but not overwhelming.

### 2.5 · Dual-prediction analysis

The master quadratic's strongest distinguishing feature: **both** roots
match physical constants. Scanning for polynomials with x+ within 1000
ppm of 1/α *and* x- within 1% of a small integer (1–10):

| Polynomial | x+ | x- | integer target | err x+ | err x- |
|---|--:|--:|---:|--:|--:|
| `16 G², 47 G²` | 137.058 | 3.002 | 3 | 163 ppm | 0.06% |
| `48 G, 9 G⁴` | 136.982 | 5.035 | 5 | 396 ppm | 0.69% |
| `49 G, 42 G³` | 137.037 | 7.938 | 8 | 9 ppm | 0.78% |
| **`16 G², 16 G³`** (MASTER) | **137.036** | **3.024** | **3** | **1.26 ppm** | **0.80%** |

Of the four dual-hits in the 60k scan, **only two have x- near N_c = 3**,
and the master quadratic is the more precise on x+ by two orders of
magnitude. **The dual-prediction structure is much stronger evidence for
the master quadratic's specialness than either root alone.** A random
polynomial has ~25 chances in 60,000 to hit 1/α within 1000 ppm, and
independently some chance to hit N_c — the coincidence that a single
natural polynomial does both is genuinely rare.

## 3 · Where the coefficient 16 comes from

Per `docs/theory/09_mathematical/MATH_MASTER_QUADRATIC.md` §4 and the
motivic proof script, **six independent arithmetic routes all yield 16**
for the curve E: y² = x³ − x:

1. `|Aut(E)|² = 4² = 16`
2. `|E(ℚ)_tors|² = 4² = 16`
3. `|Stab_Oh(axis)| = |O_h|/3 = 48/3 = 16` (octahedral)
4. `Cond(E)/2 = 32/2 = 16`
5. `|Δ(E)|/4 = 64/4 = 16`
6. `DOF_gauge(temporal) = 24 − 7 − 1 = 16`

Routes 1, 2, 4, 5 are all tied to the same curve's symmetry group of order
4 — these are not fully independent; they all factor through the fact
that `y² = x³ − x` has automorphism group of order 4. Routes 3, 6 are
distinct (lattice geometry; gauge-fixing counting).

Honest count: **~3 genuinely independent routes** (curve arithmetic,
lattice octahedral counting, gauge DOF counting). Still noteworthy, and
this layer is rigorously proven in the referenced documents.

## 4 · Where the claim is weaker

### 4.1 · Why this curve?

The choice E: y² = x³ − x is canonical (the unique simplest CM curve
with j = 1728 and complex multiplication by ℤ[i]) — but canonical ≠
forced. FTD argues the choice is dictated by the 3D cubic lattice's
having `ℤ[i]` as its natural CM structure via the BCC sublattice's
Watson integral W₃ = G*²/(2π). That argument is laid out in
`DERIV_WATSON_GSTAR_IDENTITY.md` and is rigorous — but it still
requires the starting premise "3D cubic lattice" to be the right
physical object, which is axiomatic in FTD.

**Status: [SELECTION]** — defensible, but not derived from more
primitive inputs.

### 4.2 · Why degree 2?

Two arguments, both acknowledged as motivational-not-forcing in
`DERIV_QUADRATIC_NECESSITY.md`:

1. **Ontological**: the ternary axiom is degree 1; self-referential
   closure doubles it to degree 2. The "one layer of closure" step is
   a [SELECTION], not a [THEOREM].
2. **Number-theoretic**: CM field ℚ(i) has degree 2 over ℚ, bounding
   any CM-period relation by degree 2. This proves degree ≤ 2 (given
   CM field) but doesn't prove exactly 2.

Together these make degree 2 motivated but not forced. A
transcendental relation, or a degree-4 relation with two extra
roots, isn't structurally excluded.

**Status: [SELECTION].**

### 4.3 · Why x+ = 1/α?

This is the crucial physical identification. The current justification
is:

- Numerical: 1/α ≈ 137.036, master quadratic gives 137.036, match to
  1.26 ppm.
- Dual: x- = 3.024 matches N_c = 3, simultaneously.
- Structural: in FTD's gauge-theory narrative, x+ is the "largest
  coupling" → identified with U(1) electromagnetism; x- is the
  "second coupling" → identified with SU(3) color.

The dual-prediction argument (§2.5) strengthens this considerably
beyond "one-digit coincidence," but it is still not a dynamical
derivation of α from first principles. The engine-side test of this
identification (via Phase H's explicit coupling) now works — but that
only verifies the plumbing, not the uniqueness of α as the output.

**Status: [STRONGLY MOTIVATED CONJECTURE]**, conditional on the curve
and polynomial-form selections above.

## 5 · The 7-term precision series — demote to [CONJECTURE]

CLAUDE.md and several summary docs claim the master quadratic gives
α "to 0.001 ppt with a 7-term expansion." This is materially misleading
and should be rewritten:

- The 7-term series with coefficients {9/47, 5/64, 4/141, 141/11,
  1472/21, 416/21, 299/8} matches `1/α_CODATA` to **24 digits**.
- CODATA 2022 itself only reports 1/α to ~11 digits (±2.1 × 10⁻⁸).
- The 13 extra digits of agreement have no experimental content.
- The coefficients were **fitted** to CODATA, then observed to have a
  clean base-integer decomposition in post-hoc form. Per
  `CONJ_SEVEN_TERM_PRECISION_SERIES.md` §3.3 rigidity audit (2026-04-17):
  6 out of 7 coefficients are unique in the "clean integer" family, but
  only *after* they are constrained to match CODATA. That is circular.

**Status: [CONJECTURE], not [DERIVATION].** CLAUDE.md headline
"< 0.001 ppt with 7-term expansion" should read "algebraic identity
matching the fitted CODATA digits to 24-digit precision; no new
physical content beyond the 11-digit measurement."

## 6 · Verdict

### What is [THEOREM] after this audit

- The polynomial identity `x² − 16G*²x + 16G*³ = 0` has roots
  `x+ = 137.0361714...` and `x- = 3.0239639...` (pure algebra).
- G* = Γ(1/4)/Γ(3/4) is a period of the motive h¹(E) for E: y² = x³ − x.
- The coefficient 16 arises as `|Aut(E)|²` (plus 2 other independent
  routes), purely from the curve's arithmetic.
- Watson's W₃ = G*²/(2π) is the BCC lattice self-energy.

### What is [SELECTION]

- Curve choice E: y² = x³ − x (defensible via Watson ↔ BCC, but axiom
  of 3D cubic lattice is an input).
- Degree-2 polynomial form (two independent motivational arguments,
  neither forcing).
- Physical identification x+ = 1/α, x- = N_c (numerically tight but
  not derived dynamically — conditional on the curve + form).

### What is [CONJECTURE] / [FIT]

- 7-term precision series — post-hoc algebraic decomposition matching
  CODATA digits beyond measurable precision. Should not be labeled
  [DERIVATION].

### What the dual-prediction buys

The strongest honest claim is: among all natural quadratic relations on
the graded period ring of E, **the master quadratic is the unique
polynomial whose two roots simultaneously match the physical constants
1/α (to 1.26 ppm) and N_c (to 0.8%).** This joint match is roughly
1-in-10,000 to 1-in-100,000 against random scan baselines. It is not
proof of an identity, but it is much stronger than either single-root
match alone, and it is the piece of the argument most worth taking
seriously.

## 7 · Recommendations — what would elevate [SELECTION] to [THEOREM]

1. **Prove the L → ∞ limit rigorously.** The gap-equation derivation
   works in the thermodynamic limit but hasn't been proven to converge
   from finite tori. An analytical proof would close a real gap.
2. **Run the same structural search on alternative CM curves.** If the
   argument "y² = x³ − x is forced because of ℤ[i] = BCC CM" is right,
   then no other CM curve (e.g., y² = x³ − 15x + 22 over ℚ(i)) should
   produce a master-quadratic-shaped polynomial with roots matching
   physical constants. If some other curve also works, the uniqueness
   claim collapses.
3. **Isolate the dynamical derivation of α.** Phase H verified that an
   engine with explicit coupling g_c reproduces Coulomb. But the
   engine still needs g_c as input — it doesn't produce it. A
   first-principles derivation of g_c from lattice action alone (no
   master quadratic) would make the master quadratic's prediction
   falsifiable.
4. **Retire the "< 0.001 ppt" headline** in CLAUDE.md and summary docs.
   Replace with honest framing: "1.26 ppm tree-level agreement from a
   uniquely-coefficient polynomial; 24-digit expansion is a post-hoc
   algebraic fit to CODATA digits that exceed experimental precision."

## 8 · Reproducibility

```
scripts/proofs/proof_motivic_master_quadratic.py      # 50-digit algebra, [THEOREM] layer
scripts/proofs/audit_master_quadratic_rigidity.py     # NEW: 60k polynomial scan + sensitivity
docs/theory/09_mathematical/MATH_MASTER_QUADRATIC.md  # the 250-page math layer
docs/theory/09_mathematical/DERIV_QUADRATIC_NECESSITY.md  # why degree 2 (honest [SELECTION])
docs/theory/08_structural/DERIV_WATSON_GSTAR_IDENTITY.md  # BCC ↔ period bridge
docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md  # BCC / Watson
docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md   # existing catalog of selections
```

Run `python scripts/proofs/audit_master_quadratic_rigidity.py` to
reproduce the numerical rigidity table.
