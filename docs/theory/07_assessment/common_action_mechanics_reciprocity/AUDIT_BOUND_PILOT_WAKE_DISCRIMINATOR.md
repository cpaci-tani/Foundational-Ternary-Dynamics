# FTD-0475 — Bound / Pilot / Wake Discriminator Audit

**Date:** 2026-07-25  
**Status:** `[MEASURED — CURRENT SCENARIO CO-MOVING PLANE PACKET]` +
`[MEASURED — FINITE PACKET RESHAPING WITHOUT QUALIFIED WAKE]` +
`[CLOSED NEGATIVE — SELECTED ONE-WAY LEADING RESPONSE]` +
`[OPEN — RECIPROCAL GUIDANCE/RADIATIVE WAKE]`  
**Locked verdicts:** `MIXED_OR_UNRESOLVED_MORPHOLOGY` and
`NO_QUALIFIED_LEADING_RESPONSE`

## 1. The current vacuum scenario is not displaying a wake

The corrected current `s0-vacuum-photon` initializer produces a transverse
one-way plane packet. At both registered volumes it passes every locked
co-moving-bound clause:

| `L` | tick-32 displacement | speed | overlap | explained | trailing excess |
|---:|---:|---:|---:|---:|---:|
| 49 | 18.2091 | 0.569033 | 0.985767 | 0.908943 | `2.94e-8` |
| 65 | 18.2160 | 0.569251 | 0.985759 | 0.908903 | `1.07e-5` |

The packet retains about `94.9%` of its positive activity inside the locked
six-site co-moving core. Its relative exact tick-energy drift is below
`9.09e-13`. The small residual is leading, about `1.15%`, rather than trailing.

Within the current engine and registered time window, the flux shown by this
scenario is the propagating field excitation itself. It is not energy deposited
behind a separate photon object. Calling those streamlines a wake is therefore
unsupported.

This corrects the historical scenario state audited by FTD-0434. That audit
closed the old mismatched `J_z=g,W_x=g` initializer. The source now uses the
shared one-way packet construction. FTD-0434 remains valid provenance for the
removed initializer, not a description of the current source.

## 2. A finite three-dimensional packet is less rigid but still not a wake

The finite discrete-curl packet translates in both directions with exact
amplitude independence. At tick 32:

- `|Delta x|=14.624` at `L=49` and `14.526` at `L=65`, corresponding to
  centroid speeds `0.4570` and `0.4539` sites/tick;
- overlap is `0.918273` in every amplitude/direction arm;
- core fraction is `0.95376` at `L=49` and `0.94849` at `L=65`;
- trailing excess is only `2.293--2.311%`;
- leading excess is `1.124--1.682%`;
- relative exact-energy drift is at most `1.87e-13` in the finite arms.

All eight finite runs fail the deliberately strict bound clause only because
the shifted tick-zero profile explains `0.77497` rather than the required
`0.85`. Width grows from `2.307` to `3.644--4.115`. This is internal packet
reshaping/dispersion. It is not a qualified detached wake: the registered wake
gate required at least `10%` trailing excess plus a ten-point loss of core
fraction, and every run fails both conditions.

The finite packet's sub-cone x-centroid speed is expected from its transverse
and carrier momentum content; it is not the one-dimensional limiting speed.
The scenario's plane packet, whose Fourier support is parallel to x, resolves
the production cone much more closely. The plane packet is transverse-plane
extensive and therefore cannot establish a finite-energy photon on the
uncontained ontology.

## 3. The current selected force supplies no pilot-like leading response

Revision 1 placed the manifested probe on the exact nodal axis of the
discrete-curl packet and was retained as a symmetry control. Revision 2 moved
only the probe to the existing FTD-0457 transverse lobe, before execution and
under a new disclosed lock.

Neither geometry produces a qualified response. In revision 2:

- maximum absolute registered force anywhere in 512 samples is
  `1.03712e-18`;
- the smallest per-run maximum while the probe lies ahead of the core is
  `5.24582e-20`, versus the locked `1e-10` threshold;
- `F_+(t)+F_-(t)=0` exactly at every paired sample;
- the no-field control is exactly zero.

Thus the selected one-way production law

`F=G_C s grad |J|_tier2`

is polarity-odd but effectively blind to this travelling transverse packet in
the registered geometries. The result closes the claimed leading-response
mechanism for this branch and fixture family. It is not a no-go theorem for
every possible local coupling.

No pilot-wave claim survives this campaign. The probe is locked, the packet
does not receive reciprocal backreaction, and no manifested trajectory is
generated. FTD-0457's constructive transaction remains an existence result;
the production engine still does not select it.

## 4. Ontological statement now supported

The most economical current interpretation is:

1. a vacuum flux packet is a propagating substrate excitation, not a wake of a
   second hidden voxel-object;
2. its co-moving portion is the packet itself or its bound field dressing;
3. finite transverse localization causes ordinary reshaping without the
   registered signature of deposited trailing radiation;
4. a true wake should be reserved for detached field energy created by an
   interacting or accelerating manifested history;
5. pilot-like guidance remains open until one local common-action rule makes a
   manifested history follow the leading field while conserving the complete
   transaction ledger.

The boat analogy therefore belongs at an interaction boundary: bow field,
co-moving dressing, and radiative wake may all coexist around matter. It does
not describe the current source-free vacuum packet as a boat leaving energy in
its path.

## 5. Reproducibility and limits

- focused CTest: 1/1 pass under pinned MSVC 14.44;
- rebuilt golden targets and serial golden gate: 7/7 pass;
- morphology rows: 90 per revision;
- probe rows: 512 per revision;
- observer state/RNG neutrality: exact;
- revision-2 morphology reproduces revision 1 byte-for-byte;
- production tick modified: no;
- CPU scope: host exact-energy, force-diagnostic, and neutrality observers.

The repository-wide full build is independently blocked by duplicate
`expected_n`, `expected_q`, and `checkpoint` declarations plus missing
counterparts in the user-modified `test_scenario_behavior.cpp`. FTD-0475's
focused target and all seven golden targets build and pass; the unrelated file
was not changed.

Artifacts:

- preregistrations: `PREREG_BOUND_PILOT_WAKE_DISCRIMINATOR_v1.md` and `_v2.md`;
- source: `engine/tests/campaign_bound_pilot_wake_discriminator.cpp`;
- observer: `engine/include/ftd/eft/wave_morphology_observer.h`;
- records: `engine/results/ftd_0475/`;
- manifest: `engine/results/ftd_0475/manifest.json`.
