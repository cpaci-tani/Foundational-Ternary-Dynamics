# FTD-0433 — Native Dressed-Hazard Infrared-Scaling Audit

**Date:** 2026-07-23  
**Status:** `[MEASURED — OUTCOME C: UNRESOLVED SCALING]`  
**Scope:** finite volumes `L=12...48`, one axial fundamental source, first
native field antinode; no asymptotic or conservation claim.

## 1. Locked result

FTD-0433 holds the source family fixed at `<100>,n=1`, varies only
`k=2 pi/L`, and samples the exact FTD-0432 production hazard at a tick chosen
from the native 18-point pole before any result is observed. All execution,
observer, conditional-expectation, and backend gates pass.

The preregistered verdict is **C — UNRESOLVED SCALING**. The largest-volume
tail decreases with positive local effective exponents, but the complete
seven-volume sequence is nonmonotonic and does not show the locked fourfold
suppression. The data therefore establish neither a vanishing infrared hazard
nor a finite nonzero asymptote.

## 2. Pole-defined sampling

For each volume,

\[
 k_L=2\pi/L,\qquad
 \omega_L=\arccos\left(1-\frac12 C_{\rm WAVE}^2M_{18}(k_L,0,0)\right),
\]

and the observer samples transition

\[
 t_L^*=\operatorname{round}(\pi/\omega_L)-1.
\]

The registered target transitions are `9,13,16,20,27,34,41` for
`L=12,16,20,24,32,40,48`. Every phase error is below the locked half-step
bound. No measured source, hazard minimum, or fitted lifetime selects a tick.

## 3. Validity gates

The primary WSL2 CUDA campaign contains all seven volumes and eight seeds.
The Windows CPU reproduction contains the complete `L=32` matrix and journals
every accepted removal as evaporation.

| Diagnostic | Locked maximum | Observed |
|---|---:|---:|
| source standardized residual, max | 6 | 3.4592 |
| source standardized residual, RMS | 2.5 | 1.0313 |
| occupancy standardized residual, max | 6 | 3.6675 |
| occupancy standardized residual, RMS | 2.5 | 1.0302 |

CPU/CUDA registered complex fields agree to `5.56e-17`; the largest scalar
difference is `4.55e-13`, below the locked `1e-10`. Full initial occupancy,
zero initial signed state, positive source projection, monotone occupancy, and
site-probability bounds all pass.

## 4. First-antinode measurements

| `L` | `t_L^*` | `h_L^*` | jackknife SE | `A_L^*` |
|---:|---:|---:|---:|---:|
| 12 | 9 | 0.00430045 | 0.00010626 | 0.744021 |
| 16 | 13 | 0.00531305 | 0.00010686 | 0.660553 |
| 20 | 16 | 0.00436908 | 0.00005228 | 0.601786 |
| 24 | 20 | 0.00460117 | 0.00003182 | 0.545446 |
| 32 | 27 | 0.00438695 | 0.00003991 | 0.455450 |
| 40 | 34 | 0.00386511 | 0.00001641 | 0.385322 |
| 48 | 41 | 0.00351325 | 0.00001889 | 0.329902 |

Adjacent-volume effective exponents are

\[
(-0.7350,\ 0.8766,\ -0.2839,\ 0.1657,\ 0.5675,\ 0.5235).
\]

The last two are positive and exceed the locked `0.25` threshold. The full
sequence nevertheless rises at `12 -> 16` and `20 -> 24`. Its endpoint ratio
is

\[
 h_{48}^*/h_{12}^*=0.81695,
\]

not the outcome-A requirement `<=0.25`. The `L=48` 95% interval is
`[0.0034762,0.0035503]`, and survival remains `0.3299`.

## 5. Locked outcome accounting

Outcome A fails because strict monotonicity and the endpoint-ratio gate fail.
Passing the upper-bound, tail-exponent, and survival clauses cannot substitute
for those failures.

Outcome B fails because the `L=48` lower bound is below `0.01`, while the last
two effective exponents are not flat (`|p|<0.25`). The large endpoint ratio
alone cannot substitute for the flat-tail clause.

The only admissible result is **Outcome C**. The late-volume tail is evidence
of finite-volume suppression over `L=32...48`, but the preregistration assigns
no infrared inference to a nonmonotonic sequence. No polynomial intercept is
fit after the fact.

## 6. Correct scope and reproducibility

FTD-0433 does not establish exact polarity conservation, a zero-momentum
decay rate, an asymptotic plateau, `U(1)`, a hydrodynamic pole, or a common
matter/light cone. It also does not establish a finite hazard floor. Genesis,
annihilation, pair production, and weak transmutation are outside this run.

- preregistration:
  `PREREG_NATIVE_DRESSED_HAZARD_IR_SCALING_v1.md`
- source lock: `native_dressed_hazard_ir_scaling_lock.json`
- lock verifier: `proof_native_dressed_hazard_ir_scaling_lock.py` — 31/31
- result verifier: `proof_native_dressed_hazard_ir_scaling_results.py` —
  74/74
- records: `engine/results/ftd_0433/manifest.json` plus eight hash-locked CSVs

No production source, observer formula, constant, toggle default, event order,
or RNG stream was changed.
