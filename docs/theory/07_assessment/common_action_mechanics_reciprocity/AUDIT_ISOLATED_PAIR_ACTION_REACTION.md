# FTD-0437 — Isolated-pair action-reaction mirror

**Date:** 2026-07-24  
**Status:** `[MEASURED — SELECTED FLUX-GRADIENT FORCE]`  
**Verdict:** `DIPOLE_ORIENTED_SELF_PROPULSION`  
**Particle-subsystem action-reaction:** `[CLOSED NEGATIVE — LOCKED PROTOCOL]`  
**Total particle-plus-central-field momentum:** `[CLOSED NEGATIVE BY FTD-0438]`

## 1. Result

FTD-0437 mirrored the wave-free control exposed by FTD-0436 over all three
axes, both polarity orientations, and both injection orders. Every arm
survived, retained separation `8`, remained finite, and repeated exactly.

For a pair with `+1` at the lower coordinate and `-1` at the higher coordinate,
the center of mass moves `+0.628381254469` sites along the pair axis. Swapping
the signs reverses the displacement to `-0.628381254469`.

| quantity | result |
|---|---:|
| common displacement magnitude | `0.6283812544690912...0.6283812544690919` |
| net particle-force RMS | `4.65291140823e-5` |
| integrated net particle force | `0.00639091754492` |
| maximum polarity-odd mirror residual | `3.54e-16` |
| polarity-even residual | `1.0` |
| injection-order residual | `0` |
| minimum pair separation | `7.999999999999998` |

The effect rotates exactly across `x,y,z`, follows dipole orientation, and is
independent of which particle was injected first. It is not a particle-ID or
construction-order artifact.

## 2. What is closed

The selected force gives the two equal-mass manifested sites a nonzero summed
force:

$$
F_++F_-\ne0.
$$

Their relative displacement remains zero while their common displacement is
macroscopic. Therefore the particle subsystem does not obey equal-and-opposite
internal forces in this isolated-pair protocol. No conservative two-particle
potential depending only on separation can reproduce this result.

The mechanism follows directly from the polarity-even source/probe sign
cancellation already isolated in FTD-0435/0436: the opposite source halos and
the explicit probe sign combine so that both manifested sites are pushed along
the oriented dipole.

## 3. Successor total-momentum result

Field theories need not conserve particle momentum separately. FTD-0437 alone
therefore did not prove total momentum creation. FTD-0438 subsequently derived
and source-free-validated the local central-generator candidate

$$
P_i^{field}=-\sum_x W\cdot D_iJ.
$$

It is conserved by free production waves to `7.99e-12`, yet remains below
`4.91e-18` while the pair carries `6.405e-3` particle momentum. The central
local compensating-recoil route is closed negative. Conceivable nonlocal
lattice quasimonenta remain outside that closure.

## 4. Correct statement

> The selected `G_C s grad|J|` channel causes exact, cubic, dipole-oriented
> self-propulsion of an isolated neutral pair at fixed separation. This closes
> Newton-third-law balance for the particle subsystem. FTD-0438 finds no
> compensation in the free-wave central local field-momentum generator.

The result supplies no electric, atomic, photon, pilot-wave, gauge, or total-
momentum-conservation claim.

## 5. Artifacts

- preregistration:
  `docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_ISOLATED_PAIR_ACTION_REACTION_v1.md`
- campaign: `engine/tests/campaign_isolated_pair_action_reaction.cpp`
- run record: `engine/results/ftd_0437/windows_msvc_cpu_L33.csv`
- manifest: `engine/results/ftd_0437/manifest.json`
- source SHA256:
  `783f186187b33e07ad467b4f4a54ce8ef78010e1d752099a7245d2035a674c6a`
