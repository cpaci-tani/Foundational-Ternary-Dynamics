# PROTOCOL — First Measured Native Operator-Mixing Matrix

**Tag:** [PROTOCOL · pre-registration]
**Date:** 2026-04-26
**Author:** FTD EFT program
**LEDGER row:** FTD-0098 (assigned ahead of measurement)
**Supersedes:** none — first measurement of its kind in this codebase.
**Companion docs:** [`SPEC_OPERATOR_BASIS.md`](SPEC_OPERATOR_BASIS.md), [`SPEC_FTD_NATIVE_BLOCKING_MAP.md`](SPEC_FTD_NATIVE_BLOCKING_MAP.md), [`AUDIT_OPERATOR_SPECTRUM.md`](AUDIT_OPERATOR_SPECTRUM.md), [`PROTOCOL_BETA_MEASUREMENT.md`](PROTOCOL_BETA_MEASUREMENT.md).

This protocol is **pre-registered before measurement** per CLAUDE.md epistemic discipline rules and per the [PARTIAL] outcome of [`AUDIT_OPERATOR_SPECTRUM.md`](AUDIT_OPERATOR_SPECTRUM.md), which warned that post-hoc bracket-fitting weakens classification claims. All thresholds, basis choices, definitions, and acceptance criteria below are committed prior to seeing any campaign output.

---

## 1 · Why this protocol exists

[`STATUS_EFT_CHECKLIST.md`](STATUS_EFT_CHECKLIST.md) names exactly one explicit "next milestone" (lines 222–225):

> "The next milestone should be the nonlinear operator-flow campaign: consume `RenderBridge::continuity_step()` histories, block them, compute the operator moment vector before/after blocking, and assemble the first measured native mixing matrix."

This protocol pre-registers the design of that measurement. The deliverable is the first measured native operator-mixing matrix `M_ab(b=2)` in this codebase, derived from b=2-blocked Langevin+genesis ensemble snapshots.

It addresses the [OPEN] checklist rows:

- §5 line 78: "Define operator mixing matrix from blocked full-history ensembles."
- §5 line 79: "Classify relevant, marginal, and irrelevant directions from measured native flow." (currently [PARTIAL] from `AUDIT_OPERATOR_SPECTRUM.md`).
- §6 line 88: "Measure operator mixing under blocking."
- §6 line 86: "Build systematic nonlinear b=2 flow campaigns from engine histories." (partial seed.)
- §9 line 127: "Connect the action/measure to the observed operator-flow matrix."

---

## 2 · Pre-registered operator basis

Six operators from [`SPEC_OPERATOR_BASIS.md`](SPEC_OPERATOR_BASIS.md) §2:

| ID | Operator | Naive Δ | Native discretization on `DualCellFields` |
|---|---|---|---|
| O1 | J · J            | 2 | `J_α(i) = ½(phi_α[i] + phi_α[i − e_α])`, sum over α |
| O2 | (∇·J)²           | 4 | `div_face_at(i)²` (existing helper) |
| O3 | (∇×J)·(∇×J)     | 4 | central-difference curl on cell-centered J |
| O4 | J · ∇(∇·J)       | 5 | central-difference gradient of `div_face_at` |
| O5 | (J·J)²           | 4 | square of O1 |
| O6 | s · s            | 2 | `rho_cell[i]²` |

The Langevin+genesis ensemble drives non-zero state s, so unlike the pulse-only spectrum audit, **all 6 operators are measurable in this campaign**. If the Langevin run produces too few non-zero-state snapshots (<30%), the headline drops to a 5×5 matrix excluding O6 (R4 in plan risk register), with O6 retained in a secondary CSV.

---

## 3 · Pre-registered ensemble parameters

Mirror [`test_nonlinear_flow_multiscale.cpp`](../../../engine/tests/test_nonlinear_flow_multiscale.cpp) verbatim (proven Gaussian-flow ensemble, 21/21 ctests PASS on RTX 5090, 2026-04-26):

| Parameter | Production | Smoke |
|---|---|---|
| Lattice size L | 16 | 8 |
| Burn-in N_BURN | 200 | 50 |
| Samples per seed N_SAMPLES | 40 | 4 |
| Sample stride | 5 ticks | 5 ticks |
| Number of seeds N_SEEDS | 5 | 1 |
| Total samples N_total | 200 | 4 |
| Toggles | wave + gauss + genesis + langevin | same |
| Langevin T | 0.005 | 0.005 |
| Langevin γ | 0.02 | 0.02 |
| Dual substrate | OFF | OFF |
| Initial seed flux | `(3·K_GENESIS, 0, 0)` at lattice center | same |
| Per-seed RNG seed | 0xF10412E5 + s·0x100 (deterministic) | s=0 only |

Backend: GPU (RTX 5090, WSL2 build at `engine/build_wsl/`), per CLAUDE.md GPU mandate for measurement campaigns.

---

## 4 · Pre-registered mixing-matrix definition

For each snapshot k of seed s, compute the operator moment vectors

```
M_fine[s,k][a]   = (1/N_fine)   Σ_{voxels of fine}   O_a(fine_fields)
M_coarse[s,k][a] = (1/N_coarse) Σ_{voxels of coarse} O_a(coarse_fields)
```

where `coarse_fields = block_dual_cell_b2(fine_fields)` and `N_fine = L³`, `N_coarse = (L/2)³`.

After collecting all `N_total` snapshots, the operator-mixing matrix `M_ab(b=2)` is defined as the regression-coefficient matrix in:

```
M_coarse_a  =  Σ_b  M_ab(b=2) · M_fine_b   +   ε_a            (*)
```

solved via the Wilsonian normal equations:

```
Σ_ab(b=2) = ⟨ ΔM_coarse_a · ΔM_fine_b ⟩         (cross-covariance)
S_bc      = ⟨ ΔM_fine_b   · ΔM_fine_c ⟩         (fine-only auto-covariance)

M_ac(b=2) = Σ_ab · (S^{-1})_bc                   (coefficient matrix)
```

where `Δ` denotes mean-subtraction across the full `N_total` ensemble. `S` is inverted via Gauss-Jordan with partial pivoting (inline; condition number reported).

### Bootstrap error bars

Resample the `N_total` snapshots with replacement, 100 times. Recompute `M_ab(b=2)` for each resample. Per-entry stderr is the standard deviation of the resampled `M_ab` distribution.

### Headline outputs

- `M_ab(b=2)` — 6×6 mean matrix
- `σ(M_ab)` — 6×6 bootstrap stderr matrix
- Diagonal entries `M_aa` reported as approximate eigenvalues (basis is expected near-diagonal at the Gaussian fixed point per [`AUDIT_OPERATOR_SPECTRUM.md`](AUDIT_OPERATOR_SPECTRUM.md) diagnostic)
- `cond(S)` (condition number of fine-only covariance)

---

## 5 · Pre-registered acceptance thresholds

| Gate | Threshold | If passed | If failed |
|---|---|---|---|
| Q conservation | `|Q_coarse − Q_fine| = 0` per snapshot, exact integer | continue | abort campaign with diagnostic |
| Gauss residual | `max\|D'Φ' − Q'\| < 1.0` per snapshot (loose tolerance from existing test) | accept snapshot | drop snapshot from ensemble; report `% dropped` in meta.json |
| Bootstrap convergence | per-entry σ(M_ab) / |M_ab| < 30% for ≥30 of 36 entries | LEDGER tag = [MEASUREMENT] | LEDGER tag = [PARTIAL] |
| Eigenvalue diagnostic | M_aa real and finite for all 6 operators | append eigenvalue classification to ANALYSIS | report ill-conditioning as a finding |
| Diagonal dominance | ≥4 of 6 operators with `\|M_aa\| / Σ_b\|M_ab\| ≥ 0.5` | basis declared "approximately fixed-point eigendirections" | basis declared "non-trivially mixed; Wilson coefficients required for clean classification" |
| `cond(S)` | < 1e8 → use full 6×6; 1e8–1e10 → drop most-degenerate operator → 5×5; > 1e10 → report ill-conditioning | follow degradation ladder | LEDGER tag = [PARTIAL] |

### Pre-registered scaling-dimension classification

For each diagonal entry `λ_a = M_aa(b=2)`, the per-step scaling dimension is

```
Δ_a = D − log₂(λ_a)            (D = 4 spacetime dimensions)
```

Classification per [`SPEC_EFT_RECOVERY_PROGRAM.md`](SPEC_EFT_RECOVERY_PROGRAM.md) §6.1:

- **relevant**:  Δ_a < D − 0.5 = 3.5
- **marginal**:  3.5 ≤ Δ_a ≤ 4.5
- **irrelevant**: Δ_a > 4.5

The naive expectation (column "Naive Δ" in §2 above) is the comparison anchor. Stratification across the basis means at least 3 different tier assignments (rather than the [PARTIAL] result of all-relevant collapse).

---

## 6 · Out of scope (deferred)

- BCC/corner-channel inclusion — separate FTD-0093 track (`PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`).
- Trace/determinant comparison to master-quadratic Vieta data — requires its own pre-registration per `STATUS_EFT_CHECKLIST.md` §6 line 92.
- Continuum-symmetry behavior under flow — secondary observable, reserved.
- Full 12-operator basis from Phase-3 SPEC — operators beyond the 6 here aren't implemented in `operator_spectrum.h` yet.
- Multilatitude (L ≥ 64) classification — stretch goal only; pre-registered to fall back to a follow-up campaign if not reached.
- Ledger-history (continuity-step) channel-conditioned moments — operators here are static-snapshot functions of (s, J) only.

---

## 7 · Output artifacts

Result directory: `engine/results/operator_mixing_2026-04-26/`

| File | Content |
|---|---|
| `meta.json` | Campaign name, commit SHA, GPU/driver, wall time, M_ab + σ(M_ab) (6×6 each), eigenvalue diagnostic, condition number cond(S), Q conservation status, `% snapshots dropped`, `% snapshots with non-zero state` |
| `mixing_matrix.csv` | Headline 6×6 matrix M_ab |
| `mixing_matrix_stderr.csv` | Bootstrap σ(M_ab) per entry (6×6) |
| `per_snapshot_moments.csv` | One row per snapshot: 12 columns (6 fine, 6 coarse) for full inspection |
| `eigenvalues.csv` | Per-operator diagonal eigenvalue + Δ_a + tier (relevant/marginal/irrelevant) |
| `run.log` | Full stdout (per-seed progress, sanity checks, headline summary) |
| `ANALYSIS.md` | Post-hoc narrative (method, results, interpretation, follow-up tickets) |

---

## 7b · FTD-0099 extensions (pre-registered 2026-04-26, ahead of measurement)

After FTD-0098 closed [PARTIAL] with the 5×5 reduced subspace and small-L all-relevant-tier compression, three direct follow-ups are pre-registered before re-running:

### F1 — Multilatitude run (L = 32)

Same campaign with `--L=32` CLI flag. Identical ensemble parameters except L. Pre-registered acceptance:
- L=32 should still satisfy Q-conservation (zero violations) and the loose Gauss-residual gate (<1.0).
- s² zero-variance is expected to persist (state saturation is L-independent at this Langevin/genesis tuning); 5×5 reduced subspace expected.
- Hypothesis: at least one of the 5 measurable diagonal eigenvalues should shift such that its measured Δ_a crosses the relevant/marginal boundary (Δ = 3.5). If it does, declare basis-stratification as L-dependent. If all 5 still classify "relevant" at L=32, this confirms the audit's hypothesis that L≥64 is needed for clean tier separation.

### F5 — RG semigroup test M(b=4) ≈ M(b=2) · M(b=2)

`--b4` CLI flag enables blocking the fine snapshot twice (b=2 → b=4) per snapshot, computing both M(b=2) and M(b=4), and testing the multiplicative property predicted by the Wilsonian RG flow (composition of two b=2 steps must equal the single b=4 step on the same operator basis).

Pre-registered acceptance:
- Compute `max_relerr = max_{i,j ∈ active} |M_b4[i,j] − M_b2_squared[i,j]| / max(|M_b4[i,j]|, |M_b2_squared[i,j]|, 1e−12)` over the active subspace.
- **PASS** if `max_relerr < 0.5` (50% — generous threshold reflecting bootstrap noise at this ensemble size).
- **FAIL** if `max_relerr ≥ 0.5` — would indicate the regression-derived M is not strictly multiplicative; would constitute structural finding (basis is not closed under nonlinear blocking, or finite-sample noise dominates the test, or both).

Threshold is loose because the M(b=4) regression operates on a coarser b=4 grid (which at L=16 is just 4³ = 64 voxels vs 16³ = 4096 fine; the moment estimate is noisier by sqrt(64) ≈ 8×).

### F3 — Wilson-coefficient eigendecomposition

Diagonalize the symmetric part `(M + M^T)/2` of the headline mixing matrix on the active subspace via Jacobi rotation. Output:
- `wilson_eigenvalues[k]` for k = 0..K−1 (where K is the active-subspace dimension)
- `Δ_eig[k] = D − log₂(λ_k)` per-eigendirection scaling dimension
- `tier_eig[k]` (relevant / marginal / irrelevant)
- `wilson_eigenvectors[a,k]` 6×6 column-eigenvector matrix (NaN-padded for dropped operators)

Pre-registered acceptance:
- All eigenvalues must be finite. If complex eigenvalues appear (impossible for a symmetric input by construction; would indicate a bug), abort with diagnostic.
- This is a diagnostic-only output — no PASS/FAIL gate. The eigenvector basis IS the result. Eigenvalues that cross the relevant/marginal boundary on the eigendirection basis (when they don't in the original-operator basis) would be a stratification finding worth highlighting.

### Output additions

Same `engine/results/operator_mixing_2026-04-26/` directory; meta.json gains:
- `wilson_eigenvalues`, `wilson_delta`, `wilson_tiers` (arrays of length kNumOps)
- `include_b4`, `semigroup_test_ran`, `semigroup_max_relerr`, `semigroup_verdict`

New CSV files:
- `wilson_eigenvalues.csv`, `wilson_eigenvectors.csv`
- `mixing_matrix_b4.csv`, `mixing_matrix_b2_squared.csv` (only if `--b4` given)

### Ledger row

Single new row **FTD-0099** for the F1+F5+F3 bundle. Tag determined by the empirical outcome: [MEASUREMENT] if all three extensions cleanly land; [PARTIAL] if any of them returns ill-conditioned data.

---

## 8 · Cross-references

- Template PROTOCOL doc: [`PROTOCOL_BETA_MEASUREMENT.md`](PROTOCOL_BETA_MEASUREMENT.md)
- Template campaign source: [`engine/tests/test_nonlinear_flow_multiscale.cpp`](../../../engine/tests/test_nonlinear_flow_multiscale.cpp)
- Template result schema: [`engine/results/gaussian_expansion_2026-04-26/meta.json`](../../../engine/results/gaussian_expansion_2026-04-26/meta.json)
- Operator basis SPEC: [`SPEC_OPERATOR_BASIS.md`](SPEC_OPERATOR_BASIS.md)
- Blocking map SPEC: [`SPEC_FTD_NATIVE_BLOCKING_MAP.md`](SPEC_FTD_NATIVE_BLOCKING_MAP.md)
- Audit closing this protocol partially supersedes: [`AUDIT_OPERATOR_SPECTRUM.md`](AUDIT_OPERATOR_SPECTRUM.md)
- Plan: `~/.claude/plans/let-s-plan-a-way-ethereal-sonnet.md` (FTD-0098 single-session implementation plan)

---

## 9 · Open questions this campaign does NOT resolve

These remain open after the campaign lands; they are flagged here for transparency and to prevent overclaiming the result:

1. Whether a nontrivial native fixed point exists (requires multi-scale flow, not just one b-step).
2. Whether continuum symmetries improve or degrade under nonlinear flow.
3. Whether the BCC corner-channel sector mixes with the 6-operator basis (requires extending operator basis with BCC observables).
4. Whether the ensemble at L=16 is large enough to resolve the marginal/irrelevant tiers — the audit recommended L≥64; this campaign does not attempt that.
5. Whether the dual-cell-form operator definitions (with face-averaging J→cell-J) systematically differ from the engine-cell form used in the spectrum audit. This is reported as a `[CONJECTURE-LEVEL ASSUMPTION]` in §2 and revisited in ANALYSIS.

---

**End of pre-registration.** No measurement code lands until this protocol is reviewable.
