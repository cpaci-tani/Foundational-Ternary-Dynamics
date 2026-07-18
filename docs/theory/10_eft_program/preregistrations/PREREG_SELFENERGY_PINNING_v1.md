# PREREG: Self-Energy Pinning — Does the Gauss Projector's Fixed Point Assign K_MANIFEST a Substrate-Geometric Origin?

**Status:** [PRE-REGISTRATION — LOCKED 2026-07-17, tag `preregister-selfenergy-pinning-v1`]. Predictions frozen 2026-07-16, computed before any engine measurement of the target observable existed; lock cut on owner execution instruction of 2026-07-17 ("lets get the self energy measurement"), per LOCK-STD v1. Manifest row: `docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md` (Self-energy pinning section). Execution record appended below (§9) after the run; no frozen content above §9 was modified post-lock.
**Provenance:** the surviving lead of the 2026-07-16 dual adversarial review (math + physics redteams) of the state-sector action proposal — see `docs/theory/03_derivations/foundational_mechanics/EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS.md` §9 item 7 for the refutation record. The value-match form of the conjecture ("K_B ≈ W_SC, 1.1%") was ruled inadmissible ([COORDINATE COINCIDENCE] class, AUDIT_MASS_CHAIN_REDTEAM Axis B precedent); only the prescriptive form below is registered.
**Prediction generator (recomputable):** `scripts/proofs/prereg_selfenergy_pinning_predictions.py`.

---

## §1. Registered hypothesis (prescriptive, no value asserted)

> **H:** The self-energy of the Gauss projector's unit-charge fixed point — the flux configuration the engine's own constraint machinery builds around a single manifested voxel, measured in the pinned convention of §2 — is a substrate-geometric constant, and `K_MANIFEST` (the genesis/evaporation kinetics scale, currently `= K_B = 0.511` by the FTD-0130 role fusion) should be identified with it.

H asserts no number in advance. The measurement determines which operator family the projector realizes (§3), and the measured value — whichever it is — becomes the sole candidate for the identification. What H would buy if adopted: one of `K_B`'s four FTD-0130 roles (`K_MANIFEST` only; `M_REST` stays theorem-fused to the mass anchor) moves from [IMPOSED — calibration] to [DERIVED — substrate geometry, candidate], at the price of an owner-adjudicated redefinition with engine-behavioral consequences (§5).

## §2. Frozen conventions (D-CONV — no execution-time definitions)

- **Canonical functional for H:** the tracker convention `E_half = ½·Σ_sites |J|²` of the settled projector-only field. Pinned now; the redteam established the three live conventions give three different constants, so H is meaningful only with one declared.
- **Cross-check functionals (frozen, reported alongside, not adjudicating):** `E_local7 = Σ_{site+6 face nbrs} |J|²` (the evaporation rule's own reading) and the pairs-once Term-6 gradient energy `E_term6 = (c²/2)·Σ_pairs w|ΔJ|²` (the action's functional; note the coded per-site diagnostic double-counts pairs — chip task_fdba30eb).
- **Charge configuration:** single `s = +1` voxel at lattice center, all other sites void; periodic boundary; mean-charge neutralization as engine-native (`ρ = s − mean_charge`).
- **Lattice sizes:** L ∈ {17, 33, 65} (odd — no checkerboard null modes; the central-difference constraint is exactly satisfiable, so the finite-L predictions are exact numbers, not fits).

## §3. Frozen predictions

Two candidate operator families; exact finite-L values (machine precision, k-space):

| L | **P1** central-difference family: `E_half` | P1 `E_local7` | P1 implied `p_evap`/tick | P1 `E_term6` | **P2** matched 18-pt family: `E_half` |
|---|---|---|---|---|---|
| 17 | **0.478917129** | 0.666395305 | 7.792×10⁻³ | 0.642599213 | **0.151842301** |
| 33 | **0.491780491** | 0.666629565 | 7.785×10⁻³ | 0.657863335 | **0.155069378** |
| 65 | **0.498515103** | 0.666661812 | 7.784×10⁻³ | 0.665790987 | **0.156754562** |
| ∞ | W_SC = 0.5054620197 | 2/3 | 7.78×10⁻³ | (4/3)·W_SC = 0.67395 | ½·G18(0) ≈ 0.15849 |

P1 is the fixed point if the projector's composite (18-pt solve, central-difference divergence measure, central-difference gradient correction) converges onto the central-difference constraint surface — the analytic expectation, since the correction lives in the range of the central-difference gradient. P2 is the fixed point a genuinely matched-stencil solve (FTD-0350 apparatus) would realize. **Discrimination gap at L=65: 0.342 absolute (3.2×)** — the outcome cannot be ambiguous by noise or tolerance choice. W_SC is the *simple-cubic* Watson integral; it is NOT the algebraic spine's W₃ (BCC, 1.3932), and no relation to the spine is asserted or available (W_SC is Γ(1/24)-class, outside ℚ(G*, π) — the corpus's E1 frontier).

## §4. Measurement protocol (engine of record, not Python)

1. C++ engine (CPU SOR or GPU FFT path; record which — both must be reported if they disagree). Configure: single `s=+1` at center, `J = wave_vel = 0` everywhere, ALL dynamics toggles off except `gauss_projection` (projector-only; the GF-A isolation pattern of `test_gauss_law_fidelity.cpp`).
2. Apply the projection repeatedly until convergence: `max|div_c J − (s − mean)| < 10⁻⁸`, or 10⁴ applications, whichever first. **A run that does not meet the residual criterion is INVALID for adjudication** (vacuity firewall, §6) and triggers mechanism investigation instead.
3. Measure `E_half`, `E_local7`, `E_term6` per §2 at each L ∈ {17, 33, 65}.
4. Adjudication tolerance (frozen): match = within **0.5%** relative of a family's exact finite-L value at ALL three L simultaneously. The 3.2× inter-family gap makes the tolerance uncritical; it exists to catch partial convergence masquerading as agreement.

## §5. Frozen outcome map

- **OUTCOME-P1** (all three L match P1): the projector realizes the central-difference fixed point; the candidate identification is `K_MANIFEST := E_half → W_SC = 0.50546…` — H **advances to owner adjudication** as [DERIVED — substrate geometry, CANDIDATE]. Adoption is NOT automatic: it is a redefinition decision (an adoption, never a derivation of the current 0.511) with engine-behavioral consequences — `K_GENESIS = 3·K_MANIFEST → 1.51639`, a golden-hash-breaking change requiring the merge-gate/golden-gate discipline — and it must be priced as a registered line (the identification itself remains a declared type: the substrate forces the *value of the self-energy*; identifying the kinetics scale *with* it is the adopted commitment).
- **OUTCOME-P2** (all three L match P2): the projector realizes the matched-18-pt fixed point; the W_SC attribution is **CLOSED for this engine** (FTD-0116 protocol: analytic attribution killed by the measured stencil), and P2's value (0.1568 at L=65, → 0.1585) becomes the sole candidate under the same adjudication path. Note this value sits INSIDE the §7 stability window — a consonance to record if realized, not evidence.
- **OUTCOME-C** (neither family within 5× the tolerance at any L, with the §4 residual criterion met): **INDETERMINATE** — the projector realizes neither idealized operator; mechanism investigation required (SOR truncation, boundary, the coupling-source interference measured by the Gauss-fidelity chip); no adoption, no closure, and H returns to [OPEN] with the measured value logged.
- **Kill condition for the line:** if after mechanism investigation the measurement is confirmed sound and matches neither family, the "self-energy origin of K_MANIFEST" line **CLOSES NEGATIVE permanently** (FTD-0116 precedent governs re-attempts).

## §6. Vacuity firewall

Invalid (non-adjudicating) runs: residual criterion unmet; even L substituted (null-mode ambiguity); any dynamics toggle active during settling; convention substitution after data (the §2 pinning is final); tolerance revision after data; comparing against the ∞ limits instead of the exact finite-L values. The Python generator is the prediction side only — under `feedback_measurement_platform`, the C++ engine is the canonical measurement and Python results adjudicate nothing.

## §7. Scope, limits, standing rails

- H prices the **idealized constraint's** cost. The live engine currently does not sustain this fixed point (measured fidelity at charge sites f = −0.095, wrong-signed — the coupling source opposes the projector; under investigation, chip task_92dc33a4). H's relevance to *live* dynamics is conditional on that investigation's resolution; this prereg adjudicates only what the projector-only machinery builds.
- H touches `K_MANIFEST` only. `M_REST = m_e` (mass anchor) is untouched; the MeV calibration remains closed by FTD-0059/0096; the value-match numerology (W_SC vs 0.511, 1.1%) remains inadmissible under every outcome; nothing here bears on `x₊ = 1/α` or any spine claim.
- Prior art: the corpus computes the W_SC sum in `scripts/proofs/proof_partition_function_gstar.py` (as `watson_sc_origin`); the exact folding identity `E_half(L) = S(L/2)` (even L) and the odd-L exact values are the 2026-07-16 session's [THEOREM]-grade additions; the redteam record and refutation of the surrounding action proposal are in the EXPLR doc §9 item 7.

## §8. Execution

To be run after (or coordinated with) the Gauss-fidelity chip (task_92dc33a4), whose findings determine whether a projector-convergence obstruction exists in the current code path. Execution requires the lock to be cut first (owner: tag + manifest row). Results are booked against this document's outcome map with no post-hoc reinterpretation.

---

## §9. Execution record

### §9.1 Run 1 (2026-07-17, lock commit `66a830ac`, tag `preregister-selfenergy-pinning-v1`)

Driver: standalone C++ over the engine's `gauss_project_cpu` (`engine/src/poisson_solvers.cpp`), compiled from source with no engine-tree edits (WSL2 g++ -O3 -std=c++17 -fopenmp, OMP 24 threads); driver SHA256 `abfd727da466fdc1f994108efaca4055867815cd55076b907af3d9b844213572`. Protocol exactly as locked (§4): projector-only, single s=+1 at center, engine defaults, 6 SOR iterations/application, cap 10⁴ applications, residual gate 10⁻⁸.

| L | applications | final residual | gate met? | E_half measured | P1 frozen | Δ vs P1 | Δ vs P2 | E_local7 | E_term6 |
|---|---|---|---|---|---|---|---|---|---|
| 17 | 1,380 | 9.238×10⁻⁹ | **YES** | 0.478916856 | 0.478917129 | **−0.00006%** | +215.4% | 0.666395293 | 0.642598853 |
| 33 | 4,300 | 9.922×10⁻⁹ | **YES** | 0.491779402 | 0.491780491 | **−0.0002%** | +217.1% | 0.666629552 | 0.657861882 |
| 65 | 10,000 (cap) | 7.037×10⁻⁸ | **NO** | 0.498486463 | 0.498515103 | (−0.006%, descriptive only) | (+218.0%) | 0.666661718 | 0.665752170 |

**Validity ruling (per §4/§6, applied as written):** L=17 and L=33 are VALID and match P1 far inside the 0.5% tolerance; the P2 family is excluded at >215%. L=65 did not meet the residual gate at the application cap and is therefore **INVALID for adjudication** — its descriptive value is not used. OUTCOME-P1 requires all three L and is **not declared on run 1**.

**Mechanism (the §6-triggered investigation, resolved):** the L=65 residual trace is clean geometric decay (×≈0.727 per 500 applications at the cap, no floor), with convergence controlled by the near-checkerboard modes of the mismatched composite (rate ∝ sin²(π/L) ⇒ applications ∝ L²: measured 1,380 → 4,300 → ≈16,000 projected). The 10⁴ cap was mis-calibrated for L=65; this is an iteration-budget miss, not an operator pathology or measurement floor.

**Non-adjudicating diagnostic:** `exact_dual_gauss = true` is bit-identical to the default at all three L (identical E_half, E_local7, residual traces) — for a symmetric isolated charge the skip-manifested-sites correction rule is a no-op (∇φ at the symmetric center is zero regardless). Relevant to the Gauss-fidelity investigation: the skip rule is not a projector-level under-enforcement mechanism for an isolated charge.

## §10. Amendment v1.1 (procedural; tag `preregister-selfenergy-pinning-v1-1`)

**Sole change:** the L=65 application cap is raised from 10⁴ to **2.5×10⁴**, justified by the measured L² convergence scaling above (projected convergence ≈1.6×10⁴ applications). **Nothing else changes**: predictions, conventions, tolerances, residual gate, outcome map, and the run-1 booking above are all untouched; L=17 and L=33 are not re-run (they adjudicated validly under v1). The amendment is cut as its own tagged lock BEFORE the L=65 re-run; a stopping-resource extension cannot alter the fixed point the projector converges to, only whether the pre-set gate is reached.

### §10.1 Run 2 (L=65 only, under v1.1)

(appended after the re-run)
