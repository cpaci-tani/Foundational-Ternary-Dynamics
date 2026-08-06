# FTD-0475 — Bound / Leading-Response / Wake Discriminator v2

**Status:** [PRE-REGISTRATION — LOCKED/RUN]  
**Date locked:** 2026-07-25  
**Production tick:** frozen

## 1. Disclosed revision-1 result

Revision 1 executed exactly as locked. Its morphology verdict was
`MIXED_OR_UNRESOLVED_MORPHOLOGY`: none of the eight localized runs met the
strict translated-profile explained-fraction gate, but none met either the
detached-wake or symmetric-dispersion gate. Both exact current
`s0-vacuum-photon` scenario arms passed the co-moving-bound clause and neither
passed the wake clause. The exact-energy drift remained below
`9.31e-13`.

Revision 1's response verdict was `NO_QUALIFIED_LEADING_RESPONSE`, with maximum
registered leading forces of only `5.25e-20` to `2.55e-19`. That result is a
valid measurement of the registered geometry but not a general packet-probe
test: the discrete-curl construction

`J=(0,D_z psi,-D_y psi)`

vanishes identically on its propagation axis `y=y0,z=z0`, and revision 1 put
the probe exactly on that nodal line. The zero-field control was exactly zero
and polarity oddness closed exactly, but the response arm sampled a symmetry
node.

The revision-1 artifacts are retained without alteration as
`morphology.csv`, `probe_response.csv`, and `verdict.txt` in
`engine/results/ftd_0475/`.

## 2. Sole revision-2 physics change

All packet parameters, volumes, amplitudes, directions, ticks, observers,
tolerances, row counts, morphology clauses, response clauses, controls, and
scope limits from revision 1 remain unchanged.

The sole physics-fixture repair moves the locked probe from

`(x,y,z)=(L/2,L/2,L/2)`

to

`(x,y,z)=(L/2,L/2-sigma_t,L/2)` with `sigma_t=3`.

This is the existing transverse-lobe placement already used by FTD-0457; it is
not selected by scanning the revision-1 output. The packet centre remains
`y0=z0=L/2`. The probe is still locked, movement remains off, and
`coupling=false`, so a positive result remains only a one-way leading response.

Revision 2 writes `morphology_v2.csv`, `probe_response_v2.csv`, and
`verdict_v2.txt`. The unchanged morphology matrix is rerun as an exact
reproducibility check; the revision-1 morphology values are already known and
no morphology threshold may change.

## 3. Locked interpretation

The morphology and response verdict definitions are incorporated by reference
from `PREREG_BOUND_PILOT_WAKE_DISCRIMINATOR_v1.md` without modification.

In particular, `ONE_WAY_POLARITY_ODD_LEADING_RESPONSE` still requires all 16
lobe-probe runs to exceed `1e-10` while the core centroid is more than six sites
behind the probe, exact polarity oddness within `1e-12`, and zero-field force
at most `1e-15`.

No revision-2 result establishes pilot guidance, reciprocal backreaction,
photon identity, quantization, a mechanical wake, or a common-action moving
manifestation.

## 4. Locked implementation

The revision-2 target compiled successfully before lock and was not executed.
The revision-1 files listed above were the only files present in the output
directory at lock time.

- revision-2 campaign source SHA-256:
  `9E705BC930CAEFC05A9AE25739EFD5235DEC51027068B3718F6C6A24BB874B43`
- unchanged morphology observer SHA-256:
  `10F485DBCEAC044C300A710EFCEFD6DDB5FA21B8DE2C0AA1E9AA6FC13278A558`
- unchanged localized packet helper SHA-256:
  `8AA0E4DBE189D2EADD277F43A5E7652D8459663F463B0B7654CA623CF02F64BA`
- unchanged exact scenario source SHA-256:
  `CDDDD9914588BE30FD539F773B006CED914C86158408BDE675FDC6865855855E`
- unchanged shared scenario helper SHA-256:
  `5B6E421ECA88B4B22A17D63E6002343E03028451DD2358F076F0FACB174152B8`

## 5. Run-of-record result

Revision 2 reproduced `morphology_v2.csv` byte-for-byte against revision 1.
Moving the probe to the transverse lobe changed individual force samples but
did not change the locked verdict: maximum absolute force over the complete
probe record was `1.03712e-18`, and the minimum per-run leading maximum was
`5.24582e-20`, far below `1e-10`. Polarity oddness and the no-field control
closed exactly. Verdicts remain `MIXED_OR_UNRESOLVED_MORPHOLOGY` and
`NO_QUALIFIED_LEADING_RESPONSE`.
