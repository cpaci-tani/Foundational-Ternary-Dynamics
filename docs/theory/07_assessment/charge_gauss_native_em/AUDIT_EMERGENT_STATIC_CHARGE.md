# FTD-0426 — Polarity-sourced static-charge discriminator

**Date:** 2026-07-22  
**Status:** `[MEASURED — CPU/WSL2 CUDA]` + `[SELECTED CONSTRAINT REALIZATION]` + `[SCOPED CLOSED NEGATIVE — autonomous static dressing in the frozen live profile]`  
**Verdict:** `POLARITY-GAUSS-READOUT-PASS; AUTONOMOUS-DRESSING-FAIL`

## 1. Question and epistemic split

Treat the ternary values `+1/-1` as primitive **polarity**, not as already-
identified electric charge. If one member of an initially neutral polarity
pair is transported from body A to body B, does the native flux field acquire
an operational static-charge readout?

The preregistration split this into two claims:

1. a selected Gauss law may map separated polarity to equal/opposite closed-
   surface flux;
2. the live wave/coupling dynamics may or may not maintain that field as a
   radius-independent static dressing.

This split is load-bearing. The production Gauss projector explicitly uses
signed state as its source. Passing the first gate cannot by itself count as
emergence of electromagnetism.

## 2. Frozen campaign

Two neutral composites were prepared on periodic lattices. In each mirror arm,
A and B initially contained one `q` and one `-q` site. The production movement
phase transported A's mobile `-q` member to B at `0.99*C_SPEED`; no direct
state teleport was used. The resulting bodies had polarity sums `Q_s(A)=q`
and `Q_s(B)=-q`, while the global sum remained zero.

The read-only observer measured the exact boundary form of the engine's
central-difference divergence on cube radii `3,4,5,6`:

$$
Q_{\partial R}=\sum_{i\in R}\nabla_c\cdot J_i.
$$

The field was first relaxed with Gauss projection alone, then evolved for 128
ticks with `wave_propagation`, state/flux `coupling`, `damping`,
`selective_damping`, and `gauss_projection` enabled. Manifestation, reactions,
forces, movement, clocks, gauge links, and phenomenological potentials were
off during the live stage.

Runs of record:

- Windows CPU, `L=32`, 30 SOR sweeps per tick;
- WSL2 Ubuntu-22.04 CUDA / RTX 5090, `L=64`.

Both polarity orientations were run on both backends. The surface telescope
closed to `2.56e-15` on CPU and `2.00e-15` on CUDA.

## 3. Results

The table reports the mean over radii for the `q=+1` arm. The `q=-1` arm is
the exact sign mirror to printed precision.

| backend | stage | `Q_A` | `Q_B` | radius spread A | radius spread B | max Gauss residual |
|---|---:|---:|---:|---:|---:|---:|
| CPU `L=32` | neutral | `-0.000173` | `-0.000173` | — | — | — |
| CPU `L=32` | projected | `+0.999252` | `-0.999079` | `0.00656` | `0.00774` | `0.00469` |
| CPU `L=32` | live | `+1.166000` | `-1.001700` | `0.37848` | `0.54539` | `0.33728` |
| CUDA `L=64` | neutral | `-0.000122` | `-0.000122` | — | — | — |
| CUDA `L=64` | projected | `+0.997216` | `-0.996793` | `0.02623` | `0.02973` | `0.01822` |
| CUDA `L=64` | live | `+1.173667` | `-1.011103` | `0.36575` | `0.52468` | `0.33859` |

The preregistered readout gate passes on CPU and CUDA. The neutral baseline is
below `1.8e-4`; separated surfaces have the correct signs, nontrivial
amplitude, equal/opposite means, radius spread below 3%, and Gauss residual
below 0.019. CPU/CUDA means agree within 0.010 at every promoted stage.

The autonomous-dressing gate fails on both backends. The live radius spreads
are 37–55%, versus the locked 15% ceiling, and the live Gauss residual is about
0.338, versus the locked 0.15 ceiling. The same signed failure in both mirror
arms rules out a sign accident. The same outcome at two volumes/backends rules
out a CPU-only fixture artifact.

## 4. What is established

Within the reaction-free preparation sector, the engine implements a coherent
static-electricity analogue:

$$
\text{neutral polarity pairs}
\xrightarrow{\text{production transport}}
\text{separated polarity}
\xrightarrow{\text{selected Gauss projector}}
Q_{\partial R}\simeq \pm1.
$$

Thus polarity can serve as the source label of an **effective Gauss charge**.
The operational charge is the closed-surface flux, not merely the local sign.
This is useful ontology: local `s` is the manifested polarity and the extended
`J` field is its dispositional dressing.

The result is nevertheless a `[SELECTED CONSTRAINT REALIZATION]`, not a native
derivation. `gauss_project_cpu` explicitly constructs its source from
`state-mean_state`, and the live coupling explicitly contains `-G_C grad(s)`.
The campaign shows that these rules realize the intended polarity-to-field
map; it does not show that the map was forced by the five postulates.

## 5. What fails

The current live wave/coupling/projector split does not maintain a localized,
radius-independent static dressing under the frozen low-energy profile. The
continually generated wave/coupling contribution outruns the approximate
cell-centered projection sufficiently to leave an order-`0.3` Gauss defect.
Increasing SOR sweeps or changing the stencil after seeing the result is not
admitted under this lock. This is a failure of the tested production profile,
not an impossibility theorem for every future local electrodynamics.

FTD-0421 remains controlling for microscopic conservation: genesis,
evaporation, and weak transmutation make the exact additive feature nullspace
trivial. FTD-0426 therefore does not reopen native `U(1)`, the dependency-
closed charged-pole campaign, or the native dimension-four flow campaign.

**FTD-0429 successor correction (2026-07-23):** the preceding statement is
limited to exact microscopic additive `U(1)` over the FTD-0421 basis. A later
unprojected native wave/coupling campaign derives and measures a finite
long-wavelength susceptibility `(div J)_k/s_k -> 3G_C`. Coarse-scale emergent
charge is therefore positive in that restricted reaction-free linear sector;
reaction-complete conservation and microscopic gauge charge remain open or
closed only as stated above.

## 6. Correct ontological statement

> FTD presently has primitive manifested polarity and a selected Gauss rule
> that turns separated polarity into an effective closed-surface flux charge
> in a restricted reaction-free sector. It does not yet have a native conserved
> gauge charge or a live, autonomous electromagnetic dressing.

Static electrification is modeled here only as transport of polarity between
neutral bodies. The material-specific transfer mechanism, low-energy
quasi-conservation rate under rare reactions, local Maxwell/Ampere evolution,
gauge redundancy, force law, photon pole, and empirical coupling normalization
remain unestablished.

## 7. Artifacts

- preregistration: `docs/theory/10_eft_program/preregistrations/lorentz_recovery_causal_structure/PREREG_EMERGENT_STATIC_CHARGE_v1.md`
- observer: `engine/include/ftd/eft/emergent_charge_surface.h`
- campaign: `engine/tests/campaign_emergent_static_charge.cpp`
- source lock: `scripts/proofs/emergent_static_charge_lock.json`
- lock verifier: `scripts/proofs/proof_emergent_static_charge_lock.py` (`22/22`)
- run manifest/data: `engine/results/ftd_0426/`
- result verifier: `scripts/proofs/proof_emergent_static_charge_results.py` (`25/25`)
