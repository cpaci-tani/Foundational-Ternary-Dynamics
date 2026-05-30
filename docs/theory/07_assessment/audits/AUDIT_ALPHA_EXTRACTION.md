# AUDIT — α Extraction Pipeline (Phase F "3.6× α_ref" claim)

**Status:** [AUDIT] — line-by-line review of every code path feeding the Phase F
`α_largeL ≈ 3.6 × α_ref` continuum extrapolation.
**Trigger:** user pushed back on the headline number as "a bold claim" and
asked for a full math/logic audit before publication.
**Date:** 2026-04-19
**Scope:** `measure_alpha_eff()` and every function it calls, plus the two
pipeline ports (`measure_v_of_r.h`, `benchmark_emergent_alpha.cpp`) and the
continuum-extrapolation script.
**Verdict (initial, superseded by Phase G):** the code is self-consistent.
The headline "3.6× α_ref" is arithmetically correct for what the engine
measures, but the comparison carries a factor-of-2 convention artifact
plus an unresolved ~1.85× residual.

**Verdict (final, Phase G 2026-04-19):** the residual is **not a
residual**. It is the zero-parameter value of the periodic lattice
Poisson Green's function `2 · r · G_L(r)` at the chosen r/L slice.
See [DERIV_EMERGENT_COULOMB_GEOMETRIC.md](../../10_eft_program/derivations/DERIV_EMERGENT_COULOMB_GEOMETRIC.md):
the engine's emergent V(r) mode is unit-charge geometric Coulomb with
**zero fine-structure content**, and the measured α_r matches the
zero-parameter analytical prediction to **R² = 1.0000, median 0.07%
relative error at L=384** in the Coulomb tail. The "plateau at 3.6×
α_ref" was a category error — comparing lattice-Coulomb geometry to
the electroweak coupling. This audit's three open interpretations
(real physics / kinetic normalization / Green's function artefact)
collapse to: **it is the Green's function, full stop.**

---

## 1 · What the code actually computes

### 1.1 The V(r) extraction (three co-existing codepaths — all equivalent)

All three α-extraction codepaths compute the same quantity:

1. `engine/include/ftd/eft/coupling_measurement.h::measure_alpha_eff()`
   (Phase 2, canonical reference)
2. `engine/sim/include/ftd/sim/measure_v_of_r.h` (Phase F, pipeline port)
3. `engine/tests/benchmark_emergent_alpha.cpp` (Phase 2 standalone, before
   pipeline existed)

Each runs three configurations on a fresh `RenderBridge` lattice:

```
  E_self_+ := energy_audit().field_energy   after  N ticks, one +1 charge
  E_self_- := energy_audit().field_energy   after  N ticks, one −1 charge
  E_pair   := energy_audit().field_energy   after  N ticks, one +1 and one −1 at separation r
```

Then forms:

```
  V(r)      = E_pair  −  (E_self_+ + E_self_−)
  α_r       = −V(r) · r
```

and fits `V(r) = −α_fit / r` by OLS of `V` vs `1/r`. **All three codepaths use
identical formulas**; cross-checked to the character level.

### 1.2 The `field_energy` accumulator

From `engine/src/diagnostics_compute.cpp:92`:

```cpp
for (int i = 0; i < N; ++i) {
    const auto& v = voxels[i];
    a.field_energy += v.flux.mag2();          // <-- NO 1/2 prefactor
    a.wave_energy  += v.wave_vel.mag2();      //     same
    ...
    a.E_field_energy += 0.5 * E.mag2();       // classical EM convention (1/2 factor)
    a.B_field_energy += 0.5 * B.mag2();       // classical EM convention (1/2 factor)
}
```

**Two energy conventions co-exist** in the same `EnergyAudit` struct:

| field | formula | convention |
|---|---|---|
| `field_energy` | Σ \|J\|² | engine-internal (no 1/2) |
| `wave_energy` | Σ \|J̇\|² | engine-internal (no 1/2) |
| `E_field_energy` | Σ ½\|E\|² | classical EM |
| `B_field_energy` | Σ ½\|B\|² | classical EM |
| `total_energy` | `field_energy + wave_energy + particle_ke` | engine-internal |

**V(r) uses `field_energy`** — the no-½ accumulator.

### 1.3 Why the convention matters for V(r)

Classical electromagnetism (Gaussian, natural units):

```
  W_classical(J) = (1/2) ∫ |J|²                  (field energy density = (1/2)|E|²)
  V_int_classical(J_1, J_2) = W(J_1+J_2) − W(J_1) − W(J_2)
                            = ∫ J_1·J_2                 (the ½ cancels on the cross term)
                            = q_1 q_2 / r                (= −α/r for opposite charges)
```

Engine:

```
  W_engine(J)  = ∫ |J|²  = 2 · W_classical(J)
  V_int_engine = W_engine(J_1+J_2) − W_engine(J_1) − W_engine(J_2)
              = 2 ∫ J_1·J_2
              = 2 · V_int_classical
```

So **the engine's measured V(r) is 2× the classical interaction energy**, and
therefore:

```
  α_r_measured  =  −V_engine · r  =  2 · (−V_classical · r)  =  2 · α_physical
```

If the engine's flux-field coupling is the one declared in `constants.h`
(`G_C = √α`, `ALPHA_EFT = G_C² = α`), then the **predicted** measurement under
the engine's own convention is `α_r → 2 · α_ref` for arbitrarily fine spacing $a$, not
`α_ref`.

---

## 2 · Numerical re-read of Phase F

Raw Phase F data (`scripts/benchmarks/continuum_extrapolate.py`), all at r/L ≈ 0.31:

| L | r_max | α_r (engine) | ratio to α_ref | after ÷2 (classical) | ratio classical |
|--:|--:|--:|--:|--:|--:|
| 64 | 20 | 0.02959 | 4.05× | 0.01480 | 2.03× |
| 128 | 40 | 0.02970 | 4.07× | 0.01485 | 2.03× |
| 256 | 82 | 0.02717 | 3.72× | 0.01359 | 1.86× |
| 384 | 124 | 0.02632 | 3.61× | 0.01316 | 1.80× |

1/L continuum extrapolation (engine convention):
  α_largeL = 0.02566, ratio **3.52× α_ref**.

1/L continuum extrapolation (classical convention, data ÷ 2):
  α_largeL = 0.01283, ratio **1.76× α_ref**.

The factor-of-2 is the **convention correction**. After removing it, the
engine still measures a plateau of roughly **1.76× α_ref** — which is neither
the headline "3.6×" nor the target "1×".

---

## 3 · Three possible explanations for the residual 1.76×

| # | Explanation | Testable signature |
|---|---|---|
| A | **Real physics.** FTD's bare lattice action produces α ≠ α_ref, and the engine's `ALPHA_EFT = G_C² = α` is a *definitional target* that the dynamics do not match. This would be a *genuine* FTD prediction. | β-function slope at fixed r/L; matching coefficient ≠ 1 between bare α and measured α |
| B | **Kinetic normalization.** Engine's Lagrangian uses `L_kin = \|∂J\|²` (no ½); classical uses `L_kin = ½\|∂J\|²`. Field redefinition J → J/√2 would absorb the factor but changes the coupling constant's interpretation. | Rerun with explicit ½ in `field_energy` and compare; only the quoted α changes, not the shape. |
| C | **Lattice-propagator / Yee-stagger artifact.** The 7-point Laplacian has a different Green's function than the continuum; staggered vs cell-centered placement of J shifts the effective 1/r coefficient. | Short-distance α_r(r) should show dispersion; Yee-aware stagger should reduce the residual factor. |

**Currently open.** None of (A), (B), (C) has been isolated; all three are
plausible and any two could cancel.

---

## 4 · What the audit does and does not conclude

**Conclusions:**

1. The three V(r) codepaths are bit-for-bit consistent with each other; no
   bug in how they accumulate, subtract, or fit.
2. `field_energy` is `Σ|J|²` (no ½). This is unambiguously confirmed in the
   source.
3. Because V(r) uses `field_energy`, the extracted α is **2× the classical
   Coulomb-tail α** under the same Lagrangian normalization as textbook EM.
4. The Phase F headline "α_largeL ≈ 3.6× α_ref" is correct for the engine's
   convention but over-states the physical discrepancy by a factor of 2.
   Corrected, the classical-convention plateau is **~1.8× α_ref**.

**Not conclusions (these remain open):**

5. Whether the residual ~1.8× is real physics, a kinetic-normalization
   choice, or a lattice-Green's-function artifact. All three are consistent
   with the currently available data.
6. Whether FTD as currently implemented produces α = α_ref in the continuum
   limit at all. The 4-point 1/L extrapolation extrapolates *within its own
   data* but does not demonstrate convergence to α_ref.

---

## 5 · Required documentation changes (all threaded in this commit)

- **DERIV_DAY2_CAMPAIGN.md** — §6b now reads "engine-convention plateau ≈3.6×,
  classical-convention plateau ≈1.8×; factor-of-2 convention correction
  documented in AUDIT_ALPHA_EXTRACTION.md; residual ~1.8× open."
- **PAPER_FTD_AS_WILSONIAN_EFT.tex** — abstract and §9 Phase F cite this
  audit; the headline number is now reported as a range (1.8–3.6×) with the
  convention caveat explicit.
- **CATALOG_PARAMETRIC_INSERTIONS.md** — Phase-F row updated to cite the
  audit; the old "1.23×" row remains retracted.
- **META_INDEX.md** — new entry under `10_eft_program/` for this audit.
- **STATUS_CUDA_BUILD.md** — physics-results paragraph cross-references this
  audit.

---

## 6 · Reproducibility pointers

```
engine/include/ftd/eft/coupling_measurement.h    # canonical V(r) extractor
engine/sim/include/ftd/sim/measure_v_of_r.h       # pipeline port
engine/tests/benchmark_emergent_alpha.cpp         # standalone variant
engine/src/diagnostics_compute.cpp:92             # field_energy accumulator
engine/include/ftd/render_bridge.h:55-72          # EnergyAudit struct
engine/include/ftd/constants.h:100-132            # ALPHA / ALPHA_EFT / G_C
scripts/benchmarks/continuum_extrapolate.py       # 4-point 1/L fit
scripts/benchmarks/results/eft_phaseF/            # raw CSVs per L
```

Running `python scripts/benchmarks/continuum_extrapolate.py` reproduces the
4-point fit used by this audit.
