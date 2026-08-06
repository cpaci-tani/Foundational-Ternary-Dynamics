# FTD-0723 — Field-assisted capture window v1

**Status:** `[SELECTED DYNAMICS + MEASURED — LOCKED WINDOW CLOSED
NEGATIVE]`  
**Verdict:** `NO_CAPTURE_WINDOW_OBSERVED_LOCKED_V1`  
**Production status:** unchanged

## Result

The FTD-0722 field export cannot be treated as a momentum-independent energy
loss. In the preregistered interval `p=0.0200--0.0300`, the absolute energy
export falls with incident momentum and never exceeds the incoming pair
energy. All 260 initially unbound arms enter and leave the instantaneous
interaction graph with positive final pair energy. None reaches the
negative-energy sector and none captures. All 52 already-bound controls remain
connected and negative.

The atomic transaction itself remains constructive. All 312 histories execute
and pass the current, Gauss, work, total-energy, state-only inverse,
translation/polarity, and symmetric-recoil gates. This closes the locked
FTD-0723 window, not field-assisted formation in every energy range.

## Preregistered prediction and campaign

FTD-0722 measured export

\[
\Delta E_f\in[0.0012012704657176076,
               0.001374812629945682]
\]

at `p=0.07`. Equating that range to the exact outside-support pair energy

\[
K_{\rm pair}(p)=2\left(
\sqrt{E_{\rm REST}^2+\frac{p^2}{3}}-E_{\rm REST}\right)
\]

gave the preregistered constant-export bracket
`[0.02479781232348018,0.02653199646140160]`. The locked five momenta were
`0.0200`, `0.0225`, `0.0250`, `0.0275`, and `0.0300`. The first two were
predicted to capture, the last two to escape, and `0.0250` was left
unclassified.

The prediction failed in both low-momentum families:

| `p` | initial pair energy | field-energy export | final pair energy | active ticks | negative/capture |
|---:|---:|---:|---:|---:|---:|
| 0.0200 | 0.000781882 | 0.000310209--0.000359039 | 0.000422842--0.000471673 | 17 | 0/52 |
| 0.0225 | 0.000989268 | 0.000344350--0.000403119 | 0.000586149--0.000644919 | 16 | 0/52 |
| 0.0250 | 0.001220904 | 0.000379162--0.000448183 | 0.000772721--0.000841742 | 15 | 0/52 |
| 0.0275 | 0.001476741 | 0.000414657--0.000494044 | 0.000982697--0.001062084 | 16 | 0/52 |
| 0.0300 | 0.001756723 | 0.000451725--0.000541685 | 0.001215037--0.001304998 | 15 | 0/52 |

Every unbound arm changes graph membership exactly twice. Dynamic field and
magnetic energies are nonzero, but the dynamic-field median doubled radius is
two in every arm, below the unchanged detached-field threshold four.

```text
complete histories                         312 / 312
common-action identity arms                312 / 312
state-only inverse arms                    312 / 312
symmetric recoil arms                      312 / 312
unbound negative-sector arms                 0 / 260
unbound captured arms                        0 / 260
already-bound controls retained             52 / 52
maximum common residual                    1.980e-11
maximum pair/field energy-balance residual 7.431e-12
maximum recoil defect                      9.467e-12
maximum 24-step inverse recovery           3.694e-10
translation/polarity scalar-history spread 5.310e-10
```

## Descriptive extrapolation

The following fit was not a preregistered FTD-0723 acceptance test and has no
claim status beyond `[MEASURED — DESCRIPTIVE]`. Across the five momentum
means,

\[
\langle\Delta E_f\rangle
\simeq 0.0160828p+8.19\times10^{-6},
\qquad R^2=0.99967,
\]

while a zero-intercept fit gives `0.0164040 p`. A log fit gives exponent
`0.9764`. Thus the narrow observed range is approximately linear in momentum,
not constant. Since the exact low-momentum pair energy begins quadratically,
continuing the zero-intercept fit predicts a mean crossover at
`p=0.0083841`. Applying the same zero-intercept construction separately to
the measured minimum and maximum export envelopes gives
`p=0.0077583--0.0091870`.

This interval is a new falsifiable extrapolation, not evidence of capture. The
fit may fail as the encounter time grows, and the detached-field gate may
remain closed even if pair energy becomes negative. A fresh lower-momentum,
longer-horizon protocol must test it without reusing its output to move the
bracket.

## Ontological consequence

The result does not force a new matter primitive. Existing constituent
position/momentum plus face-electric and edge-magnetic variables still solve a
complete reciprocal encounter and route energy exactly. What fails is the
specific assumption that the FTD-0722 export supplies a fixed energy quantum
independent of encounter momentum.

For this selected action, formation remains a dynamical threshold problem:
the pair must enter a negative internal-energy basin before the field receiver
decouples. FTD-0723 shows that simply lowering `p` into `0.020--0.030` does not
do so. It does not decide whether a still slower single pass, repeated passes,
a third constituent, an environmental pressure, or a different action-selected
interaction is required.

## Scope

The compact pair well remains selected. The five-point scaling law is
descriptive and finite-volume. No physical particle, photon, radiation law,
capture cross section, quantum bound state, or production dynamics is derived.
The closed claim is exactly the preregistered `L=33`, 24-tick,
`p=0.0200--0.0300` capture window under the frozen FTD-0722 action.
