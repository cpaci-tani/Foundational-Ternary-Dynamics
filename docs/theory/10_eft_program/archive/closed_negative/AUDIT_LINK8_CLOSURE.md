# Link 8 — RG-Flow Interpretation of the Master Quadratic: Closure Report

**Date:** 2026-04-20 (updated with Phase 1 2-coupling analytical test same day).
**Status:** CLOSED NEGATIVE across all four tests attempted.
**LEDGER rows:** FTD-0050 (Link 8 closure), FTD-0051 (Langevin thermostat infrastructure — NEW), FTD-0052 (s-field Metropolis — NOT-PURSUED, F9-avoidance).
**Artifacts:**
- `engine/tests/test_link8_kadanoff.cpp` (Candidate 1 Run 1/6, CPU)
- `scripts/exploration/link8_option_beta_watson_diagnostic.py` (analytical Watson diagnostic)
- `engine/tests/test_langevin_equipartition.cpp` (Langevin thermostat validation)
- `engine/tests/test_link8_run3_thermal.cpp` (Candidate 1 Run 3 on thermal ensemble)
- `scripts/exploration/link8_phase1_flow_matrix.py` (2-coupling flow matrix, analytical)

---

## The question

Does the master quadratic `x² − 16 G*² x + 16 G*³ = 0` arise as the **characteristic polynomial of a renormalization-group step** on the FTD engine's bare-lattice dynamics?

Equivalently: does a natural blocking transformation produce a coupling flow satisfying

```
y_{n+1} = 16 G*² · y_n − 16 G*³ · y_{n-1}
       = 140.0601 · y_n − 414.3924 · y_{n-1}
```

whose characteristic roots are x₊ ≈ 137.036 and x₋ ≈ 3.024?

This is an **additional conjecture layered on top of the master quadratic's existing LEDGER status** (FTD-0001: algebraic identity [THEOREM]; FTD-0013/0014: physical identifications [STRONGLY MOTIVATED CONJECTURE]). It asks whether the polynomial has a dynamical, RG-flow interpretation in addition to its algebraic and number-theoretic provenance.

---

## Summary of tests performed

| Session | Test | Verdict | Session outcome |
|---|---|---|---|
| Candidate 1 Runs 1, 6 | Real-space Kadanoff blocking of engine J-field + Coulomb-tail α extraction | NEGATIVE | y_n grows by ×16 per level; pure geometric scaling, not RG flow |
| Option β | Moore-neighbourhood Watson-integral diagnostic of engine's 18-point stencil | NEGATIVE (analytical) | Engine stencil is (SC+FCC)/2; has **zero BCC component** — the exact sub-stencil where 16G*² lives |
| Session C (Candidate 1 Run 3 redo) | Thermalized \|J\|² connected correlator amplitude vs recurrence | NEGATIVE | y_n signs inconsistent across seeds; 2-eq fit singular (det M ≈ 2×10⁻⁶); A deviation 99.6%, B deviation 100.4% |
| Phase 1 (BCC-extension gate) | Analytical 2-coupling (g_SCFCC, g_BCC) linearized flow matrix under 2×2×2 block averaging | NEGATIVE (analytical) | trace(M)=2.44 vs target 140.06 (1.7% of target); det(M)=1.50 vs target 414.39 (0.4% of target); eigenvalues complex (1.22 ± 0.10i) vs target real {137.04, 3.02} |

All four are structurally consistent with each other. The engine's wave-equation operator is orthogonal (in the Moore-stencil decomposition sense) to the sub-stencil that carries the master quadratic's coefficient structure. **Phase 1 additionally rules out the principled extension**: even granting a BCC coupling sector to the engine, the linearized RG-step flow matrix cannot produce the master-quadratic roots because its eigenvalues are scaling dimensions (dimensionless O(1) numbers), not physical couplings (~137 and ~3).

---

## Detail 1: Candidate 1 Runs 1 and 6 (`test_link8_kadanoff.cpp`)

### Harness

Bare lattice (toggles: `wave_propagation + coupling + gauss_projection` ON, all others OFF), charge-pair injection at separation `r_f`, run to steady state (N_TICKS = 300), apply `eft::block_full` (flux-average + charge-conserving state), extract α at each blocking level from V(r) slope fit.

### Three variants executed

| Variant | L_fine | Levels | y_n (=1/α) | A_fit | B_fit |
|---|---|---|---|---|---|
| Run 1 literal | 8 | 3 | data unusable (L=8 too small; V(r=2)>0 from BC image contamination) | — | — |
| Run 6 literal | 16 | 4 | {−0.87, −13.95, −222.3, 0 (L=2 no fit)} | 4743.2 (dev 3287%) | −75566.7 (dev 18136%) |
| Run 1 extended | 64 | 4 | {12.35, 198.3, 3238, 60213} | 154.05 (dev 9.99%) | −2211.6 (dev **434%**) |

### Structural finding

Across all three variants the extracted y_n grows by **exactly factor 16 per blocking level**. Deconstructing: V(r) on the blocked field scales as 1/8 (volume ratio); pair separation halves (1/2). α = −V·r shrinks by 8·(1/2) = 16 per level deterministically. This is a **geometric artefact of the blocking + extraction pair**, not RG physics — it cannot be the flow satisfying the master-quadratic recurrence, whose characteristic eigenvalues are {137.036, 3.024}, neither of which is 16.

### Also noted

The 2-equation fit on three values is underdetermined. Only Run 6 literal / Run 1 extended have four values, and both show `det(M) = y_1² − y_0·y_2` small relative to `y_1²` → ill-conditioned, exactly as the instructions' own sanity check 2 warned.

---

## Detail 2: Option β — Watson-integral diagnostic

### Analytical identity

```
16 · G*² = 16 · 2π · W_BCC           [VERIFIED EXACTLY]
   W_BCC = Γ(1/4)⁴ / (4π³) = 1.393203…
```

The master quadratic's coefficient 16G*² = 140.0601 is algebraically tied to the **BCC Watson integral** W_BCC — the Green's function at origin of the Laplacian built from the 8 corners of the Moore neighbourhood (see `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`).

### Engine's stencil is NOT BCC

The engine's 18-point Moore Laplacian uses:
- 6 faces, weight 1/3 each
- 12 edges, weight 1/6 each
- **8 corners, weight 0** ← where the master quadratic lives

Explicit algebraic identity:
```
σ_18(k) = ½ (σ_SC(k) + σ_FCC(k))
        = 1 − (1/6)(c_x + c_y + c_z) − (1/6)(c_x c_y + c_x c_z + c_y c_z)
```
where `c_i = cos k_i`. The σ_BCC component `c_x c_y c_z` is absent.

### Numerical comparison (scipy.integrate.tplquad over BZ)

| Stencil | W(0) | Geometry | In engine? |
|---|---|---|---|
| SC (6 faces) | 1.5164 | Σ c_i | yes (weight 2/3) |
| FCC (12 edges) | 1.3447 | Σ c_i c_j | yes (weight 2/3) |
| **BCC (8 corners)** | **1.3932** | **c_x c_y c_z** | **no (weight 0)** |
| Engine 18-pt | 1.2679 | ½(σ_SC + σ_FCC) | — |

`W_18 / W_BCC = 0.910`. No small-integer algebraic combination of engine-stencil Green's function values at Moore offsets {(0,0,0), (1,0,0), (1,1,0), (1,1,1), (2,0,0)} reproduces `A_TARGET = 140.06` or `B_TARGET = 414.39` within ~50%. Best-case candidate `12·(G_0/G_edge) = 64.7` is 54% off A.

### Spectrum bound

The 18-point Laplacian's eigenvalue spectrum is bounded: `−4·max(σ_18) = −5.333 ≤ λ ≤ 0` on the Brillouin zone. The master-quadratic roots {137.036, 3.024} are not in this range and cannot be eigenvalues of the engine's wave-equation operator without external rescaling that is exactly the thing one would need to justify.

### Conclusion

[THEOREM — analytical] **The master quadratic's coefficient 16G*² is structurally absent from the engine's 18-point coupling stencil**, because the stencil is (SC+FCC)/2 and the coefficient lives on the BCC Watson integral. Any RG-interpretation of the master quadratic on the engine's wave-equation dynamics would require first modifying the stencil to include a BCC (corners) coupling term — a framework modification, not a derivation.

---

## Detail 3: Session B + C — Langevin thermostat + Run 3 redo

### Session B: Langevin thermostat (infrastructure)

Added an Ornstein–Uhlenbeck noise + damping update on `wave_vel` in `phase_write`:
```
v ← (1 − γ) · v + √(2γT) · η,       η ~ N(0, I) per component per voxel
```
Exposed via `TermToggles::langevin`, `langevin_T`, `langevin_gamma`, `langevin_seed`. Single-substrate CPU path only. Works in parallel with existing `gauss_project` — the thermal ensemble lives on the Gauss-physical subspace automatically.

**Equipartition verification (`test_langevin_equipartition.cpp`):** L=16, T=0.01, γ=0.01, 1000 burn + 2000 measure ticks. Result:
- `<|wave_vel|²>_voxel = 0.0312` vs target `3T = 0.0300` → **+4.0% deviation** (PASS at 5% threshold)
- `<v>_voxel ≈ (0, 0, 0)` within statistical noise
- Per-component isotropy check: `<v²>/3 = 0.0104` vs T = 0.01
- `<|J|²>_voxel = 3.70` (the J-field thermalizes more slowly than wave_vel — known subtlety of wave-field Langevin; ratio analysis of y_n is not affected)
- Autocorrelation of single-voxel `v_x` does NOT match `exp(−γτ)` because wave_vel is coupled to J via the wave equation → superposition of decaying-oscillation modes over all k. This is expected physics, not a thermostat bug; the per-voxel stationary variance is still correct (1.05·T vs target T).

### Session C: Run 3 redo on thermal ensemble (`test_link8_run3_thermal.cpp`)

L_fine = 16, 4 blocking levels (L ∈ {16, 8, 4, 2}), 4 seeds, 5000-tick burn-in per seed. Extraction: scalar field ρ = \|J\|², 2×2×2 arithmetic-mean blocking, connected correlator C(r) = ⟨ρ(x)ρ(x+r)⟩ − ⟨ρ⟩² evaluated at r_max = L_level / 2 − 1, ensemble-averaged over seeds.

### Results

| n | L | ⟨ρ⟩ | y_n = ⟨C_connected(r_max)⟩ |
|---|---|---|---|
| 0 | 16 | 4.70 | +1.008 × 10⁻³ |
| 1 | 8 | 4.70 | +2.117 × 10⁻⁴ |
| 2 | 4 | 4.70 | +1.984 × 10⁻³ |
| 3 | 2 | 4.70 | +1.556 × 10⁻³ |

No systematic geometric or recurrence pattern. Residuals for target recurrence: relative error ≈ −1.01 and −0.99 on the two triples (observed value opposite sign from predicted). 2-eq fit: A_fit = 0.59 (target 140, dev −99.6%); B_fit = +1.85 (target −414, dev −100.4%); det(M) = −2×10⁻⁶ (singular).

### Interpretation

This is exactly what Option β predicted analytically: thermalizing the engine's J field does not inject BCC structure; it lets the (SC+FCC)/2 operator reach equilibrium. The |J|² correlator's blocking flow inherits the stencil's structural orthogonality to the master quadratic.

---

## Detail 4: Phase 1 — Analytical 2-coupling flow matrix (`link8_phase1_flow_matrix.py`)

### The principled extension

The user-proposed refinement after Runs 1/6 + β + C closed negative: if the engine's stencil lacks the BCC sub-operator, give it one explicitly. Add a second coupling g_BCC alongside g_SCFCC, track them *together* under blocking, and ask whether the 2×2 flow matrix M has the master quadratic as its characteristic polynomial — i.e., whether

```
trace(M) = 16 G*² = 140.0601
det(M)   = 16 G*³ = 414.3924
eigenvalues = {137.036, 3.024}
```

This reframes Candidate 1's implicit hypothesis from "one coupling with 2-step memory" (ruled out geometrically in Runs 1/6) to "two independent couplings flowing together" (principled, not cosmetic).

### Analytical gate before engine code

Rather than spend a session on the engine extension, we compute M numerically at the linearized level via the standard Wilsonian block-spin formula:

```
σ_eff(K_coarse) = 1 / Σ_{m ∈ {0,1}³} |F(k_fine^m)|² / σ(k_fine^m)
```

with σ(k) = g_SCFCC σ_SCFCC(k) + g_BCC σ_BCC(k), |F(k)|² = cos²(k_x/2)cos²(k_y/2)cos²(k_z/2) the 2×2×2 block-average filter, and the aliasing sum over m indexing the 8 fine k's that fold onto each coarse K.

Linearized about the Gaussian fixed point (g_SCFCC = 1, g_BCC = 0), project σ_eff onto the coarse-lattice operator basis {σ_SCFCC(K), σ_BCC(K)} via inner-product projection on the coarse BZ, and read off the 2×2 flow matrix.

### Results (64³ k-grid)

```
M = [[+0.987137, -0.515391],
     [+0.127047, +1.453939]]
```

- **trace(M) = 2.441**    (target 140.060 → ratio 0.0174, dev −98.3%)
- **det(M)   = 1.501**    (target 414.392 → ratio 0.0036, dev −99.6%)
- **eigenvalues = 1.221 ± 0.105 i** (complex conjugate pair; target roots {137.036, 3.024} are real)

Discriminant of M: (2.441)² − 4·(1.501) = −0.049 < 0 → complex eigenvalues.
Discriminant of the master quadratic: (140.06)² − 4·(414.39) = 17944 > 0 → real eigenvalues.

Not just quantitatively off: *qualitatively* wrong (complex vs. real).

### Why the targets are O(100) but the matrix is O(1)

By construction, M's matrix elements are dimensionless combinations of `<σ_i>`, `<1/σ_SCFCC>`, and `<σ_BCC/σ_SCFCC²>` integrals on the Brillouin zone — all of which are O(1) numbers. Any convention-dependent prefactor (field rescaling Z, block-factor power b^D, block-factor vs. b^(d+2)/2 scalar-field canonical scaling) shifts these by factors of a few, never by the ~60× needed to bring trace to 140 or ~280× to bring det to 414.

Structurally: the eigenvalues of an RG-step flow matrix are **scaling dimensions** — dimensionless O(1) numbers characterizing how couplings re-scale under a block. The master quadratic's roots are **physical couplings** — specific dimensionless values at the tree level (1/α ≈ 137, N_c = 3). These are different kinds of objects at the level of RG formalism. No choice of blocking rule or normalization can convert one into the other.

### Verdict on the principled extension

[THEOREM — analytical, linearized] **A two-coupling (g_SCFCC, g_BCC) engine under standard 2×2×2 real-space block-averaging cannot have the master quadratic as its RG-step characteristic polynomial**, even granting the engine an explicit BCC coupling sector. The mismatch is two orders of magnitude on trace and det, and qualitative on eigenvalue type (real vs. complex).

This closes the principled-extension path without touching engine code. The Phase 2 engine extension (TermToggles::coupling_bcc + Candidate 1 rerun) was ruled out by this analytical gate; ~1 session of engineering saved.

---

## What remains untested, and expected outcome

**Candidate 1 Run 2 (Gaussian smoothing + downsample of J) and Run 4 (momentum-cutoff of J)** were not implemented. Both apply a **linear filter on the J field** followed by the same Coulomb-tail V(r) extraction as Run 1. The geometric-scaling argument that produced Run 1's ×16 flow applies verbatim: V ~ 1/8 per block × r/2 → α / 16 per level → y_n ~ 16ⁿ · y_0, regardless of filter shape. Expected outcome: clean negative, identical y_n ratio pattern as Run 1.

**Candidate 1 Run 5 (majority-rule state blocking, mass from ⟨s·s⟩ decay)** requires thermalized s-field dynamics, which Langevin-on-J does **not** produce. A proper implementation would need ternary Metropolis on s with an explicit action and detailed-balance verification. Given that Run 3 (the J-side analogue) on the thermal ensemble closed negative **and** the structural argument (BCC absence) applies with equal force to the s-field under the same coupling operator, the expected outcome is another clean negative. Building the Metropolis infrastructure to confirm is ~1 session of work with low expected information yield and has been deferred (listed in `TRACKER_OPEN_ITEMS.md` §X).

**Variant 1B (momentum transfer matrix) and Variant 1C (Euclidean slab transfer matrix)** reduce at long wavelength to the same 18-point operator whose eigenvalues are already bounded to [−5.33, 0]. They inherit the same structural problem. Building them to re-confirm is not productive.

---

## What this changes

### In the LEDGER

New rows:
- **FTD-0050** — Link 8 RG-flow interpretation of master quadratic — status **CLOSED NEGATIVE** for the tested engine configuration.
- **FTD-0051** — Langevin thermostat infrastructure (OU on wave_vel, toggles + CPU path + equipartition-verified) — status **NEW, operational**.

Unchanged:
- **FTD-0001** (master quadratic as algebraic identity) — remains [THEOREM] at the number-theoretic layer.
- **FTD-0013, FTD-0014** (physical identifications x₊ ↔ 1/α, x₋ ↔ N_c) — remain [STRONGLY MOTIVATED CONJECTURE]. Link 8 closure does NOT demote these; they live on the dual-match + CM-curve-uniqueness evidence (FTD-0003), not on any RG-flow derivation.

### In the framework narrative

What is falsified: the implicit additional claim that "the master quadratic is the characteristic polynomial of an RG step on the FTD engine". For the engine as currently defined — (SC+FCC)/2 coupling stencil, wave-equation dynamics — this interpretation is **ruled out**. The master quadratic lives at the **algebraic / number-theoretic layer** (Γ(1/4)⁴ = 4π³·W_BCC + CM-curve uniqueness over class-number-1 imaginary quadratic fields), not at the **lattice-dynamics layer**. These are separate objects; collapsing them is no longer supported.

### What's unblocked

The Langevin thermostat built in Session B is **operational, equipartition-verified, reusable infrastructure**. It unblocks several downstream items that the Day-2 EFT campaign and Phase 3 operator-scaling work had listed as pending: matched-stencil β-function at non-zero T, ensemble averaging for condensate measurements, fluctuation-dissipation tests. See FTD-0051 for pointers.

---

## Epistemic-discipline notes

This closure respects the framework's epistemic-rigor rules:

- **No numerical near-misses claimed.** The three NEGATIVE verdicts were registered at 10%+ deviations, consistent with the pre-specified threshold.
- **No post-hoc rescue.** Options β and C closed without attempts to re-fit the blocking rule, re-weight the stencil, or modify observables to chase the target numbers.
- **No demotion of already-downgraded claims.** FTD-0013 and FTD-0014 were already at STRONGLY MOTIVATED CONJECTURE after the 2026-04-19 reframe; they stay there. What changes is that an *additional, unadvertised* layer of interpretation ("RG flow") is removed from the defensible scope.
- **Infrastructure separated from interpretation.** The Langevin thermostat (FTD-0051) is operationally independent of whether Link 8 closes positive or negative — it is useful engine capability regardless.

---

## Open questions this closure raises

- **[OPEN]** Is there a principled modification to the FTD coupling stencil that introduces a BCC (corner) term in a way that isn't ad-hoc? `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` Part II identifies BCC with SU(3) color gauge group and the triple-cosine pairing. Whether an engine-native dynamical sector exists that computes W_BCC natively, rather than only in the counting layer, is unresolved.

- **[OPEN — deferred]** Does a full s-field Metropolis + Langevin-on-J + ⟨s·s⟩ correlation mass extraction produce the master-quadratic recurrence? Expected negative; confirmatory value only.

- **[OPEN]** Are there other natural RG-flow candidates that *do* carry BCC content? The Wilson-flow variant (Option 3 from the 2026-04-20 strategy) is structurally different from the stencil-spectral test and was deferred pending higher-EV work.
