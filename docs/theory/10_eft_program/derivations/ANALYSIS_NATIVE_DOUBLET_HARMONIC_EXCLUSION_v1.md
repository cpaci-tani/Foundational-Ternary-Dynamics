# FTD-0780 — Native Doublet Harmonic Exclusion v1

> ## ⚠ AMENDED 2026-08-03 AFTER ADVERSARIAL AUDIT
>
> The **harmonic conclusion stands**, verified independently from raw `q0` without
> using `unwrapped_phase`. Three supporting claims are corrected:
>
> 1. **`unwrapped_phase` runs at exactly 2x the physical frequency** (measured ratio
>    `2.00120 +- 0.0007` across all 74 labels) - it is a phase on a quantity bilinear
>    in amplitude. So **"89.008681 cycles" is really ~44.5 cycles**, which is what
>    FTD-0772 reported all along.
> 2. **Sampling is 5.756 ticks/cycle, not 2.887.** Three independent methods on raw
>    `q0` agree: zero-crossings `5.743-5.822`, FFT `5.756-5.766`, three-term SHM
>    recursion `5.7549/5.7563/5.7557`. **The claim that Gate A is not runnable on this
>    corpus is RETRACTED** - at 5.756 ticks/cycle the true second difference recovers
>    acceleration with correlation `1.000000` and only a 9.5% amplitude bias.
> 3. **Precision overstated.** Independently measured `omega = 1.091789 / 1.091539 /
>    1.091645` is amplitude-independent to `2.3e-4`, not to six decimals. The
>    six-decimal agreement was a property of `unwrapped_phase`, a modal-projection
>    quantity that structurally cannot express amplitude dependence.

**Status:** `[ENGINE FACT — MEASURED, CORPUS-SCOPED NEGATIVE]` +
`[GATE A RUNNABLE — supersedes the retracted "sampling inadequate" reading]` +
`[OPEN — A CLOSING NATIVE OBSERVABLE]`
**Verdict:** `NATIVE_DOUBLET_EXCLUDED_HARMONIC_MODE`
**Parents:** `FTD-0659`, `FTD-0772`, `FTD-0778`, `FTD-0779`
**Production impact:** none; read-only, no engine execution, no artifact modified

## 1. Result in one sentence

The FTD-0659 doublet — the framework's strongest registered native phase-bearing
candidate — is a **harmonic** mode: its modal amplitudes stand in the exact ratio
`1 : 2 : 4` while its independently measured angular frequency is amplitude-independent
to `2.3e-4`, so `dOmega/dA = 0` and the quartic requirement `Omega ~ A` fails outright.

## 2. Method and its epistemic status

The load-bearing measurement is taken from the **raw `q0` column**, independently of
the stored `unwrapped_phase` (which §4 shows runs at twice the physical rate and is a
modal-projection quantity structurally unable to express amplitude dependence).

Epistemic status, stated honestly: the measurement has **low analytic degrees of
freedom** and reports a **negative with a 4x predicted effect against a 1x observed
one**, which is what limits selection risk here — not, as an earlier version claimed,
the total absence of anything selectable. Selection did occur: which corpus, which
quantity, which aggregation across 24 arms. The result arose exploratorily from the
sampling check of §4 rather than from a preregistration; FTD-0779's "GATE B cheap
pre-test" declares this test in advance for future candidates.

**Provenance caveat [OPEN].** Modal amplitudes of exactly `1.000 : 2.000 : 4.000` are
the signature of a *linearly prepared or rescaled* mode. If the doublet is driven rather
than dynamical, `dOmega/dA = 0` would be a property of how the data was generated rather
than a measurement — and the framework's standing rule forbids treating imposed phase as
native evidence. The raw-`q0` frequency measurement of §4 is dynamical and does not rely
on the stored phase, which mitigates but does not eliminate this. Establishing the
provenance of the FTD-0659 preparation is required before the harmonic reading is
treated as a substrate fact.

Corpus: `engine/results/ftd_0659/ftd_0659_native_excited_matter_clock_{arms,ticks}_v1.csv`,
72 phase-defined arms of 257 ticks (labels `o{orientation}_p{polarization}_a{amplitude}_q{quadrature}`).

## 3. Measurement

| amplitude index | `modal_amplitude` | ratio | measured `omega` (raw `q0`) | cycles |
|---|---|---|---|---|
| `a0` | `5.325323e-06` | `1.000` | `1.091789` | `~44.5` |
| `a1` | `1.065065e-05` | `2.000` | `1.091539` | `~44.5` |
| `a2` | `2.130129e-05` | `4.000` | `1.091645` | `~44.5` |

```text
cycle ratios observed : 1.000000, 1.000000, 1.000000
quartic requires      : 1.000000, 2.000000, 4.000000     (Omega proportional to A)
harmonic requires     : 1.000000, 1.000000, 1.000000     (Omega independent of A)
```

The amplitude range is exactly `4x`. The measured frequencies agree to `2.3e-4`
under the audit's estimator; an independent median-based SHM re-estimate gives a more
conservative spread of `1.6e-3` (the `a0` arm's amplitudes are ~`5e-6`, where relative
numerical noise is largest, giving it `sd ~ 5e-3`). The spread is estimator-dependent
within `[2e-4, 2e-3]`; against the required factor of `4` (`3e-1`), the conclusion is
unchanged under every estimator.

**Amplitude-regime caveat.** The absolute amplitudes span only `4.8e-6` to `2.1e-5`. In
that regime *any* smooth even potential is quadratic-dominated, so this is a weak test of
quartic-versus-harmonic in general. It is decisive only for **this corpus at these
amplitudes**, which is exactly what is claimed.

## 4. Sampling — corrected, and Gate A is runnable [RETRACTION]

The original version of this section claimed `2.887` ticks per clock cycle and concluded
Gate A was uncomputable here. **Both claims are retracted.**

`unwrapped_phase` advances at **twice** the physical rate: the ratio
`mean_phase_step / omega_raw` measures `2.00120 +- 0.0007` across all 74 labels. It is a
phase on a quantity bilinear in the amplitude (note `z_abs ~ amplitude^2`), so it
advances at `2 theta`. Three independent methods on the raw `q0` column agree on the
true period:

| method | period (ticks) |
|---|---|
| zero-crossings | `5.743 - 5.822` |
| FFT peak | `5.756 - 5.766` |
| three-term SHM recursion `q[k+1]+q[k-1] = 2 cos(omega) q[k]` | `5.7549 / 5.7563 / 5.7557` |

So the corpus spans **~44.5 cycles at 5.756 ticks/cycle**, not 89.0 cycles at 2.887.
FTD-0772's reported `44.5044` cycles was correct.

At `5.756` ticks/cycle the **true second difference** `q[k+1] - 2q[k] + q[k-1]` recovers
the acceleration with correlation `1.000000` and a 9.5% amplitude bias. **Accelerations
are entirely computable and Gate A is runnable on this corpus** with a correct
estimator. (`np.gradient^2` carries a 34% amplitude bias at this sampling - a further
reason it is the wrong operator; see FTD-0778 §6.)

## 5. Consequences

1. **The doublet is excluded as a quartic-clock candidate**, on grounds independent of
   closure and independent of the sampling problem. This closes Gate B negatively for
   this corpus without Gate A ever running.
2. **A harmonic mode is rate-degenerate.** With `H0'' = dOmega/dI = 0`, the map
   `I -> Omega` is constant, so the rate carries no information about internal action;
   the mode cannot serve as a clock that distinguishes states.
3. **This independently confirms FTD-0772 §7.** That section noted the doublet's
   positive Hessian eigenvalue `lambda_1 = 0.75321764` is structurally incompatible
   with the quartic requirement `V''(0) = 0`. The frequency-amplitude measurement is
   the direct dynamical statement of the same fact.
4. **Telemetry requirement, retained on other grounds.** FTD-0779 carries a `>= 20`
   ticks/cycle sampling requirement. The original justification (that FTD-0659's `2.887`
   made accelerations uncomputable) is **retracted** — the true rate is `5.756` and
   accelerations are computable. The requirement is nonetheless retained as good
   practice, since `np.gradient^2` carries a 34% amplitude bias even at `5.756` and
   comfortable oversampling removes estimator choice as a confound.

## 6. Status of the A -> B -> C sequence

| gate | corpus | outcome |
|---|---|---|
| A | FTD-0776 `q_active` / `q_all` | `UNINFORMATIVE` — metric degenerate on drift (FTD-0778, amended) |
| A | FTD-0776 `q_center` | **not yet validly screened** — live lead, hundreds of normal modes |
| A | FTD-0659 doublet | **runnable, not yet run** — 5.756 ticks/cycle with a correct estimator |
| B | FTD-0659 doublet | **`NATIVE_DOUBLET_EXCLUDED_HARMONIC_MODE`** (this result) |
| C | — | unreachable; no closing observable established |

**The corpora are not exhausted**, contrary to the original version of this section. Two
avenues remain in recorded data: a valid Gate A screen of the FTD-0659 doublet with a
correct estimator, and a first proper screen of `q_center`. Both require the repaired
metric of FTD-0778 §6. The new telemetry dump of FTD-0779 remains the route to a
purpose-built candidate.

## 7. Scope

An `[ENGINE FACT]` about one corpus at one amplitude regime, subject to the §2
provenance caveat. It licenses no claim that the substrate lacks a natural coordinate, a
recurrence, or a clock. FTD-0772 and FTD-0776 remain binding; FTD-0778 is superseded only
in the specific claims its own amendment banner retracts. FTD-0772's occupancy analysis
is **not** invalidated — it counts and averages rather than differentiating, and its
reported `44.5044` cycles was correct where this document's original `89.008681` was
not.
