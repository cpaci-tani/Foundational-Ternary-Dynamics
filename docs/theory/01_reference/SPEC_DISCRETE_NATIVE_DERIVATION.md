# SPEC: Discrete-Native Derivation Program

**Status:** Methodological framework specification
**Tag:** [METHODOLOGICAL REFRAME] — not a derivation, not a claim
**LEDGER row:** FTD-0136
**Supersedes (in scope):** the implicit goal "substrate-derive SM theoretical structures" embedded in Phase A-F EFT campaigns, FTD-0050, FTD-0135, and the MC-T4.3 framing as currently stated

---

## 1. The reframe

Existing physics (SM, GR) is a haphazard mixture of discrete and continuous structures:

- Continuous Hilbert space over discrete particle states
- Continuous spacetime carrying discrete quantum numbers
- Continuous gauge fields with discrete symmetry groups
- Continuous renormalization group flow between discrete fixed points
- Continuous path integrals discretized only as a regulator

This mixture is empirically excellent. It is also ontologically incoherent — the discrete pieces and continuous pieces are not derived from a single substrate; they are stitched together by convention.

**FTD is fully discrete.** A 3D cubic lattice (no defined boundary), ternary states `s ∈ {-1, 0, +1}`, integer ticks, 26-neighbor causality. There is no continuous layer. There is no path integral. There is no action functional. There are *update rules*.

### 1.1 The wrong question

> "How does FTD reproduce SM's theoretical apparatus (Lagrangians, Feynman diagrams, Wilson loops, action functionals)?"

This is the question Phase A-F (EFT recovery), FTD-0050 (master quadratic as RG-step), FTD-0025 night-annotation (confinement substrate routes), and FTD-0135 (substrate Yukawa vertex) all attempted. All four closed NEGATIVE.

The convergent diagnostic from these closures (FTD-0129) is that *action-based mechanisms structurally fail to substrate-derive matter-sector couplings*. The diagnostic was previously read as evidence of an MC-T4.3-class obstruction internal to FTD. Under this reframe, the diagnostic reads instead as evidence that **the question was malformed**: importing the continuous *shape* of QFT objects into the discrete substrate cannot succeed because the substrate does not produce continuous objects.

### 1.2 The right question

> "What observables does FTD's discrete update rule produce, and how do those observables compare to *measurement* (not SM theoretical prediction)?"

Measurement is the only ground truth. SM is a currently-best-known competing theoretical framework whose predictions FTD must compare against *only at the empirical level*. Where SM and FTD predict the same measurable quantity, they must agree to within experimental precision. Where SM predicts a structure (Lagrangian, vertex, gauge group) that has no direct experimental analog, FTD has no obligation to reproduce that structure — only to produce the same measurements via whatever discrete mechanism it actually has.

### 1.3 What this reframe is NOT

- **Not a claim that SM is wrong.** SM is empirically excellent. Every SM-derived measurement is a constraint FTD must satisfy.
- **Not a license to dismiss measurements.** Muon lifetimes, electron g-2, GPS time dilation, hadron cross-sections, neutrino oscillations — all are constraints. The discrete substrate must reproduce them.
- **Not a tag change for any existing LEDGER row.** Existing tags reflect honest evidence-based status under their own framings; this reframe changes future framing, not past accounting.
- **Not a derivation.** This is a methodological spec, not a result.
- **Not a replacement for MC-T4.3.** MC-T4.3 stays open as "structural decoupling of algebraic spine and dynamical EFT under action-based derivation"; the reframe widens the closure path beyond action-based mechanisms.

---

## 2. Four classes of FTD-native observables

The discrete substrate produces observables that fall into four natural classes. Each class is defined by what the *engine actually measures*, not by mapping to SM's theoretical structure.

### 2.1 Class A — Cluster size (= rest mass)

- **Engine observable:** number of voxels in a stable manifested cluster at amplitude `A`, after equilibration.
- **Existing FTD work:**
  - FTD-0110 [DERIVED at linear level]: `N(A) ≈ ¼·(A/K_GENESIS)²`, with `¼ = 1/N_base` from O_h A_{1g} multiplicity.
  - FTD-0015 [STRONGLY MOTIVATED CONJECTURE]: `m_e = m_P · √(2π) · (16/3) · α^{11}` (mass anchor + ladder).
  - SM mass identifications: e/μ/π/K/p/τ within ~5% (FTD-0110).
- **Status:** linear-level [DERIVED]; full nonlinear regime [SMC]; mass anchor [SELECTION].
- **Native discreteness:** cluster size is *literally an integer* — no continuous limit needed. The engine produces N ∈ ℕ.

### 2.2 Class B — Cluster persistence (= lifetime / inverse decay rate)

- **Engine observable:** integer tick-count `τ_persist` until a manifested cluster dissolves (returns to void) under specified perturbation conditions.
- **Existing FTD work:** essentially none. Decay rates are currently quoted via SM formulas with FTD-derived constants inserted (PARAMETRIC).
- **Status:** [OPEN] — measurement infrastructure missing.
- **Native discreteness:** lifetime is *literally an integer* in tick units — no continuous time needed.
- **Required infrastructure:**
  - Engine instrument: cluster-persistence measurement protocol (specify perturbation, measure tick-count to dissolution).
  - Theory: identify what FTD-native quantity `τ_persist` corresponds to in measurement (likely: invariant lifetime in particle rest frame, divided by `t_tick`).

### 2.3 Class C — Cluster-cluster interaction (= coupling / force)

- **Engine observable:** force/effective-potential between two clusters as a function of separation `r`, amplitudes `(A_1, A_2)`, cluster types, and tick.
- **Existing FTD work:**
  - Phase G [THEOREM]: static Coulomb between *point sources* = lattice Poisson Green's function `G_+(r) → 1/(4π·r)` at large r.
  - Phase J [THEOREM at L=2]: ultralocality of partition function (orthogonal, not relevant here).
- **Status:**
  - Static-EM-point-source case: [DERIVED via Phase G].
  - Cluster-cluster (extended source) case: [OPEN].
  - Non-EM (Yukawa-like, color-like, gravitational): [OPEN].
- **Native discreteness:** force is measured per-tick as voxel-displacement gradient — no continuous-spacetime interpretation needed.
- **Required infrastructure:**
  - Engine instrument: cluster-cluster scattering/binding measurement (place two clusters, measure relative motion vs separation).
  - Theory: relate engine-measured force law to whatever *measurement* corresponds (scattering cross-section, binding energy, decay-channel branching ratio).

### 2.4 Class D — Cluster spectrum (= bound-state energies)

- **Engine observable:** stable (or quasi-stable) energy levels of bound multi-cluster systems.
- **Existing FTD work:**
  - Phase H: hydrogen `1/n²` to 0.001% — but this is for an *electron in an external Coulomb potential*, not a bound multi-cluster substrate state.
  - Hadron spectroscopy: no direct FTD substrate calculation; mass-formula matches at the level of single-cluster identification (Class A).
- **Status:** [OPEN] for substrate-bound multi-cluster spectra.
- **Native discreteness:** spectrum is *literally a discrete set* of stable cluster configurations — no continuous Hilbert space needed.
- **Required infrastructure:**
  - Engine instrument: bound-state finder (initialize multi-cluster configuration; equilibrate; record stable energies).
  - Theory: relate engine-measured spectrum to measured hadron / atomic spectrum.

---

## 3. Infrastructure dependency order

Recommended build order, by smallest scope first:

| Order | Class | Build | Scope estimate | Closes |
|-------|-------|-------|---------------|--------|
| 1 | B (persistence) | Engine cluster-persistence instrument + theory mapping | 2-4 sessions | New observable; no existing work to retrofit |
| 2 | C (interaction) | Engine cluster-cluster scattering instrument + theory mapping | 4-8 sessions | MC-T4.3 path; FTD-0135 promotion path (a) |
| 3 | D (spectrum) | Engine bound-state finder + theory mapping | 4-8 sessions | Hadron-spectrum substrate derivation; depends on (2) for stability analysis |

Class A (cluster size = mass) is largely already built (FTD-0110). Refinement work continues there but no new infrastructure is needed.

### 3.1 What each piece does NOT require

Critically, none of these infrastructure builds require:

- A substrate-level "action functional"
- A discrete "path integral"
- A discrete "Lagrangian"
- A discrete "Feynman rule" derivation
- A discrete "Wilson loop" interpretation
- A discrete "renormalization group flow"

Each of those is a *continuous-QFT shape* that the substrate need not reproduce. The substrate produces engine observables directly. The mapping to measurement does not route through reconstructed continuous-QFT machinery.

### 3.2 What each piece DOES require

- A clear specification of the engine measurement protocol (deterministic, hash-locked under the FTD-0027 pre-registration discipline)
- A theory document specifying the FTD-native observable's identification with a *measured* quantity (cross-section in barns, lifetime in seconds, binding energy in eV)
- A comparison protocol that respects the calibration ladder (FTD-0041: a_phys ≡ ℓ_P, t_tick ≡ ℓ_P/(√3·c), K_B ≡ m_e [provisional])

---

## 4. Per-class measurement-comparison protocol

For each engine observable `O_engine`, we need three pieces:

1. **Lattice-units value** `O_lat`: what the engine produces in voxel/tick/state-amplitude units
2. **Calibration conversion** `O_lat → O_SI`: via FTD-0041 calibration ladder + dimensional analysis
3. **Measurement** `O_meas`: the experimental value with its uncertainty

The comparison is `O_SI` against `O_meas` *only*. SM's theoretical prediction `O_SM_theory` is a *separate* comparison — useful for context but not load-bearing for FTD's own evaluation.

### 4.1 Tagging discipline for the comparison

When comparing `O_SI` to `O_meas`:

- **Match within experimental uncertainty:** [PREDICTION VERIFIED] — does NOT promote the underlying derivation tag (which carries its own [DERIVED]/[SMC]/[SELECTION] grade independently)
- **Match within order of magnitude only:** [PREDICTION CONSISTENT AT OOM] — informative but weak
- **Disagreement beyond experimental uncertainty:** [PREDICTION FALSIFIED] OR [CALIBRATION GAP] OR [UNMODELED PHYSICS] — diagnostic required to distinguish

When comparing `O_SI` to `O_SM_theory` (separate axis):

- Agreement is *not* a derivation of SM from FTD; it means both predict the same measurement
- Disagreement is *not* a falsification of FTD; it means FTD and SM predict different things at the SM-theoretical level, and *measurement* arbitrates

---

## 5. Worked example — what changes for FTD-0135 (Yukawa vertex)

Under the old framing, FTD-0135 closed NEGATIVE because the substrate-vertex argument required:

- Lattice Feynman rules from a Born-Infeld-style action (does not exist)
- A substrate-level matter-field definition (does not exist)
- A substrate-level Higgs-field definition (does not exist)
- An SM-Yukawa-vertex-shaped 3-point function (does not exist as a substrate object)

Under the reframe, the substrate-vertex *question itself* is malformed: the substrate does not produce SM-shaped Yukawa vertices; it produces *cluster-cluster interactions*. The right question becomes:

> "Measure the engine's interaction strength between two clusters identified with electron and Higgs (or electron and electron via Higgs exchange). Compare that interaction strength to whatever measurement corresponds (electron-electron Yukawa-mediated scattering rate; or electron-Higgs production cross-section)."

If the engine-measured interaction strength reproduces the measurement, FTD has substrate-derived the Yukawa coupling *as a measurement-level prediction*. The fact that it does so without an "action" or "Feynman rule" or "vertex" in the SM-theoretical sense is fine — those structures are continuous-QFT scaffolding, not measurements.

This is the path forward for closing the Class C infrastructure and, downstream, promoting FTD-0134 from [STRUCTURALLY MOTIVATED PARAMETRIC] to [DERIVED at measurement level].

---

## 6. Honest scope statement

### 6.1 What this SPEC delivers

- A methodological reframe articulating why action-based substrate-derivation has structurally failed
- A four-class taxonomy of FTD-native observables
- A dependency order for infrastructure builds
- A measurement-comparison protocol that does not route through SM-theoretical reconstructions
- A worked example showing how the reframe re-opens FTD-0135's closure as a Class C measurement question

### 6.2 What this SPEC does NOT deliver

- Any new derivation
- Any new tag promotion in LEDGER
- Any engine code
- A guarantee that the discrete-native infrastructure builds will succeed (they may close-negative for substantive reasons, just as the action-based attempts did)
- A substrate-level definition of "matter-field" or "Higgs-field" — those concepts may simply not be discrete-native, and the engine may produce equivalent measurements via cluster-cluster mechanics without ever needing them

### 6.3 What could falsify this reframe

If after building Classes B/C/D infrastructure, the engine-measured observables systematically disagree with measurement at precision better than experimental error, then:

- Either the discrete substrate is wrong, OR
- The reframe is correct but the cluster-identification map (FTD-0110) is wrong, OR
- The calibration ladder (FTD-0041) is wrong

This is the falsifiability surface. It is real and finite. The reframe is not an unfalsifiable epistemic dodge.

---

## 6.4 Calibration feasibility audit

A computational audit (`scripts/exploration/audit_calibration_feasibility_2026-05-04.py`) confirms the per-class feasibility under FTD-0041 calibration. Aggregate matrix:

| Class | Observable | Absolute | Dimensionless ratios |
|-------|-----------|----------|---------------------|
| A | cluster size = mass | FEASIBLE | FEASIBLE (FTD-0110 already done) |
| B | persistence = lifetime | INFEASIBLE | FEASIBLE (~10¹⁰ ratio span) |
| C | interaction = coupling | INFEASIBLE | FEASIBLE (Phase G shows the path) |
| D | spectrum = energies | BORDERLINE | FEASIBLE (hadronic + gross atomic) |

**Universal pattern**: FTD-native observables are measurable in dimensionless form but not at absolute physical scales under `a_phys = ℓ_P`. This is not a framework defect — it is a computational consequence of declaring the smallest-possible voxel scale, which forces all physical scales of interest to be 16-25 orders of magnitude larger than feasible engine lattices. The dimensionless-vs-dimensional split was already documented in `SPEC_DIMENSIONAL_MAP.md` as the "falsifiable spine vs calibration-conditional" distinction; this audit confirms it computationally for the discrete-native program.

**Cleared scope per class:**
- Class A: SM mass spectrum (FTD-0110 done; refinements continue).
- Class B: lifetime RATIOS for muon/pion/kaon/tau cluster (PDG ratios within 10¹⁰).
- Class C: dimensionless couplings (α, α_s, sin²θ_W) via static-potential extraction (Phase G template).
- Class D: hadronic mass splittings + gross atomic structure (Bohr levels). Precision spectroscopy (Lamb shift, hyperfine, g-2) requires amplitude-resolution improvements.

**Calibration architecture decision (FTD-0130 path-(b))** remains a separately-deferred ontological decision; it is NOT a blocker for the Phase B/C/D builds. The audit clears the program to proceed with ratio-based deliverables.

---

## 7. Cross-references

- **FTD-0136** (LEDGER row recording this methodological position)
- **FTD-0135** (substrate Yukawa vertex closed-negative under old framing; re-opened as Class C measurement question under reframe)
- **FTD-0129** (structural-decoupling diagnostic; re-read under reframe as evidence the question was malformed)
- **MC-T4.3** (central foundational obstruction; closure path widened beyond action-based mechanisms by this reframe)
- **FTD-0110** (Class A native observable; load-bearing for cluster-mass identification)
- **FTD-0004 / Phase G** (Class C native observable for static EM point sources; load-bearing for the discreteness model)
- **FTD-0041** (calibration ladder; required for measurement comparison)
- **FTD-0027** (pre-registration discipline; required for engine-measurement campaigns under this program)
- **CHECKLIST_MATH_COMPLETE** (Tier IV items reframed; Tier III items unaffected)

---

## 8. Open questions surfaced by the reframe

1. **What is the discrete-native equivalent of "particle"?** Cluster (FTD-0110) is the operational answer, but is a single-cluster always identifiable with one SM particle, or can multi-cluster bound states be identified with composite particles (hadrons)?
2. **What is the discrete-native equivalent of "vacuum"?** Likely the equilibrium void state (`s=0` everywhere with thermalized `J`), but this needs explicit specification before Class C measurement protocols can be hash-locked.
3. **What is the discrete-native equivalent of "external probe"?** SM scattering experiments use idealized incoming/outgoing plane-wave states; the engine has no plane waves. The cluster-cluster scattering protocol must replace plane waves with concrete cluster initial conditions. How is the result Lorentz-transformed back to lab-frame measurements?
4. **Is the discrete substrate Lorentz-invariant at the measurement level?** The lattice manifestly breaks Lorentz invariance at the substrate level. Existing measurements constrain the substrate-level breaking to be unobservably small. The discrete-native infrastructure must respect this constraint.

These are honest open questions, not architectural blockers. They are noted here so that the first Class B/C/D infrastructure builds can address them as they arise.

---

**Authoring note (per CLAUDE.md F1/F9 + GTCA F9):** the reframe is structurally legitimate but creates the F9 risk that any future "discrete-native derivation" is rubber-stamped because it bypasses SM-shaped scrutiny. The discipline that prevents this is §6.3 (the falsifiability surface) and §4.1 (the tagging discipline distinguishing measurement-comparison from theoretical-comparison). Future work under this program must hold both.
