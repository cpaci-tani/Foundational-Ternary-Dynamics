# Derivation: Material Emergence from the FTD Lattice

**Date:** 2026-04-24 (Phase-4h)
**Status:** [MEASURED] on GPU
**Purpose:** Empirically determine the smallest particle that emerges from the FTD lattice under the engine's native genesis rule, and classify its quantum numbers against the Standard Model.
**Ledger row:** FTD-0076

---

## 1. Question

What is the smallest particle that manifests spontaneously from FTD's lattice dynamics? Specifically, when a high-flux void region triggers the genesis rule, what single-voxel object is produced, and which Standard-Model particle does it correspond to?

## 2. Engine genesis rule (summary)

From [`kernels_stencil_single.cu`](../../../engine/cuda/kernels_stencil_single.cu) and the matching CPU path:

A void voxel $s(x) = 0$ with $|\mathbf{J}(x)| > 3 K_B$ becomes manifested according to:

$$ s_{\mathrm{new}}(x) = \mathrm{sign}(\nabla \cdot \mathbf{J}(x)) \in \{+1, -1\} $$

At the same time, **spin** and **color** are assigned:

$$ \mathrm{spin}(x) = \mathrm{sign}((\nabla \times \mathbf{J})_{\mathrm{dominant}}(x)) \in \{+1, -1, 0\} $$

$$ \mathrm{color}(x) = \mathrm{axis\text{-}of\text{-}dominant\text{-}flux}(\mathbf{J}(x)) \in \{1, 2, 3\} $$

with the color label 1 = red (x-axis), 2 = green (y-axis), 3 = blue (z-axis).

Crucially: **the color assignment is always nonzero** — the dominant-axis rule never yields $\mathrm{color} = 0$ except in the measure-zero case of exactly balanced flux across three axes (broken by the tie-breaking rule to always pick axis 0 = red).

## 3. Measurement

Test: [`engine/tests/test_smallest_particle_emergence.cpp`](../../../engine/tests/test_smallest_particle_emergence.cpp), CTest `smallest_particle_emergence` (labels gpu native eft). Six distinct flux injections at the centre of an $L = 16$ lattice, each triggering exactly one genesis event. Results:

| Injection | Emergent particle |
|---|---|
| (i) flux = $(A, 0, 0)$ | state=$-1$, spin=$\uparrow$, color=red |
| (ii) flux = $(0, A, 0)$ | state=$-1$, spin=$\uparrow$, color=green |
| (iii) flux = $(0, 0, A)$ | state=$-1$, spin=$\uparrow$, color=blue |
| (iv) flux $(A, 0, 0)$ + curl | state=$-1$, spin=$\downarrow$, color=red |
| (v) flux $(A, A, A)/\sqrt{3}$ balanced | state=$-1$, spin=$\uparrow$, color=red (tie→red) |
| (vi) flux $(A, A/10, 0)$ | state=$-1$, spin=$\uparrow$, color=red |

**All six genesis events produced a colored single-voxel particle.** Zero colorless emergences. The axis of dominant flux deterministically selected the color label.

## 4. Classification against SM

Standard Model quantum numbers of the lightest charged fermions:

| Particle | Electric charge | Spin | Color | Flavor |
|---|---|---|---|---|
| Electron $e^-$ | $-1$ (integer) | $\pm 1/2$ | colorless | lepton |
| Positron $e^+$ | $+1$ | $\pm 1/2$ | colorless | antilepton |
| Up quark $u$ | $+2/3$ (fractional) | $\pm 1/2$ | $\{R, G, B\}$ | none |
| Down quark $d$ | $-1/3$ (fractional) | $\pm 1/2$ | $\{R, G, B\}$ | none |

The single-voxel FTD emergence has:
- **Integer charge** $\pm 1$ — matches lepton, not quark
- **Single color** $\{R, G, B\}$ — matches quark, not lepton
- **Spin** $\pm 1/2$-like — matches both

**The emergent object is neither a clean lepton nor a clean quark under SM quantum numbers.** It carries integer charge (leptonic) but also carries a color label (quark-like). This is a **hybrid** object in SM terms.

## 5. FTD's own convention (from explicit constructors)

From [`engine/include/ftd/constructors.h`](../../../engine/include/ftd/constructors.h):

```cpp
electron(...):  state = -1,  color = 0  (colorless)
positron(...):  state = +1,  color = 0
quark(..., charge, color, ...):  state = ±1,  color ∈ {1, 2, 3}
```

FTD represents **both** electrons and quarks with **integer state** $\pm 1$ — the fractional quark charge ($\pm 2/3$, $\pm 1/3$) is an emergent property of composite baryons, not a primitive. A single-voxel FTD "quark" carries charge $\pm 1$, not $\pm 2/3$.

So under FTD's own representation:
- **Electron** = integer-charge voxel with `color = 0`
- **Quark** = integer-charge voxel with `color ∈ {1, 2, 3}`
- **Baryon** = composite of 3 quarks with color-singlet (R+G+B) state, net integer charge built from 3 × ±1 / 3 = $\pm 1, \pm 1/3$ ratios through combinatorics

## 6. What actually emerges from genesis

**The genesis rule never sets color = 0.** It always picks one of {R, G, B} based on the dominant flux axis. Therefore:

> **The smallest particle that emerges spontaneously from the FTD lattice via the genesis rule is a single-voxel QUARK** (by FTD's own color-label convention).

**Electrons do not emerge spontaneously.** They exist in FTD only as intentional constructions via the explicit `electron()` / `positron()` stamp functions, which override the dominant-axis rule and force `color = 0`.

## 7. Implications for FTD's emergence story

The FTD theory has:
- **Derived formulas** for lepton masses (electron 0.19%, muon 0.11%, tau 0.006%)
- **No clean derivation** for quark masses ([OPEN], see `DERIV_QUARK_MASSES_FROM_LATTICE.md`)

The engine has:
- **Spontaneous emergence** of quark-like colored objects from genesis
- **No spontaneous emergence** of electrons

There's a tension between theory and dynamics:
- Theory computes electron mass via the $\alpha$-ladder formula ($\alpha^{11}$ position)
- Engine produces quarks, not electrons, from void

**Possible resolutions:**

1. **The electron formula is an algebraic/structural prediction**, not a dynamical emergence. It predicts mass of an object that doesn't dynamically arise from the engine — the electron is a [SELECTION], not a [THEOREM], in the current FTD corpus. This resolution is consistent with the existing epistemic tagging.

2. **Electrons emerge from color-singlet bound states.** A baryon (3 quarks with complementary R+G+B colors) has color sum = 0 (in SU(3) singlet sense). If the engine's `color` field is summed over a bound triad and found to cancel, the composite is "colorless" and could function as a lepton. But this gives a 3-voxel object (baryon-like), not a single voxel. So electrons would be bound states of 3 quark-like voxels.

3. **The "color" label is misnamed.** If FTD's "color" is really just a directional tag (which axis the flux was aligned with) and has nothing to do with SU(3) color, then the single-voxel emergence is simply a CHARGED PARTICLE with a directional marker — closer to an electron than a quark. This would reframe the entire FTD color interpretation.

4. **Electrons emerge via weak transmutation.** Enabling the weak substrate + `weak_transmutation` toggle could in principle flip a colored state to a flavor-tagged leptonic state, erasing the color label. This is not currently implemented as such but is a testable conjecture.

## 8. Recommended next measurements

| Test | Question | Estimated effort |
|---|---|---|
| 3-quark binding → color singlet | Can 3 close-by R/G/B voxels bind into a color-neutral composite via exchange + triad + strong forces? | ~1 hour |
| Weak-transmutation electron emergence | Under dual substrate + weak, does a colored voxel ever transmute to color=0? | ~30 min |
| Genesis color-balanced test | Can a perfectly axis-balanced flux triple the color-tie-breaking rule to produce color=0? | ~15 min |
| FTD "color" vs SU(3) color comparison | Do the three engine colors actually transform as SU(3) under rotation, or as axis labels under $O_h$? (The latter would break the SU(3) identification.) | ~2 hours, analytical |

## 9. Epistemic tags

| Piece | Tag | Justification |
|---|---|---|
| Genesis rule sets state = sign(div J) | [THEOREM] (by code inspection) | Engine source |
| Genesis rule sets color = argmax(|flux_i|) ∈ {1,2,3} | [THEOREM] (by code inspection) | Engine source |
| Every single-voxel genesis emergence has color ≠ 0 | [THEOREM] | §6 conclusion from the above |
| Single-voxel emergent particle is SM quark | [CONJECTURE] | Depends on identifying FTD "color" with SU(3) color, which is an open claim (§7 option 3) |
| Electrons do not emerge spontaneously from void under current genesis rule | [THEOREM] (empirical + code inspection) | §6 |
| Electrons require explicit stamp via `constructors::electron()` | [THEOREM] (by code inspection) | Engine API |
| FTD electron-mass formula describes a physical emergent particle | [SELECTION] / [OPEN] | The formula is a parametric insertion — the lattice's own genesis rule does not produce the object the formula describes |

## 10. Relation to other ledger items

- **FTD-0016** ($m_p/m_e$ formula, 174-ppm gap): describes the relationship between two particles — but the PROTON in FTD is also a composite (uud baryon), so the ratio is between a bound-state baryon and an intentionally-stamped electron. The relationship is well-defined only once both particles are "created".
- **FTD-0060** ($K_{\mathrm{comp}} = m_e/\pi$): closed negative. Consistent with this doc's §7 option 1 (electron is not a dynamical emergent).
- **FTD-0063** ($\alpha/42$ mass gap closed negative): ditto.
- **Moore Layer Theorem** (`THEOREM_MOORE_LAYER_DECOMPOSITION.md`): claims the 12-edge decomposition of the Moore neighborhood contains 3 generations × 4 fermions. If this is correct, the 12 fermion SPECIES live in the edge structure of the Moore-26 decomposition — not at individual voxel sites. The engine's single-voxel "quark" is therefore only the 0th generation representative; the 12 fermion types would emerge as different edge-indexed labels on a composite.

## 11. Bottom line

**Spontaneous material emergence in FTD produces colored single-voxel QUARKS from the void.** Electrons are not spontaneously produced; they are intentional constructions in the current engine, consistent with the electron mass formula being tagged [SELECTION] rather than [DERIVED].

For the Branch-A native EFT paper, this means the honest framing of material content is:

> FTD's native genesis rule produces colored single-voxel fermions from high-flux void regions. The color label is deterministically set by the dominant flux axis; the state label (charge sign) follows the divergence sign. Leptons, atoms, and composite hadrons are not spontaneously produced by the engine and must be prepared as initial conditions or inferred as composite objects bound through the exchange / triad / strong force toggles. The relationship between the engine's single-voxel emergent and Standard-Model quarks depends on identifying FTD's "color" (spatial axis label) with $SU(3)$ color, which is itself a separate open claim tagged [SELECTION] in the theory corpus.

---

*Filed 2026-04-24 as Phase-4h. First empirical characterization of spontaneous material emergence in FTD. Upgrades the particle-physics ledger with a dynamical result that complements the algebraic mass formulas.*
