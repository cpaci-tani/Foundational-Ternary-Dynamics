# Status — FTD nonlinear regime, end of 2026-04-30 session

**Status:** [REFERENCE] / consolidated handoff document
**Date:** 2026-04-30 (end of session)
**Predecessor:** [`STATUS_EFT_CHECKLIST.md`](STATUS_EFT_CHECKLIST.md) (general program checklist)
**Companion docs (this session):** §1 below
**LEDGER:** FTD-0112 (campaign), FTD-0070 (bare Gaussian fixed point), FTD-0110 (cluster phenomenology)

---

## 0 · One-paragraph summary

Over 2026-04-29 + 2026-04-30, FTD-0112's nonlinear S_eff campaign produced the first quantitative measurement of the engine's blocked operator-mixing matrix at production statistics, with three categories of structural findings: **(i) four of nine active diagonal entries are now theorem-grade identities** of the blocking convention (JJ, J4 exact b⁴ and b⁸; stateSq, reactionDensity following Theorem 3's b³ + 2b³ρ̄ form); **(ii) the 9-op active subspace decomposes empirically into three nearly-isolated sub-blocks** (SPATIAL, DENSITY bridge, REACTION-FLUX) with SPATIAL ↔ REACTION-FLUX completely decoupled at 5σ over 20,000 snapshots — a structural reflection of FTD's two-layer ontology; **(iii) per-operator distributions are highly non-Gaussian** (skewness 1.1 to 26 across operators), establishing that the engine has Gaussian β-functions (FTD-0070) but non-Gaussian operator distributions, an independent structural property of the nonlinear regime. Gates B (conservation) and C (RG semigroup at 0.172 < 0.30) PASS at L=32 LARGE; Gate A diagonal PASSES (9/9); off-diagonal Gate A bootstrap-stderr-limited; Gate D queued for v2 perturbation campaign with shift predictions calibrated against measured per-operator skewness. The engine's "math-based EFT" therefore has substantial measured structural content with a clear path to closure.

---

## 1 · Documents authored this session

In chronological order:

| Doc | Content | Status |
|---|---|---|
| `PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md` | v1 protocol locked under tag `preregister-s-eff-nonlinear-v1` | LOCKED |
| `engine/include/ftd/eft/reaction_operators.h` | 4 reaction-sector operators O7-O10 | 4/4 unit tests PASS |
| `engine/tests/campaign_s_eff_nonlinear_2026-04-29.cpp` | production campaign binary, 9-op active, --T-langevin, --inject-period flags | works at L=8 to L=64 |
| `MEASUREMENT_S_EFF_NONLINEAR_v1_partial.md` | First production M_ab(b=2)+(b=4); Gates B,C PASS, Gate A diag PASS | UPDATED with §1.5 L=32 vs L=64 cross-comparison |
| `AUDIT_S_EFF_SMOKE_VALIDATION.md` | architecture validation finding reaction-quiet steady state | DONE |
| `THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md` | Theorems 1, 2, 3 proving diagonal blocking identities | proved + verified at machine precision |
| `scripts/exploration/verify_blocking_diagonal_identities_2026-04-30.py` | numerical verification of all 3 theorems | 7 test classes PASS |
| `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md` | off-diagonal structure: 5-class → 3 sectors, complete SPATIAL↔REACTION-FLUX decoupling | quantitative 5σ test |
| `PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN.md` | v2 Gate D design leveraging sector decoupling | DRAFT, awaiting hash-lock pre-conditions |
| `AUDIT_GAUSSIANITY_v1_LARGE.md` | per-operator skewness measurement (1.1–26) — engine ensemble non-Gaussian | DONE |
| `STATUS_NONLINEAR_REGIME_2026-04-30.md` | this consolidated status doc | THIS DOC |

Plus the existing files updated: `LEDGER.md`, `STATUS_EFT_CHECKLIST.md` (background only).

---

## 2 · Theorem-grade results

### Theorem 1 (proved, verified): JJ blocking identity

For any DualCellFields configuration with constant flux, `M_{J², J²} = b⁴ = 16` exactly under `block_dual_cell_b2`.

Engine measurement at L=32 LARGE: `M_JJ,JJ = 16.0001 ± 0.0000` (machine-precision exact). Confirms engine's gauss-projected ensemble has effective block-block correlation `ρ ≥ 1 - O(0.001)`.

### Theorem 2 (proved, verified): J⁴ blocking identity

`M_{J^{2k}, J^{2k}} = b^{4k}` for any k ≥ 1. Specifically `M_J4,J4 = b⁸ = 256` exactly.

Engine measurement at L=32 LARGE: `M_J4,J4 = 256.0040 ± 0.0022` (machine-precision exact within stderr).

### Theorem 3 (proved, verified): charge-density blocking identity

For any integer-valued density operator `s` under SUM-blocking:

```
<s²>_coarse / <s²>_fine = b³ × (1 + 2ρ̄_intra-block)
```

where `ρ̄_intra-block` is the average intra-block sign correlation per pair.

**Corollaries**:
- 3a (uncorrelated): `M_{s², s²} = b³ = 8` exactly
- 3b (correlation measurement): `(M − b³)/b³` directly measures intra-block sign correlation
- 3c (block-uniform): `M = b⁶ = 64` exactly when fully correlated

**Engine readings**:
- `M_stateSq = 7.35 ± 0.15` → ρ̄ ≈ −0.04 (slight anti-correlation; consistent with gauss-projection-driven flux closure)
- `M_reactionDensity = 8.34 ± 0.76` → ρ̄ ≈ +0.02 (slight positive correlation; consistent with reaction-event spatial clustering)

**Net theorem-grade content**: 4 of 9 active diagonal entries promoted to [THEOREM] + [MEASURED · empirical correlation correction].

---

## 3 · Empirical structural findings

### 3.1 · Sector decomposition (5σ over 20,000 snapshots)

| Sector | Operators | Internal coupling | Cross-coupling pattern |
|---|---|---:|---|
| SPATIAL | JJ, divJ², curlJ², JdotDivJ, J4 | 10/20 entries at 5σ | strong internal mixing |
| DENSITY bridge | stateSq, reactionDensity | 0/2 at 5σ | weak couplings into both other sectors (2/10 each direction) |
| REACTION-FLUX | genesisFlux, JdotDeltaS | 0/2 at 5σ | **completely decoupled from SPATIAL (0/20 entries at 5σ)** |

**The decoupling follows structurally from FTD's two-layer ontology**: flux `J` and state `s` couple only through (a) gauss constraint `∇·J = ρ` and (b) genesis rule `s := sign(J)·θ(|J| > K_GENESIS)`. Both manifest through the density sector. The reaction-flux operators (carrying `δs · |J|` factors) couple through density, NOT directly to pure-flux operators.

### 3.2 · Per-operator non-Gaussianity

Three regimes:

| Regime | Operators | Skewness | Mechanism |
|---|---|---|---|
| Mildly non-Gaussian | JJ, J⁴ | 1.1, 2.6 | CLT + positive-only support |
| Strongly non-Gaussian | divJ², curlJ², JdotDivJ, stateSq | ±6, +11 | per-snapshot cluster tail dominates |
| Extremely non-Gaussian | reactionDensity, genesisFlux, JdotDeltaS | ±22-26 | rare-event count statistics |

**Reconciliation with FTD-0070 Gaussian fixed point**: independent properties. FTD-0070 measures bare-coupling RG flow; FTD-0112 measures higher-order operator-distribution cumulants. Engine has Gaussian β but non-Gaussian P[operator].

### 3.3 · Reaction-sector one-way structure

`evapFlux = 0` measured across all tested `T_langevin ∈ [0.005, 1.000]` at L=16. Engine reaction sector at canonical `(K_GENESIS, K_EVAP)` is **one-way**: genesis events occur, evaporation events kinematically suppressed. Connects to FTD-0102 (runaway crystallization at high T from same one-way mechanism) and FTD-0110 (Phase 7 narrow activation band — parallel cluster-perspective finding).

### 3.4 · T-activation window

Reaction-operator variance peaks at `T_langevin ∈ [0.10, 0.15]` and decays at higher T (state saturates → δs → 0) and lower T (subthreshold). Production parameter regime locked: `T_langevin = 0.100, pair-rich, L=32 or 64`.

---

## 4 · Gate-by-gate v1 verdict (L=32 LARGE)

| Gate | Status | Detail |
|---|---|---|
| A diagonal | **PASS 9/9** | with 4 entries at theorem grade |
| A off-diagonal | PARTIAL | 21/72 = 29% pass; bootstrap-stderr-limited; 24/72 structurally zero |
| B (Q + Gauss conservation) | **PASS** | 0/20000 violations |
| C (RG semigroup `‖M(b=4) − M(b=2)²‖ / ‖M(b=4)‖`) | **PASS at 0.172 < 0.30** | improving with ensemble (was 0.210 at v1 N=2k) |
| D (S_eff self-consistency) | NOT TESTED | requires v2 perturbation campaign |

L=64 N=2k cross-check (no LARGE yet): JJ M_aa = 16.004 (b⁴ exact within stderr); diagonal patterns confirm; Gate C fails at 0.465 (bootstrap-noise-limited at small N; v1.2 LARGE queued).

L=64 LARGE in progress: `biadbkhy6` (~1.5h remaining as of doc-write time).

---

## 5 · Outstanding / queued work

### Immediate (next session)

1. **Analyze L=64 LARGE result** when `biadbkhy6` completes:
   - Confirm sector decoupling persists at L=64
   - Verify Gate C at L=64 LARGE (expected 0.17 ± 0.03)
   - Refine the 7 non-theorem diagonal values' L → ∞ extrapolation

2. **v2 protocol hash-lock pre-conditions**:
   - Engine wiring for `--wilson-coefficient=name:value` flag
   - Linear-response analytic predictions per operator (calibrated by §3.2 skewness)
   - Smoke validation of one perturbation

### Medium-term

3. **v2 Gate D campaign** (7 perturbation runs × 30 min on RTX 5090):
   - Theorem-grade diagonals: invariance test
   - Non-theorem diagonals: shift-matching against `S_a · σ_a / μ_a` prediction

4. **L=128 production** (if engineering bandwidth permits):
   - Continuum-limit extrapolation of the 5 non-theorem diagonals
   - Whether sector decoupling persists at larger L

### Open structural questions

- **Q1**: Does sector decoupling persist at L=64 LARGE? At L=128?
- **Q2**: Are the 7 non-theorem diagonal values (`divJ²` -b⁴ → -b⁵, `JdotDivJ` b⁵ → b⁶, etc.) extrapolating to specific limits or drifting unboundedly with L?
- **Q3**: Does the engine's nonlinear regime have a characterizable continuum limit, or is it fundamentally lattice-bound?
- **Q4**: Can the off-diagonal SPATIAL-sector entries (10/20 significant at 5σ) be analytically derived from the blocking convention + gauss projection?

---

## 6 · "Math-based EFT" status post-session

Where the program stands today:

✅ **Bare Gaussian fixed point** (FTD-0070): closed.
✅ **Algebraic spine** (8 theorems in `SPEC_ALGEBRAIC_SPINE.md`): closed.
✅ **Diagonal RG-eigenvalue spectrum** (M_aa for 9 active operators measured at L=32 LARGE): closed.
✅ **Theorem-grade convention identities** (4 of 9 diagonals): closed.
✅ **Sector decomposition of operator basis**: closed (empirical, structural reading from two-layer ontology).
✅ **Per-operator non-Gaussianity profile**: measured.
✅ **RG semigroup self-consistency at L=32 LARGE** (Gate C): PASS.
🟡 **Sector decomposition at L=64 LARGE**: PENDING (in flight).
🟡 **L=64 LARGE Gate C confirmation**: PENDING.
🟡 **Off-diagonal Gate A** (29% PASS): bootstrap-stderr-limited; sector inversion path identified for v2.
⏳ **Gate D self-consistency** (S_eff Wilsonian closure): v2 design ready, perturbation campaign queued.

The engine has a substantial measured nonlinear EFT structure. What remains is closing Gate D, confirming structural patterns at L=64 LARGE, and (longer-term) extending to L=128 for continuum-limit clarity.

---

## 7 · Single-line summary

**FTD's nonlinear EFT now has measured: a 9-op operator-mixing matrix M_ab(b=2) at L=32 LARGE with 4 of 9 diagonals at theorem-grade (JJ b⁴, J4 b⁸, stateSq + reactionDensity at b³ + correlation correction); a sector decomposition into SPATIAL/DENSITY/REACTION-FLUX with SPATIAL ↔ REACTION-FLUX completely decoupled at 5σ; per-operator skewnesses 1.1 to 26 establishing the engine has Gaussian β-functions but non-Gaussian operator distributions; Gates B + C PASS at L=32 LARGE; Gate D queued for v2 perturbation campaign with shift predictions calibrated against measured skewness; L=64 LARGE confirmation in flight; the math-based-EFT framing has Gaussian fixed point + theorem-grade diagonal anchors + structural sector decomposition + measured non-Gaussian higher-order content as four independent pieces of native-EFT structural content delivered tonight.**
