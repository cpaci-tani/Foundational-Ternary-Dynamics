# FTD-0737 — Precontact energetic-capture delay v1

**Status:** `[SELECTED DYNAMICS + MEASURED — PRECONTACT DELAYED ENERGETIC
CAPTURE]`  
**Verdict:** `PRECONTACT_DELAYED_ENERGETIC_CAPTURE_CONSTRUCTIVE`  
**Production status:** unchanged

## Result

All three preregistered symmetry-ray histories reproduce the FTD-0736 graph
sequence and the newly locked energetic prediction. A continuously
graph-inside, `E_pair<-1e-6` tail begins exactly 15 ticks after the third graph
transition in every direction and remains present through tick 122.

| ray | graph re-entry | negative-tail onset | delay | field-energy gain | `E_pair(122)` |
|---|---:|---:|---:|---:|---:|
| `<001>` | 63 | 78 | 15 | `1.170471873e-3` | `-8.88787938e-4` |
| `<01-1>` | 79 | 94 | 15 | `8.039155628e-4` | `-5.22231628e-4` |
| `<111>` | 96 | 111 | 15 | `4.115781704e-4` | `-1.29894236e-4` |

The complete numerical gates pass:

```text
histories and delay predictions                                   3 / 3
maximum measured current-source radius                                  3
earliest possible periodic self-contact tick                           123
last stored forward tick                                                122
maximum common-action residual                                9.46842e-14
maximum complete energy residual                              3.92915e-15
maximum recoil defect                                         3.16463e-14
maximum inverse recovery                                      2.53488e-11
maximum pair-plus-field balance defect                        6.39972e-15
```

The independent certificate also requires exact string equality of every
shared forward-prefix field with FTD-0736 through tick 112. The extension is
therefore not a reconstructed approximation of the parent histories.

## Dynamical interpretation

Within the selected common action, formation has an ordered sequence:

1. **encounter:** the constituents are graph-disconnected and unbound;
2. **interaction activation:** the derived graph turns on at the cutoff;
3. **relaxation and energy export:** current couples the moving constituents
   to the face/edge field while pair energy remains positive;
4. **energetic capture:** the connected core crosses into negative internal
   energy and the field carries the compensating gain.

The field is therefore a constitutive energy receiver in this transaction,
not merely an image surrounding pre-existing matter. The exact complete
energy identity requires field-energy gain when the pair crosses from
positive to negative internal energy.

The common delay of 15 ticks is a numerical fact for this locked depth,
cutoff, momentum, time step, and field normalization. No universality claim is
made. Deriving its value requires an independent local estimate from the
coupled action equations.

## Strict boundary

The final recorded state is the last tick before possible periodic
self-contact under the measured support bound. The result therefore says
nothing about persistence after contact, an outgoing tail on an uncontained
substrate, an invariant basin, attraction, or asymptotic stability. The
initial longitudinal dress remains quotient-defined. The compact potential,
constituent phase space, and common action remain selected research dynamics,
not consequences of the five postulates.

## Verification anchors

- protocol `677B054C…16C7`;
- runner `BA7141F6…5D1C`;
- JSON `E5622A9C…8CDA`;
- CSV `F164E336…731` (`735` rows);
- certificate `7931F639…1EEB`, `9271/9271 PASS`.
