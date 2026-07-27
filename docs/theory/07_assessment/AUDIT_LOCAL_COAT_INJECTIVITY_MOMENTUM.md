# AUDIT — Local-coat injectivity and momentum

**Identifier:** `FTD-0465`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — LOCAL TRANSLATION NONINJECTIVE]` +
`[MEASURED — MOMENTUM MISMATCH]` +
`[CLOSED NEGATIVE — FTD-0464 MAP AS PHYSICAL EVENT]`  
**Run of record:** `engine/results/ftd_0465/windows_msvc_cpu.csv`

## Result

The additive partial-history translation admitted by FTD-0464 is not a
physical event map. It is noninjective and fails equal-and-opposite momentum
exchange in every registered arm. The locked verdict is

`LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_MISMATCH`.

Energy admissibility survives: all 84 events have valid particle kinematics
and zero measured energy residual. That is insufficient for transport.

## Exact noninjectivity

For radius `R`, consider each site on the selected cube's forward face. Put a
unit perturbation in one `J/W` component on that selected site and the opposite
perturbation on its unselected forward neighbor. The local map removes the
selected value from its old site and adds it to the neighbor, cancelling both
perturbations. The output is the zero field.

These witnesses are linearly independent because each has a unique forward-
face site and vector component. Therefore the map has nullity at least

`6(2R+1)^2`.

The campaign certifies:

| Radius | Independent zero-image witnesses | Image residual |
|---|---:|---:|
| R1 | 54 | `0` |
| R2 | 150 | `0` |
| R3 | 294 | `0` |

Global periodic translation is a permutation and reverses with residual `0`.
The information loss is caused specifically by adding the moved boundary to
an unmoved field value without retaining two separate native channels.

## Momentum failure

The registered field momentum change was compared directly with the particle
update's required recoil in all 42 `R=1` events, with initial dressing off and
on:

| Dressing | Momentum passes | RMS residual | Minimum | Maximum |
|---|---:|---:|---:|---:|
| off | 0/42 | `0.00203404` | `5.02183e-5` | `0.00408521` |
| on | 0/42 | `0.00225716` | `5.99471e-4` | `0.00438161` |

The mismatch is not a tolerance-edge effect: even the smallest residual is
more than seven orders of magnitude above the `1e-12` gate. Adding or removing
the selected initial dressing does not change the verdict.

## Correction to FTD-0464

FTD-0464 remains a valid constructive statement about energy and particle
kinematics: a 36-site-supported field reassignment can make the selected
one-step particle energy update real. It does not establish a viable local
field transaction. The post-event `J/W` field does not contain enough
information to reconstruct its input, and its field momentum does not balance
the particle.

The planned sequential replay of that map is therefore inadmissible. Reversing
it with an observer journal would hide missing native state in instrumentation,
and adding a separate recoil after the fact would define a new event.

## Ontological consequence

A material coat cannot be defined as "take whatever field lies in this cube
and add it one site forward." Bound field and ambient/radiative field overlap
in the same `J/W` variables. Once added, they have no native provenance label.
Either the bound coat must be represented by a dynamically distinct mode, or
motion must act through an injective transformation of the complete local
field.

## Next gate

Construct the smallest injective endpoint-local control: a cyclic permutation
of the four-site-long `R=1` old/new support columns. This moves the 27-site
interior coat forward while transferring the leading ambient face into the
vacated trailing face, preserving all six field components and admitting an
exact inverse. Test its event energy, particle kinematics, and field-momentum
closure without a compensating impulse. The permutation is a selected control,
not yet a production proposal.
