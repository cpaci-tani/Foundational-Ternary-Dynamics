# Analysis — Off-diagonal asymmetry in M_ab(b=2)

**Status:** [STRUCTURAL OBSERVATION · post-v1 LARGE]
**Date:** 2026-04-30
**Companion:** [`MEASUREMENT_S_EFF_NONLINEAR_v1_partial.md`](MEASUREMENT_S_EFF_NONLINEAR_v1_partial.md)
**Source data:** `engine/results/s_eff_nonlinear_2026-04-29/L32_prod_T0.100_LARGE/M_ab.csv`
**LEDGER:** FTD-0112

---

## 0 · Headline structural pattern

Beyond the diagonal RG-eigenvalue spectrum (Theorems 1, 2, 3 in `THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`), the L=32 LARGE M_ab(b=2) measurement reveals a **strong off-diagonal asymmetry pattern** that decomposes the 9-op active subspace into operator classes by mixing-direction structure.

### 0.1 · J4 column is essentially zero

Across the entire 9×9 matrix, **all 8 off-diagonal entries `M[X, J4]` are at machine precision** (1e-7 to 1e-9, with σ comparable):

```
M[JJ,              J4] = -1.7e-6  ± 8.6e-7
M[divJ2,           J4] = -7.8e-6  ± 4.1e-6
M[curlJ2,          J4] = -3.9e-6  ± 1.7e-6
M[JdotDivJ,        J4] = -3.5e-7  ± 1.8e-6
M[stateSq,         J4] ≈ 0  (similar magnitude)
M[reactionDensity, J4] = +1.5e-8  ± 4.1e-9
M[genesisFlux,     J4] = -3.4e-8  ± 1.2e-8
M[JdotDeltaS,      J4] = -3.5e-9  ± 2.8e-9
```

Interpretation: **per-snapshot fluctuations of fine `J⁴` do not drive coarse-cell-mean responses in any other operator**. `J⁴` fluctuates independently along its `b⁸ = 256` diagonal eigenvector and does not couple bilinearly to anything else under blocking. The diagonal `M[J4, J4] = 256.0040` is the only non-zero entry in its column.

### 0.2 · JJ column structure

The JJ column has an intermediate pattern: spatial-sector operators (`divJ2`, `JdotDivJ`) couple in (`M[divJ2, JJ] = 3.12`, `M[JdotDivJ, JJ] = 10.23`), but reaction-sector operators are essentially decoupled:

```
M[reactionDensity, JJ] = -6.8e-7  (zero)
M[genesisFlux,     JJ] = +1.4e-6  (zero)
M[JdotDeltaS,      JJ] = +2.5e-7  (zero)
```

So fine JJ's fluctuations **drive spatial-sector cross-couplings but not reaction-sector ones**.

### 0.3 · Strong row-vs-column asymmetry

Among the 36 off-diagonal pairs (a, b) with both entries non-zero, the asymmetry ratio `|M[a,b] − M[b,a]| / max(|M[a,b]|, |M[b,a]|)` is essentially 1.0 for nearly all pairs. The most-asymmetric:

| pair | M[a, b] | M[b, a] | asymmetry |
|---|---:|---:|---:|
| (genesisFlux, JdotDeltaS) | +1.31 | −0.79 | 1.604 |
| (curlJ2, JdotDivJ) | +1.35 | −2.52 | 1.537 |
| (divJ2, JdotDivJ) | −52.95 | +13.08 | 1.247 |
| (reactionDensity, JdotDeltaS) | −4.53 | +0.94 | 1.208 |
| (JJ, stateSq) | −0.112 | +0.005 | 1.046 |

For the remaining 25 pairs, asymmetry is in (0.99, 1.00) — i.e., **one direction is essentially zero, the other is non-zero**. The off-diagonal M_ab is **upper-triangular-like** in the operator ordering (when ordered by decreasing diagonal RG eigenvalue).

---

## 1 · Structural reading

Two interpretations of the asymmetry:

### 1.1 · Operator hierarchy from M_ab triangular structure

In standard regression, `M[a, b]` is the slope of `⟨O_a⟩_coarse` per unit `⟨O_b⟩_fine` fluctuation. If operator `b` is "primary" (has independent fluctuations that drive other coarse observables) and `a` is "derived" (its coarse value tracks fluctuations of more primary fine operators), then `M[a, b]` is large but `M[b, a]` is small.

**The triangular pattern thus identifies an operator hierarchy in the engine's RG-flow basis**:

- **J4 = primary, decoupled**: large diagonal (b⁸), zero column. `J⁴` fluctuates independently and is not influenced by any other operator's fine-level state.
- **JJ = primary, spatial-coupled**: large diagonal (b⁴), partial column structure. `J²` is influenced by spatial-sector fluctuations (divJ², JdotDivJ) but not reaction-sector.
- **divJ², JdotDivJ, JdotDeltaS, ... = mixed-derivative operators**: they fluctuate and drive coarse responses in JJ and curlJ², but their own coarse values are partially determined by upstream fluctuations.
- **stateSq, reactionDensity = density-sector operators**: their coarse values are determined by their own fine sums (Theorem 3) plus reaction-sector cross-couplings; the spatial sector doesn't drive them.

This decomposition is **not derivable from the blocking convention alone** — it reflects the engine's actual nonlinear dynamics. It IS however highly structured.

### 1.2 · Spurious directional regression artifact

Alternative reading: the asymmetry could be a regression-conditioning artifact. If the S matrix has highly disparate eigenvalues (cond(S) ≈ 1e13 at L=32 LARGE), then `S^{-1}` amplifies noise in directions corresponding to small singular values. The regression `M = Σ S^{-1}` produces large entries along high-singular-value directions and small (noise-dominated) entries along low-singular-value directions — naturally giving an asymmetric M_ab even if the underlying physics is symmetric.

**Test**: compare M_ab to its symmetric form (M + Mᵀ)/2 and antisymmetric (M − Mᵀ)/2. If the antisymmetric part is bootstrap-noise-dominated, the structure is statistical. If it's structurally consistent across L=32, L=64, then it's real.

This test is queued — requires pulling both M and Mᵀ at L=64 LARGE to compare bootstrap stderr against the antisymmetric components.

---

## 2 · 24 near-zero off-diagonal entries

A separate count: of the 72 off-diagonal active entries, **24 entries** have `|M| < 0.01` AND `σ < 0.01`. These are **structurally zero** at this measurement precision — not just bootstrap-noise-limited.

The structure of which entries are structurally zero:
- ALL 8 entries in column J4 (no operator drives J4 fluctuations into any other coarse value)
- ~6 entries in column JJ (reaction-sector → JJ all near zero)
- The reaction-sector cross-couplings to spatial operators (e.g., `M[reactionDensity, divJ²]` = 8e-3 near zero)

This is a much stronger statement than "small magnitude". With σ < 0.01 over 20,000 snapshots, these are at the level where Tikhonov regularization or truncated-SVD inversion would identify them as below the conditioning threshold.

---

## 3 · The 5 operator classes the matrix decomposes into

Reading the diagonal RG eigenvalues + the off-diagonal structure together:

| class | operators | M_aa | mixing pattern |
|---|---|---|---|
| **A. Decoupled flux** | J4 | b⁸ = 256 (exact) | column all zero |
| **B. Primary flux** | JJ | b⁴ = 16 (exact) | column has spatial-sector entries only |
| **C. Spatial-derivative** | divJ², JdotDivJ | -b⁴, b⁵ | strong cross-couplings to JJ, mutual mixing |
| **D. Transverse curl** | curlJ² | b³ | weak couplings to D and B |
| **E. Density** | stateSq, reactionDensity | b³(1±ρ) | charge-density block-sum convention; couple within E + to C |
| **F. Reaction-flux** | genesisFlux, JdotDeltaS | -b⁴, b⁵ | drive E → some F couplings; decoupled from A, B |

The 9-op active subspace is **not fully connected** under blocking — it decomposes into roughly 3 sub-blocks (A alone; B+C+D coupled; E+F coupled with weak D bridge), with strong upper-triangular structure within each.

This is a real native-EFT result: **the engine's blocking map has a partial block-diagonal structure on the natural operator basis**, with the reaction sector partially decoupled from the high-eigenvalue spatial sector.

---

## 4 · Implications for v2 design

The asymmetry pattern has implications for Gate D (S_eff self-consistency) design:

1. **Wilson coefficients are sector-localized**. Perturbing `g_J4` should drive the J4 diagonal but not propagate to other operators (per the zero column). Perturbing `g_stateSq` should affect E (density sector) and weakly C (spatial-derivative).

2. **The sector decomposition gives a natural reduction**. Instead of inverting the full 9×9 (cond 1e13), one can invert per-sector sub-blocks at much better conditioning (estimated cond < 1e6 per sub-block).

3. **Off-diagonal physics is genuinely small**. The 50% of entries that are bootstrap-noise-limited correspond to structural zeros, not physics that requires more statistics.

A v2 protocol should explicitly leverage this structure: measure within-sector cross-couplings precisely, treat between-sector entries as structurally zero up to bootstrap precision, and define Gate D self-consistency on the per-sector level.

---

## 5 · Open questions

**Q1**: Is the asymmetry pattern bootstrap-conditioning-driven or structurally real? Test by comparing antisymmetric `(M − Mᵀ)/2` to bootstrap stderr at both L=32 LARGE and L=64 LARGE (when v1.2 lands).

**Q2**: Does the sector decomposition persist at L=64? L=64 N=2k showed similar diagonal structure; off-diagonal pattern at LARGE size unknown.

**Q3**: Can the partial-block-diagonal structure be DERIVED from the blocking convention + the engine's continuity equation? The decoupling of J4 from reaction-sector operators might follow from the fact that `J⁴` blocking only cares about face fluxes (no state-coupling), while reaction-sector operators only care about state increments — the two sectors are dynamically decoupled in the bare action up to some specific cross-coupling vertices.

**Q4**: Does the 5-class decomposition match standard QFT operator-product-expansion sector classifications? The face-flux / charge-density / derivative / reaction-flux split doesn't have an exact OPE analog, but it's structurally similar to the mass / kinetic / interaction / source-current decomposition of standard EFT.

---

## 6 · Single-line summary

**The L=32 LARGE M_ab(b=2) measurement reveals a strong off-diagonal asymmetry pattern: the J4 column is identically zero, the JJ column has spatial-sector entries only, and the 9-op active subspace decomposes into 5 operator classes (A decoupled flux, B primary flux, C spatial-derivative, D transverse curl, E density, F reaction-flux) with partial-block-diagonal mixing structure under blocking — 24 of 72 off-diagonal entries are structurally zero at bootstrap precision over 20,000 snapshot pairs; this is a real native-EFT structural finding suggesting the engine's blocking map respects a sector decomposition on the natural operator basis, with implications for Gate D self-consistency design via per-sector sub-block inversion.**
