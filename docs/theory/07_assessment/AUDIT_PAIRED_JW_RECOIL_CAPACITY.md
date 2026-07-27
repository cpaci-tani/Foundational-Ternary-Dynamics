# AUDIT — Paired J/W recoil capacity

**Date:** 2026-07-24  
**Identifier:** `FTD-0454`  
**Status:** `[THEOREM — GLOBAL ADDITIVE PAIRED-IMPULSE MINIMUM]` + `[CLOSED NEGATIVE — ZERO-ENERGY RECOIL FOR REGISTERED MINIMAL FIELD]`  
**Verdict:** `PAIRED_JW_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD`  
**Pre-registration:** [`PREREG_PAIRED_JW_RECOIL_CAPACITY_v1.md`](../10_eft_program/preregistrations/PREREG_PAIRED_JW_RECOIL_CAPACITY_v1.md)  
**Run of record:** `engine/results/ftd_0454/windows_msvc_cpu.csv`

## 1. Result

FTD-0454 lifts FTD-0453's fixed-`J` restriction in the way required by the
production symplectic ordering. Relative to the exact source-free control tick,
an arbitrary event impulse updates

```text
W_event = W_control + S,
J_event = J_control + S.
```

The optimizer was allowed to use all 3993 impulse components. It included the
particle energy change, the post-hop interaction Hamiltonian at the updated
`J`, and the exact modified wave tick energy.

All 26 recoil vectors are realizable. Momentum closes to `1.32e-18`, the
complete analytic minimum agrees with direct recomputation to `3.47e-17`, and
reverse subtraction restores the control state to `9.97e-18`. Nevertheless,
the global minimum remains strictly positive:

| Hop orbit | Arms | Minimum complete energy change | Cost / work |
|---|---:|---:|---:|
| face | 6 | `0.0105624175766` | `105.62` |
| edge | 12 | `0.0190287358204` | `190.29` |
| corner | 8 | `0.0215408259467` | `215.41` |

No zero-energy solution exists in the registered affine recoil space.

## 2. Exact reduction

The field momentum is

```text
P_i(J,W) = -sum_x W_x dot D_i J_x.
```

For the paired impulse, periodic summation by parts cancels the apparently
quadratic `S dot D_i S` term and gives

```text
Delta P_i = -sum_x S_x dot D_i(J_control-W_control)_x
          = -sum_x S_x dot D_i J_old_x.
```

The momentum constraint therefore remains linear. The exact wave-energy
change plus the field-dependent interaction-energy change is

```text
F(S) = 0.5 ||S||^2 + c dot S.
```

Consequently the same completed-square calculation used in FTD-0453 finds the
global minimum; this is not a one-kick ansatz or a finite search.

## 3. What changed relative to fixed J

Allowing the conjugate field coordinate to move matters. Compared with
FTD-0453, the minimum cost fell by approximately:

- `86.9%` for face hops;
- `88.2%` for edge hops;
- `91.1%` for corner hops.

The interaction-divergence term and paired field update therefore supply real
recoil capacity. They simply do not supply enough capacity in an initially
quiet minimal field. The result is not “J cannot carry momentum”; it is “this
vacuum-like field cannot carry the required momentum at zero total energy.”

## 4. Intuitive consequences

1. **Was freezing J the whole problem?** No, although it exaggerated the
   energy obstruction by nearly an order of magnitude.
2. **Does the native symplectic pairing help?** Yes. It is quantitatively
   important and must be retained in every future mechanics test.
3. **Can the interaction term pay the remaining recoil cost?** Not in the
   registered minimal background.
4. **Can a wider impulse support help?** No. The optimizer already used the
   whole periodic lattice.
5. **Is exact reversal enough?** No. Reversal is exact, but the forward event
   still lies above the conserved-energy surface.
6. **What native resource has not been tested?** Pre-existing wave phase and
   momentum. A travelling background changes the linear coefficient and may
   provide the missing negative energy capacity.

## 5. Next decisive gate

The next campaign should not invent a new channel field yet. It should add a
pre-existing source-free travelling mode and determine the exact amplitude and
phase conditions under which the paired-impulse minimum crosses zero.

This is a physical discriminator:

- if no subcausal finite-energy background crosses zero, the native
  three-vector branch is effectively closed for additive event impulses;
- if a threshold exists, matter transport becomes conditional on local wave
  phase—an explicit pilot-wave-like mechanism that must then be tested for
  directionality, reversibility, and spontaneous signalling artifacts.

The threshold must be derived from the constrained quadratic. It must not be
tuned to a measured physical constant.

## 6. Scope boundary

The closure applies to additive paired impulses, the central momentum
generator, the selected FTD-0450 particle branch, initially quiet minimal
backgrounds, and the registered work/velocity. It does not exclude pre-existing
waves, non-additive canonical maps, alternative momentum generators, or a
13-channel ontology.

No production dynamics were changed.

## 7. Reproducibility

- campaign SHA256: `4801c383d6f8193c5ef355b16ffc309fb4c0291e294ffbf40066a1d249de85c7`
- helper SHA256: `c1ac49fb0ee222e3192b89d6b64f4e382e7b18c3edfd64327275beb5ed30aec5`
- record SHA256: `5221b51933a3ce5e109249fb687e299dbf8acb91b56bd02062d156dec3c63494`
- compiler: pinned MSVC `14.44.35207`, Release

