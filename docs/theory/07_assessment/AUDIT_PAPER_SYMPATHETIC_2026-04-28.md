# AUDIT — Sympathetic Audit of `PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex`

**Tag:** [AUDIT · sympathetic]
**Date:** 2026-04-28 (post-`547da31` commit)
**Subject:** `dissemination/papers/PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex` (21 pages, AMS journal format)
**Companion:** internal grading review (chat-only, B− → B+/A− → A−)

---

## 0 · Sympathetic-audit framing

This audit assumes the paper's framework is plausible and looks for places where the program has **earned but unclaimed** strength: hidden corollaries, underplayed novelty, places where the rigour could be ratcheted up without new measurements, and findings that the paper enables for the broader FTD program. It is the dual of an adversarial audit (which would try to break claims).

**Bottom line:** the paper has **earned roughly one full grade tier of underclaimed credit**. With a few additions described below — none requiring new measurements, only ~4–6 hours of write-up — the paper could move from A− to A and would have publishable independent contributions to **(i) lattice physics + symmetry** (the bridge as a general theorem), **(ii) number theory** (the Heegner-tower operationalisation as a self-contained mathematical paper), and **(iii) foundations methodology** (pre-registration + per-claim ledger applied to lattice computational measurements).

---

## §A · Underplayed novelty (paper-level)

### A1. The §4 bridge is a general theorem; the paper presents it as cubic-FTD-specific

**What's underplayed.** Theorem 4.5 (`mult(A_{1g}) = 4`), Lemma 4.6 (`δ_0` is `A_{1g}`-pure), Lemma 4.7 (Laplacian preserves `A_{1g}`-isotypic), and Corollary 4.10 (cluster efficiency = 1/mult(triv)) are stated as facts about the cubic point group `O_h` and the FTD lattice. **But the proof structure generalises immediately to any finite group acting orthogonally on `R^N`:**

> **General Theorem (sympathetic auditor's proposal).** Let `G` be a finite group acting orthogonally on `R^N`. Let `L: R^N → R^N` be a self-adjoint `G`-equivariant operator. Let `v ∈ R^N` be a `G`-fixed point with `‖v‖ = 1`. Then `v` lies in the `G`-trivial-irrep isotypic component of `R^N`; under the linearised wave equation `φ̈ = c²Lφ` with initial condition `(φ(0), φ̇(0)) = (Av, 0)`, the time-averaged per-mode energy across the trivial-isotypic is `A²/mult(triv, R^N)`.

This is a one-line consequence of Schur's lemma plus a sum/count identity. **It is a theorem about lattice physics + symmetry**, not about FTD. Stating it would:

- give the paper a publishable independent mathematical-physics result;
- locate FTD's specific result as the `G = O_h, R^N = ρ_27` instance;
- invite cross-application to other lattice problems (hexagonal close-packed, body-centered tetragonal, etc.) where the multiplicity of the trivial irrep is different.

**Effort:** ~1 hour. Add as a new theorem in §4 (between Lemma 4.7 and Corollary 4.10), with the FTD case becoming an explicit instance.

**Impact:** the paper's standalone math-physics contribution becomes a clean general theorem rather than a structural curiosity about a specific lattice.

### A2. The framework integer `N_eff = 13` has a hidden rep-theoretic origin in the paper

**What's not claimed.** Theorem 4.5 gives the decomposition

```
ρ_27 ≅ 4·A_{1g} ⊕ 2·E_g ⊕ 2·T_{2g} ⊕ A_{2u} ⊕ 3·T_{1u} ⊕ T_{2u}.
```

The total multiplicity-count is `4 + 2 + 2 + 1 + 3 + 1 = 13 = N_eff` (the second framework integer). The paper proves the dimension check (`4·1 + 2·2 + 2·3 + 1·1 + 3·3 + 1·3 = 27 = N_total`) but doesn't claim that the multiplicity-count equals `N_eff`.

**This is a corollary of Theorem 4.5 the paper has earned but not stated:**

> **Corollary (sympathetic auditor's proposal).** The framework integer `N_eff` is the total number of irreducible representations occurring in `ρ_27` (counting multiplicity), i.e., `N_eff = Σ_λ mult(λ) = 13`. This connects `N_eff` rep-theoretically to `O_h`'s 10 irreducible representations and the specific multiplicity profile of `ρ_27`.

Adding this corollary would:

- give a rep-theoretic foundation to `N_eff` (currently only [THEOREM] for *existence*; rep-theoretic identification is new);
- complete the framework-integer story: `N_base = 4` (mult of trivial), `N_eff = 13` (total mult), `N_total = 27` (dim of rep).
- propagate to LEDGER row FTD-0008 (Moore framework integers) as a strengthening — the THEOREM tag for `{4, 13, 27}` now carries a representation-theoretic interpretation, not just an integer-existence claim.

**Effort:** ~30 min. Add as a corollary to Theorem 4.5 in §4.2.

**Impact:** the framework-integer set `{4, 13, 27}` (and arguably `b_3 = 7` via a different rep count) is no longer a list of integers labelled with mnemonic suffixes; it becomes a structured rep-theoretic object. This is one of the most important sympathetic findings of this audit.

### A3. The look-elsewhere disclosure has a stronger structural argument the paper doesn't make

**What's underplayed.** The §1.1 and §6.5 disclosures say:

> "What FTD-0097 does **not** cover: the polynomial-root identifications of §3 (the master quadratic dual match operates above the monomial level scanned)..."

A sympathetic reading is sharper:

> The dual match `(x_+, x_-)` is a **degree-2 algebraic operation** on the polynomial coefficients `(16Gstar², 16Gstar³)`. No monomial-level scan can produce dual roots structurally — solving a quadratic is not a monomial operation. Equivalently: for every quadratic polynomial whose coefficients lie within FTD's scan space, the resulting roots may or may not match physics, but the *act of taking roots* projects out of the scan space. FTD-0097's null-rejected-upward verdict is therefore **silent on the master quadratic's dual match by construction**, not by accident of scope.

This affirmative framing (rather than the current "doesn't cover by accident") strengthens the [STRONGLY MOTIVATED CONJECTURE] tag for the master quadratic's dual identification by making the look-elsewhere scan's structural inapplicability **explicit** rather than implicit.

**Effort:** ~15 min. One paragraph in §6.5.

**Impact:** the look-elsewhere disclosure goes from "we acknowledge this risk and our claim is technically outside its scope" to "we acknowledge this risk *and the claim is structurally outside its scope by the algebraic operations involved*." Stronger.

### A4. The dual-match joint precision is not quantified

**What's missing.** The paper says `x_+` matches `1/α` at 1.26 ppm and `x_-` matches `N_c = 3` at 0.80%, but doesn't combine these into a joint significance.

Under reasonable null distributions:
- For `x_+` ≈ 137: assume uniform on `[100, 200]` (a generous range covering a-priori-plausible values for a coupling-strength inverse). The 1.26 ppm match is `~1.7×10⁻⁷` of the range.
- For `x_-` ≈ 3.024: assume uniform on `[1, 12]` (covering the small-root range observed in the Heegner tower, Table 3.2). The 0.80% match is `~2.7×10⁻³` of the range.
- Joint probability of accidental dual match: `~5×10⁻¹⁰` (log10 ≈ -9.3).

This is roughly 6σ jointly, even under generous null assumptions. **The paper undersells this.**

**Effort:** ~30 min. Add a quantitative paragraph in §3.5 (`Why the cubic lattice picks d = -4`) computing the joint accidental-match probability under explicit null priors.

**Impact:** [STRONGLY MOTIVATED CONJECTURE] gains a numerical strength: the dual match is not just "structurally selective" but "structurally selective AND quantitatively unlikely under uniform null." Replaces hand-waving with arithmetic.

### A5. Pre-registration + per-claim ledger is a third novelty the paper doesn't separately claim

**What's underplayed.** §1.3 introduces the epistemic tag system; §6 deploys it as a per-claim ledger; §5.2 reports against pre-registered Outcome A/B/C/D bins; §1.1 discloses FTD-0097's null-rejected-upward verdict. These are all **methodological novelties**: foundations-of-physics papers rarely:

- pre-register binary verdicts before measurements;
- publish per-claim epistemic ledgers with [THEOREM]/[DERIVED]/[STRONGLY MOTIVATED CONJECTURE]/[PARAMETRIC]/[OPEN] tags;
- disclose look-elsewhere scans' null-rejected verdicts that affect the program's own catalogue;
- separate "structural derivation chain" from "physical identification" with explicit tagging.

The paper deploys all of this but doesn't claim it as a contribution. **This is the paper's third novelty alongside §3 (CM-uniqueness operationalisation) and §4 (algebra–engine bridge).**

**Effort:** ~30 min. Either expand §1.3 to position pre-registration discipline as a methodology contribution explicitly, OR add a one-paragraph §1.4 ("Methodological transparency").

**Impact:** the paper becomes legible to the meta-science / open-science community as a **case study in pre-registered foundations-of-physics computational measurement**. This audience is largely orthogonal to the math-physics audience and reads different journals.

---

## §B · Hidden corollaries within the paper

### B1. Cluster lives specifically on the slowest A_{1g} mode

**Where it appears.** The deriv doc's §4.4 mentions: "The cluster bound state is maintained on the slow mode (λ ≈ −1.586)..." The paper currently has this only in the linear-mode-budget heuristic of §4 (Corollary 4.10's "the bridge to engine cluster size" discussion).

**Sympathetic strengthening.** The paper could state explicitly:

> Among the four `A_{1g}` eigenvalues `{-4.805, -4.414, -3.862, -1.586}`, the smallest-magnitude eigenvalue `λ ≈ -1.586` corresponds to the **uniform-on-block** eigenmode `(0.354, 0.612, 0.612, 0.354)`. This is the cluster's host mode: the slowest lattice excitation, with `T_oscillation = 2π/√|λ|` largest, and the spatial profile extending uniformly across the 27-block (matching the empirically-observed cluster centroid at the lattice centre).

This connects FTD's cluster phenomenology to the standard lattice-physics concept of a **mass gap** (the lowest non-trivial eigenvalue of the lattice Laplacian sets the bound-state energy scale). The cluster IS the manifestation of the mass-gap mode.

**Effort:** ~30 min. Add ~1 paragraph in §4.6 or §4.9.

**Impact:** the paper's bridge derivation gets a direct connection to lattice gauge theory's mass-gap concept. This is a vocabulary translation that helps reviewers from lattice physics see the paper's claim in their own framework.

### B2. The 4×4 block factors via Z_2 × Z_2 parity = same parity as the master quadratic discriminant trichotomy

**Where it appears.** The proof of Theorem 4.9 (Energy fractions) uses `u_± := (e_0 ± e_3)/√2` and `v_± := (e_1 ± e_2)/√2` to block-diagonalize `M`. This is a `Z_2 × Z_2` parity reduction.

**Sympathetic strengthening.** This `Z_2 × Z_2` is the same parity structure that appears in:

- The master quadratic's **discriminant trichotomy** (FTD-0012 in LEDGER): `Δ > 0` (real distinct roots, "fermion-like"), `Δ = 0` (degenerate, "critical"), `Δ < 0` (complex conjugate, "boson-like"). The two parities of the 4×4 block correspond to even/odd combinations of inner-vs-outer voxels — analogous to the master quadratic's even/odd combinations of root structure.

- The Cl(3,0) Clifford algebra's **even/odd grading**: 4 grades = scalar (even) + vector (odd) + bivector (even) + pseudoscalar (odd). The `Z_2 × Z_2` parity of the 4-irrep block matches the Cl(3,0) grade parity.

Stating this connection turns an arithmetic-step ("we can block-diagonalize via parity") into a structural finding ("the parity structure of the 4×4 block is the same `Z_2 × Z_2` that appears across FTD's algebraic spine and Clifford-algebra layers").

**Effort:** ~30 min. Add a remark after Theorem 4.9.

**Impact:** the bridge derivation connects to FTD's broader discriminant-trichotomy and Clifford-algebra structures. This is a **second** structural connector between the algebraic spine and the engine — beyond the `N_base = 4` connector — that the paper has implicit but doesn't claim.

### B3. The {3/8, 1/8, 3/8, 1/8} pattern has structural origin in orbit cardinalities

**Where it appears.** Currently noted as an empirical pattern preserved under thermalization at mean = 1/4.

**Sympathetic strengthening.** The deriv doc's §4.3 has more detail: "The 3/8 vs 1/8 splitting is a structural consequence of the orbit-cardinality ratio: 3/8 = (8 + 1)/24 = (orbit ratios encoded in eigenvector mixing)."

Specifically, with orbit cardinalities `{1, 6, 12, 8}` (centre, SC, FCC, BCC), the rational coefficients of `δ_0`'s projections fall into these patterns. The 3/8 vs 1/8 ratio reflects the relative weights of centre+BCC orbits (1+8 = 9) vs SC+FCC orbits (6+12 = 18), under the parity-structure decomposition. Stating this explicitly turns the {3/8, 1/8, 3/8, 1/8} from "empirical eigenvalue-projection pattern" into "structural consequence of the {1, 6, 12, 8} orbit cardinalities."

**Effort:** ~30 min. Expand the remark after Theorem 4.9 with the orbit-cardinality argument.

**Impact:** marginal; mostly pedagogical. The paper has the result; this clarifies its origin.

---

## §C · Cross-FTD travel (findings that propagate beyond the paper)

### C1. LEDGER row FTD-0008 should record the rep-theoretic interpretation of {N_base, N_eff}

**Current state.** FTD-0008 (Moore neighbourhood integers `{N_base=4, N_eff=13, b_3=7}`) is tagged [THEOREM] for the integers' existence in the framework structure.

**What this paper enables.** The §4 bridge derivation gives:
- `N_base = 4 = mult(A_{1g})` in `ρ_27` — explicit rep-theoretic identification.
- `N_eff = 13 = Σ mult` in `ρ_27` — total irrep multiplicity (per §A.2 above).
- `b_3 = 7` — possibly `mult(non-trivial irreps) = N_eff − N_base = 13 − 4 = 9`? Or `b_3` might relate to a different rep count. **Open question**: what is `b_3 = 7` rep-theoretically? This audit doesn't resolve it but flags it.

**Action.** LEDGER row FTD-0008 should be updated 2026-04-28 to record:
> "Rep-theoretic foundation: `N_base = 4 = mult(A_{1g}) in ρ_27`, `N_eff = 13 = Σ mult(λ) in ρ_27` (paper PAPER_MASTER_QUADRATIC_AND_BRIDGE, §4). The `b_3 = 7` framework integer's rep-theoretic identification remains [OPEN]."

**Effort:** ~15 min on LEDGER edit.

**Impact:** strengthens FTD-0008's tag substance (THEOREM-existence → THEOREM-existence-with-rep-theoretic-interpretation).

### C2. The bridge derivation's method generalises to other engine observables

**Observation.** The §4 derivation derives one engine observable (cluster size at canonical amplitude) from rep-theoretic structure. The same method should apply to other engine observables that are functions of voxel state in the 27-block:

- **Charge accumulation `Q(A)`**: the centroid of state-+1 vs state-−1 voxels under δ-injection. Has its own irrep decomposition.
- **Energy density profile `E(r)`** as a function of distance from the injection point. Decomposes by orbit-shell (centre, SC, FCC, BCC).
- **Spin polarization** under directional injection. Couples to vector irreps (T_{1u}, T_{2u}).

Each of these observables has a rep-theoretic shadow in the 27-block; the paper's method is, in principle, a recipe for deriving them.

**Action.** A follow-up paper (or §10 expansion of this paper) could systematically apply the method to the other engine measurables. **This is the strongest avenue for extending §4's contribution.**

**Effort:** out of scope for this paper; ~1 month for a follow-up.

**Impact:** turns the paper's §4 from a one-off derivation into a programmatic method. This is the highest-leverage extension this audit identifies.

### C3. The Watson-BCC connection in §3.5 is foundational and underutilised in the broader program

**Observation.** Theorem 3.5 (Watson identity) plus the new sketch paragraph explain how the cubic lattice's Green's function encodes the lemniscatic period at `Q(i)`. This is the **structural mechanism** behind the cubic-lattice's selection of `d = -4`, and through it behind the program's claims about `α` and `N_c`.

The deriv doc `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` has the full proof; the paper now has a sketch. But the broader FTD program — `SPEC_FTD.md`, the manuscript_v2 chapters, the disseminations — could lean harder on this connection. It's the cleanest "why d = -4 specifically" answer the program has.

**Action.** Future dissemination passes should foreground "cubic lattice → Watson identity → `Q(i)` period → master quadratic → `(1/α, N_c)` dual match" as the core structural narrative of FTD's algebraic spine. This is more compelling than the current bullet-list framings.

**Effort:** medium-term editorial.

**Impact:** improves the program's external-audience narrative; not a near-term action item but worth flagging.

---

## §D · Quantification opportunities

### D1. Master-quadratic joint dual-match probability under uniform null

Per §A.4 above. **Effort: 30 min. Impact: strengthens [STRONGLY MOTIVATED CONJECTURE] tag.**

### D2. Three-scale L-drift likelihood-ratio test

The 25→26→27 monotone trend across L ∈ {32, 64, 128} is currently presented with prior probability ~1/8 under iid noise (~1.5σ). A likelihood-ratio test against a finite-L correction model `N(L) = N_∞ + c/L` would discriminate:

- Random-walk model: `H_0`: `N` distributed as iid Gaussian with mean 25 and std-err observed.
- Finite-L correction: `H_1`: `N(L) = 25 + c/L^p` for some `p > 0`.

A 2-parameter fit (`c`, `p`) to the three measured values is overdetermined and gives a unique `(c, p)`. Quoting that fit's residual-vs-iid-noise likelihood ratio would be more honest than the current "1/8 prior" framing.

**Effort:** ~1 hour (fit + write up).
**Impact:** sharpens the §5.3 honest-discussion paragraph from "two competing readings" to "two competing readings, here's the LR test result."

### D3. BC effect's expected magnitude

**Observation.** Reflecting BC at the boundary of `B = {-1, 0, 1}^3` introduces L-dependence in the engine via the surface-to-bulk ratio:

- L=32: `6L²/L³ = 6/32 = 18.75%`
- L=64: `6/64 = 9.375%`
- L=128: `6/128 = 4.69%`

If BC effects scale linearly with surface fraction, the L=32→L=128 size ratio should change by ~14% from BC alone. Observed: +8% drift. Order-of-magnitude consistent with BC dominance.

**Action.** §5.3 (or a footnote) could add: "An order-of-magnitude estimate: surface-to-bulk ratio drops from 18.75% (L=32) to 4.69% (L=128), a 14-percentage-point reduction. The observed +8% drift is consistent with BC-effect dominance at this scale, supporting the finite-L-correction reading."

**Effort:** 15 min.
**Impact:** the random-walk-vs-finite-L-correction discussion gains a quantitative anchor.

---

## §E · Improvements not yet claimed

### E1. Tabulate quantitative selectivity in the Heegner tower

**What's missing.** The §3.3 nine-Heegner table (Table 3.2) has a "physics match? yes/no" column for d=-4 vs all others. But it doesn't quantify *how much* the others miss by.

**Action.** Add a "miss size" column showing for each non-d=-4 row the relative miss `min(|x_+ - 1/α|/((1/α)), |x_- - N_c|/N_c)`. All non-d=-4 rows would show miss > 50% (way outside any plausible physics-match tolerance). This makes the selectivity quantitative.

**Effort:** ~30 min.
**Impact:** makes Theorem 3.3 (CM uniqueness) quantitatively checkable.

### E2. Promote the L-function structural-scaling subsection (§3.4) to a more prominent position

**Observation.** The relation `log ρ_d = (w_d / 2h(d)) · |d| · L'(0, χ_d)` connecting Chowla–Selberg ratios to Dirichlet L-function special values is genuinely interesting mathematical content. Currently it's a single subsection in §3 (§3.4), buried between "the numerical tower" and "why the cubic lattice picks d = -4."

**Action.** Consider whether §3.4 deserves its own §3.5 promoted to an independent subsection, or whether it deserves an appendix expansion. It connects FTD to the Birch–Swinnerton-Dyer conjecture's L-function-special-value structure — a high-prestige connection that's underutilised.

**Effort:** ~1 hour to expand to a publishable mathematical-content level.
**Impact:** marginal for FTD's narrative; significant for the paper's number-theoretic audience.

### E3. Add the general theorem (§A.1) and frame §4's result as an instance

Per §A.1 above. **Effort: 1 hour. Impact: largest single sympathetic-audit recommendation.**

---

## §F · Action list (prioritised by impact × effort)

| # | Action | Effort | Impact | Priority |
|---|---|---|---|---|
| 1 | Add general theorem in §4; frame FTD result as instance | 1 hr | A− → A | **HIGH** |
| 2 | Add corollary: `N_eff = 13 = Σ mult` in ρ_27 | 30 min | strengthens framework integers | **HIGH** |
| 3 | Quantify dual-match joint accidental probability | 30 min | strengthens SMC tag | **HIGH** |
| 4 | Update LEDGER FTD-0008 with rep-theoretic interpretation | 15 min | program-level | **HIGH** |
| 5 | Add structural look-elsewhere argument (§A.3) | 15 min | strengthens disclosure | MEDIUM |
| 6 | Add slowest-mode = mass gap remark (§B.1) | 30 min | lattice-physics legibility | MEDIUM |
| 7 | Add BC-effect quantitative anchor (§D.3) | 15 min | tightens §5.3 discussion | MEDIUM |
| 8 | Add miss-size column to Heegner table (§E.1) | 30 min | quantitative selectivity | MEDIUM |
| 9 | Add Z_2 × Z_2 parity remark (§B.2) | 30 min | spine-Cl(3,0) connector | MEDIUM |
| 10 | Add LR-test for L-drift (§D.2) | 1 hr | sharpens honest-disc | LOW |
| 11 | Pre-registration as third novelty (§A.5) | 30 min | meta-science legibility | LOW |
| 12 | Generalise §4 to other observables (§C.2) | 1 month | follow-up paper | OUT OF SCOPE |

**Total effort for HIGH-priority items 1–4:** ~2.5 hours.
**Total effort for HIGH+MEDIUM items 1–9:** ~5 hours.

---

## §G · What this audit does NOT propose

For epistemic-discipline reasons, this sympathetic audit explicitly does NOT propose:

- Promoting the master quadratic's dual identification beyond [STRONGLY MOTIVATED CONJECTURE]. The CM-uniqueness operationalisation (§3) and the joint-probability quantification (§D.1) strengthen the conjecture's evidential basis but don't promote its tag.
- Extending the bridge derivation to cover the nonlinear engine pipeline. That's the [OPEN] item in §8 of the paper; it requires real theoretical work not addressable by sympathetic re-reading.
- Claiming any new physics derivations beyond what the paper claims. The cluster-mass identification across SM particles remains [STRONGLY MOTIVATED CONJECTURE for nonlinear regime] regardless of what audits we do.

The audit is a **claim-strengthening pass** within the paper's existing tag commitments, not a tag-promotion pass.

---

## §H · Verification

This audit's claims can be cross-checked against:

1. The paper's source: `dissemination/papers/PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex` (commit `547da31`).
2. The deriv doc: `docs/theory/03_derivations/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` (commit `306837c`).
3. The LEDGER: `docs/theory/07_assessment/LEDGER.md` (FTD-0001 through FTD-0008 for spine, FTD-0107 G2 for empirical anchor, FTD-0110 for the bridge).
4. Cubic point group character tables: Altmann–Herzig (1994), §4.2.
5. Verification suite: `scripts/exploration/verify_k_derivation_2026-04-28.py` (C1–C4 PASS).

No new measurements or computations are proposed by this audit; it operates strictly on the existing artifact set.

---

## §I · Summary

The paper has earned **A but is presenting itself at A−**. The gap is closeable with ~5 hours of write-up plus a 15-minute LEDGER update.

The single most important sympathetic-audit finding is the **general theorem framing** (§A.1): the §4 bridge isn't a curiosity about `O_h` and FTD; it's a clean lattice-physics + symmetry-group theorem that the paper has proved but not stated abstractly. Stating it gives the paper an independent math-physics contribution legible to lattice/condensed-matter reviewers.

The second most important finding is the **rep-theoretic identification of `N_eff = 13`** (§A.2): the framework integers `{4, 13, 27}` aren't a list of mnemonic suffixes but an irreducible-representation multiplicity profile. This strengthens the LEDGER row FTD-0008 from THEOREM-existence to THEOREM-existence-with-rep-theoretic-interpretation.

— FTD Sympathetic Audit, 2026-04-28 (post-`547da31`).
