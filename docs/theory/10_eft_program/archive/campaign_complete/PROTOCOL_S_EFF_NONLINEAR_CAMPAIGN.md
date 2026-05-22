# Protocol — Nonlinear effective action `S_eff` measurement campaign

**Status:** [PRE-REGISTRATION DRAFT, not yet hash-locked]
**Date:** 2026-04-29 (late evening)
**Maintainer note:** The hash-lock applies the moment this document is committed under tag `preregister-s-eff-nonlinear-v1`. Do not modify the locked sections (§§2–5) after that tag without explicit invalidation and a new vN+1 tag.

---

## 0 · Why this protocol exists

`STATUS_EFT_CHECKLIST.md` §6 marks "Build systematic nonlinear b=2 flow campaigns from engine histories" as [PARTIAL] (FTD-0098/0099/0100 first measurements landed at L ∈ {16, 32}, bootstrap-noise-limited). §9 marks "Derive the native effective action after b=2 blocking" as [PARTIAL] with the *definition* fixed —

```
exp(-S_eff[H']) = Σ_{H : B_b H = H'} exp(-S_H[H])
```

— but the *explicit measured/fitted* `S_eff` remains [OPEN]. This is the load-bearing remaining gate to a math-based EFT in FTD: bare Gaussian fixed point is closed (FTD-0070); first M_ab measurement is partial; *the connection between blocked operator-mixing matrix and the constrained-history measure `S_H` from `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` is undocumented.*

This protocol pre-registers the campaign that closes that gate at the level of [MEASURED], conditional on the locked acceptance criteria below.

---

## 1 · Scope

The campaign measures the operator-mixing matrix `M_ab(b)` at two block sizes (`b = 2`, `b = 4`) on **four distinct nonlinear ensembles** at **two lattice sizes** (`L = 32`, `L = 64`) using a **10-operator basis** that extends the FTD-0098 6-operator set with 4 explicit reaction-sector operators.

Outputs:
- The full 10×10 `M_ab(b)` matrix at both block sizes per scenario.
- Bootstrap-stderr per entry with the FTD-0098 100-resample protocol.
- Wilson-eigendecomposition diagonals classifying operator scaling.
- Cross-scenario stratification (5/6/7-of-10-operators-stratified verdict).
- RG-semigroup test `M(b=4) ≈ M(b=2)²` per scenario per `L`.
- Inferred Wilson coefficients `g_a` from `M_ab` via the procedure of §6.
- Self-consistency check: do measured `g_a` reproduce input ensemble at next blocking?

**Goal**: produce the first quantitative connection between FTD's blocked operator-mixing matrix and the constrained-history measure `S_H`. If the connection closes self-consistently within bootstrap stderr, the nonlinear effective action moves from [PARTIAL · definition fixed] to [MEASURED · self-consistent within errors at L=32, L=64]. **This is the biggest single bar between the current "Gaussian native EFT" status and a "real nonlinear math-based EFT."**

---

## 2 · Operator basis (LOCKED at hash-lock; 10 operators)

### 2.1 · Flux/state sector (existing, 6 operators)

Identical to FTD-0098 / `SPEC_OPERATOR_BASIS.md`:

| ID  | Name      | Discretisation                  | Naive Δ | Sector |
|----|-----------|---------------------------------|--------|--------|
| O1 | `JJ`      | J · J                           | 2      | flux mass |
| O2 | `divJ2`   | (∇·J)²                          | 4      | gauge-kinetic longitudinal |
| O3 | `curlJ2`  | (∇×J) · (∇×J)                   | 4      | gauge-kinetic transverse |
| O4 | `JdotDivJ`| J · ∇(∇·J)                      | 5      | derivative-contact |
| O5 | `J4`      | (J · J)²                        | 4      | flux self-interaction |
| O6 | `stateSq` | s · s                           | 2      | state mass |

### 2.2 · Reaction sector (new, 4 operators)

Defined in terms of single-tick state increment `δs(x) := s(x, t+1) − s(x, t)` measured during a `Δt = 1`-tick continuity ledger:

| ID  | Name      | Discretisation                  | Naive Δ | Sector |
|----|-----------|---------------------------------|--------|--------|
| O7 | `reactionDensity` | (δs)²                  | 2      | reaction-rate density |
| O8 | `genesisFlux`     | (δs) · θ(s_before = 0) · sign(δs) · |J_before| | 4   | from-vacuum sourcing |
| O9 | `evapFlux`        | (δs) · θ(s_before ≠ 0 ∧ s_after = 0) · |J_before| | 4 | to-vacuum sinking |
| O10| `JdotDeltaS`      | J · ∇(δs)                  | 4      | reaction-flux coupling |

`θ(·)` is the indicator (1 if condition holds, 0 otherwise). `sign(δs) ∈ {−1, +1}` carries the polarity of the new state.

**Rationale for the four.** O7 measures the total reaction rate in the cell — non-zero only when state changes occur. O8/O9 separate out genesis and evaporation channels. O10 tests cross-correlation between flux gradient and the spatial structure of reactions — the "where do reactions happen?" operator. Together they span the dim-2 to dim-4 reaction-sector landscape.

The four reaction operators are **deliberately absent from the FTD-0098 basis** because that basis was flux-and-state-only at fixed snapshot. The reaction sector requires *temporal* difference data (the per-tick continuity ledger from `RenderBridge::continuity_step()`), which is engine-side new-ish but already exposed.

### 2.3 · What this basis is *not*

- No fermion bilinears (no native fermions; FTD-0075 closed negative).
- No Chern-Simons J · (∇×J) (CP-violating, parity-symmetric expected zero).
- No off-axis composite J · s · J (already non-local; reserved for post-campaign).
- No mover/transport observables specific to a single channel (those are the engine-rule operators of `STATUS_EFT_CHECKLIST.md` §5; tracked separately).

LOCKED at 10 operators.

---

## 3 · Scenarios (LOCKED at hash-lock; 4 ensembles)

Each scenario specifies the toggle set, IC, and Langevin parameters. All scenarios share `Δt = 1`, `gauss_projection = true` (matched-stencil CG), and the same pre-burn-in protocol of `N_burn = 200` ticks before sampling.

| Scenario | Toggles                                          | IC                          | T_Langevin | γ     | Purpose |
|---------|--------------------------------------------------|-----------------------------|-----------|-------|---------|
| S1: `langevin-pure`  | `wave_propagation`, `langevin`, `gauss_projection` | random Gaussian J, s ≡ 0    | 0.005    | 0.02  | Reaction sector quiet (s ≡ 0 throughout); flux-only stratification baseline. |
| S2: `genesis-rich`   | + `genesis`                                       | random Gaussian J, s ≡ 0    | 0.005    | 0.02  | Genesis events drive s ≠ 0 ramp; O7/O8 active; existing FTD-0098 Reference. |
| S3: `pair-rich`      | + `genesis`, + `pair_production`, + `annihilation`| 5 high-|J| seeds at lattice points | 0.010 | 0.02 | High reaction-rate; O8/O9/O10 all active. |
| S4: `mixed-balanced` | + `genesis`, + `pair_production`, + `annihilation`, + `evaporation`, + `weak_transmutation` | uniform low-|J| Gaussian | 0.005 | 0.02 | Full balanced reaction sector — slow steady state. |

LOCKED at 4 scenarios.

---

## 4 · Ensemble parameters (LOCKED at hash-lock)

Per-scenario:

| Parameter | Value | Rationale |
|----------|-------|-----------|
| L_fine | 32, 64 | b=2 produces L_coarse 16, 32; L=64 lifts FTD-0098 bootstrap noise floor by ~5× |
| N_seeds | 10 | 2× FTD-0098 N_seeds=5; controls inter-seed Var_between |
| N_samples (per seed) | 200 | 5× FTD-0098 N_samples=40; controls within-seed Var_within |
| stride | 5 ticks | Same as FTD-0098 to preserve correlation profile |
| Total snapshots | 2000 / scenario / L | 10 seeds × 200 samples |
| Total snapshots (campaign) | **16,000** | 4 scenarios × 2 lattice sizes × 2000 |
| GPU walltime estimate | ~5–8 hours on RTX 5090 | Per FTD-0098 timing: 6.3 s / 197 snapshots × 16,000 ≈ 8.5 minutes raw kernel; multiplied by per-seed warm-up + ledger inflation ≈ ~6 hours wall, multi-night safe |

Total comparison count for verdict matrix calibration:
- Operators: 10
- Block factors: 2 (`b=2`, `b=4`)
- Scenarios: 4
- Lattice sizes: 2
- Cross-pair entries per `(b, scenario, L)`: 100 (10×10 mixing matrix)
- Total `M_ab` entries measured: `10×10 × 2 × 4 × 2 = 1600`

LOCKED at these parameters.

---

## 5 · Verdict matrix (LOCKED at hash-lock)

The campaign produces a [MEASURED] tag at the level required to push `STATUS_EFT_CHECKLIST.md` §6 from [PARTIAL] to [MEASUREMENT] only if **all four** of the following gates pass:

### 5.1 · Gate A — Per-entry bootstrap stderr

For each (`b`, `scenario`, `L`) `M_ab` matrix:
- ≥ 70 of 100 entries with bootstrap-stderr < 30% relative (ε / |M_ab| < 0.30).

Pre-registered allowance: 70/100 = 70% rather than FTD-0098's 30/36 = 83%; the 10-op basis introduces more low-amplitude entries by design (reaction-sector operators have smaller absolute values than `J⁴`), so 70% is the realistic threshold given the ensemble-size scaling.

### 5.2 · Gate B — Gauss + Q conservation

- 0 Q-violations across the campaign except where a reaction toggle requires them (S3, S4 may have controlled Q-shifts during pair production / annihilation events).
- Gauss residual `max|∇·J − ρ| < 1.0` per snapshot under matched-stencil CG.

### 5.3 · Gate C — RG semigroup self-consistency

Per scenario, per `L`:
- `‖ M(b=4) − M(b=2)² ‖ / ‖ M(b=4) ‖` < **30%** (Frobenius norm ratio).

Acceptance threshold relaxed from FTD-0099's 50% in light of the larger ensemble. This is the explicit RG semigroup test — the cleanest single check that the operator basis is closed under blocking.

### 5.4 · Gate D — `S_eff` self-consistency

Per scenario, per `L`:
- The Wilson coefficients `g_a` extracted from `M_ab(b=2)` via the inversion procedure of §6 produce an `S_eff[H']` that, when re-blocked at `b=2` (numerically: rerun the campaign with the inferred coefficients fed back as a perturbation to the bare action and check whether `M_ab` reproduces), agrees with the measured `M_ab(b=4)` within stderr × √2 (Bayesian propagation).

This is the load-bearing gate: **without Gate D the campaign produces a measurement of M_ab without closing the loop to S_eff**, so the question "is the EFT self-consistent under blocking?" remains open. Gates A–C are subordinate.

### 5.5 · Outcome map

| Gates passed | Verdict |
|------------|---------|
| A, B only | [MEASURED · operator-mixing only; `S_eff` connection [OPEN]] |
| A, B, C | [MEASURED · operator-mixing + RG self-consistency; `S_eff` connection [PARTIAL]] |
| A, B, C, D | [MEASURED · `S_eff` measured to self-consistency; native nonlinear EFT closes at this level] |
| A, B, C, D fails any gate | [PARTIAL] / [INCONCLUSIVE]; campaign report enumerates the failed gate; protocol vN+1 required for re-attempt |

Anti-gate (kill switch): if Gate B fails (Q non-conservation outside expected channels) the campaign output is [INVALID] regardless of other gates — engine bug or protocol error must be diagnosed before re-run.

LOCKED.

---

## 6 · `M_ab` → `g_a` inversion procedure

The bridge from operator-mixing matrix to Wilson coefficients of `S_eff`:

Define a perturbed bare action

```
S[H] = S_bare[H] + Σ_a g_a O_a[H]
```

where `S_bare` is the constrained-history measure of `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md`. After blocking at `b`, the effective action picks up a Wilson coefficient renormalization

```
g_a(b) = Σ_b M_ab(b) g_b(1)
```

at leading order. **This is linear**: at small `g`, the inverse problem `g_a(b)` from measured `M_ab(b)` is just `g(b) = M(b) · g(1)` where `g(1)` is the bare coupling vector.

The campaign extracts `g(1)` by:

1. Running each scenario at the bare action (no `g_a` perturbation).
2. Measuring `M_ab(b=2)`.
3. Solving `M(2) · g_unit = e_a` for the unit response `g_unit_a` per operator.
4. The `g(1)` inferred for the bare ensemble is the eigenvector of `M(2)` with eigenvalue 1 (the marginal direction); operators with eigenvalues > 1 are relevant (`g_a` flows up at IR), eigenvalues < 1 are irrelevant.

Gate D's self-consistency check then re-runs the campaign with `g_unit` perturbations applied as small (`ε ≈ 0.01`) coupling shifts to the toggle parameters, and verifies that `M_ab` shifts by `ε · M_aa` to leading order in the perturbation.

**Caveat**: this linearizes around the bare action and does not capture genuine nonlinear couplings between operators. The full nonlinear connection requires solving the inverse problem at next-to-leading order, which is beyond the scope of v1 of this protocol. v1 establishes whether the linear-Wilsonian connection closes; v2 (post-v1) would address NLO couplings.

---

## 7 · Pre-registration discipline

**Before any blind run:**

1. Hash this protocol (SHA-256) and compute hash of all C++ implementation files (including any new operator-extension headers).
2. Tag the commit `preregister-s-eff-nonlinear-v1`.
3. Run the campaign on locked engine code (no toggle modifications, no parameter sweeps outside the LOCKED §4 set).
4. Report all gate verdicts in `MEASUREMENT_S_EFF_NONLINEAR_v1.md` with both passing and failing gate audits.

**Anti-goals (LOCKED):**

- No re-tuning of `T_Langevin`, `γ`, `K_GENESIS`, or `K_EVAP` in response to intermediate results.
- No adding/removing operators after observing `M_ab`.
- No reformulating the verdict matrix after seeing the data.
- No reporting "near-miss" gate failures as if they were passes.
- If Gate D fails, the v1 measurement reports [PARTIAL] and v2 protocol enters draft; do not silently move to a "Gate D'" that retroactively redefines self-consistency.

**Reproducibility**: GPU runs are not bit-deterministic across hardware; reproducibility holds at the per-seed ensemble-mean level within bootstrap stderr. Per-tick state is RNG-deterministic on a fixed (seed, kernel-config) pair.

---

## 8 · Implementation status

- **Locked operator basis** (this document, §2): defined.
- **Locked scenario list** (§3): defined.
- **Locked ensemble parameters** (§4): defined.
- **Locked verdict matrix** (§5): defined.
- **Engine code**:
  - 6-operator basis (O1–O6): exists in `engine/tests/campaign_operator_mixing_2026-04-26.cpp`.
  - 4 reaction operators (O7–O10): **TO BE ADDED** in `engine/include/ftd/eft/reaction_operators.h` (new file) and wired into a v2 campaign binary `engine/tests/campaign_s_eff_nonlinear_2026-04-29.cpp`.
  - Per-tick state-increment ledger access: exists via `RenderBridge::continuity_step()` (already plumbed for FTD-0070 and FTD-0098).
  - Multi-scenario runner: extension of FTD-0098 binary with a `--scenario={langevin-pure,genesis-rich,pair-rich,mixed-balanced}` flag.
- **GPU campaign harness**: ~5–8 hours wall on RTX 5090 (WSL2 Ubuntu-22.04 path per CLAUDE.md "Environment Notes").
- **Output structure**: `engine/results/s_eff_nonlinear_2026-04-29/{S1_langevin_pure, S2_genesis_rich, S3_pair_rich, S4_mixed}/{L32_b2, L32_b4, L64_b2, L64_b4}/{M_ab.csv, M_ab_stderr.csv, eigenvalues.csv, run.log}` plus aggregate `meta.json` and `MEASUREMENT_S_EFF_NONLINEAR_v1.md`.

---

## 9 · Cross-references

- `STATUS_EFT_CHECKLIST.md` §6 (the [PARTIAL] entry being addressed)
- `DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` (the constrained-history measure `S_H`)
- `SPEC_OPERATOR_BASIS.md` (FTD-0091 6-op basis being extended)
- `PROTOCOL_OPERATOR_MIXING_MATRIX.md` (FTD-0098 protocol template)
- `engine/tests/campaign_operator_mixing_2026-04-26.cpp` (FTD-0098 implementation reference)
- `SPEC_FTD_NATIVE_BLOCKING_MAP.md` (the b=2 blocking contract)
- LEDGER FTD-0098, FTD-0099, FTD-0100, FTD-0070 (priors)
- LEDGER FTD-0112 (this campaign — to be assigned at hash-lock)

---

## 10 · Single-line summary

**The S_eff nonlinear campaign locks a 10-operator basis (6 flux/state + 4 reaction-sector), 4 scenarios spanning Langevin-pure to mixed-reaction-balanced, 2 lattice sizes (L=32, L=64), 2 block factors (b=2, b=4), and a 4-gate verdict matrix (per-entry stderr, Q+Gauss conservation, RG semigroup self-consistency, S_eff self-consistency); passing all four gates moves the native nonlinear EFT from [PARTIAL · definition fixed] to [MEASURED · S_eff self-consistent within errors] and closes the load-bearing remaining gate to a math-based FTD EFT.**
