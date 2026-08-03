# FTD-0780 — Native Doublet Harmonic Exclusion v1

**Status:** `[ENGINE FACT — MEASURED, CORPUS-SCOPED NEGATIVE]` +
`[GATE A NOT RUNNABLE — SAMPLING INADEQUATE]` +
`[OPEN — A CLOSING NATIVE OBSERVABLE]`
**Verdict:** `NATIVE_DOUBLET_EXCLUDED_HARMONIC_MODE`
**Parents:** `FTD-0659`, `FTD-0772`, `FTD-0778`, `FTD-0779`
**Production impact:** none; read-only, no engine execution, no artifact modified

## 1. Result in one sentence

The FTD-0659 doublet — the framework's strongest registered native phase-bearing
candidate — is a **harmonic** mode: its modal amplitudes stand in the exact ratio
`1 : 2 : 4` while its completed cycle counts are **identical to six decimal places**,
so `dOmega/dA = 0` and the quartic requirement `Omega ~ A` fails outright.

## 2. Method and its epistemic status

This is a **direct reading of the recorded corpus**, not a fitted statistic.
`unwrapped_phase` is a stored column; completed cycles are its span divided by `2 pi`.
There is no estimator, no bin choice, no tolerance, and nothing to select after
inspection — which is why the measurement is reported despite having arisen
exploratorily, out of the sampling-adequacy check of §4 rather than from a
preregistration. FTD-0779 §"GATE B cheap pre-test" already declares this test in
advance for future candidates.

Corpus: `engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_{arms,ticks}_v1.csv`,
72 phase-defined arms of 257 ticks (labels `o{orientation}_p{polarization}_a{amplitude}_q{quadrature}`).

## 3. Measurement

| amplitude index | `modal_amplitude` | ratio | completed cycles | spread across 24 arms |
|---|---|---|---|---|
| `a0` | `5.325323e-06` | `1.000` | `89.008681` | `2.7e-04` |
| `a1` | `1.065065e-05` | `2.000` | `89.008681` | `2.7e-04` |
| `a2` | `2.130129e-05` | `4.000` | `89.008681` | `2.7e-04` |

```text
cycle ratios observed : 1.000000, 1.000000, 1.000000
quartic requires      : 1.000000, 2.000000, 4.000000     (Omega proportional to A)
harmonic requires     : 1.000000, 1.000000, 1.000000     (Omega independent of A)
```

The amplitude range is exactly `4x`. The cycle counts agree to the sixth decimal.

## 4. Gate A is not runnable on this corpus

Sampling adequacy, measured before any hypothesis test: `257` ticks spanning
`89.008681` cycles gives

```text
2.887 ticks per clock cycle
```

against a Nyquist floor of `2` and a practical requirement of `>= 20` for reliable
second derivatives. All 72 arms sit at exactly this value; **zero arms exceed 20**.
Finite-difference accelerations carry `O((dt*omega)^2) ~ 1` error here — order unity —
so the `M1`/`M2` closure metrics of FTD-0778 cannot be computed meaningfully.

Note the asymmetry that makes §3 valid anyway: **cycle counting is robust at low
sampling because it counts rather than differentiates.** The phase is stored unwrapped;
no derivative is taken. Occupancy moments are likewise defensible if the sampled phases
equidistribute over the cycle, which is why FTD-0772's moment analysis stands even
though a closure test on the same data would not.

## 5. Consequences

1. **The doublet is excluded as a quartic-clock candidate**, on grounds independent of
   closure and independent of the sampling problem. This closes Gate B negatively for
   this corpus without Gate A ever running.
2. **A harmonic mode is rate-degenerate.** With `H0'' = dOmega/dI = 0`, the map
   `I -> Omega` is constant, so the rate carries no information about internal action.
   In the (refuted) FTD-0777-adjacent mechanism this would also give `G = 0`; more
   usefully, it means the mode cannot serve as a clock that distinguishes states.
3. **This independently confirms FTD-0772 §7.** That section noted the doublet's
   positive Hessian eigenvalue `lambda_1 = 0.75321764` is structurally incompatible
   with the quartic requirement `V''(0) = 0`. The frequency-amplitude measurement is
   the direct dynamical statement of the same fact.
4. **New telemetry requirement, now registered.** Any dump intended for Gate A must
   sample at `>= 20` ticks per clock cycle. FTD-0659 sampled at `2.887`. This is added
   to FTD-0779's admissibility rules.

## 6. Status of the A -> B -> C sequence

| gate | corpus | outcome |
|---|---|---|
| A | FTD-0776 `q_active`, `q_all`, `q_center` | `NATIVE_OBSERVABLE_CLOSURE_FAILED` (FTD-0778) |
| A | FTD-0659 doublet | **not runnable** — 2.887 ticks/cycle |
| B | FTD-0659 doublet | **`NATIVE_DOUBLET_EXCLUDED_HARMONIC_MODE`** (this result) |
| C | — | unreachable; no closing observable exists |

**Both existing corpora are now exhausted.** No further analysis of recorded data can
advance the sequence. Gate A requires the new observation-only telemetry dump specified
in FTD-0779, with the added `>= 20` ticks/cycle sampling requirement.

## 7. Scope

An `[ENGINE FACT]` about two specific corpora. It licenses no claim that the substrate
lacks a natural coordinate, a recurrence, or a clock — only that the two registered
candidate corpora do not supply one. FTD-0772, FTD-0776 and FTD-0778 remain binding and
unsuperseded; FTD-0772's occupancy analysis in particular is **not** invalidated by the
sampling finding, for the reason given in §4.
