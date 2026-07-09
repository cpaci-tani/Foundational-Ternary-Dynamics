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
| t_phys | ℓ_P/(√3·c) (CFL, c_lat=1/√3) | 3.113×10⁻⁴⁴ s | `[CALIBRATION]` |
| mass unit | K_B = m_e | 1 MeV/c² per K_B | `[IMPOSED]` |

> **Conversion note (load-bearing).** The energy-scale predictions in §1–§3 route through `E_P = ℏc/ℓ_P` (= `M_PLANCK` = 1.220890×10¹⁹ GeV, cross-checked two independent ways to 1.2×10⁻⁷) and the mass-unit `K_B` — **both independent of the tick↔second convention**. So the dimensionless map `k = E/E_P` is unambiguous regardless of the `√3`/`t_phys` factor. (Note the naive grid speed `a_phys/t_phys = c/√3` does *not* reproduce `c` — a known subtlety of the canonical time calibration, FTD-0041 / `SPEC_DIMENSIONAL_MAP.md` §4; the §1–§3 predictions are immune precisely because they route through `E_P`, not `t_phys`.) Absolute *time* conversions (Hz, seconds) use the canonical `t_phys` and inherit its convention.

### What the engine CAN vs CANNOT detect

| Detectable (engine delivers) | Tag | Not detectable (engine does NOT deliver) |
|---|---|---|
| Dimensionless ratios (mass ratios, couplings, mixing angles) | `[various]` | A CERN collision's dynamics (≈10¹⁵ voxels — infeasible) |
| The one wave-sector dispersion + UV cutoff | `[MEASURED]` (FTD-0299) | CERN-scale resolution at feasible L |
| UV-suppressed Lorentz-violation structure (k⁴ anisotropy, no linear term) | `[MEASURED]` (PL-5) | Absolute dimensional scales without the calibration register |
| The emergent cluster mass/energy ladder (in MeV via K_B) | `[SMC]` (FTD-0110/0261) | Specific SM particle masses as *derivations* (IDENT-NULL; `[PARAMETRIC]`/`[SELECTION]`) |
| Structural nulls (no monopole / SUSY / extra dimensions) | `[THEOREM]` (PL-6) | The Born rule (PL-1 is Rice), lab Bell S>2, α (MC-T4.3 obstruction) |

**Standard caveat (FTD-0258).** Engine results are substrate-level statements at the stated lattice size/stencil/protocol; physical readings are conditional on the calibration register; **dimensionless deviation structure is calibration-invariant, absolute scales are not.** A tag is a label, not a resolution.

**Anti-overclaim guard.** This doc does **not** claim: α (a `[FOUNDATIONAL OBSTRUCTION]`; `x₊=1/α` is an identification, not a prediction); the Born rule (PL-1's registered content is *Rice, not Born*); lab Bell `S>2` (substrate bound is `S≤2`, observer-layer account is `[SELECTION]+[OPEN]`); absolute scales without the calibration register; or particle masses as derivations.

---

## §1 · Dispersion → Lorentz violation (lab-facing; FTD-0299 / PL-5)

The substrate carries **one** wave sector — light and radio are the same flux wave, differing only in `k` — with dispersion, exact to the engine's own 18-point stencil eigenvalue (FTD-0299 `[LIGHT-CONFIRMED]`):

```
ω(k) = 2c·|sin(k/2)|,   c = 1/√3,   zone-edge cutoff ω_max = 2/√3 (v_g → 0 at k = π)
```

The isotropic continuum expansion (`AUDIT_LORENTZ_ANISOTROPY.md` §2.4) is `ω² = c²k² − (c²/12)k⁴ + O(k⁶)`, so the group velocity is `v_g/c = cos(k/2) ≈ 1 − k²/8`. A photon of energy `E` maps to `k = E/E_P` directly from `a_phys = ℓ_P` and the zone-scale quantum `E_P = ℏc/ℓ_P` (independent of the tick convention — see the §0 note), giving the **energy-dependent light speed**:

```
Δv/c ≈ (E/E_P)² / 8        (isotropic, dimension-6)
linear (dim-5) coefficient = 0   ←  the structural prediction: FTD FORBIDS linear Lorentz violation
first rotation-breaking:  δ(k) ≈ 6.95×10⁻⁴ · (E/E_P)⁴   (dimension-7, p = 4.0008 ± 0.0006)
```

| Photon E | k = E/E_P | Δv/c (dim-6) | δ_aniso (dim-7) | ToF delay over ~13 Gly |
|---|---|---|---|---|
| 1 GeV | 8.19×10⁻²⁰ | 8.39×10⁻⁴⁰ | 3.13×10⁻⁸⁰ | 3.4×10⁻²² s |
| 10 GeV (GRB) | 8.19×10⁻¹⁹ | 8.39×10⁻³⁸ | 3.13×10⁻⁷⁶ | 3.4×10⁻²⁰ s |
| 1 TeV | 8.19×10⁻¹⁷ | 8.39×10⁻³⁴ | 3.13×10⁻⁶⁸ | 3.4×10⁻¹⁶ s |
| 13.6 TeV (LHC) | 1.11×10⁻¹⁵ | 1.55×10⁻³¹ | 1.07×10⁻⁶³ | 6.4×10⁻¹⁴ s |
| E_Planck | 1.0 | 0.125 | 6.95×10⁻⁴ | — |

**The prediction** (cross-ref FP-3): continued **null results** in all dimension-5 and dimension-6 photon-sector Lorentz-violation searches (GRB time-of-flight, vacuum birefringence). A 10 GeV GRB photon's arrival delay is ~10⁻²⁰ s against timing resolutions of ~ms — unobservable by ~17 orders of magnitude. The *content* is the structure, not the (tiny) numbers — and the two nulls have **different origins**: the dimension-5 (linear) term is **structurally absent** (coefficient exactly 0 — FTD forbids it), while the dimension-6 isotropic term **exists** (coefficient 1/8) but is **Planck-suppressed** to ~10⁻⁴⁰ at lab energies; rotation-breaking is deferred to quartic (dim-7) order. **Falsifier:** a confirmed detection of *linear* (or quadratic-Planck) photon-sector LV contradicts the FTD wave sector (which forbids anything stronger than quartic anisotropy); engine-side, the k⁴ law reversing or plateauing at larger L kills PL-5 (see EP-3, already verified to L=768). Tag: dispersion `[MEASURED]` (FTD-0299); anisotropy PL-5 `[MEASURED]` (the closed-form continuum expansion and k⁴ law it rests on are exact stencil-eigenvalue algebra); the lab-facing null `[PREDICTION]` (FP-3).

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
