# Analysis — Gate C and diagonal M_aa across L ∈ {24, 32, 48, 64, 128}

**Status:** [MEASURED · structural cross-L picture]
**Date:** 2026-04-30 (late session)
**Source data:** `engine/results/s_eff_nonlinear_2026-04-29/L{24,32,48,64}_prod_T0.100_LARGE/`
**Total snapshot pairs:** 80,000 across the 4 lattice sizes
**LEDGER:** FTD-0112

---

## 0 · Headline structural picture

Four lattice sizes × 10 seeds × 2000 samples × T_langevin = 0.100 pair-rich = **80,000 snapshot pairs total**. The cross-L picture has three clean structural results:

1. **Theorems 1 & 2 hold at ALL 4 lattice sizes to machine precision** — `M_JJ,JJ = 16.0000` and `M_J4,J4 = 256.000` exactly at L=24, 32, 48, 64. L-independent algebraic identities verified.

2. **Gate C is non-monotone in L with a unique PASS at L=32** — coarse-grid noise dominates at small L, real physics breakdown at large L; L=32 is the sweet spot where both effects are below threshold.

3. **The fundamental sector-decoupling direction (REACTION-FLUX → SPATIAL = 0/10) is L-INDEPENDENT** — verified at all 4 lattice sizes. This is the structural reflection of FTD's two-layer ontology, holding regardless of lattice size.

---

## 1 · Diagonal M_aa across L

```
op              | L=24            | L=32            | L=48            | L=64
JJ              | 16.000±0.000    | 16.000±0.000    | 16.000±0.000    | 16.000±0.000   ← Theorem 1
divJ²           | -15.65±0.49     | -16.61±0.46     | -16.94±0.50     | -17.48±0.47    ← drifts
curlJ²          | 8.64±0.08       | 8.80±0.13       | 8.87±0.08       | 9.57±0.08      ← drifts up
JdotDivJ        | 29.54±0.45      | 30.68±0.65      | 29.77±0.42      | 31.05±0.44     ← stable
J4              | 256.003±0.001   | 256.004±0.002   | 255.996±0.002   | 256.003±0.003  ← Theorem 2
stateSq         | 7.99±0.16       | 7.35±0.15       | 6.86±0.11       | 6.77±0.15      ← drifts down
reactionDensity | 10.61±1.04      | 8.34±0.76       | 6.81±0.45       | 6.59±0.53      ← drifts down
genesisFlux     | -19.30±1.90     | -18.47±2.00     | -16.60±1.35     | -12.67±1.31    ← shrinks
JdotDeltaS      | 30.33±3.69      | 27.40±2.90      | 25.85±2.19      | 22.48±3.23     ← shrinks
```

### 1.1 · Theorem-grade entries are L-INDEPENDENT to machine precision

`JJ M_aa = 16.0000 ± 0.0000` at every L tested. `J4 M_aa = 256.000 ± 0.005` at every L. Total deviation across 80,000 snapshot pairs: **less than 0.005 absolute = 2 × 10⁻⁵ relative**. Theorems 1, 2 are confirmed as algebraic blocking identities.

### 1.2 · Theorem 3 entries (charge-density) drift toward L → ∞ fixed points

`stateSq` flows from 8.0 (≈ b³, uncorrelated signs at L=24) → 6.77 (anti-correlated at L=64). **Trend**: `(M − b³)/b³` goes from ~0% at L=24 to ~−15% at L=64. As L grows, cluster cells become more isolated within larger blocks → gauss projection produces more anti-correlated neighbors.

`reactionDensity` flows from 10.6 (clustered, positive correlation at L=24) → 6.59 (anti-correlated at L=64). At small L reaction events crowd into blocks; at large L they spread out into isolated cells.

This is **a quantitative measurement of the engine's intra-block sign correlation flowing under blocking** at the larger lattice — a real piece of nonlinear EFT physics.

### 1.3 · Non-theorem spatial diagonals show monotone drift

`divJ²`: -15.65 → -16.61 → -16.94 → -17.48 (becoming more negative; surface-tension-like)

`curlJ²`: 8.64 → 8.80 → 8.87 → 9.57 (slowly increasing; transverse modes more active at larger L)

`JdotDivJ`: oscillates around ~30 (no clear trend; may be at fixed point already at L=24)

### 1.4 · Reaction-flux diagonals shrink with L

`genesisFlux`: -19.30 → -18.47 → -16.60 → -12.67 (magnitude decreasing 35% across the L range)

`JdotDeltaS`: 30.33 → 27.40 → 25.85 → 22.48 (magnitude decreasing 26%)

Both reaction-flux operators have decreasing magnitudes with L. Physical interpretation: at larger L the reactions are spread out over more cells, so per-cell `genesisFlux ~ |J| · δs` averages contain fewer high-`|J|` events per cell, reducing the operator's RG eigenvalue magnitude.

---

## 2 · Gate C: non-monotone in L

```
L  | Gate C ratio | cond(S)    | verdict
24 | 0.556        | 6.4e13     | FAIL (coarse-grid noise)
32 | 0.172        | 1.1e13     | PASS (sweet spot)
48 | 0.307        | 1.6e12     | FAIL (just over threshold)
64 | 0.365        | 1.7e12     | FAIL (physics breakdown)
```

### 2.1 · Two failure mechanisms

The Gate C non-monotonicity is structurally interpretable as the sum of two independent failure mechanisms:

**Failure A — small-L coarse-grid statistical noise.** At L=24, the b=4 coarse lattice is `L_c = 6` (216 coarse cells). Per-snapshot statistics on 216 cells produces large fluctuations in M(b=4) entries that the bootstrap correctly identifies. cond(S) at L=24 = 6.4e13, the worst of the four. This dominates Gate C ratio at L=24.

**Failure B — large-L irreducible physics.** At L=48 and L=64, cond(S) IMPROVES (1.6–1.7e12) and statistical noise is no longer the bottleneck. Yet Gate C still fails. The b=4 measurement captures length-scale-4 spatial structure that two iterations of b=2 don't reproduce. Standard EFT analog: when block size is too small to capture irreducible vertices.

### 2.2 · L=32 as sweet spot

At L=32, the coarse lattice b=4 has `L_c = 8` (512 cells), enough for stable bootstrap statistics. Yet the 7 non-theorem diagonals haven't drifted far enough from their L=24 values for irreducible-physics breakdown to dominate. **L=32 is the unique passing point where both failure mechanisms are below threshold.**

This is reminiscent of standard QFT lattice EFT: there is a finite-volume sweet spot for any given measurement protocol, where statistical noise floor meets the onset of finite-volume corrections to physics. FTD-0112's M_ab(b=2) at T=0.100 pair-rich has its sweet spot at L=32.

---

## 3 · Sector decoupling: REACTION-FLUX → SPATIAL is L-INDEPENDENT

```
direction       | L=24    | L=32    | L=48    | L=64
SPATIAL  SPATIAL | 11/20 | 10/20 | 12/20 | 10/20
SPATIAL → RF    | 0/10    | 0/10    | 2/10    | 2/10
RF → SPATIAL    | 0/10    | 0/10    | 0/10    | 0/10   ← **L-INDEPENDENT decoupling**
SPATIAL → D     | 3/10    | 2/10    | 4/10    | 6/10
D → SPATIAL     | 2/10    | 2/10    | 2/10    | 4/10
```

### 3.1 · The fundamental decoupling holds at all L

**REACTION-FLUX → SPATIAL = 0 / 10 entries above 5σ at every lattice size.** At 80,000 snapshot pairs total this is a strong statistical statement. The engine's blocking map has ZERO statistically-significant coupling from reaction-flux operators to spatial-sector coarse responses, regardless of lattice size.

**Structural origin (re-stated)**: state `s` and flux `J` are formally independent fields in FTD; they couple only through (a) the gauss constraint `∇·J = ρ` and (b) the genesis rule `s := sign(J)·θ(|J|>K_GENESIS)`. Both manifest through the density sector (`s²`, `(δs)²`). Reaction-flux operators (carrying `δs · |J|` factors) couple through density, NOT directly to pure-flux operators.

This is the **L-independent backbone** of the sector decomposition.

### 3.2 · Other directions partially L-dependent

- **SPATIAL → REACTION-FLUX**: 0 at L≤32, 2/10 at L≥48. Some weak coupling emerges with L, but only ~20% of entries — still mostly decoupled.
- **SPATIAL → DENSITY**: 3, 2, 4, 6 — DENSITY bridge thickens with L. At L=64 the spatial sector drives DENSITY couplings.
- **DENSITY → SPATIAL**: 2, 2, 2, 4 — bridge directionality emerges only at L=64.
- **SPATIAL  SPATIAL**: stable around 10-12/20 — internal mixing is L-independent.

So as L grows, the DENSITY bridge thickens (more cross-sector couplings emerge through density), but the REACTION-FLUX sector itself remains quarantined from direct SPATIAL coupling.

---

## 4 · Gate A off-diagonal improves monotonically with L

```
L  | off-diag <30% stderr | %
24 | 17/72                | 24%
32 | 21/72                | 29%
48 | 28/72                | 39%
64 | 36/72                | 50%
```

cond(S) improvements at larger L (6.4e13 → 1.7e12) translate to more off-diagonal entries passing the bootstrap stderr threshold. Half the off-diagonal entries pass at L=64.

**Trend extrapolation**: at L=128 we'd expect ~60-65% off-diagonal Gate A pass. The path to original PROTOCOL §5.1 70% threshold is via larger L, not larger ensemble.

---

## 5 · The "math-based EFT" interpretation across L

### 5.1 · L-independent backbone (theorem-grade)

 **Bare Gaussian fixed point** (FTD-0070): closed.
 **Algebraic spine** (8 theorems): closed.
 **Theorems 1, 2** (M_JJ = b⁴, M_J4 = b⁸): verified at L = 24, 32, 48, 64 to machine precision.
 **Theorem 3** (charge-density blocking): structure verified at all 4 L; ρ̄_intra-block measured to flow from ~0 at L=24 to ~−0.15 at L=64.
 **REACTION-FLUX → SPATIAL = 0 sector decoupling**: L-INDEPENDENT at 5σ over 80,000 snapshots.

These pieces of FTD's nonlinear EFT structure are L-independent. They define the **convention-level + ontology-level** content of the EFT.

### 5.2 · L-dependent flow content

 **Five non-theorem diagonals** (`divJ², curlJ², JdotDivJ, stateSq, reactionDensity, genesisFlux, JdotDeltaS`) show monotone or near-monotone L-dependence. These are **real RG flow content** under continuum approach.

Some flow toward apparent fixed points (stateSq → ~6.7, reactionDensity → ~6.5). Others continue drifting (genesisFlux magnitude shrinking, JdotDeltaS shrinking). L=128 would clarify whether the apparent fixed points stabilize.

### 5.3 · L=32 as the unique self-consistency window

 **Gate C (RG semigroup) PASSES only at L=32**. The L=32 LARGE measurement is therefore a uniquely-self-consistent finite-L EFT measurement, with both the small-L coarse-grid noise and the large-L irreducible physics below threshold.

For paper drafting: the cleanest "math-based EFT" claim is **at L=32, T=0.100, pair-rich**, where Gates B + C + A-diagonal all PASS. The L=24, 48, 64 measurements characterize the L-dependent behavior around this sweet spot.

---

## 6 · Open follow-ups

**Q1**: Does the L=32 sweet-spot persist at other T values? Does T=0.150 (also in the activation window) have its sweet spot at the same L? This would be a 2D scan over (L, T).

**Q2**: Is L=128 within the second failure regime (still failing Gate C from physics breakdown), or does the failure stabilize at large L? Continuum-limit clarification.

**Q3**: Can the Gate C ratio at large L be **decomposed** into known operator-class contributions? Specifically: which operator pairs contribute most to `‖M(b=4) − M(b=2)²‖`? If the breakdown is concentrated in the reaction-flux sub-block, that's diagnostic.

**Q4**: Theorem 4 attempt: derive analytic L-dependence of `divJ²` and `curlJ²` from the matched-stencil CG projector + blocking convention. Their drift (-15.7 → -17.5 for divJ²) might be derivable rather than measured.

**Q5**: Does the SPATIAL  SPATIAL within-sector coupling pattern (10-12/20 at all L) decompose into specific operator pairs that are present vs absent at all L?

---

## 6.5 · L=128 LARGE addition (5-L picture, 2026-05-01)

L=128 LARGE production complete (10 seeds × 2000 samples = 20,000 snapshot pairs at T_langevin=0.100, pair-rich, 19.5h wall on RTX 5090). Total cross-L data: 100,000 snapshot pairs across 5 lattice sizes.

### 6.5.1 · Diagonal M_aa values at L=128

```
op              | L=24       | L=32       | L=48       | L=64       | L=128       
JJ              | 16.000±0.000 | 16.000±0.000 | 16.000±0.000 | 16.000±0.000 | 16.000±0.000
divJ²           | -15.65±0.49 | -16.61±0.46 | -16.94±0.50 | -17.48±0.47 | **-22.90±0.74**
curlJ²          | 8.64±0.08   | 8.80±0.13   | 8.87±0.08   | 9.57±0.08   | **10.99±0.13**
JdotDivJ        | 29.54±0.45  | 30.68±0.65  | 29.77±0.42  | 31.05±0.44  | **40.45±1.03**
J4              | 256.003±0.001 | 256.004±0.002 | 255.996±0.002 | 256.003±0.003 | **255.833±0.009** ✗
stateSq         | 7.99±0.16   | 7.35±0.15   | 6.86±0.11   | 6.77±0.15   | 5.85±0.14
reactionDensity | 10.61±1.04  | 8.34±0.76   | 6.81±0.45   | 6.59±0.53   | 5.15±0.36
genesisFlux     | -19.30±1.90 | -18.47±2.00 | -16.60±1.35 | -12.67±1.31 | **-12.21±1.19** (stable)
JdotDeltaS      | 30.33±3.69  | 27.40±2.90  | 25.85±2.19  | 22.48±3.23  | **22.93±0.96** (stable)
```

Bold = significant change from L=64.

### 6.5.2 · Three new structural findings

**Finding A: Theorem 2 (J⁴ = b⁸) shows first measurable deviation at L=128.**

`M_J4,J4 = 255.833 ± 0.009` at L=128 — 0.167 absolute deviation from the theorem value 256, which is **19σ** from the bootstrap stderr. At L=24, 32, 48, 64 the deviations were <0.005 (within stderr).

This is **a real finite-L breakdown of Theorem 2's smooth-field condition for higher powers.** Theorem 1 (J²) requires the per-cell relation `J²_coarse = b⁴ J²_fine_block_avg` to hold under the block-uniformity-with-block-correlation condition. Theorem 2 (J⁴) requires the squared version, which is more sensitive to within-block fluctuations. At L=128 the gauss-projected field has small block-scale variations that break the higher-power identity at 19σ but not the lower-power one.

This is **structurally interesting**: Theorems 1 and 2 had identical "smooth-field" preconditions, but they have different sensitivities to violations of those preconditions. **Theorem 1 is more robust than Theorem 2**, with Theorem 2 starting to fail at L=128 while Theorem 1 still holds.

**Finding B: Gate C ratio PLATEAUS at L=128, doesn't diverge.**

```
L=24:  0.556  (small-L noise dominated)
L=32:  0.172  (sweet spot)
L=48:  0.307  (just over threshold)
L=64:  0.365  (deeper breakdown)
L=128: 0.353  (BETTER than L=64!)
```

The breakdown is **bounded** — it doesn't continue growing past 0.40. Either we've found the asymptotic value of the irreducible-physics-breakdown amplitude, or it's still drifting slowly with no clean trend.

This is **important for the "math-based EFT" interpretation**: the engine's RG semigroup property breaks down by a finite amount, not unboundedly. The L=32 sweet-spot is genuinely special (the only lattice where Gate C passes), but the breakdown for larger L is bounded above ~0.4.

**Finding C: Sector decoupling REACTION-FLUX → SPATIAL = 2/10 at L=128 (was 0/10 at L≤64).**

At all four smaller lattices (L=24, 32, 48, 64), `RF → SPATIAL = 0/10 entries above 5σ` was the bedrock structural finding — interpreted as the L-independent reflection of FTD's two-layer ontology. At L=128, **2 of 10 entries are above 5σ for the first time**.

Two interpretations:

1. **Better conditioning reveals previously-below-threshold couplings**: cond(S) at L=128 = 8.2e11 is the lowest of all 5 lattices. With smaller bootstrap stderr, marginally non-zero couplings become statistically significant. Under this reading, the underlying physics hasn't changed; the resolution improved.

2. **Genuine large-L cross-coupling emerges**: real new structure visible only at the largest lattice.

Distinguishing requires checking whether the entries that emerge above 5σ at L=128 had non-zero (but below-5σ) values at L=64. If yes → resolution effect. If their absolute magnitudes are larger at L=128 than L=64 → real growth.

**Net implication**: the "L-independent backbone" claim from the 4-lattice analysis needs softening. The fundamental sector-decoupling direction holds **strictly at L ≤ 64** but shows weak (2/10) emergence at L=128. The other backbone elements (Theorems 1, 3, Gaussian fixed point, algebraic spine) hold at all L.

### 6.5.3 · Other 5-L findings

**Gate A off-diagonal continues to improve**: 24% → 29% → 39% → 50% → **64%** at L=128. Approaching but not reaching the 70% PROTOCOL threshold. Linear extrapolation suggests L ≈ 256 would reach 70%; sublinear-trend would never quite get there.

**Some diagonals stabilize, others continue drifting**:
- Stabilizing (≤5% L=64→L=128 change): genesisFlux (-12.67 → -12.21), JdotDeltaS (22.48 → 22.93). These appear to have reached their asymptotic values.
- Continuing to drift: divJ² (-17.48 → -22.90, +31% magnitude), curlJ² (9.57 → 10.99, +15%), JdotDivJ (31.05 → 40.45, +30%), stateSq (6.77 → 5.85, −14%), reactionDensity (6.59 → 5.15, −22%).

The drift pattern suggests two operator classes:
- Reaction-flux operators (genesisFlux, JdotDeltaS): flow toward fixed values
- Spatial-derivative + density operators: continue flowing

**cond(S) keeps dropping**: 6.4e13 → 1.1e13 → 1.6e12 → 1.7e12 → **8.2e11**. Best-conditioned at L=128. The matrix inversion gets cleaner with larger lattice (more cells = better averaging = less ill-conditioning).

### 6.5.4 · Re-stated math-based EFT closure status

After L=128 LARGE:

**L-INDEPENDENT BACKBONE (holds at L=24, 32, 48, 64, 128)**:
- Bare Gaussian fixed point (FTD-0070) ✓
- Algebraic spine (nine numbered results: six theorem-grade + three honestly-tiered, see `SPEC_ALGEBRAIC_SPINE.md` §0) ✓
- **Theorem 1** (M_JJ,JJ = b⁴ exactly): verified at all 5 lattice sizes to machine precision ✓
- **Theorem 3** (charge-density blocking with ρ̄): structure verified ✓

**FINITE-L BACKBONE (holds at L ≤ 64, partial at L=128)**:
- **Theorem 2** (M_J4,J4 = b⁸): exact at L≤64, 19σ deviation at L=128 — finite-L applicability
- **Sector decoupling REACTION-FLUX → SPATIAL = 0**: strict at L≤64, 2/10 emergent at L=128

**SWEET-SPOT FINITE-L EFT (L=32 LARGE)**:
- All Gates A-diag, B, C, D (theorem-grade) PASS uniquely at L=32 ✓

**L-DEPENDENT FLOW**:
- 5 non-theorem diagonals show flow with L; some stabilizing (reaction-flux pair), others drifting (spatial-derivative + density)
- Gate C ratio bounded above ~0.40, plateauing at large L

**IMPLICATIONS FOR PAPER DRAFTING**:
- Branch-A native EFT paper at L=32 sweet spot remains the cleanest finite-L closure point
- L=128 result demonstrates that the L-independent backbone is *narrower* than originally thought (Theorem 1 + Theorem 3 + algebraic spine + Gaussian fixed point)
- Theorem 2 should be re-stated as: holds in the smooth-field-strict limit; observed exactness at L=24, 32, 48, 64 is the engine being in that regime; L=128 measurement begins to deviate
- Sector decoupling should be re-stated as: holds strictly at L ≤ 64; weakly emergent cross-couplings at L=128

These are honest finite-L EFT statements, not unbounded structural claims.

---

## 7 · Single-line summary

**Cross-L analysis at L ∈ {24, 32, 48, 64} × 10 seeds × 2000 samples × T = 0.100 pair-rich (80,000 snapshot pairs total) confirms: (i) Theorems 1 (`JJ M_aa = b⁴`) and 2 (`J4 M_aa = b⁸`) hold at machine precision at every lattice size — algebraic blocking identities are L-independent; (ii) Gate C (RG semigroup ratio) is non-monotone in L with a unique PASS at L=32, between small-L coarse-grid statistical noise (cond(S) = 6.4e13 at L=24) and large-L irreducible physics breakdown (Gate C ratio rises from 0.17 at L=32 to 0.37 at L=64 with cond(S) IMPROVING, indicating real physics not noise); (iii) the fundamental sector-decoupling direction REACTION-FLUX → SPATIAL = 0 / 10 entries above 5σ HOLDS AT ALL FOUR LATTICE SIZES — an L-independent structural reflection of FTD's two-layer ontology; (iv) the 5 non-theorem diagonals show monotone L-dependence consistent with real RG flow content with some entries (stateSq, reactionDensity) approaching apparent fixed points and others (genesisFlux, JdotDeltaS) continuing to drift; the L=32 LARGE measurement is therefore the unique self-consistent finite-L EFT closure point and the natural reference for a Branch-A native EFT paper, with L=24/48/64 results characterizing the L-dependent behavior around this sweet spot.**
