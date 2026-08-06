# FOUND — Structural Decoupling of the Algebraic Spine from the Engine Action

**Status:** [SYNTHESIS — DEGRADED BY FTD-0412] — three valid diagnostic legs remain; the former Wilson-Dirac fourth leg is retracted because it evolved the wrong real-time operator. Not a new theorem.
**LEDGER row:** FTD-0129.
**Predecessors:** FTD-0121 (physics-bridge crystallisation) consolidates the *positive* empirical content; this document consolidates the *negative* dynamical content.

---

## 1 · The question

The master quadratic `x² − 16G*²x + 16G*³ = 0` has two roots, `x_+ ≈ 137.036` and `x_− ≈ 3.024`. The empirical match `x_+ = 1/α` at 1.26 ppm is the **single live physics identification** (FTD-0013). It is tagged [STRONGLY MOTIVATED CONJECTURE]; the underlying polynomial and Γ-product algebra are theorems (FTD-0001, FTD-0002), but the **identification** of the larger root with physical α is empirical, not derived. *(The historical paired identification `x_−  N_c` (the "dual-prediction" framing) is **RETIRED** per FTD/FQCR Cleanup Taxonomy v1.4 §5; LEDGER FTD-0014 removed in commit `ca7eb61`. The smaller root `x_− ≈ 3.024` is now a mathematical artifact of `P(x)` only; `N_c = 3` in FTD is independently sourced — see `DERIV_NC_FROM_TOPOLOGY.md` and the Moore Layer Theorem.)*

A natural follow-up question: **does the master-quadratic value `α = 1/x_+` play the *dynamical* role of QED's fine-structure constant in FTD's engine?** Concretely: if we measure observables in the engine that, in QED, would scale with α (the static Coulomb potential, the Schwinger anomaly, scattering amplitudes), do those observables carry the master-quadratic value?

If yes, the identification would gain a dynamical-mechanism warrant beyond the structural-uniqueness scan evidence.
If no, the identification stays formally a polynomial-root coincidence with structural-uniqueness backing — defensible empirical match, but not a derivation.

This document originally claimed four independent engine tests. FTD-0412
invalidated the Wilson-Dirac leg as a physical measurement. The surviving
statement is a three-leg, channel-limited diagnostic; it is not a foundational
obstruction and it no longer supports the claim that the matter vertex was
empirically tested.

---

## 2 · Three valid legs plus one invalidated historical leg

The first three tests remain evidence about their stated channels. The fourth
is preserved only to document the retired implementation.

### 2.1 — FTD-0004 (Phase G geometric Coulomb) [THEOREM]

The static Coulomb potential `V(r)` measured in the engine is **exactly** the periodic lattice Poisson Green's function:

> `V(r, L) = 2r · G_L(r)` with `R² = 1.0000` at `L = 384`, median residual 0.07% in the Coulomb tail.

This is a [THEOREM]: at every finite L, the measured `V(r)` matches the geometric Green's function with **zero free parameters**. The master-quadratic value α = 1/x_+ does not appear as a multiplicative prefactor on `V(r)` — and cannot, because the Green's function alone fits at machine precision with no residual room for an α scaling.

**What this leg shows:** static V(r) carries no fine-structure content. The Phase G reframe retracted the earlier "1.23×" Coulomb-deviation interpretation as under-equilibration; the corrected reading is geometric.

### 2.2 — FTD-0005 (Phase J ultralocality) [THEOREM at L=2]

The FTD partition function at L=2 satisfies an **ultralocality** identity: each lattice cell's contribution depends only on its on-site state, with no neighbour coupling at the action level. This is structurally a statement that the algebraic spine (which lives in polynomial-root / number-theoretic territory) **decouples** from action-level dynamics at the smallest non-trivial lattice.

[THEOREM at L=2 — mode-degeneracy origin]; [DISCONFIRMED for general L] (the audit closes the general-L conjecture per `scripts/proofs/proof_phase_j_general_L.py`; SPEC_ALGEBRAIC_SPINE.md §7 retagged accordingly). The L=2 result is a mode-counting accident at the Nyquist mode (the centered first-derivative ∂_i has eigenvalue i·sin(k_i) which vanishes at k_i ∈ {0, π}, the only available momenta on a 2³ lattice); at L ≥ 3 the spectrum is non-degenerate and the kinetic term picks up explicit spatial-distribution dependence, contradicting the conjectured ultralocality. Its scope is L=2 only; it cannot establish general action blindness.

**What this leg shows:** the algebraic spine and the engine action live in non-communicating sectors at the partition-function level. Any α-injection mechanism cannot operate through the action alone.

### 2.3 — FTD-0125 (Phase I gauss-projection erasure) [DERIVED + OUTCOME C]

The Phase I pre-registered campaign (tag `preregister-phase-i-native-coupling-v1`) tested the hypothesis that the engine's measured `α_r` from `V(r)` carries the master-quadratic value `G_C² = 1/x_+` as a multiplicative prefactor. The pre-registered outcome A required `g_engine² = g_FTD²` to 10⁻³ relative tolerance.

**Result — outcome C:** measured `g_engine² ≈ 0.08-0.18` (varying with r), NOT the constant `1/x_+ ≈ 0.0073`. Pre-registered hypothesis FALSIFIED at all 8 fixtures.

**Diagnosis:** the engine's tick cycle runs both the wave-propagation source coupling `δJ −= G_C · ∇s` (sign per the Term 2 amendment of 2026-07-18, `SPEC_FTD_LAGRANGIAN.md` §3.3; the erasure argument below is sign-independent) AND the gauss-projection step (`∇·J = ρ`). For static charges, the gauss-projection step **erases the longitudinal G_C contribution every tick**, leaving `V(r)` determined purely by the geometric kernel. The `G_C` factor does not survive the tick cycle.

**What this leg shows:** static V(r) measurement does not carry the coupling because the coupling channel that could carry it is being erased at every tick by the projection step. Phase G's "no fine-structure content" reading is empirically reinforced from a second angle.

### 2.4 — FTD-0126 (Phase II Wilson-Dirac fixed B) [RETRACTED AS PHYSICAL RESULT]

The historical Phase II campaign evolved the spatial operator `D_W` directly
as a Hamiltonian. FTD-0412 proved that its special-spinor norm oracle was not
the generic energy spectrum and replaced real-time evolution with the
Hermitian Wilson Hamiltonian `H_W`.

**Historical output, not a physical result:** the retired implementation gave
`a_e_lattice=0.7955` and relative error `683.95`. Those numbers characterize
the wrong operator and cannot be used as Wilson-Dirac evidence.

The conceptual diagnosis that the Schwinger anomaly requires a photon loop and
cannot be produced by a fixed classical background remains valid. It does not
convert the retired numerical run into a valid measurement.

**What this leg shows:** nothing empirical about the corrected matter-sector
vertex. A corrected-Hamiltonian campaign remains open.

---

## 3 · The convergent diagnostic

Each test attacks coupling injection at a different layer of the engine:

| Layer | Test | Channel | Result |
|---|---|---|---|
| **Static observable** | FTD-0004 | V(r) prefactor | Geometric Green's function exact; no α slot |
| **Action** | FTD-0005 | Partition function | Ultralocal at L=2; algebraic spine decoupled |
| **Dynamical observable** | FTD-0125 | Wave-prop + gauss-proj V(r) | Gauss-projection erases longitudinal G_C every tick |
| **Matter-sector vertex** | FTD-0126 | Retired `D_W` real-time evolution | **INVALIDATED by FTD-0412; no corrected result** |

The three surviving tests concern static/action/longitudinal-potential
channels. They do not establish decoupling in a matter vertex. The stronger
four-channel convergence claim is retracted.

The surviving structural-decoupling finding is channel-limited evidence, not a
load-bearing no-go.

---

## 4 · What this means

### 4.1 — What is sharpened

**MC-T4.3** is an unfinished search constrained by three surviving diagnostics,
not a foundational obstruction. FTD-0412 removes the claimed matter-vertex
leg, so an action-based or dynamical route not covered by those diagnostics
remains open.

The correct scope is “open research program with three tested channel
boundaries.”

### 4.2 — What is *not* falsified

The algebraic spine is unaffected. None of the following claims have changed tier or status:

- **FTD-0001** (master quadratic [THEOREM]) — unchanged
- **FTD-0002** (G\* identity [THEOREM]) — unchanged
- **FTD-0013** (x_+ = 1/α at 1.26 ppm [STRONGLY MOTIVATED CONJECTURE]) — unchanged
- ~~**FTD-0014** (x_− = N_c at 0.80%)~~ — **RETIRED** per v1.4 §5; LEDGER row removed in commit `ca7eb61`. `N_c = 3` independently sourced (`DERIV_NC_FROM_TOPOLOGY.md`).
- All nine numbered spine results (seven theorem-grade + two honestly-tiered — Theorem 3 at its arithmetic core only; see `SPEC_ALGEBRAIC_SPINE.md` §0 count convention) — unchanged
- ~~The structural-uniqueness scan from FTD-0121 / Paper A (0 dual-matchers across 2,871,576 polynomials, rank 1 by ~130×) — unchanged.~~ **WITHDRAWN 2026-08-04 (FTD-0802):** the count is **4** non-master dual-matchers, not 0 — the "0 genuinely-new cubic" figure was a hardcoded literal that no code tested — and the scan failed its pre-registered base-rate control (`N_null = 0.0014`, Outcome B), so a zero count would have carried no evidential weight regardless. OT-3.3 retagged `[SELECTION]`. *(The "~4×10⁵:1 Bayes factor" figure is retracted to [NUMERICAL FACT] — not runner-computed, ~19× scan-size; per the spine audit.)*
- The BCC complex-structure theorem (FTD-0122) — unchanged

What was tested and ruled out is a **specific dynamical interpretation** of the `x_+  1/α` identification: that `α = 1/x_+` plays the role of QED's coupling at the matter-sector vertex via classical gauge fields. The polynomial algebra and the empirical match remain at their established tags.

### 4.3 — Corrected external statement

The master quadratic remains a mathematical theorem and `x_+ ↔ 1/alpha`
remains an empirical identification. Three diagnostics constrain the static,
L=2 action, and longitudinal-potential channels. They do not exclude a
corrected matter vertex, a larger action, a dynamical gauge sector, or another
interacting mechanism. The broad mechanism question is unfinished.

### 4.4 — The Paper C alignment

Any Paper C conclusion relying on FTD-0126 as a load-bearing Branch-B
measurement must be narrowed. FTD-0125 survives; FTD-0126 does not.

---

## 5 · What would change this verdict

The three surviving diagnostics are empirical, not a no-go theorem. A future result that would update the structural-decoupling diagnosis:

1. **A non-action mechanism that produces the master-quadratic value in a measured observable.** For example: a finite-L boundary-condition mechanism in which the master quadratic emerges as a constraint on allowed boundary configurations (MC-T4.3 candidate class 1). This is research-program territory, not session-scale work.
2. **A dynamical gauge-field implementation** that makes the photon a quantum degree of freedom rather than a fixed classical configuration. In the matter-sector test (FTD-0126), one-loop diagrams with a dynamical gauge field would change the Schwinger prediction from "no mechanism at tree level" to "α/(2π) at one loop with a coupling = 1/x_+ if the spine plays that role." Effort: multi-week implementation; falsifiable in principle.
3. **A different observable that carries the master-quadratic value.** The surviving tests cover static V, the L=2 partition function, and dynamical longitudinal V. Matter vertices, transverse-wave radiation, Ampère-Maxwell coupling, and loop-dominated scattering remain untested by a valid protocol.

Until a corrected matter-sector result lands, only the three channel-limited
diagnostics survive.

---

## 6 · Methodology after the FTD-0412 correction

The surviving diagnostics cover three distinct but limited properties:

- **FTD-0004** is about a *static observable* (V(r) at fixed time). No dynamics involved.
- **FTD-0005** is about the *action structure* (the integrand of the partition function). No measurement involved.
- **FTD-0125** is about a *dynamical observable* through the actual tick cycle (V(r) measured after wave-prop + gauss-proj steps).
- **FTD-0126** attempted a *matter-sector vertex* test but is invalidated and contributes no evidentiary leg.

Pre-registration does not cure an invalid instrument. FTD-0125 retains its
own status; FTD-0126's pre-registration provenance remains useful, but its
outcome does not survive the operator error.

---

## 7 · Cross-references

- **LEDGER FTD-0129** (this synthesis) — the canonical entry; cite this for the convergent finding.
- **LEDGER FTD-0004** — Phase G geometric Coulomb [THEOREM].
- **LEDGER FTD-0005** — Phase J ultralocality [THEOREM at L=2].
- **LEDGER FTD-0125** — Phase I native-coupling derivation [DERIVED] + outcome C engine cross-check.
- **LEDGER FTD-0126** — historical Phase II campaign [RETRACTED AS PHYSICAL RESULT by FTD-0412].
- **LEDGER FTD-0121** — physics-bridge crystallisation [SYNTHESIS] (positive content; companion to this doc).
- **SPEC_OPEN_MATH_BY_SECTOR.md MC-T4.3** — the open native-readout search; this synthesis supplies three scoped diagnostics, not a universal obstruction.
- **Paper C** (`PAPER_FTD_AS_WILSONIAN_EFT.tex`) — Branch-A complete + Branch-B decoupled; this synthesis is the cross-leg foundation Paper C draws on.
- **`PREREG_PHASE_I_NATIVE_COUPLING.md`** — the pre-registration behind the surviving Phase-I outcome-C leg. **`PREREG_PHASE_II_WILSON_DIRAC_G2.md`** is retained as protocol provenance, but its result is invalidated by FTD-0412 and contributes no leg.
- **`AUDIT_ALPHA_EXTRACTION.md`** + **`DERIV_EMERGENT_COULOMB_GEOMETRIC.md`** — the Phase G theory record (leg 1).
- **`SPEC_ALGEBRAIC_SPINE.md`** — Theorem 7 (Phase J ultralocality at L=2; leg 2).

---

## 8 · Honest limits of this synthesis

This document does **not** claim:

- That the algebraic spine and the engine action are *necessarily* decoupled. The three surviving diagnostics are empirical and scoped.
- That α cannot be derived from FTD axioms. Other action, matter, boundary, quantization, and interacting channels remain open.
- That the `x_+ ↔ 1/α` identification is downgraded. FTD-0013 stays at [STRONGLY MOTIVATED CONJECTURE].
- That the framework's epistemic ceiling has been determined. MC-T4.3 closure remains an open research program. What this synthesis does is **bound the scope of acceptable closure mechanisms** to non-action channels.

---

**Authoring note:** this remains a [SYNTHESIS] document, now explicitly
degraded to the three surviving scoped diagnostics by FTD-0412.
