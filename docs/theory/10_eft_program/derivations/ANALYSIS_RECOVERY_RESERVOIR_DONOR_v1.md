# ANALYSIS — Recovery-reservoir donor discriminator v1

**Campaign:** `FTD-0674`  
**Status:** `[EXECUTION INVALID — NO LOCKED RECOVERY]`  
**Verdict:** `RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID`  
**Production impact:** none

## Locked result

The full `L=97`, tick-80, control/± history and reverse reconstruction
executed. Both signs preserved the selected sector, complete action, exact
FTD-0673 observer, and state-only inverse. The donor verdict nevertheless
fails two preregistered gates.

First, the canonical target-mode energy does not recover from tick 72 to tick
78:

| sign | target at 72 | target at 78 | change | target at 80 |
|---:|---:|---:|---:|---:|
| - | `0.6310795959` | `0.6058984344` | `-0.0251811615` | `0.6015651639` |
| + | `0.6310795605` | `0.6058983970` | `-0.0251811635` | `0.6015651301` |

The locked requirement was a rise of at least `+0.05`. No donor may be named
when the registered target is not a recipient.

Second, the normalized five-term interval closure is `4.87e-7` and `8.17e-7`,
above the locked `1e-8` gate. In raw units these are about `5e-18` and `9e-18`;
the normalization by an `~1.12e-11` initial target makes accumulated
floating-point/action drift visible. The observer's per-tick algebraic
residual remains below `2.82e-12` normalized.

The internal classifier printed `NONLINEAR_MATTER_REMAINDER_DONOR`, but that
label is non-reportable because execution is invalid and the target falls.

## Diagnostic consequence

The discrepancy with FTD-0670/0672 exposed the mass-metric defect proven by
FTD-0675. Those campaigns used `v^T delta x` as a normal coordinate despite
mass-orthonormal eigenvectors. FTD-0674 uses `v^T M delta x`, so it measures
canonical quadratic mode energy. The apparent earlier trough/recovery is not
present in the corrected observable through tick 80.

## What remains learned

The fresh corrected history supports a narrower behavior:

```text
canonical target-mode energy decreases
dynamic difference-field self energy increases
state-only inverse still closes.
```

This is reversible energy transfer from a prepared localized matter mode into
the coupled field over the measured causal window. It is not yet asymptotic
radiation, irreversible decay, a lifetime, or a resonance width.

## Run of record

- protocol: `EC89065A9996C233978E164533D878200275B203646921222500928062C60383`;
- runner: `6593028B7137CD35EA7E6EECC301C1E1EFF3ADF3110527B9837B463B35CE0638`;
- JSON: `1848283E5AF91B076E7DD69CB24B4677159FED8594F2C78A5D8D858F441044CB`;
- tick CSV: `DEA0582DD2E135071524CBAB6F532A74FCCEE49D2F88E163966E6CC6DE4364E9`.
- independent certificate:
  `C66F60018BAAEA87D14D1194DBE78774414AB4A07672FC756B58788C2270AE0F`.

The runner hash includes the post-invalid initialization normalization that
sets the actual maximum constituent momentum to exactly `1e-6`. The first
attempt exposed no tick history and produced only an initialization-invalid
artifact, which the run of record replaced.
