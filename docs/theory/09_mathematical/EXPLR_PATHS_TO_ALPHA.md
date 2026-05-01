# EXPLR — Logical Paths to α: Exhaustive Survey (Honest Negative Result)

**Document type:** Exploratory survey (deliberate negative result for the record)
**Status:** [SURVEY] — exhaustive enumeration with honest "no new path" verdict
**Created:** 2026-05-01
**Provenance:** User request "explore everything we have and see if there's any logical path to alpha", following completion of FTD-0110 closure attempt (closed-negative for representation-theoretic frameworks)
**Related:** Algebraic spine `SPEC_ALGEBRAIC_SPINE.md`; Maxwell-exploit thread (FTD-0113 through FTD-0120); FTD-0110 closure attempt (FTD-0119 + Phase B/C falsifications); FTD-0117 G* typo fix

---

## 0 · The question

Given everything established in the project — 9 algebraic-spine theorems, the Maxwell-exploit closure (FTD-0113 through FTD-0120), Phase A/B/C of the FTD-0110 closure attempt, the canonical-reference G\* typo fix — **is there a logical path that DERIVES α (the fine-structure constant) from FTD axioms?**

This document records an exhaustive search. The honest verdict is **no new path exists** beyond what's already in the LEDGER. The existing path through the algebraic spine remains [STRONGLY MOTIVATED CONJECTURE] (1.26 ppm match), not a derivation.

---

## 1 · What we have that's α-relevant

### 1.1 · Direct α paths (existing, status as of 2026-05-01)

| Path | Status | Strength |
|---|---|---|
| **Algebraic spine** (FTD-0001/0013): `x_+ = 137.036` from master quadratic; conjectured = 1/α | [STRONGLY MOTIVATED CONJECTURE] | 1.26 ppm match to CODATA |
| **Closed-form algebra** (FTD-0111 Theorem 8): α = 1/(2G\*) − √(4G\* − 1)/(4·G\*^(3/2)) | [DERIVED] from Theorem 8, conditional on x_+ = 1/α | Algebraic identity, not derivation of α itself |
| **EFT recovery** (R1, R2, R3 in `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md`): derive α from FTD action | **CLOSED NEGATIVE** | Three independent routes failed |
| **Z-factor reading** (FTD-0116): G\*² as lattice Z-factor analog | **CLOSED NEGATIVE** | Z_FTD = 1.99 (G18) ≠ G\*² = 8.75 |
| **7-term precision series** (`CONJ_SEVEN_TERM_PRECISION_SERIES.md`): post-hoc fit | [CONJECTURE] | Observationally underdetermined at CODATA precision |

### 1.2 · α-adjacent structures (could be combined?)

- **Phase G geometric Coulomb** (Theorem 6): α_r(r, L) = 2r·G_L(r). Continuum amplitude 1/(2π).
- **Watson identity** (Theorem 5): W₃ = G\*²/(2π).
- **Q(G\*) field structure** (Theorem 9, FTD-0112): Q(G\*) is a maximal π-free subfield of Q(π, Γ(1/4)).
- **Retarded Phase G** (FTD-0113): time-resolved coupling on light cone has universal amplitude 1/(2π).
- **Lattice Liénard-Wiechert** (FTD-0115): closed form via `(c|k̂|)² − (k·v)²` substitution.
- **Hodge duality preserved** (FTD-0114): Bianchi identities exact at the lattice level.
- **FTD-0110 multi-block** (Phase A): universal slow mode λ = −1.585786 across all 6 symmetry types.

### 1.3 · Empirical engine measurements

- Engine extracts α ≈ 0.042 (Rutherford cross-check, `DERIV_DAY2_CAMPAIGN.md`) — **factor ~6× off** from CODATA α ≈ 0.0073. Engine α is calibration-dependent (depends on g_s normalization).
- Cluster physics: k(A) ≈ ¼·(1 − 0.030·ln(A/2)). FTD-0110 [STRONGLY MOTIVATED CONJECTURE] for full nonlinear regime.

---

## 2 · Combinations checked in this exploration

### 2.1 · Algebraic combinations (mpmath, 50-digit precision)

| Combination | Value | Cleanness |
|---|---|---|
| `α · G*²` | 0.06388 | ≈ 1/16 within 2.2% — not exact |
| `α · 16·G*³` | 3.0240 | **= x_- exactly** (Vieta product) |
| `α · 16·G*²` | 1.02207 | **= x_+/(x_+ + x_-) form**, Vieta-derivable |
| `1/(2π) − α` | 0.15186 | not clean |
| `α · 2π` | 0.04585 | not clean |
| `α · 4π` | 0.09170 | not clean |
| `√α · G*` | 0.25274 | not clean |
| `α/G*²` | 0.000834 | not clean |
| `1/(2π·G*)` | 0.05379 | not clean |

**All "clean" combinations reduce to Vieta identities** (`x_+ + x_- = 16G*²`, `x_+ · x_- = 16G*³`). No NEW algebraic identity emerges.

### 2.2 · Logarithmic combinations

| Combination | Value | Cleanness |
|---|---|---|
| `ln(x_+)` | 4.920 | — |
| `ln(2π) + x_-` | 4.862 | within 1.2% of `ln(x_+)`, not exact |
| `ln(x_+) − ln(x_-)` | 3.814 | not clean |
| `ln(x_+ · x_-) = ln(16·G*³)` | 6.027 | trivial Vieta |

The "near-miss" `ln(x_+) ≈ ln(2π) + x_-` (1.2% off) doesn't elevate to an identity. It's coincidental at this precision; structural justification absent.

### 2.3 · Phase G coupling × α

```
α / (1/(2π)) = 2π·α = 0.04585          (not a clean number)
1/(2π·α) = x_+/(2π) = 21.81             (not a recognized constant)
```

The Phase G amplitude `1/(2π)` and α do not combine to anything recognizable. Their connection is through the Watson identity (already a theorem) and gives `G*² ≠ 2π/α`.

### 2.4 · FTD-0110 slow-mode eigenvalue × α

```
|λ_slow| · α = 1.586/137 = 0.01158      (not clean)
α / |λ_slow| = 0.0046                   (not clean)
|λ_slow| · 16·G*² = 222.2               (not clean)
```

The universal slow eigenvalue −1.586 has no clean algebraic relationship to α.

---

## 3 · Why the spine path is not a derivation (and what would close it)

The algebraic spine produces x_+ ≈ 137.036 as a **structurally rigid** number:
- Theorem 1 fixes G\* = Γ(1/4)/Γ(3/4) (algebraic identity).
- Theorem 4 fixes the master quadratic coefficient as 16 = |Aut(E)|².
- Theorem 3 (CM uniqueness) fixes d = −4 among class-number-1 fields.
- Theorems 2 + 8 fix the polynomial form and the closed-form root.

The IDENTIFICATION x_+ = 1/α is **separate**: it's the empirical claim that this structurally-rigid number equals the experimentally-measured fine-structure constant.

**For the spine path to become a derivation, one of these would have to land:**

1. **Structural mechanism forcing α = 1/x_+** (analogous to derivations in QFT where dimensionless ratios are fixed by symmetry). No candidate.

2. **Independent FTD route to α matching x_+** (engine measurement, EFT calculation, alternative algebraic derivation). EFT routes R1/R2/R3 closed-negative; engine α off by 6×; Z-factor reading falsified.

3. **Look-elsewhere argument that x_+ ≈ 1/α to 1.26 ppm is statistically forced** under a specific null model. FTD-0097 ran this for monomial-level fits and found the catalog over-rich — but the master quadratic's *dual* prediction (x_+ = 1/α AND x_- = N_c simultaneously, the single polynomial matching two independent physical constants) was distinguished from the chance-level monomial hits. FTD-0097 demoted L2 (monomial-level identity) to PARAMETRIC but left FTD-0001/0013/0014 as STRONGLY MOTIVATED CONJECTURE.

None has succeeded. The spine path remains [STRONGLY MOTIVATED CONJECTURE].

---

## 4 · Honest verdict

**No new logical path to α emerges from combining what we have.**

The existing path (algebraic spine + empirical identification) is the only path. It produces α = 1/137.036 conditional on the conjecture x_+ = 1/α, which has 1.26 ppm strength but is not a derivation.

The 9 spine theorems collectively give a *highly constrained mathematical object* (the master quadratic with x_+ = 137.036), and this object has clean structural properties (Watson identity, π-free in Q(G\*), harmonic invariant tower, Hodge duality, retarded Phase G, etc.). But none of these properties **forces** the identification x_+ = 1/α — they're all properties of x_+ AS A MATHEMATICAL NUMBER, not properties that elevate the identification to theorem-grade.

### 4.1 · What this exploration rules out

- **Algebraic combinations of {α, G\*, x_+, x_-, 1/(2π), |λ_slow|}** do not yield new structural identities beyond Vieta. (§2.1, §2.2, §2.3, §2.4)
- **Combining Maxwell-exploit results (FTD-0113-0120) with the algebraic spine** does not reveal a new α-derivation route. Phase G amplitude 1/(2π) is structurally distinct from α; the 1/(2π) comes from 3D solid angle, α has a different origin.
- **Combining FTD-0110 multi-block analysis with α** does not work either — the universal slow eigenvalue is independent of α, and the per-block frameworks (1/√d, Langevin equipartition) failed at the cluster-physics level.

### 4.2 · What's still open (but not addressed by this exploration)

- **Mechanism β (genesis-kink statistical mechanics):** untested; could in principle produce a derivation of g_s² = α from FTD's nonlinear thresholding dynamics, though no candidate calculation has been proposed.
- **Mechanism γ (Langevin amplitude crossover):** untested; could give A-dependent corrections that bear on α-related observables.
- **A fundamentally new framework:** RG flow on the global lattice, direct simulation matching, or something not yet conceived.

These remain genuinely open. They could, in principle, yield an α-derivation. But this exploration does not establish any of them.

### 4.3 · CLAUDE.md anti-target discipline

The exploration adheres to the project's anti-target rule:
- Did NOT search for combinations that "happen to fit" α to high precision (would be Koide-style fishing).
- Did NOT promote any near-coincidence (e.g., `α·G*² ≈ 1/16` within 2%, or `ln(x_+) ≈ ln(2π) + x_-` within 1.2%) to structural significance — these are explicitly tagged as numerical coincidences.
- Did report the negative result (no new path) honestly.

---

## 5 · Where the project actually stands on α

| Question | Answer | Status |
|---|---|---|
| Does FTD have an algebraic object equal to 137.036? | YES, x_+ = root of master quadratic | [THEOREM] |
| Does this number match the empirical fine-structure constant? | YES, to 1.26 ppm | [STRONGLY MOTIVATED CONJECTURE] |
| Is the match a derivation? | NO | The IDENTIFICATION x_+ = 1/α is conjecture |
| Have alternative derivation routes been tried? | YES (R1, R2, R3, Z-factor) | All [CLOSED NEGATIVE] |
| Are there untried derivation routes? | YES (β, γ, RG, simulation) | All [OPEN] |
| Does this exploration reveal a NEW path? | **NO** | Honest negative result, this document |

---

## 6 · LEDGER status

This document does NOT introduce a new LEDGER entry. It records an
exhaustive negative-result survey. The status of FTD-0001 (master
quadratic) and FTD-0013 (x_+ = 1/α conjecture) is unchanged.

The intended use is forward-looking: when someone proposes a "new
path to α" (as is likely given the framework's structure), they can
cross-check against this document to see whether their proposed path
has been considered before. If yes, the failure mode is documented
here. If no, that's a genuine new direction worth pursuing.

---

## 7 · What this document does NOT claim

- **NOT a closure of FTD-0001/0013.** The master quadratic theorem and
  the empirical x_+ = 1/α conjecture are unchanged.
- **NOT a falsification of α-derivation prospects.** Future work might
  yield a derivation; this exploration just establishes that none of
  the currently-available structures combines into a new path.
- **NOT a proof that no derivation is possible.** Mechanisms β, γ, and
  fundamentally new approaches remain open.
- **NOT a statement about the empirical strength of the conjecture.**
  The 1.26 ppm match remains the strongest single piece of evidence.
  This exploration says only that no NEW evidence emerges from
  combining other project structures.

---

## 8 · Single-line summary

**With the 9 algebraic-spine theorems, the Maxwell-exploit thread
(FTD-0113-0120), the FTD-0110 closure-attempt artifacts, and the
post-FTD-0117 canonical references all in clean state, no NEW logical
path to deriving α emerges from algebraic, logarithmic, or structural
combinations. The existing path through the master quadratic remains
[STRONGLY MOTIVATED CONJECTURE] at 1.26 ppm strength. Future progress
requires either Mechanism β/γ analysis or a fundamentally new
framework — neither of which is supplied by current project structures.**

---

*End of exploration.*
