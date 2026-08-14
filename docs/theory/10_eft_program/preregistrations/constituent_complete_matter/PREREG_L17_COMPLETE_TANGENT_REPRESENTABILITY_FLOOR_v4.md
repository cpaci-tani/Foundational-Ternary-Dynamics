# FTD-0831 — L=17 complete tangent representability-floor repair v4

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE SUCCESSOR EXECUTION]`  
**Scope:** machine-derived backward-error completion of the face-harmonic codec gate  
**Physical question and gates:** inherited unchanged from FTD-0774  
**Production impact:** none  
**Date:** 2026-08-10

## 1. Why the pure-relative gate is not numerically well posed

FTD-0830 applied four target-blind post-reinsertion mean corrections. It again
completed all `64/64` signed probe endpoints, and every non-harmonic codec gate
passed. Forty-nine primary probe rows still failed only the face-harmonic
relative reconstruction threshold. The remaining residuals are not reduced by
uniform correction because the required change is below one binary64 ULP of
the stored face entries.

This is an a priori representability problem. A completed face field can have
entries of order `s`, while its retained uniform coefficient `a` is near zero.
The mean is computed from `N=L^3=4913` binary64 entries. A demand

\[
 |\operatorname{mean}(e)-a|/|a|\le10^{-12}
\]

therefore asks for an absolute accuracy arbitrarily below the rounding floor
as `a -> 0`. No fixed-precision implementation can satisfy that demand for all
valid zero-harmonic tangents. This is a defective numerical interpretation of
the intended statement that the separately retained harmonic coordinate is
not lost.

FTD-0830 is preserved as `[EXECUTION INVALID — NO PHYSICS VERDICT]`. This
successor registers a standard mixed relative/backward-error gate before a new
execution. It does not inspect or change a phase, candidate, spectrum, or
physical threshold; no prior run reached Krylov.

## 2. Frozen inheritance

Inherit unchanged:

- all FTD-0774 physical inputs, endpoints, probes, scales, energy form,
  Krylov construction, qualification thresholds, verdicts, and stop conditions;
- FTD-0829's range-aware periodic Hodge compatibility and exact ledger order;
- FTD-0830's four post-reinsertion corrections and fail-closed verifier labels;
- the `1e-12` relative face-harmonic target whenever it lies above the
  arithmetic representability floor;
- the edge-harmonic gate without modification.

## 3. Repair R5 — declared binary64 backward-error floor

The locked platform is MSVC 14.44 binary64; on this platform C++ `long double`
has the same precision as `double`. Let

\[
 u=2^{-53}=\tfrac12\,\texttt{numeric_limits<double>::epsilon()},
 \qquad k=N+32,
 \qquad \gamma_k=\frac{ku}{1-ku}.                            \tag{R5.1}
\]

The `32` covers the `N-1` sequential mean additions together with division,
reinsertion, four correction passes, and the final reconstruction arithmetic.
It is fixed before execution and is not inferred from observed residuals.

For the three retained raw means `a=(a_x,a_y,a_z)`, the completed face field
`e`,

\[
 A=\max_k |a_k|,
 \qquad S=\max_{i,k}|e_{i,k}|,
 \qquad E=\max_k|\operatorname{mean}(e_k)-a_k|,              \tag{R5.2}
\]

define the recorded face-harmonic residual by

\[
 r_{\rm face}^{\rm v4}
 =\frac{E}
 {\max(A,10^{-30})+\gamma_k S/10^{-12}}.                    \tag{R5.3}
\]

Retain the threshold

\[
 r_{\rm face}^{\rm v4}\le10^{-12}.                          \tag{R5.4}
\]

Equivalently,

\[
 E\le10^{-12}\max(A,10^{-30})+\gamma_k S.                  \tag{R5.5}
\]

Thus the old relative gate remains exactly as the first term, while the second
term admits only the declared forward-error floor of the arithmetic used to
represent and sum the completed field. `S` must be serialized in the hexfloat
codec detail as `face_completed_max`; the independent certificate must
reconstruct (R5.1)--(R5.4) from `face_raw`, `face_rebuilt`, and this primitive.

The producer may not use the observed failure maximum, a fitted multiplier, a
candidate outcome, or any later-stage quantity. If (R5.4) fails, preserve the
failure and stop.

## 4. Source closure and corpus

The v4 test-only closure is:

| role | file |
|---|---|
| shared complete runner | `engine/tests/test_l17_complete_tangent_candidate.cpp` |
| v4 compile wrapper | `engine/tests/test_l17_complete_tangent_representability_floor_v4.cpp` |
| shared test-only codec | `engine/tests/support/connected_moore_tangent_codec.h` |
| independent replay implementation | `scripts/proofs/proof_l17_complete_tangent_candidate.py` |
| v4 replay wrapper | `scripts/proofs/proof_l17_complete_tangent_representability_floor_v4.py` |

The final protocol SHA-256 must be embedded in producer and verifier before
execution. The corpus manifest must hash the complete source closure against
the executing checkout. The result root is `engine/results/ftd_0831/`, stem
`ftd_0831_l17_complete_tangent_representability_floor_v4`.

## 5. Verdict and licensing boundary

Use the unchanged four FTD-0774 physical verdict strings under JSON identity
`FTD-0831`. Producer and independent replay must agree.

Only a constructive positive-tangent verdict licenses a separately
preregistered localization/volume successor. It is not yet a bounded
autonomous clock; finite-amplitude recurrence, repeated gates, energy/work
closure, and held-out orientation robustness remain open under FTD-0828.

## 6. Stop conditions

- Do not execute before the final protocol hash is embedded in producer and
  verifier.
- Do not change `k=N+32`, the arithmetic model, or any inherited threshold
  after execution.
- A source/hash/schema/replay disagreement is execution-invalid.
- No near-miss search, phase fit, threshold fit, dimension fit, or
  outcome-conditioned repair is permitted.
