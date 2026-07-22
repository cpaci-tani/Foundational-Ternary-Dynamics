# DERIV — The electron Yukawa prefactor `16√2/3` from O_h character theory + Z[i]-norm structure

**Tag:** [STRUCTURALLY MOTIVATED PARAMETRIC] — the multiplicity/norm factors are [THEOREM], the α³ ladder exponent is [SELECTION] (FTD-0397), and the *combination as the Yukawa coupling* is [SELECTION] justified by the empirical match.
**LEDGER:** FTD-0134 (partial closure of FTD-0133's open question).
**Verification script:** `scripts/proofs/proof_yukawa_from_27block_character.py` (ALL STEPS PASS, 0.14% precision).
**Depends on:**
- DERIV_K_FROM_OH_A1G_MULTIPLICITY.md (FTD-0110, [DERIVED at linear level]) for the 27-block O_h decomposition framework
- Theorem 8 (FTD-0111) for the (1+i)-tower structure where `1+i` lives
- FTD-0015 mass formula being decomposed
- FTD-0017 m_H formula (parallel structure)
- HIGGS-4 v formula (where the √(2π) factor lives)
- FTD-0397 order-type no-go (the `n_e-n_v=3` ladder difference is [SELECTION], not forced by the unordered multiset)
- FTD-0133 (the audit that surfaced the prefactor question)
**Closes (partial):** FTD-0133's open question "where does 16/3 come from structurally?" — answer here is "from the 27-block O_h character decomposition's mult ratio."
**Does NOT close:** The substrate-level Yukawa coupling derivation; this remains MC-T4.3-class work.

---

## 0 · Summary

The standard-model electron Yukawa coupling `y_e = √2·m_e/v` factorizes in FTD as

$$\boxed{\;y_e \;=\; \frac{(\text{mult}_{A_{1g}})^{2}}{\text{mult}_{T_{1u}}}\cdot|1+i|\cdot\alpha^{N_c} \;=\; \frac{16\sqrt{2}}{3}\,\alpha^{3}\;}$$

where the multiplicities are computed in the natural 27-dimensional permutation representation `ρ_27` of the cubic point group `O_h` on the 3³ Moore block, and `|1+i| = √2` is the norm of the unique prime above 2 in the Gaussian integers `Z[i]`.

**Each factor in the decomposition has a clean structural identification:**

| Factor | Structural reading | Tag |
|---|---|---|
| 16 | `mult(A_{1g})² = 4²` in 27-block O_h decomposition | **[THEOREM]** (character formula) |
| 1/3 | `1/mult(T_{1u})` in 27-block O_h decomposition | **[THEOREM]** (character formula) |
| √2 | `|1+i|` = norm of the prime above 2 in `Z[i]` | **[THEOREM]** (algebraic number theory) |
| α³ | `α^{N_c}` = selected cumulative ladder difference, `n_e − n_v = 11 − 8 = 3` | **[SELECTION]** (FTD-0397) |

**The combination as the Yukawa coupling itself is [SELECTION]** — we identify this product with `y_e` because the empirical match is 0.14%. Promoting to [DERIVED] requires deriving the Yukawa formula from FTD substrate dynamics (MC-T4.3-class).

**This is a structural-identification proposal, not a derivation.** Tag class: [STRUCTURALLY MOTIVATED PARAMETRIC]. The theorem-grade multiplicities and norm do not promote the selected α³ exponent or the selected combination.

---

## 1 · The 27-block O_h decomposition (verified independently)

The natural 27-dimensional permutation representation `ρ_27` of the cubic point group O_h on the 3×3×3 Moore block decomposes as:

$$\rho_{27} \;\cong\; 4\cdot A_{1g} \;\oplus\; 2\cdot E_g \;\oplus\; 2\cdot T_{2g} \;\oplus\; A_{2u} \;\oplus\; 3\cdot T_{1u} \;\oplus\; T_{2u}$$

This decomposition was originally given in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §2` (FTD-0110). It was independently verified in this work via the standard O_h character table + per-class fixed-point counts on the 27-block (see verification script).

### 1.1 Independent verification

The 10 conjugacy classes of O_h with sizes `[1, 8, 6, 6, 3, 1, 8, 6, 3, 6]` (sum 48 = |O_h|), and the per-class fixed-point count `χ_27 = [27, 3, 3, 3, 3, 1, 1, 9, 9, 1]` for the 27-block permutation representation, give multiplicities via the standard character formula:

$$\text{mult}(\rho_i) \;=\; \frac{1}{|G|}\sum_{\text{classes}\,c}|c|\cdot\chi_{27}(c)\cdot\chi_{\rho_i}(c)^{*}$$

Computed multiplicities (verified to be all non-negative integers; dim sum = 27):

| Irrep | Dim | Mult | Contribution |
|---|---|---|---|
| `A_{1g}` | 1 | **4** | 4 |
| `A_{2g}` | 1 | 0 | 0 |
| `E_g` | 2 | 2 | 4 |
| `T_{1g}` | 3 | 0 | 0 |
| `T_{2g}` | 3 | 2 | 6 |
| `A_{1u}` | 1 | 0 | 0 |
| `A_{2u}` | 1 | 1 | 1 |
| `E_u` | 2 | 0 | 0 |
| `T_{1u}` | 3 | **3** | 9 |
| `T_{2u}` | 3 | 1 | 3 |
| **Total** | | | **27 ✓** |

Both load-bearing multiplicities verified: `mult(A_{1g}) = 4` (matches FTD-0110) and `mult(T_{1u}) = 3` (NEW THIS WORK).

### 1.2 What `T_{1u}` is structurally

`T_{1u}` is the 3-dimensional **vector representation** of O_h. It transforms like the position vector `x = (x, y, z)`. Its multiplicity 3 in `ρ_27` means there are three independent vector-like sub-representations of the 27-site permutation rep — naturally corresponding to the three Cartesian axes' worth of polarization on the Moore block.

The numerical equality `mult(T_{1u}) = 3 = N_c` is the pivot of this work: it identifies the "3" in the Yukawa prefactor `16/3` with a *character-theoretic multiplicity* rather than just "the color count" or "the spatial dimension." This sharpens the structural reading without changing the numerical value.

---

## 2 · The structural decomposition

The empirical Standard-Model Yukawa coupling for the electron is

$$y_e \;=\; \frac{\sqrt{2}\cdot m_e}{v} \;=\; 2.935\times 10^{-6}$$

(using `m_e c² = 0.5110 MeV` and `v = 246.22 GeV`).

The FTD prefactor formula (from FTD-0015 + the m_e/v reduction surfaced in FTD-0133):

$$y_e \;=\; \frac{16\sqrt{2}}{3}\cdot\alpha^{3}$$

Numerical agreement: **0.14%** (matches FTD-0015's existing precision exactly, since the formulas are algebraically equivalent under the `m_e = y_e · v / √2` SM relation).

### 2.1 Decomposing the prefactor

Each factor has a [THEOREM]-grade structural identification:

**Factor 1: 16 = `mult(A_{1g})²`**

The squared multiplicity of the trivial irrep in the 27-block. Numerically `4² = 16`. Structurally: the dimension of the "scalar subspace" of the 27-site permutation representation, squared. The squaring corresponds (under SM Yukawa-coupling normalization) to "two scalar legs meeting at a vertex" — one from the matter side, one from the Higgs.

This is the SAME 16 as `|Aut(E)|²` for the lemniscatic CM curve `y² = x³ − x` (Theorem 4 of the algebraic spine). Both readings give 16; the multiplicity reading is structurally cleaner because it's directly tied to the lattice's symmetry rep theory.

**Factor 2: 3 = `mult(T_{1u})`**

The multiplicity of the vector irrep in the 27-block. Numerically `3 = N_c = D`. The structural identification ties the "color count" / "spatial dimension count" to the *number of independent vector-like polarizations* available on the Moore block.

This is one structural reading of the "3 in the denominator," which is otherwise ambiguous between Reading A (D = 3 spatial dimensions) and Reading B (N_c = 3 color charges) across FTD docs. The character-theoretic reading `mult(T_{1u})` is the cleanest of the three because it depends only on the O_h action on the 27-block — not on a separate physics-interpretation choice.

**Factor 3: √2 = `|1+i|`**

The norm of the unique prime above 2 in the Gaussian integers `Z[i]`. The prime `(1+i)` is exactly the generator that defines the (1+i)-tower of master quadratics in Theorem 8 (FTD-0111). Its norm `N(1+i) = (1+i)(1−i) = 2`, so `|1+i| = √2`.

This factor identifies the √2 in the Yukawa formula as the **first (1+i)-tower step contribution** — the natural normalization step from level k=0 to k=1 in the algebraic-spine tower. The same `(1+i)` whose 4th power gives the canonical master quadratic at level k=4 (whose coefficient is `2^4 = 16` — the same 16 as Factor 1, in a different reading).

**This identification supersedes the previous "√(2π) inherited from HIGGS-4 v formula" reading (which FTD-0133 honestly downgraded to [SELECTION]).** The `√2 = |1+i|` reading is a [THEOREM]-grade algebraic identification; the `√(2π) = √(2π)` reading is `(√2)·(√π)` where `√π` does need to come from elsewhere (e.g., the v formula's normalization). So the `√(2π)` factor is `|1+i| × √π`, with `|1+i|` identified algebraically and `√π` still inherited from the v formula's [SELECTION].

**Factor 4: α³ as the selected `α^{N_c}` ladder difference**

The exponent 3 equals `N_c` for the selected ladder assignment `n_e=11`, `n_v=8`. FTD-0397 proves that the unordered FTD-0084 multiset and every permutation-invariant function of it cannot select the electron ordering. Therefore the difference `n_e-n_v=3` is **[SELECTION]** unless independently derived order-bearing dynamics supplies those positions. The numerical equality to `N_c` is structural motivation, not a forced exponent.

### 2.2 The combined formula

Assembling: `y_e = (mult(A_{1g})² / mult(T_{1u})) · |1+i| · α^{N_c} = (16√2/3) · α^3`.

Numerically: `(16·1.4142/3)·(1/137.036)³ = 7.542 × 3.892×10⁻⁷ = 2.935×10⁻⁶` ≈ measured `y_e`.

---

## 3 · Honest scope and tag

### 3.1 Structural factors and the selected exponent

The multiplicities and the norm in `Z[i]` are theorem-grade structural objects. The α³ exponent is not: it inherits the selected ladder ordering and remains [SELECTION] under FTD-0397. This still sharpens the prefactor's structural vocabulary, but it does not derive the Yukawa power.

### 3.2 The combination as the Yukawa is [SELECTION]

What this work does NOT establish: that the SPECIFIC COMBINATION `mult(A_{1g})² / mult(T_{1u}) · |1+i| · α^{N_c}` IS the electron Yukawa coupling. We identify this product with `y_e` because the empirical match is 0.14%. Promoting the identification to [DERIVED] would require:

- A substrate-level computation showing that the Yukawa coupling formula necessarily has this form (specifically, that the matter-Higgs coupling vertex on the FTD lattice picks up factors `mult(A_{1g})²/mult(T_{1u})` from group-theoretic projection and `|1+i|` from Z[i]-arithmetic in the manifestation rule).
- Equivalently: closing MC-T4.3 (the central foundational obstruction) at least in a sector that includes the matter-Higgs coupling.

Until such derivation lands, the COMBINATION is [SELECTION]. The tag for the overall claim is **[STRUCTURALLY MOTIVATED PARAMETRIC]** — better than [SELECTION] (because the factor identifications are structural, not interpretive labels) but not [DERIVED] (because the combination isn't forced from substrate dynamics).

### 3.3 Cascade to other claims

**FTD-0015 (m_e formula)**: the prefactor `√(2π)·(16/3)` was [SELECTION] post-FTD-0133. With this work, we can re-read it as `√π·|1+i|·mult(A_{1g})²/mult(T_{1u})`, where:
- `√π` remains [SELECTION] (inherited from HIGGS-4 v formula)
- `|1+i| = √2` is now [THEOREM]
- `mult(A_{1g})²/mult(T_{1u}) = 16/3` is now [THEOREM]
- Combined: still [STRUCTURALLY MOTIVATED PARAMETRIC] for the m_e formula, with the [SELECTION] floor located at `√π` only (not the entire `√(2π)·(16/3)` prefactor).

**FTD-0131 (substrate Newton, gravitational hierarchy)**: the prediction `α_G(e,e) = 2π·(256/9)·α^{22}` factorizes as `2π · (mult(A_{1g})²/mult(T_{1u}))² · α^{2N_c+...}`. The 256/9 factor inherits the [STRUCTURALLY MOTIVATED PARAMETRIC] upgrade. The `2π` factor remains [SELECTION] (inherited from HIGGS-4 v² normalization). Net: gravity hierarchy stays [SMC] with the [SELECTION] floor located only at `2π`, not at the entire prefactor.

**FTD-0017 (Higgs mass m_H = (N_eff/α²)·m_e)**: doesn't depend on the 16/3 factor directly; unchanged.

**FTD-0133 (audit of √(2π)·(16/3) prefactor)**: partial closure. The `16/3` portion is now structurally identified at [THEOREM] level for each factor and [SMP] for the combination. The `√(2π)` portion remains as flagged in FTD-0133 (`√2 = |1+i|` now identified, but `√π` still [SELECTION]).

**Structural conjecture 4-fold catalogue (per `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md §6`)**: this work adds another entry — the appearance of `mult(A_{1g})² = 16` in the Yukawa coupling decomposition, parallel to its appearance as `N_base² = |Aut(E)|² = 4²` in the master quadratic coefficient. The "all 4s share a common origin" conjecture (currently [STRONGLY MOTIVATED CONJECTURE]) gains another co-occurrence at the same epistemic standing.

---

## 4 · What this is NOT

- **Not a derivation of the Yukawa formula from FTD substrate dynamics.** The COMBINATION of factors as `y_e` is [SELECTION].
- **Not a closure of the m_e formula to [DERIVED].** FTD-0015 stays [STRONGLY MOTIVATED CONJECTURE]; the load-bearing [SELECTION] floor is now `√π` (from HIGGS-4 v formula) plus the SELECTION-level identification of the combination.
- **Not a closure of FTD-0131's gravitational hierarchy to [DERIVED].** Same reason.
- **Not a new spine theorem.** The spine count is unchanged — nine numbered results (seven theorem-grade + two honestly-tiered — Theorem 3 at its arithmetic core only; see `SPEC_ALGEBRAIC_SPINE.md` §0 count convention).
- **Not a derivation of `√π`.** That factor is inherited from HIGGS-4's v formula and remains the load-bearing [SELECTION] piece in dimensional predictions.
- **Not a substrate explanation of why `y_e ∝ α^{N_c}` rather than `α^{N_base}` or `α^{D}`.** The exponent 3 is [SELECTION]: FTD-0397 shows the unordered multiset cannot select the required ladder difference without order-bearing dynamics.

---

## 5 · The next derivation step (what would close it to [DERIVED])

To upgrade the COMBINATION from [SMP] to [DERIVED], one path is:

**Compute the matter-Higgs Yukawa vertex on the FTD lattice.**

Specifically: write down the manifestation rule + Born-Infeld-action expansion around the SU(2)-doublet representation (per `DERIV_LATTICE_SU2_WEAK.md`), compute the 3-point vertex coupling matter (an electron-like cluster, FTD-0110) to the Higgs (the manifestation-threshold transition, per `DERIV_HIGGS_FROM_MANIFESTATION.md`), and verify that the projection picks up:

- `mult(A_{1g})²` from the A_{1g}-pure projection of both incoming and outgoing scalar legs (one on matter side, one on Higgs side; squared because two legs)
- `1/mult(T_{1u})` from averaging over the vector polarization states (3 channels)
- `|1+i|` from the (1+i)-tower normalization step that defines the Yukawa-vertex amplitude
- `α^{N_c}` from a separately derived order-bearing color-sector mechanism (not currently supplied; FTD-0397)

Each of these would be a substrate-level computation, not a structural assertion. Closing this would resolve the SM "fermion mass hierarchy puzzle" for the electron specifically and would constitute MC-T4.3-class progress.

**Effort estimate:** 5-15 sessions of focused work, with the most likely outcome being either (a) the computation succeeds and we get [DERIVED] for the electron Yukawa, or (b) the computation reveals a structural obstruction (most likely MC-T4.3's non-action mechanism requirement) and we honestly close-negative.

This is the natural P1 follow-up: take the structural decomposition filed here and turn it into a substrate computation.

---

## 6 · Single-line summary

**The proposed factorization `y_e = (mult(A_{1g})²/mult(T_{1u}))·|1+i|·α^{N_c}` uses theorem-grade multiplicities and Gaussian norm, but the α³ ladder difference and the identification of the product with the electron Yukawa are [SELECTION]. FTD-0397 proves that the unordered FTD-0084 multiset cannot select that difference through invariant data. Net tag: [STRUCTURALLY MOTIVATED PARAMETRIC]. Promotion requires a substrate-level vertex computation that independently supplies order-bearing dynamics.**

---

## 7 · Provenance

This work addresses the FTD-0015 prefactor question (P1 = "Close FTD-0015 prefactor"). The decomposition draws on the reformulation `m_e/v = (16/3)·α³` (Route E), following the seed observation that the prefactor admits an O_h character-theory treatment analogous to FTD-0110; the analysis verifies `mult(T_{1u}) = 3` independently from the standard character table. Each structural identification is honest at the per-factor level; the [SMP] tag for the combination is the calibrated honest position per CLAUDE.md F1/F9 discipline.

The [SMP] tag rather than [DERIVED] reflects that this is a **structural sharpening**, not a derivation. The multiplicity/norm factors are theorems; the α³ exponent and the combination are structurally evidenced selections.
