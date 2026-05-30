# EXPLR — Logical Paths to α: Exhaustive Survey (Honest Negative Result)

**Document type:** Exploratory survey (deliberate negative result for the record)
**Status:** [SURVEY] — exhaustive enumeration with honest "no new path" verdict
**Created:** 2026-05-01
**Provenance:** User request "explore everything we have and see if there's any logical path to alpha", following completion of FTD-0110 closure attempt (closed-negative for representation-theoretic frameworks)
**Related:** Algebraic spine `SPEC_ALGEBRAIC_SPINE.md`; Maxwell-exploit thread (FTD-0113 through FTD-0120); FTD-0110 closure attempt (FTD-0119 + Phase B/C falsifications); FTD-0117 G* typo fix

---

## 0 · The question

Given everything established in the project — the algebraic spine (nine numbered results: six theorem-grade + three honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0), the Maxwell-exploit closure (FTD-0113 through FTD-0120), Phase A/B/C of the FTD-0110 closure attempt, the canonical-reference G\* typo fix — **is there a logical path that DERIVES α (the fine-structure constant) from FTD axioms?**

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

3. **Look-elsewhere argument that x_+ ≈ 1/α to 1.26 ppm is statistically forced** under a specific null model. FTD-0097 ran this for monomial-level fits and found the catalog over-rich — but the master quadratic's polynomial-template uniqueness was distinguished from the chance-level monomial hits. FTD-0097 demoted L2 (monomial-level identity) to PARAMETRIC but left FTD-0001/0013 as [THEOREM] / [STRONGLY MOTIVATED CONJECTURE]. *(Historical framing referenced the "*dual* prediction (x_+ = 1/α AND x_- = N_c simultaneously)" — the `x_- ↔ N_c` identification is **RETIRED** per v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The polynomial-template-uniqueness fact stands independently and has been substantially strengthened by FTD-0189: the master quadratic is the unique dual-matcher across 2.65 M degree-2 polynomials over an 18-constant FTD-undesigned basket — 0 non-G\* dual-matchers, rank 1 by ~130×.)*

None has succeeded. The spine path remains [STRONGLY MOTIVATED CONJECTURE].

---

## 4 · Honest verdict

**No new logical path to α emerges from combining what we have.**

The existing path (algebraic spine + empirical identification) is the only path. It produces α = 1/137.036 conditional on the conjecture x_+ = 1/α, which has 1.26 ppm strength but is not a derivation.

The nine numbered spine results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0) collectively give a *highly constrained mathematical object* (the master quadratic with x_+ = 137.036), and this object has clean structural properties (Watson identity, π-free in Q(G\*), harmonic invariant tower, Hodge duality, retarded Phase G, etc.). But none of these properties **forces** the identification x_+ = 1/α — they're all properties of x_+ AS A MATHEMATICAL NUMBER, not properties that elevate the identification to theorem-grade.

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

## 7.5 · RG-running approach (added 2026-05-01 evening)

After the initial survey returned negative, the user requested attempting
"a mechanism we haven't tried" successfully producing α. The genuinely
**untried route** is **RG running from FTD lattice scale to lab scale**:
take FTD's natural coupling at the lattice scale (= M_Pl per a_phys ≡
ℓ_P) and run it under some β-function down to laboratory energy
(Q ~ m_e); if the result equals 1/137, FTD has derived α.

### 7.5.1 · Setup

**FTD natural couplings at M_Pl:**
- Phase G amplitude (geometric, `1/(2π)`):     **0.1592**
- Engine Rutherford α (Day-2 EFT):             **0.0420**
- ALPHA_EFT = G_C² in `constants.h`:           **0.333** (= 1/3)

**Standard QED running** (asymptotically non-free, β = +α²/(3π)):
```
α(m_e) = 1/137.036  →  α(M_Pl) = 1/126.10 = 0.00794
```
QED's α at M_Pl is ~6× smaller than FTD's engine measurement.

### 7.5.2 · Required FTD β-function

To run FTD's natural coupling at M_Pl DOWN to physical α at m_e
(asymptotic freedom, β = −c·α):

```
c = ln(α_FTD_lat / α_phys) / ln(M_Pl / m_e)
```

| FTD natural at M_Pl | Required c | Compare to QED β = +1/(3π) ≈ 0.106, QCD β ≈ 0.557 |
|---|---|---|
| Engine = 0.042 | **0.0340** | ~3× smaller than QED, ~16× smaller than QCD |
| Phase G = 1/(2π) | 0.0598 | ~2× smaller than QED |

### 7.5.3 · Result: no clean structural match

Searching for clean structural numbers near c = 0.034:

| Candidate | Value | Off by |
|---|---|---|
| 1/30 | 0.03333 | 1.9% |
| 1/(2π·5) | 0.03183 | 6.3% |
| α·N_eff/3 | 0.03162 | 6.9% |
| 1/(3π·3) | 0.03537 | 4.1% |
| 1/(8·G\*) | 0.04225 | 24% |

None of these are structurally motivated FTD constants. The "1% match"
candidates (`α/0.215`, `0.44/N_eff`) found by my search are tautological
(rearranging the equation) or arbitrary (where does 0.44 come from?).

### 7.5.4 · Verdict

**The simple RG-running approach (β = −c·α) does NOT yield α from FTD
axioms.** The required β-coefficient is empirically ≈ 0.034, but no
clean FTD structural constant matches this.

**Possible escapes:**
- **More complex β-function** (non-power-law, multi-loop): could in
  principle hit any required value but requires deriving the actual
  FTD β-function from genesis dynamics + lattice action (a substantial
  research program).
- **Different UV scale** (not M_Pl): if FTD's effective UV scale is
  some Q* < M_Pl, the running distance is shorter. But this would
  require structural justification for Q*.
- **Different IR matching point** (not m_e): perhaps the engine α
  matches at a higher scale and runs only to some intermediate scale.
  Same justification problem.

**None of these escapes has a candidate in current project structures.**

### 7.5.5 · Cumulative status of α-derivation routes (2026-05-01)

Adding RG-running to the prior list:

| Route | Status | Reason |
|---|---|---|
| Algebraic spine (master quadratic) | [STRONGLY MOTIVATED CONJECTURE] | 1.26 ppm match; identification empirical |
| EFT recovery R1, R2, R3 | [CLOSED NEGATIVE] | Three routes failed |
| Z-factor reading (FTD-0116) | [CLOSED NEGATIVE] | Z_G18 ≠ G\*² |
| Algebraic combinations of spine theorems | [NEGATIVE] | §2.1-2.4 of this doc |
| RG running (simple β = −c·α) | **[NEGATIVE]** (this section) | Required c ≈ 0.034 not clean structural |
| Mechanism β (genesis-kink for α) | [UNTRIED] | Speculation only |
| Mechanism γ (Langevin amplitude crossover for α) | [UNTRIED] | Speculation only |
| Look-elsewhere refinement (polynomial-level) | [UNTRIED] | Could strengthen empirical case but not derive |

### 7.5.6 · Honest meta-assessment after all attempts

**Every session-scale α-derivation attempt has reached the same conclusion:** the existing path through the algebraic spine (with empirical identification x_+ = 1/α at 1.26 ppm) is the only path with substantial evidence, and no derivation route has converted this conjecture to a theorem.

This is not a "we haven't tried hard enough" situation. It's a **structural shape of the problem**: FTD's mathematics gives a rigid prediction (x_+ = 137.036 with 1.26 ppm error to physical α), but the *bridge from mathematics to physics* — connecting the dimensionless number x_+ to the dimensional fine-structure constant — requires a mechanism that hasn't materialized despite multiple attempts.

The candidates that remain genuinely untried (Mechanism β/γ for α; non-trivial β-function derivation; statistical look-elsewhere refinements) are all substantial research programs (1-4 weeks each), not session tasks. None has clear prospects for closure.

**The α-derivation problem is the central open question of FTD.** This series of negative results sharpens that statement: *if* the conjecture is correct, the closure requires structural machinery we don't yet have. *If* the conjecture is wrong (i.e., x_+ = 137.036 is coincidental), the empirical strength is very strong and the catalog-level look-elsewhere arguments don't decisively rule out coincidence.

The framework's strongest external move remains **Paper A**: peer-reviewed publication of the algebraic spine (without claiming α-derivation), which converts the rigorous mathematical content into validated external standing without relying on the open conjecture.

---

## 8 · Single-line summary

**With the algebraic spine (nine numbered results: six theorem-grade + three honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0), the Maxwell-exploit thread
(FTD-0113-0120), the FTD-0110 closure-attempt artifacts, and the
post-FTD-0117 canonical references all in clean state, no NEW logical
path to deriving α emerges from algebraic, logarithmic, structural,
or RG-running combinations. The existing path through the master
quadratic remains [STRONGLY MOTIVATED CONJECTURE] at 1.26 ppm strength.
The FTD-required β-coefficient for RG running (c ≈ 0.034) does not
match any clean structural constant. Future progress requires either
Mechanism β/γ analysis, non-trivial β-function derivation, or a
fundamentally new framework — none of which is supplied by current
project structures.**

---

*End of exploration.*
