# AUDIT — FTD-0110 nonlinear-bridge "resolution" (2026-05-27 §6 theorems)

**Tag:** [P4 ADVERSARIAL AUDIT] — challenges a recent tag promotion.
**Date:** 2026-05-27
**Subject:** the 2026-05-27 retag of FTD-0110 to `[DERIVED]` at nonlinear pipeline level via the "Orbit-Equipartition Theorem" and "Timescale Separation Theorem" introduced in [`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md) §6.
**Recommended outcome:** **revert the 2026-05-27 LEDGER update**; restore the 2026-05-04 honest position (bridge `[OPEN]` after Option A falsification, `[SMC]` for the cluster-size formula, `[DERIVED]` retained for the linear-level theorem and Bridge-I global O_h-equivariance only). Queue Mechanism α perturbation calculation as the actual closure work, per the 2026-05-23 scoping memo (FTD-0203).

---

## 0 · What this audit does NOT challenge

The following claims are NOT challenged and remain `[DERIVED]` / `[THEOREM]`:

- **Linear-level theorem** in [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md): under δ-localised injection at the O_h-fixed centre voxel, the mean energy fraction across the 4 A_{1g} eigenmodes of the 18-point Laplacian on the 27-block equals $1/N_\text{base} = 1/4$ exactly. **Solid `[THEOREM]`.**
- **Bridge-I global O_h-equivariance** ([`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md) §§2.1–2.7): every pipeline step ($T_1$ wave, $T_2$ genesis-in-expectation, $T_3$ Langevin, $T_4$ Gauss projection, $T_5$ evaporation, $T_6$ back-reaction) commutes with $\rho_{27} \otimes T_{1u}$ in expectation, so the time-averaged flux $\langle \phi \rangle$ remains A_{1g}-isotypic globally. **Solid `[DERIVED]`.**
- **Empirical 5% multi-dimension match** (11 amplitudes × 5 SM particles × 3 lattice scales × 2 injection geometries). **Solid empirical fact.** Audit does not touch.

What this audit challenges is the *upgrade from those pieces to a full nonlinear-pipeline `[DERIVED]` for $k = 1/4$* via the §6 theorems.

---

## 1 · The four §6 defects

### 1.1 · Arithmetic error at the load-bearing step of "Orbit-Equipartition"

§6.1 Step 3 derives the cluster-size formula:

$$ \langle N(A) \rangle \approx \sum_{i=1}^4 |\mathcal{O}_i| \cdot \frac{\langle E_i \rangle}{|\mathcal{O}_i| \, K_\text{GENESIS}^2} \;=\; \sum_{i=1}^4 \frac{A^2}{4} \;=\; \frac{A^2}{4} $$

Reading the middle expression: $|\mathcal{O}_i|$ cancels, leaving $\langle E_i \rangle / K^2 = A^2/4$ per term (since equipartition gives $\langle E_i \rangle = A^2 K^2 / 4$). The sum is four copies of $A^2/4$:

$$ \sum_{i=1}^4 \frac{A^2}{4} \;=\; 4 \cdot \frac{A^2}{4} \;=\; A^2 $$

The right-hand side claims $A^2/4$, dropping the factor of 4 from the sum. **The conclusion does not follow from the line preceding it.** Either the sum is supposed to be bounded by counting (e.g., only one orbit's contribution survives, in which case the equipartition argument is not the mechanism), or the formula yields $A^2$ (which doesn't match the empirical or linear-level prediction), or there is a missing factor of $1/N_\text{orbit}^2$ upstream. The proof as written has an arithmetic error at the step that delivers the $1/4$.

### 1.2 · Conceptual error: single-block analysis applied to a multi-scale phenomenon

Even if the arithmetic of §6.1 were corrected, the proof's underlying analysis applies to a single 27-block, not to the multi-block clusters the formula is supposed to describe. Run §6.1 honestly at typical amplitudes:

| $A$ | $A^2$ | Manifestation thresholds $A^2 \geq 4 \mid \mathcal{O}_i \mid$ | Predicted $N$ |
|---:|---:|---|---:|
| 10 | 100 | $\{4, 24, 48, 32\}$ all satisfied → all 4 orbits manifest | $1 + 6 + 12 + 8 = 27$ (saturated) |
| 20 | 400 | all satisfied | 27 |
| 50 | 2500 | all satisfied | 27 |
| 100 | 10000 | all satisfied | 27 |

The single-block orbit-equipartition prediction is **saturation at 27** for $A \geq \sqrt{48} \approx 6.93$ — a step function, not an $A^2$ scaling. The empirical scaling $N(A) \approx A^2/4$ comes from the cluster extending across many 27-blocks (at $A=20$, $N \approx 100$ voxels = a $5^3 = 125$-voxel ball; at $A=50$, $N \approx 625$ = an $\approx 8^3$ ball). This multi-scale extension is exactly what §3.2 of the same document explicitly leaves open:

> "Status: Route A is structurally sound but has a quantitative boundary correction that's not yet computed. Closing it rigorously requires: 1. Translation-invariance of L_18 formal proof in the bulk lattice [trivial, ~30 min]. 2. Boundary-correction estimate via discrete-PDE tools [~1 week, lattice-physics standard]."
> 
> ([`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md) §3.2 Route A)

The §6.1 hand-wave "averaged over the ensemble of orbits (or in the continuous multi-scale cluster limit), the sum evaluates to" papers over the gap that §3.2 explicitly names. **The single-block equipartition argument does not produce the $A^2/4$ scaling at all** — it produces saturation at 27.

### 1.3 · §6.2 "Timescale Separation Theorem" is a phenomenological fit, not a derivation

§6.2 Step 1 posits the exponential-decay model

$$ \frac{df_{A_{1g}}}{dt} = -\gamma_\text{mix} (f_{A_{1g}}(t) - 4/27) $$

as a *given*, then reads off $\gamma_\text{mix} = 1/100$ and $\tau_\text{form} = 10$ from the empirical §5.1 measurement (which shows $f_{A_{1g}}$ relaxing from 1.0 to $\approx 4/27$ over $\sim 100$ ticks under `gauss_projection`). The exponential ansatz is not derived from the engine pipeline; the timescales are fit parameters. Step 3's "rigorous bound $f_{A_{1g}}(\tau_\text{form}) \geq 0.92$" is a parametric calculation against fit parameters, not a theorem-grade statement.

Calling this a "theorem" and using it to upgrade FTD-0110 to `[DERIVED]` is the F10 failure mode (per `gtca/references/failure-modes.md`): correctly applying a label does not resolve the underlying question the label only names. The 2026-05-04 empirical falsification of local A_{1g} preservation was honest; covering it with a phenomenological exponential and calling the cover a theorem is not.

### 1.4 · The §6 theorems would predict pure $1/4$ — contradicting empirical log-A drift

The empirical cluster-efficiency $k(A)$ from [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) §0:

| $A$ | 2 | 10 | 15 | 20 | 28.77 | 30 | 33.05 | 50 | 62.42 | 85.70 | 117.93 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| $k_\text{eng}$ | 0.250 | 0.252 | 0.224 | 0.234 | 0.253 | 0.262 | 0.245 | 0.222 | 0.224 | 0.212 | 0.206 |

Empirical fit: $k(A) \approx \tfrac{1}{4}\bigl(1 - 0.030\,\ln(A/2)\bigr)$ — a **logarithmic drift** of $\sim 18\%$ across two decades of $A$. The §6 theorems (if they worked) would predict $k = 1/4$ exactly, no drift, no $A$-dependence. They are therefore not only internally broken but also **empirically inadequate**: they would predict the wrong functional form for the empirical scaling they are claimed to explain.

The HONEST mechanism, identified in [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) §2.1 (Mechanism α, multi-block irrep mixing), gives a $\sum_{r=1}^{R(A)} 1/r \sim \ln R \sim (2/3) \ln A$ correction — structurally matching the empirical log-A signature. This is the route that needs the actual ~1-week perturbation calculation.

### 1.5 · Eigenmode/orbit conflation

The linear theorem ([`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) §4.4) derives $k = 1/4$ via the **mean energy fraction across the 4 $A_{1g}$ eigenmodes of $L_{18}$ restricted to $V_{A_{1g}}$**. The §6.1 "Orbit-Equipartition Theorem" invokes equipartition across the **4 $O_h$ orbits in the 27-block** (centre / SC face / FCC edge / BCC corner). These are different physical objects: the four orbit-averaged scalar fields span $V_{A_{1g}}$ as a basis, but the eigenmodes of $L_{18}$ on $V_{A_{1g}}$ are *linear combinations* of those orbit-sums with eigenvalue-dependent coefficients (the explicit 4×4 diagonalization in `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` §3.1). Equipartition over orbits is not the same statement as equipartition over eigenmodes, and the §6 derivation does not reduce one to the other.

---

## 2 · What the verification scripts actually verify

[`scripts/proofs/proof_ftd0110_full_aggregation.py`](../../../scripts/proofs/proof_ftd0110_full_aggregation.py) and [`scripts/proofs/proof_ftd0110_active_partition.py`](../../../scripts/proofs/proof_ftd0110_active_partition.py) — referenced in some §6 cross-checks — are **not proofs of the §6 theorems**. Their headers state explicitly:

- *"Phase B falsified the naive 'f_slow per block = 1/√d' hypothesis. This script tests the NEXT candidate framework based on Phase B's actual findings: LANGEVIN EQUIPARTITION FRAMEWORK..."* (full_aggregation.py)
- *"This script formally verifies the active-block partitioning aggregation rule (AP-no-over-count) designed to resolve the multi-block over-counting bug in FTD-0110."* (active_partition.py)

They explore independent multi-scale aggregation mechanisms ($\eta(x) = \text{block-energy}(x) \cdot 1/d_G(x) / E_\text{total}$ where $d_G$ is the trivial-irrep dimension of the *local* point-group symmetry of each off-centre voxel — $\{O_h: 4, C_{4v}: 9, C_{3v}: 10, C_{2v}: 12, C_s: 18, C_1: 27\}$). They cite `EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` and `LEDGER FTD-0119`, not the §6 theorems. **No verification artifact exists for the §6 theorems.**

---

## 3 · The honest path forward

The 2026-05-01 [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) and the 2026-05-23 scoping memo [`SCOPE_FTD_0110_NONLINEAR_BRIDGE.md`](../10_eft_program/SCOPE_FTD_0110_NONLINEAR_BRIDGE.md) already laid out the actual closure path:

- **Mechanism α (multi-block irrep mixing)**, ~1 week of careful calculation. Compute the per-shell A_{1g} → non-A_{1g} leakage of the lattice Green's function $G_L(r)$ from the central injection projected onto the 4 A_{1g} eigenvectors of each off-centre 27-block. Sum over shells; the predicted log-A coefficient is the dominant-mechanism test. Status (per scoping memo): session-scoped desk track. **This is the real Phase 1 work for FTD-0110.**
- **Mechanism β (genesis-induced irrep mixing)** and **Mechanism γ (Langevin non-equipartition)**, 3-5 days each — bound the other two contributions.
- **D3a–D3d engine sweeps**, ~2 weeks GPU wall-time on WSL2/CUDA — only if the desk tracks don't land cleanly.

Closure would require **pre-registration** of the perturbation calculation (per PREREG template precedents), with hash-locked falsifier rules; the F1/F10 pattern is otherwise too easy to repeat.

---

## 4 · Recommended LEDGER tag movement

The 2026-05-04 entry in LEDGER row FTD-0110 is the canonically-correct honest position:

> "what's lost is the [DERIVED] tag for the nonlinear-pipeline coefficient origin"
> "§2's argument is correct as a global statement but does not prove the local-block invariance Bridge-II §3.1 actually needs"

**Recommended action: revert the 2026-05-27 LEDGER FTD-0110 row update** and restore the 2026-05-04 honest position.

| Claim | Pre-2026-05-27 | Post-2026-05-27 (current) | Recommended |
|---|---|---|---|
| Linear-level $k = 1/4$ via A_{1g} eigenmode mean | `[DERIVED]` (linear) | `[DERIVED]` | `[DERIVED]` (unchanged) |
| Bridge-I global O_h-equivariance | `[DERIVED]` | `[DERIVED]` | `[DERIVED]` (unchanged) |
| Local 27-block A_{1g} preservation under full pipeline | `[FALSIFIED]` empirically (2026-05-04) | `[RESOLVED via Timescale Separation]` | **revert to `[FALSIFIED]`** |
| Bridge-II single-block linear-budget argument | `[CONDITIONAL]` after 2026-05-04 | `[DERIVED via Orbit-Equipartition & Timescale Separation]` | **revert to `[CONDITIONAL]` / `[OPEN]`** |
| Nonlinear-pipeline $k = 1/4$ coefficient origin | `[OPEN]` after 2026-05-04 | `[DERIVED]` | **revert to `[OPEN]` (`[SMC]` supported by 5% empirical match)** |
| Multi-scale extension across cluster spatial extent | `[SMC]` / `[OPEN]` for analytical closure | implicit `[DERIVED]` via §6 hand-wave | **`[SMC]` / `[OPEN]`, three candidate mechanisms identified** |
| Cluster-mass identification across SM particles | `[SMC]` | `[SMC]` | `[SMC]` (unchanged) |

Also propagate to: `TRACKER_ONTIC_TRUTH.md` OT-3.4 — the ★★★ rating cites the two §6 theorems; with this audit, OT-3.4 likely drops from "bedrock" until the actual closure lands.

---

## 5 · Falsifier list for this audit

This audit itself is `[ADVERSARIAL CLAIM]` and is falsifiable. The audit fails if any of the following lands:

- **F-a (arithmetic).** Someone shows the §6.1 step-3 sum genuinely evaluates to $A^2/4$ (not $A^2$) under a reading I missed. The arithmetic must close, not be parameterized away.
- **F-b (scale).** Someone derives $N(A) \propto A^2$ from the single-block orbit-equipartition without invoking the §3.2 multi-scale extension that the same document marks `[OPEN]`. The derivation must produce $A^2$ scaling, not the saturation-at-27 step function the single-block analysis actually predicts.
- **F-c (phenomenology).** Someone derives the §6.2 exponential decay $df/dt = -\gamma_\text{mix}(f - 4/27)$ and the timescales $\gamma_\text{mix} = 1/100$, $\tau_\text{form} = 10$ from the engine pipeline rather than fitting them to the §5.1 empirical curve. Derivation, not fit.
- **F-d (empirical adequacy).** Someone shows the §6 theorems predict log-A drift with slope $-0.030/\!\ln(\text{A/2})$ rather than pure $k = 1/4$. Match the data's functional form, not just the leading term.
- **F-e (eigenmode/orbit).** Someone shows the orbit-equipartition statement is equivalent to the linear theorem's eigenmode-equipartition statement under the relevant pipeline dynamics. Reduce one to the other, with proof, not assertion.
- **F-f (verification).** Someone produces a numerical verification artifact that tests the §6 theorems specifically (not the §3.2 Route A or the candidate aggregation mechanisms in `proof_ftd0110_*.py`).

If any falsifier lands, this audit is closed-negative and the 2026-05-27 retag stands.

---

## 6 · Cross-references

- **Canonical derivation docs:** [`DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md), [`DERIV_FTD0110_NONLINEAR_BRIDGE.md`](DERIV_FTD0110_NONLINEAR_BRIDGE.md).
- **Honest analysis the §6 resolution displaces:** [`EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md`](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md).
- **Scoping memo (2026-05-23, FTD-0203):** [`SCOPE_FTD_0110_NONLINEAR_BRIDGE.md`](../10_eft_program/SCOPE_FTD_0110_NONLINEAR_BRIDGE.md).
- **Candidate-aggregation proof scripts (not §6 verifications):** [`scripts/proofs/proof_ftd0110_full_aggregation.py`](../../../scripts/proofs/proof_ftd0110_full_aggregation.py), [`scripts/proofs/proof_ftd0110_active_partition.py`](../../../scripts/proofs/proof_ftd0110_active_partition.py).
- **LEDGER row:** [`LEDGER.md`](../07_assessment/LEDGER.md) FTD-0110 (with 2026-04-27 / 2026-05-04 / 2026-05-27 maintenance history).
- **Tracker entry:** [`TRACKER_ONTIC_TRUTH.md`](../07_assessment/TRACKER_ONTIC_TRUTH.md) OT-3.4.

---

## 7 · Single-line summary

**The 2026-05-27 §6 "Orbit-Equipartition Theorem" and "Timescale Separation Theorem" do not close the FTD-0110 nonlinear-bridge gap: the orbit-equipartition derivation contains an arithmetic error at the load-bearing $A^2/4$ step, applies single-block analysis to a multi-scale phenomenon, and would predict pure $k = 1/4$ with no drift — contradicting the empirical log-A signature that Mechanism α (multi-block irrep mixing, ~1 week's actual perturbation calculation) is structurally consistent with. Recommended: revert the 2026-05-27 LEDGER FTD-0110 retag, restore the 2026-05-04 honest position, queue Mechanism α as the real Phase 1 closure work, and pre-register the falsifier rules before any further attempt.**
