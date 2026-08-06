# FTD-0736 — Causal-buffer relational formation v1

**Status:** `[SELECTED DYNAMICS + MEASURED — STRICT REENTRY CLASSIFIER
CLOSED NEGATIVE / PRECONTACT REENTRY AND FIELD RECEIVER POSITIVE]`  
**Verdict:** `PRECONTACT_REENTRY_WITHOUT_PERSISTENT_CORE`  
**Production status:** unchanged

## Result

The `L=129`, 112-tick campaign validly failed its locked conjunction: none of
the four initially unbound histories has pair energy below `-1e-6` at every
stored state beginning with the third graph transition. The failure is not an
execution failure. Every algebra, support, control, covariance, inverse, and
field-receiver gate passed.

```text
complete histories                                                5 / 5
unbound histories with registered third transition                4 / 4
strict negative-from-reentry histories                             0 / 4
qualified dynamic-field receivers                                  4 / 4
bound controls                                                      1 / 1
maximum measured current-source radius                                  3
earliest possible periodic self-contact tick                           123
locked observation horizon                                             112
maximum common-action residual                                9.63548e-14
maximum complete energy residual                              3.58275e-15
maximum recoil defect                                         3.16463e-14
maximum inverse recovery                                      2.20424e-11
maximum pair-plus-field balance defect                        6.94236e-15
```

The graph-transition sequences are exactly

```text
<001>    7;26;63
<01-1>   7;26;79
<111>    7;26;96
```

for both body-diagonal polarity orders. The maximum scalar difference between
those conjugate histories is zero.

## Causal conclusion

The registered current support stays within radius three of the matter
center. With the finite local field stencil, an emitted disturbance cannot
contact a periodic image and return to the source region before tick 123.
Every third graph transition occurs by tick 96. Periodic return of the newly
emitted disturbance therefore cannot be the cause of first re-entry in this
selected realization.

This conclusion does not remove the quotient-defined longitudinal dress
present at tick zero. It distinguishes newly emitted causal support from that
global initial condition.

## Why the locked classifier fails

Pair energy at third graph entry is still positive:

| ray | third transition | pair energy at entry |
|---|---:|---:|
| `<001>` | 63 | `+4.81810e-5` |
| `<01-1>` | 79 | `+6.41234e-5` |
| `<111>` | 96 | `+7.11121e-5` |

The raw histories then enter continuously graph-inside negative-energy tails
at ticks 78, 94, and 111: exactly 15 ticks later in each direction. This
post-result observation did not change FTD-0736's verdict. It became the
locked prediction of FTD-0737.

FTD-0738 proves the structural reason the strict classifier was too strong:
the selected potential and its first derivative vanish at the graph cutoff,
whereas a moving pair has positive kinetic energy. Generic graph entry must
therefore precede energetic binding.

## Field morphology

All four unbound histories satisfy the registered non-static receiver test:
dynamic-field norm exceeds `1e-8`, magnetic energy exceeds `1e-10`, and the
doubled median radius reaches at least five at a registered observation time.
This is evidence that the face/edge field is dynamically participating. It is
not a claim that the visual streamlines are literal strands, photons, wakes,
or pilot waves.

## Strict boundary

The result establishes precontact relational re-entry and a participating
field receiver for one selected common action. It does not establish negative
binding beginning at re-entry, persistence after tick 112, an uncontained
solution, an invariant basin, a production particle, or a native binding law.

## Verification anchors

- protocol `955FC333…EFFAB`;
- runner `B01CFCB…931F`;
- JSON `E6C8ECBC…B53F`;
- CSV `9B0C8296…1BDE` (`1125` rows, including tick zero);
- certificate `F320FB2F…654B`, `7951/7951 PASS`.
