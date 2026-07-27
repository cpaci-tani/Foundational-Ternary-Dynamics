# AUDIT — Travelling-wave recoil threshold

**Date:** 2026-07-24  
**Identifier:** `FTD-0455`  
**Status:** `[CONSTRUCTIVE EXISTENCE — CONDITIONAL ZERO-ENERGY RECOIL]` + `[MEASURED — MIXED LOCAL/GLOBAL SCALING]` + `[OPEN — LOCAL PRODUCTION MECHANISM]`  
**Verdict:** `TRAVELLING_WAVE_THRESHOLD_CONSTRUCTED_SCALING_MIXED`  
**Pre-registration:** [`PREREG_TRAVELLING_WAVE_RECOIL_THRESHOLD_v1.md`](../10_eft_program/preregistrations/PREREG_TRAVELLING_WAVE_RECOIL_THRESHOLD_v1.md)  
**Run of record:** `engine/results/ftd_0455/windows_msvc_cpu.csv`

## 1. A pre-existing travelling wave opens the energy-momentum surface

FTD-0454 proved that the quiet minimal field has no zero-energy paired recoil.
FTD-0455 superposes an exact divergence-free travelling eigenmode of the
production symplectic tick and repeats the global constrained calculation.

Every one of the 12 registered `(L,phase,propagation sign)` arms crosses from a
positive to a negative constrained minimum within `A in [0,1]`. At the crossing
the covariant-null construction produces an impulse satisfying both:

```text
Delta P_field = required recoil,
Delta(E_particle + H_interaction + E_tick) = 0.
```

The worst direct residuals are `3.30e-15` in energy and `3.68e-16` in momentum.
Thus a pre-existing native wave supplies genuine conditional phase-space
capacity that the quiet field lacks.

## 2. Thresholds

Mean thresholds by volume and propagation direction are approximately:

| `L` | Against hop (`sigma=-1`) | With hop (`sigma=+1`) | Mean wave energy |
|---:|---:|---:|---:|
| 11 | `3.90615e-4` | `3.62725e-4` | `9.74166e-6` |
| 17 | `3.04087e-4` | `2.82716e-4` | `9.42524e-6` |
| 33 | `2.14544e-4` | `1.99590e-4` | `9.26590e-6` |

Changing phase from `0` to `pi/2` changes the threshold only at rounding
level. Reversing propagation changes it by about `7%`: the wave travelling
with the hop requires the lower amplitude. This is a directional background
effect, not a polarity or phase fit.

The threshold wave energy is about one tenth of the registered particle work.
It is not paying the work directly—that exchange already closes against
`H_interaction`. It changes the geometry of the field's momentum-energy
constraint so recoil can lie on the zero-energy surface.

## 3. Scaling is deliberately classified mixed

Two observables have nearly volume-independent normalization:

- coefficient of variation of threshold wave energy: `2.09%`;
- coefficient of variation of `A_* sqrt(L)`: `2.04%`.

This is the scaling expected for a normalized box-wide `n=1` mode whose total
energy remains fixed as its wavelength grows.

However, the constructed event impulse is not box-wide:

- effective participation remains between `17.16` and `18.30` sites;
- `57.3–60.5%` of impulse norm lies inside the union of the source/target
  Moore-radius-one balls;
- participation fraction falls from `1.33%` at `L=11` to `0.050%` at `L=33`.

The preregistered global-reservoir verdict therefore does not fire. The
background is globally normalized, while the selected impulse is mostly local
with finite tails. The current result cannot decide whether those tails are
essential.

## 4. Ontological extrapolation

The strongest native story now available is conditional:

1. `J/W` carries a propagating dispositional wave before manifestation moves.
2. A site-to-site manifestation event changes particle momentum and the
   interaction Hamiltonian.
3. The pre-existing wave determines whether a simultaneous field recoil can
   satisfy both energy and momentum.
4. Co-propagating wave flow lowers the event threshold.
5. The manifested state may therefore be a localized readout riding a field
   configuration, rather than an autonomous bead pushing against empty space.

This is recognizably pilot-wave-like, but it is not yet a derived pilot-wave
dynamics. The optimizer proves that an allowed transaction exists; production
does not yet select or execute it.

## 5. Next decisive gate

Restrict the impulse support to nested causal neighborhoods of the hop:

```text
R=1: union of source/target Moore balls,
R=2,
R=3,
global control.
```

For each support, derive the constrained threshold rather than truncating the
global solution. If `R=1` or another fixed radius crosses with stable threshold
as `L` grows, the mechanism has a genuinely local realization. If only support
growing with `L` crosses, the apparent pilot-wave mechanism depends on
instantaneous access to a global reservoir and fails the native causality gate.

After locality, the selected transaction must be converted into an explicit
event rule and tested for independent reversal, multi-event interference, and
no-superluminal signalling. No production integration is justified yet.

## 6. Scope boundary

The result is for one face-hop orientation, one transverse mode number,
two phases, two propagation signs, three volumes, the selected particle branch,
and a globally optimized additive paired impulse. It does not establish edge/
corner thresholds, uniqueness of the impulse, local causality, or spontaneous
selection by the production engine.

No production dynamics were changed.

## 7. Reproducibility

- campaign SHA256: `621e89db07792f71cf10bdeac6ab0d2488c8b036e0fa246e56d8cae777e6d269`
- helper SHA256: `52b9c0679e55d08008feaae894b2d48c050581293864e50b822ee941b3ff4738`
- record SHA256: `5f4595cc9c58a138ca827d39afdc75d8d1bcbef8f9cb6e58395cb4f8506395c1`
- compiler: pinned MSVC `14.44.35207`, Release

