# AUDIT — Local-support recoil threshold

**Date:** 2026-07-24  
**Identifier:** `FTD-0456`  
**Status:** `[CONSTRUCTIVE EXISTENCE — R=1 LOCAL ZERO-ENERGY RECOIL]` + `[MEASURED — EXTENSIVE IR BACKGROUND COST]` + `[OPEN — NATIVE EVENT SELECTION]`  
**Verdict:** `R1_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED`  
**Pre-registration:** [`PREREG_LOCAL_SUPPORT_RECOIL_THRESHOLD_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_LOCAL_SUPPORT_RECOIL_THRESHOLD_v1.md)  
**Run of record:** `engine/results/ftd_0456/windows_msvc_cpu.csv`

## 1. The recoil transaction has a strictly local realization

FTD-0456 re-solves the exact FTD-0454 paired-impulse quadratic after restricting
every nonzero `Delta J=Delta W=S` component to nested unions of the source and
target Chebyshev balls. It does not truncate the global answer.

Every registered arm crosses already at `R=1`, whose support contains 36 sites:

```text
Delta P_field = required face-hop recoil,
Delta(E_particle + H_interaction + E_tick) = 0,
S(x) = 0 exactly outside the R=1 support.
```

All six `L x propagation-sign` arms pass. The worst complete-energy residual is
`4.00e-15`, the worst momentum residual is `9.77e-16`, and the outside-support
maximum is exactly zero. The causal-support objection raised by FTD-0455 is
therefore closed for this registered face-hop construction.

## 2. Thresholds and the infrared cost

| `L` | `R=1`, against hop | `R=1`, with hop | mean `R=1` wave energy | mean global threshold |
|---:|---:|---:|---:|---:|
| 11 | `2.29161e-3` | `2.12777e-3` | `3.35255e-4` | `3.76670e-4` |
| 17 | `3.06355e-3` | `2.84796e-3` | `9.56546e-4` | `2.93401e-4` |
| 33 | `5.49505e-3` | `5.11151e-3` | `6.07794e-3` | `2.07067e-4` |

The local threshold is `5.87x`, `10.07x`, and `25.61x` the global threshold as
`L` increases. A three-point log-log diagnostic gives

```text
A_R1 ~ L^0.804,
E_R1 ~ L^2.650,
A_global ~ L^-0.543.
```

Those exponents are not promoted as asymptotic fits. Their direction is the
important result. `A_R1/L` and `E_R1/L^3` decrease toward roughly constant
values over the registered boxes, consistent with the expected long-wavelength
limit `A_R1 ~ 1/k ~ L` and an extensive background energy.

The reason is elementary: the local momentum rows see the wave through a local
gradient of order `k A`. A fixed recoil at fixed support requires `k A` to stay
finite. For the registered `n=1` mode, `k~1/L`, so `A` must grow approximately
as `L`. The local energy density then remains finite while total mode energy
grows with the volume.

## 3. Correct ontological statement

The result supports a local field-mediated transaction, but not yet a
finite-energy one-particle pilot wave in the infrared.

- The *response* is local: only the 36-site causal neighborhood changes.
- The *enabling background* is box-wide: its local slope remains finite only
  because its total energy trends extensively.
- The global construction retains finite total wave energy by distributing the
  recoil over the whole box, but that is precisely the nonlocal option the new
  gate was designed to exclude.

Thus FTD currently has two different limits: local mechanics with a macroscopic
background, or finite background energy with global response. It has not yet
shown both locality and finite isolated excitation energy in one construction.

## 4. Selection remains underdetermined

`R=1` contains 108 real impulse components. Three linear momentum constraints
leave a 105-dimensional affine space. Away from the exact minimum threshold,
intersecting it with the one scalar zero-energy surface generically leaves a
104-dimensional family. The campaign selects one member through the global
quadratic minimizer and its covariant-null construction; the production tick
contains no corresponding event rule.

At the precise threshold the strictly convex constrained minimum is unique,
which suggests a possible local trigger principle: a manifestation hop occurs
when its causal neighborhood first makes the minimum transaction cost zero.
That is an extrapolation, not a rule derived from the frozen postulates.

## 5. Intuitive questions that now control the research path

1. Is a manifested particle the *crest*, the *zero crossing*, or the *threshold
   event* of a localized flux packet?
2. Can a finite-energy packet carry the same local slope as the box-wide mode,
   or does recoil require a background energy density filling all space?
3. If several zero-energy recoil patterns are available, what local fact tells
   the lattice which one happens?
4. Does the event occur exactly when the minimum cost touches zero, making
   manifestation a threshold crossing rather than continuous motion?
5. If the travelling field reverses, does the preferred hop reverse with it,
   or does the state retain an independent inertia?
6. Can two overlapping packets steer one manifested state through their local
   superposition without introducing nonlocal coordination?
7. Does an `R=1` transaction emit a residual outgoing wave that carries a
   memory of the hop, or does the optimizer silently erase that information?
8. Can the same rule handle edge and corner Moore hops without selecting a
   hidden x/y/z route?

## 6. Next decisive gate

Replace the box-wide eigenmode with a finite-energy, localized, source-free wave
packet. Evolve it under the exact production wave tick, evaluate the `R=1`
minimum transaction cost as the packet passes the oriented link, and require:

- a zero-cost crossing at finite total packet energy stable with increasing box;
- a local rule based only on pre-event `R=1` history;
- independent reversal of the complete packet-plus-hop state;
- a positive outgoing spectral residue rather than deleted field information.

If the required packet energy stabilizes, FTD gains a plausible localized
pilot-field mechanism. If it grows with the box or transverse packet width, the
current construction is macroscopic-background mechanics, not isolated matter.

## 7. Scope and implementation provenance

The result covers a `+x` face hop, one transverse `n=1` mode at phase zero, two
propagation signs, and three periodic volumes. It does not establish edge or
corner transport, packet localization, production selection, repeated motion,
or no-faster-than-manifestation signalling. No production dynamics changed.

The first execution correctly returned `PROTOCOL_INVALID`: the new masked
helper evaluated its `K` stencil before fully populating the copied control
field. That failure is preserved in `invalid_first_execution.json`. The helper
was corrected without changing the registered arms, thresholds, campaign
source, or classification.

- campaign SHA256: `B01E6016F72C6D1929FCC55FA73ECFDA9B58CC35F695B909C0AC027E0D7A0D9A`
- corrected helper SHA256: `60F192FD417EBC94AF7D156EEB2C951AD3A75BF5A2397F7717AD3AE36533D376`
- record SHA256: `7DB8DD366DB2EC3D45195ADF702348CF8F423B4428BB4A8694C954CEA2980ECE`
- invalid-run record SHA256: `C3CD689AC378A83600A940ACADC1F497F64DC691B83382FC471AEAE122D7D8A5`
- compiler: pinned MSVC `14.44.35207`, Release

