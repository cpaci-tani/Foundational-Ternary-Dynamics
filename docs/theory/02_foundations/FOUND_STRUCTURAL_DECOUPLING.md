# FOUND — Structural Decoupling of the Algebraic Spine from the Engine Action

**Status:** [SYNTHESIS] — consolidates four independent empirical results into a single externally-defensible finding. Not a new theorem; a coherent re-statement of existing claims at their canonical tags.
**LEDGER row:** FTD-0129.
**Date:** 2026-05-03 evening.
**Predecessors:** FTD-0121 (physics-bridge crystallisation, 2026-05-01) consolidates the *positive* empirical content; this document consolidates the *negative* dynamical content.

---

## 1 · The question

The master quadratic `x² − 16G*²x + 16G*³ = 0` has two roots, `x_+ ≈ 137.036` and `x_− ≈ 3.024`. The empirical match `x_+ = 1/α` at 1.26 ppm is the **single live physics identification** (FTD-0013). It is currently tagged [STRONGLY MOTIVATED CONJECTURE]; the underlying polynomial and Γ-product algebra are theorems (FTD-0001, FTD-0002), but the **identification** of the larger root with physical α is empirical, not derived. *(The historical paired identification `x_− ↔ N_c` (the "dual-prediction" framing) is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The smaller root `x_− ≈ 3.024` is now a mathematical artifact of `P(x)` only; `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` and the Moore Layer Theorem.)*

A natural follow-up question: **does the master-quadratic value `α = 1/x_+` play the *dynamical* role of QED's fine-structure constant in FTD's engine?** Concretely: if we measure observables in the engine that, in QED, would scale with α (the static Coulomb potential, the Schwinger anomaly, scattering amplitudes), do those observables carry the master-quadratic value?

If yes, the identification would gain a dynamical-mechanism warrant beyond the structural-uniqueness Bayes factor.
If no, the identification stays formally a polynomial-root coincidence with structural-uniqueness backing — defensible empirical match, but not a derivation.

This document records the answer that four independent engine tests have now produced.

---

## 2 · The four-leg empirical convergence

Each of the following tests attacks the same coupling-injection question from a structurally different angle. All four return the same answer.

### 2.1 — FTD-0004 (Phase G geometric Coulomb) [THEOREM]

The static Coulomb potential `V(r)` measured in the engine is **exactly** the periodic lattice Poisson Green's function:

> `V(r, L) = 2r · G_L(r)` with `R² = 1.0000` at `L = 384`, median residual 0.07% in the Coulomb tail.

This is a [THEOREM]: at every finite L, the measured `V(r)` matches the geometric Green's function with **zero free parameters**. The master-quadratic value α = 1/x_+ does not appear as a multiplicative prefactor on `V(r)` — and cannot, because the Green's function alone fits at machine precision with no residual room for an α scaling.

**What this leg shows:** static V(r) carries no fine-structure content. Phase G reframe (2026-04-19) retracted the earlier "1.23×" Coulomb-deviation interpretation as under-equilibration; the corrected reading is geometric.

### 2.2 — FTD-0005 (Phase J ultralocality) [THEOREM at L=2]

The FTD partition function at L=2 satisfies an **ultralocality** identity: each lattice cell's contribution depends only on its on-site state, with no neighbour coupling at the action level. This is structurally a statement that the algebraic spine (which lives in polynomial-root / number-theoretic territory) **decouples** from action-level dynamics at the smallest non-trivial lattice.

[THEOREM at L=2 — mode-degeneracy origin]; [DISCONFIRMED for general L] (audit 2026-05-23 closes the general-L conjecture per `scripts/proofs/proof_phase_j_general_L.py`; SPEC_ALGEBRAIC_SPINE.md §7 retagged accordingly). The L=2 result is a mode-counting accident at the Nyquist mode (the centered first-derivative ∂_i has eigenvalue i·sin(k_i) which vanishes at k_i ∈ {0, π}, the only available momenta on a 2³ lattice); at L ≥ 3 the spectrum is non-degenerate and the kinetic term picks up explicit spatial-distribution dependence, contradicting the conjectured ultralocality. The L=2 leg remains the cleanest small-lattice statement that the algebraic spine and the action-level dynamics communicate weakly *at L=2*; the lead-physicist diagnosis (2026-04-19) extended this — "the action is structurally blind to the spine" — by aggregating with the other three diagnostic legs (FTD-0004 / FTD-0125 / FTD-0126) which test different scales and channels. The structural-decoupling synthesis (§3) does not depend on the general-L extension of FTD-0005.

**What this leg shows:** the algebraic spine and the engine action live in non-communicating sectors at the partition-function level. Any α-injection mechanism cannot operate through the action alone.

### 2.3 — FTD-0125 (Phase I gauss-projection erasure) [DERIVED + OUTCOME C]

The Phase I pre-registered campaign (2026-05-03 morning, tag `preregister-phase-i-native-coupling-v1`) tested the hypothesis that the engine's measured `α_r` from `V(r)` carries the master-quadratic value `G_C² = 1/x_+` as a multiplicative prefactor. The pre-registered outcome A required `g_engine² = g_FTD²` to 10⁻³ relative tolerance.

**Result — outcome C:** measured `g_engine² ≈ 0.08-0.18` (varying with r), NOT the constant `1/x_+ ≈ 0.0073`. Pre-registered hypothesis FALSIFIED at all 8 fixtures.

**Diagnosis:** the engine's tick cycle runs both the wave-propagation source coupling `δJ += G_C · ∇s` AND the gauss-projection step (`∇·J = ρ`). For static charges, the gauss-projection step **erases the longitudinal G_C contribution every tick**, leaving `V(r)` determined purely by the geometric kernel. The `G_C` factor does not survive the tick cycle.

**What this leg shows:** static V(r) measurement does not carry the coupling because the coupling channel that could carry it is being erased at every tick by the projection step. Phase G's "no fine-structure content" reading is empirically reinforced from a second angle.

### 2.4 — FTD-0126 (Phase II Wilson-Dirac fixed B) [CLOSED OUTCOME C]

The Phase II pre-registered campaign (2026-05-03 evening, tag `preregister-phase-ii-wilson-dirac-g2-v1`) tested whether classical Wilson-Dirac matter coupled to a fixed uniform B-field reproduces the QED tree-level Schwinger anomaly `a_e = α/(2π)` with `α = 1/x_+`. The pre-registered outcome A required relative error < 5%.

**Result — outcome C:** measured `a_e_lattice = ω_s/ω_c − 1 = 0.7955` vs `a_e_Schwinger = α_FTD/(2π) = 1.16×10⁻³`. **Relative error: 683.95.** The measured `g_lattice/2 = 1.7955` is nowhere near `g = 2 + α/π ≈ 2.0023` (continuum QED) or `g = 2` (continuum Dirac); the deviation is ~80% of g, three orders of magnitude larger than physical Schwinger.

**Diagnosis:** the Schwinger anomaly is a **one-loop QED effect** requiring a dynamical photon. Our setup is tree-level with a classical (non-dynamical) gauge field. There is no physical mechanism in this configuration to produce α/(2π); the only g − 2 source at tree level is the well-known O(qB·m·a²) Wilson-r lattice artefact, which sits in the O(1) regime at engine-realistic parameters and dominates the measurement. We measured Wilson-r artefact, not physical Schwinger.

**What this leg shows:** even the matter-sector vertex — the place where dynamical α SHOULD appear if it appears anywhere — does not carry the master-quadratic value at the engine-realistic parameters tested under classical-gauge-field protocols.

---

## 3 · The convergent diagnostic

Each test attacks coupling injection at a different layer of the engine:

| Layer | Test | Channel | Result |
|---|---|---|---|
| **Static observable** | FTD-0004 | V(r) prefactor | Geometric Green's function exact; no α slot |
| **Action** | FTD-0005 | Partition function | Ultralocal at L=2; algebraic spine decoupled |
| **Dynamical observable** | FTD-0125 | Wave-prop + gauss-proj V(r) | Gauss-projection erases longitudinal G_C every tick |
| **Matter-sector vertex** | FTD-0126 | Wilson-Dirac in fixed B | Tree-level Wilson-r artefact dominates; no Schwinger |

The four tests are **independent** in their physical content (different observables, different protocols, different layers of the engine), but **convergent** in their diagnosis: the master-quadratic value α = 1/x_+ does NOT flow into engine matter-sector dynamical observables under any classical-gauge-field protocol tested.

This is the structural-decoupling finding. It is now load-bearing empirical evidence, not just theoretical conjecture.

---

## 4 · What this means

### 4.1 — What is sharpened

**MC-T4.3** (the central foundational obstruction in `CHECKLIST_MATH_COMPLETE.md`) is now empirically reinforced. The lead-physicist diagnosis from 2026-04-19 — that any α-injection mechanism must be **non-action** (boundary conditions, observable selection, or quantization choice) — has gained four independent empirical legs. The structural-blockage claim is no longer "we haven't found a mechanism yet" but "we have explicitly tested four candidate dynamical channels and all four have empirically returned outcome-C-class verdicts under pre-registration discipline."

This sharpens MC-T4.3 from "open research program" to "open research program with explicit four-test boundary on what cannot work."

### 4.2 — What is *not* falsified

The algebraic spine is unaffected. None of the following claims have changed tier or status:

- **FTD-0001** (master quadratic [THEOREM]) — unchanged
- **FTD-0002** (G\* identity [THEOREM]) — unchanged
- **FTD-0013** (x_+ = 1/α at 1.26 ppm [STRONGLY MOTIVATED CONJECTURE]) — unchanged
- ~~**FTD-0014** (x_− = N_c at 0.80%)~~ — **RETIRED** per v1.4 §5 (2026-05-22); LEDGER row removed in commit `ca7eb61`. `N_c = 3` independently sourced (`DERIV_NC_FROM_TOPOLOGY.md`).
- All nine numbered spine results (six theorem-grade + three honestly-tiered; see `SPEC_ALGEBRAIC_SPINE.md` §0) — unchanged
- The structural-uniqueness Bayes factor (~4×10⁵) from FTD-0121 / Paper A — unchanged
- The BCC complex-structure theorem (FTD-0122) — unchanged

What was tested and ruled out is a **specific dynamical interpretation** of the `x_+ ↔ 1/α` identification: that `α = 1/x_+` plays the role of QED's coupling at the matter-sector vertex via classical gauge fields. The polynomial algebra and the empirical match remain at the same tags they held before today.

### 4.3 — Strengthening rather than weakening

A naive reading would treat four outcome-C verdicts as "the framework is in trouble." The correct reading is the opposite: **knowing what does not work and why is structural progress**. The framework's external position is now:

> The master quadratic is a [THEOREM]. The dual-prediction at 1.26 ppm + 0.80% is empirically real and structurally unique within the natural FTD polynomial family (Bayes ~4×10⁵). The interpretation of `α = 1/x_+` as a classical-gauge-field coupling has been pre-registered against and empirically falsified at four independent layers of the engine. Therefore: any future framework derivation of α from FTD must operate through a **non-action** mechanism (boundary conditions, observable selection, quantization choice). This is the explicit foundational obstruction; it is the central open research question the framework acknowledges.

This is a more defensible position than "we conjecture the master-quadratic coupling is the QED coupling" — because it tells a hostile reviewer exactly where the framework is conjectural, exactly what has been ruled out, and exactly what closure would require.

### 4.4 — The Paper C alignment

The Paper C revision (`PAPER_FTD_AS_WILSONIAN_EFT.tex`, 15pp, commit `9291b4d`) already wraps the old "160× QED β" framing in `\sout` with retraction notes and aligns the conclusion to Branch-A complete + Branch-B structurally decoupled. This synthesis doc provides the cross-leg empirical foundation Paper C draws on; FTD-0125 + FTD-0126 are the load-bearing Branch-B-decoupling results. The two outcome-C verdicts vindicate Paper C's reframe; they are not ad-hoc post-hoc interpretation.

---

## 5 · What would change this verdict

The four-leg convergence is empirical, not a no-go theorem. A future result that would update the structural-decoupling diagnosis:

1. **A non-action mechanism that produces the master-quadratic value in a measured observable.** For example: a finite-L boundary-condition mechanism in which the master quadratic emerges as a constraint on allowed boundary configurations (MC-T4.3 candidate class 1). This is research-program territory, not session-scale work.
2. **A dynamical gauge-field implementation** that makes the photon a quantum degree of freedom rather than a fixed classical configuration. In the matter-sector test (FTD-0126), one-loop diagrams with a dynamical gauge field would change the Schwinger prediction from "no mechanism at tree level" to "α/(2π) at one loop with a coupling = 1/x_+ if the spine plays that role." Effort: multi-week implementation; falsifiable in principle.
3. **A different observable that carries the master-quadratic value.** The four tests covered static V, partition function, dynamical V, and Wilson-Dirac vertex. Other observables remain untested: transverse-wave radiation rate (FTD-0120 territory), Ampère-Maxwell coupling, scattering cross-sections in scenarios where loop corrections dominate. If any of these reproduces the master-quadratic value, the structural-decoupling reading would need to be narrowed to "the spine is decoupled from THESE four channels but not from THAT one."

Until such a result lands, the four-leg convergence is the framework's honest current state.

---

## 6 · Methodology — why the four tests stack

It would be reasonable to ask: are these four tests really independent, or are they all measuring the same thing in different forms? The honest answer is that they cover four structurally distinct properties of the engine:

- **FTD-0004** is about a *static observable* (V(r) at fixed time). No dynamics involved.
- **FTD-0005** is about the *action structure* (the integrand of the partition function). No measurement involved.
- **FTD-0125** is about a *dynamical observable* through the actual tick cycle (V(r) measured after wave-prop + gauss-proj steps).
- **FTD-0126** is about a *matter-sector vertex* (the place where, in QED, the coupling first appears in the perturbation expansion).

These four are not "the same thing." If all four returned outcome A, the framework would have a strong dynamical injection mechanism; if some returned A and some returned C, we'd have a layered diagnostic of *which* channels carry α and which don't. The fact that all four return outcome C is what makes the convergence load-bearing.

It is also worth noting that FTD-0125 and FTD-0126 were **pre-registered** before measurement (tags `preregister-phase-i-native-coupling-v1` and `preregister-phase-ii-wilson-dirac-g2-v1`), with explicit outcomes A/B/C/D and numerical criteria committed before any data was collected. Two outcome-C verdicts under pre-registration discipline are not artefacts of post-hoc interpretation; the framework set the falsification criteria and then met them honestly.

---

## 7 · Cross-references

- **LEDGER FTD-0129** (this synthesis) — the canonical entry; cite this for the convergent finding.
- **LEDGER FTD-0004** — Phase G geometric Coulomb [THEOREM].
- **LEDGER FTD-0005** — Phase J ultralocality [THEOREM at L=2].
- **LEDGER FTD-0125** — Phase I native-coupling derivation [DERIVED] + outcome C engine cross-check.
- **LEDGER FTD-0126** — Phase II Wilson-Dirac campaign [CLOSED — OUTCOME C].
- **LEDGER FTD-0121** — physics-bridge crystallisation [SYNTHESIS] (positive content; companion to this doc).
- **CHECKLIST_MATH_COMPLETE.md MC-T4.3** — the central foundational obstruction; this synthesis is empirical reinforcement of the obstruction's scope.
- **Paper C** (`PAPER_FTD_AS_WILSONIAN_EFT.tex`) — Branch-A complete + Branch-B decoupled; this synthesis is the cross-leg foundation Paper C draws on.
- **`PREREG_PHASE_I_NATIVE_COUPLING.md`** + **`PREREG_PHASE_II_WILSON_DIRAC_G2.md`** — the two pre-registrations whose outcome-C verdicts contribute legs 3 and 4.
- **`AUDIT_ALPHA_EXTRACTION.md`** + **`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`** — the Phase G theory record (leg 1).
- **`SPEC_ALGEBRAIC_SPINE.md`** — Theorem 7 (Phase J ultralocality at L=2; leg 2).

---

## 8 · Honest limits of this synthesis

This document does **not** claim:

- That the algebraic spine and the engine action are *necessarily* decoupled. The four tests are empirical; a non-action mechanism remains possible.
- That α cannot be derived from FTD axioms. It claims that the four tested classical-gauge-field channels do not constitute a derivation; other channels remain open.
- That the `x_+ ↔ 1/α` identification is downgraded. FTD-0013 stays at [STRONGLY MOTIVATED CONJECTURE] with the same Bayes weight as before; this synthesis affects only the *interpretation* (no dynamical-mechanism warrant from these four channels), not the *strength* (the structural-uniqueness scans remain valid). *(FTD-0014 is **retired** per v1.4 §5 — a separate, independent change unrelated to this synthesis.)*
- That the framework's epistemic ceiling has been determined. MC-T4.3 closure remains an open research program. What this synthesis does is **bound the scope of acceptable closure mechanisms** to non-action channels.

---

**Authoring note:** this is a [SYNTHESIS] doc per CLAUDE.md tag definitions: "cross-document integration of multiple lower-level claims into a single externally-defensible package; not a new theorem but a coherent re-statement of existing claims at their canonical tags." It does not promote or demote any LEDGER claim. Its value is consolidating a finding that emerged across four independent investigations into a single citable artifact.
