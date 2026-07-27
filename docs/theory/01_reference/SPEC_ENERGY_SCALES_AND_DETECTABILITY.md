# SPEC — Energy Scales & Detectability: where the FTD lattice sits, and what it can reasonably predict

**Tag:** `[SYNTHESIS]` — an energy-scale *lens* on already-tagged claims. **No row is promoted by inclusion here**; every claim carries its source tag, and the LEDGER wins on any disagreement (precedence: LEDGER > constitution > this doc).
**LEDGER:** registry row (this synthesis).
**Companions:** [`SPEC_DIMENSIONAL_MAP.md`](SPEC_DIMENSIONAL_MAP.md) (the calibration register), [`SPEC_PREDICTIONS_FORWARD_2026.md`](SPEC_PREDICTIONS_FORWARD_2026.md) (FP/EP rows), [`SPEC_PREDICTION_LEDGER_DEVIATIONS.md`](SPEC_PREDICTION_LEDGER_DEVIATIONS.md) (PL-1..PL-6), [`SPEC_FTD_FRAMEWORK_V1.md`](SPEC_FTD_FRAMEWORK_V1.md) (constitution).
**Computed numbers:** every figure below is computed by [`scripts/exploration/energy_scales_2026.py`](../../../scripts/exploration/energy_scales_2026.py) (SHA256 `15d01ccca5a635aa3d24ec2608e5796d0e8218aa347a86f3fc0023aa3dcdc464`) — none is recalled or hand-copied from prose. Framework constants are imported from `scripts/constants.py`; SI/CODATA constants (ℓ_P, c, ℏc) are declared per [`REF_EXTERNAL_CONSTANTS.md`](../../reference/REF_EXTERNAL_CONSTANTS.md) (CODATA 2022 / PDG 2024).

---

## §0 · The framing — a Planck-scale instrument, and the detectability map

### The crux

The calibration register fixes `a_phys ≡ ℓ_P`: **one voxel = one Planck length** (1.616255×10⁻³⁵ m), theorem-enforced as the irreducible minimum (FTD-0059 no-go), not a convenience choice (`SPEC_DIMENSIONAL_MAP.md` §4). The lattice is therefore a **Planck-scale instrument** — it already resolves the smallest length there is, but can only hold a tiny patch of it at once.

CERN sits at the *opposite* extreme. The LHC uses energy as resolution: 13.6 TeV resolves `ℏc/E = 1.451×10⁻²⁰ m`, which is **8.98×10¹⁴ voxels** wide. The largest practical lattice (L=256) spans only `256·ℓ_P = 4.14×10⁻³³ m` — about **3.5×10¹² times shorter than one LHC resolution element** (and one resolution element is itself ~9×10¹⁴ voxels wide, with a full collision region spanning many such elements). Direct simulation of a CERN collision is therefore infeasible by many orders of magnitude. In energy terms the LHC is deep IR relative to the substrate: `E_LHC / E_Planck = 1.1×10⁻¹⁵`.

So the honest statement is **not** "the engine sees what CERN sees, only smaller." CERN probes the **IR particle structure** at energies far below the cutoff; the engine sits at the **UV (Planck) substrate**. The bridge between them is the part of physics that is scale-free or scales in a known way: **dimensionless ratios**, the **dispersion/cutoff** of the one wave sector, **UV-suppressed deviations** that die as powers of `E/E_P`, and **structural nulls**.

### Calibration register (verbatim, `SPEC_DIMENSIONAL_MAP.md` §4)

| Anchor | Formula | Value | Tag |
|---|---|---|---|
| a_phys | 1 voxel ≡ ℓ_P | 1.616255×10⁻³⁵ m | `[CALIBRATION]` |
| t_phys | ℓ_P/(√3·c) (edge gauge, selected c_lat=1/√3) | 3.113×10⁻⁴⁴ s | `[CALIBRATION]` |
| mass unit | K_B = m_e | 1 MeV/c² per K_B | `[IMPOSED]` |

> **Conversion note (load-bearing).** The energy-scale predictions in §1–§3 route through `E_P = ℏc/ℓ_P` (= `M_PLANCK` = 1.220890×10¹⁹ GeV, cross-checked two independent ways to 1.2×10⁻⁷) and the mass-unit `K_B`. With the current edge-gauge calibration, `a_phys/t_phys = √3 c` and the selected lattice wave speed gives `c_lat a_phys/t_phys = (1/√3)(√3c)=c`. The map `q = E/E_P` is the leading low-q relation; finite-q energy uses the exact arcsin pole below.

### What the engine CAN vs CANNOT detect

| Detectable (engine delivers) | Tag | Not detectable (engine does NOT deliver) |
|---|---|---|
| Dimensionless ratios (mass ratios, couplings, mixing angles) | `[various]` | A CERN collision's dynamics (≈10¹⁵ voxels — infeasible) |
| The one wave-sector dispersion + UV cutoff | `[MEASURED]` (FTD-0299) | CERN-scale resolution at feasible L |
| Direct free-flux Lorentz-violation structure (`q²` boost term, `q⁴` directional spread) | `[DERIVED/MEASURED, sector-scoped]` (FTD-0407/PL-5) | Interacting/common-cone Lorentz recovery (open) |
| The emergent cluster mass/energy ladder (in MeV via K_B) | `[SMC]` (FTD-0110/0261) | Specific SM particle masses as *derivations* (IDENT-NULL; `[PARAMETRIC]`/`[SELECTION]`) |
| Structural nulls (no monopole / SUSY / extra dimensions) | `[THEOREM]` (PL-6) | The Born rule (PL-1 is Rice), lab Bell S>2, α (MC-T4.3 obstruction) |

**Standard caveat (FTD-0258).** Engine results are substrate-level statements at the stated lattice size/stencil/protocol; physical readings are conditional on the calibration register; **dimensionless deviation structure is calibration-invariant, absolute scales are not.** A tag is a label, not a resolution.

**Anti-overclaim guard.** This doc does **not** claim: α (a `[FOUNDATIONAL OBSTRUCTION]`; `x₊=1/α` is an identification, not a prediction); the Born rule (PL-1's registered content is *Rice, not Born*); lab Bell `S>2` (substrate bound is `S≤2`, observer-layer account is `[SELECTION]+[OPEN]`); absolute scales without the calibration register; or particle masses as derivations.

---

## §1 · Free-flux dispersion → Lorentz violation (FTD-0299 / FTD-0407 / PL-5)

FTD assigns light and radio to the same free flux-wave carrier. For an axis mode, the default engine's **fully discrete** pole is:

```
theta(q) = 2 asin[c_lat sin(q/2)],   c_lat = 1/√3,
theta_max = 2 asin(1/√3),   v_g → 0 at q = π
```

FTD-0407 combines the temporal and spatial symbols and obtains `theta²=q²/3-q⁴/54-q⁶/4860+O(q⁸)` on an axis. Hence
`v_g/c = cos(q/2)/sqrt(1-c_lat² sin²(q/2)) = 1-q²/12+O(q⁴)`.
At leading order `q = E/E_P` under the calibration above, giving the direct free-pole energy dependence:

```
Delta v/c ≈ (E/E_P)² / 12       (isotropic boost violation, dimension 6)
direct free-pole dim-5 coefficient = 0  (tree-level and sector-scoped)
first cubic rotation breaking: delta(q) ≈ 6.95×10^-4 (E/E_P)^4
                                      (dimension 8, p = 4.0008 ± 0.0006)
```

| Photon E | q = E/E_P | Δv/c (dim-6) | δ_aniso (dim-8) | ToF delay over ~13 Gly |
|---|---|---|---|---|
| 1 GeV | 8.19×10⁻²⁰ | 5.59×10⁻⁴⁰ | 3.13×10⁻⁸⁰ | 2.29×10⁻²² s |
| 10 GeV (GRB) | 8.19×10⁻¹⁹ | 5.59×10⁻³⁸ | 3.13×10⁻⁷⁶ | 2.29×10⁻²⁰ s |
| 1 TeV | 8.19×10⁻¹⁷ | 5.59×10⁻³⁴ | 3.13×10⁻⁶⁸ | 2.29×10⁻¹⁶ s |
| 13.6 TeV (LHC) | 1.11×10⁻¹⁵ | 1.03×10⁻³¹ | 1.07×10⁻⁶³ | 4.24×10⁻¹⁴ s |
| E_Planck | 1.0 | 0.0833 (leading expansion only) | 6.95×10⁻⁴ | — |

**Scoped prediction:** if physical photons are exactly the uncoupled production flux mode and the Planck-spacing calibration holds, their direct tree-level time-of-flight correction is quadratic with coefficient `1/12`, while cubic directional spread starts at fourth order in `E/E_P`. The 10 GeV direct delay is about `2.3×10^-20 s`. This is **not yet a whole-theory null prediction**.

FTD-0411/0413 define a separate, default-off selected free flux/matter branch
whose q2 speed correction is cancelled through q4 in the pole. FTD-0414 gives
its leading all-direction/all-sector spread as

```
Delta v_max/c_s = (11/540) (E/E_P)^4 + O((E/E_P)^6)
```

under the same `a=ell_P` calibration. The leading term is `3.1×10^-62` at
13.6 TeV, `9.2×10^-55` at 1 PeV, and `9.2×10^-35` at `10^20 eV`. This is a
conditional free-tree estimate, not a replacement for the default flux pole
and not a protected phenomenological pass. Physical photons are not yet
identified with the selected surrogate, manifested matter is not the Wilson
spinor, and no interacting operator-mixing calculation exists. FTD-0407 shows
that CPT-even dimension-four preferred-frame operators are symmetry-allowed.
Linear absence and q4 suppression are therefore sector-scoped tree facts, not
radiatively stable whole-theory results. LR-2 through LR-5 must close before
comparison with photon-sector SME bounds can be claimed as a protected FTD
prediction.

FTD-0415 makes the last sentence exact at the symmetry level: independent
dimension-four gauge, matter, and scalar time-space kinetic ratios are allowed,
and the native vector sector admits a cubic-only marginal gradient invariant.
The missing quantity is no longer an operator inventory; it is the generated
coefficient/mixing matrix of a frozen interacting FTD action.

---

## §2 · Manifestation / pair-production threshold

The engine's matter-creation energy scale is the genesis threshold:

```
K_GENESIS = N_c · K_B = 1.533 MeV      (fill all N_c color channels) [IMPOSED]
QED pair-production threshold 2·m_e   = 1.022 MeV
ratio  K_GENESIS / (2 m_e) = 1.500     (= 3/2)
```

The engine's lowest stable-state nucleation energy is ~1.5 MeV (3 electron masses), against the QED pair-production threshold of 2 electron masses. The genesis dynamics are a one-shot burst (~5 events at A=10, FTD-0267 `SURVIVAL-NULL`), not a survival-driven population. **Honest tag:** the *engine threshold* `K_GENESIS = 3K_B` is `[IMPOSED]` (calibration kinetics, FTD-0130); the *physical identification* of it with a pair-production threshold is `[CONJECTURE]` / calibration-dependent — the factor 3-vs-2 is **flagged, not claimed** (no numerological identification, anti-target rule).

---

## §3 · The emergent mass/energy ladder (N(A)·K_B; FTD-0261)

The engine's accessible energy range is the cluster spectrum. The engine inertial relation `mass = N·M_REST = N·K_B` is `[IMPOSED]` (FTD-0250; default-off `cluster_inertia`, collective-coordinate reduction `[OPEN]`); the *identification* of cluster size `N` with a physical particle mass is `[SMC]` (FTD-0110). Engine-level; the calibration register applies. On the canonical current stack (L=32, FTD-0261 run of record), the ladder is:

| A (injection) | N̄ (cluster) | E = N·K_B (MeV) | note |
|---|---|---|---|
| ~2 | 1.0 | 0.511 | electron anchor (N=1) `[MEASURED-exact]` (FTD-0262) |
| 16 | 21.6 | 11.0 | knee of the broken power law |
| 50 | 130.2 | 66.5 | |
| 70 | 260.2 | 133.0 | |
| 90 | 383.3 | 195.9 | flooding boundary (L=32, no absorber) |

**Demonstrated span at L=32: 0.511 MeV (N=1) → ~196 MeV (N=383)** — the engine can *represent* energies across roughly the electron-to-light-hadron range; larger L extends the ceiling. **Lead with the honesty:** identification of rungs with specific SM particles is **IDENT-NULL** — there is no SM-ratio specialness in the spectrum (`p_local = 2.052`, FTD-0262); the electron anchor is exact but the ladder above it is a continuum, not the SM mass set. The cluster-mass identification stays `[SMC]`; the current-stack law `N(A)` (broken power law, knee A≈16, `k_eff ≈ 0.05` — **not** the historical ¼) is `[MEASURED]` (FTD-0261), with two blind interpolations confirmed (EP-2: N(35), N(60)).

---

## §4 · Structural nulls at the frontier (PL-6 `[THEOREM]` + caveats)

The ontology *forbids* a set of energy-frontier positives — FTD's sharpest energy-scale predictions: not deviations to be measured small, but structural nulls. The clean `[THEOREM]`-grade set (PL-6's monopole / SUSY / extra-D) is each killed immediately by a single contrary observation; two related claims carry honest caveats below (sources: `SPEC_PREDICTION_LEDGER_DEVIATIONS.md` PL-6, `scripts/proofs/proof_complete_sm.py`):

| Forbidden | Reason | Experiment | Tag |
|---|---|---|---|
| N_monopole = 0 | ∇·B = ∇·(∇×J) = 0, an identity | monopole searches | `[THEOREM]` (PL-6) |
| N_SUSY = 0 | ternary state space carries no fermionic grading | collider SUSY | `[THEOREM]` (PL-6) |
| extra dimensions = 0 | the arithmetic 2^D·(D−1)!=16 has D=3 as its unique solution `[THEOREM]`; dimension-forcing is `[SELECTION — declared]`, FTD-0355 | short-range gravity / collider | `[THEOREM]` (PL-6) |
| fermion generations = 3 (no 4th) | Moore-layer decomposition yields exactly 3 | collider | `[THEOREM]` (topological) / `[OPEN]` (dynamical) |

**Two honesty caveats.** (i) **Proton decay is *not* a forced null.** PL-6 lists `τ_proton = ∞`, but the canonical LEDGER (FTD-0301) re-tags it `[MEASURED — UNFORCED-METASTABLE BOUNDARY]`: the substrate carries only the U(1) Σs charge (no baryon / B−L current), the proton is a mixed-sign `uud` cluster the triad-lock cannot protect, and FTD's *own* weak/baryogenesis sector decays it — so a forced infinite proton lifetime is **not** a prediction (`proof_complete_sm.py` tags it `[SELECTION]`; PL-6, FP-4, NP-16/NP-34, and CATALOG §16 now carry the same `[SELECTION]/[BOUNDARY]` retag per FTD-0301). (ii) The three-generation count is `[THEOREM]` topologically but `[OPEN]` dynamically (`CATALOG_PARAMETRIC_INSERTIONS.md` §16) — an addition beyond PL-6's registered four.

This is the framework's most robust energy-frontier signature: the world's existing null-search program **is** the test, and every clean null (monopole, SUSY, extra dimensions) supports FTD against frameworks (GUTs, strings) that require positives. Cross-ref FP-4 (which likewise cross-refs PL-6 rather than re-grading it).

---

## §5 · Maintenance

Rows are added or updated only with: a source doc at its canonical tag + the computed-numbers artifact (`energy_scales_2026.py`, hash above). This doc introduces **no derivations and runs no searches** — it re-frames already-tagged results through the energy-scale lens and supplies the scale arithmetic. When a source row is retagged, the LEDGER is edited first; this doc is annotated second. **Conflict precedence: LEDGER > constitution > this doc.**
