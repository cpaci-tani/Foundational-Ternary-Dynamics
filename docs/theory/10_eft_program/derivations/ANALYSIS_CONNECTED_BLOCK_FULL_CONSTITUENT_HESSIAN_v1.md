# Connected-block full constituent Hessian and spline regularity

**Campaigns:** FTD-0634 through FTD-0636  
**Status:** `[MEASURED — THREE EXECUTION-INVALID CAMPAIGNS; STRUCTURAL C1 DIAGNOSIS]`  
**Production impact:** none

## Full-space attack

The complete 48-coordinate finite-difference Hessians of the FTD-0633 state
contain no observed negative eigenvalue. Across all three registered scales,
the least eigenvalue is positive:

| campaign | gradient step | Hessian step | least eigenvalue | formal result |
|---|---:|---:|---:|---|
| FTD-0634 | `2e-4` | `2e-4` | `0.0016760` | execution-invalid: gradient |
| FTD-0635 | `2e-5` | `2e-4` | `0.0016760` | execution-invalid: translation identity |
| FTD-0636 | `4e-6` | `4e-5` | `0.0019086` | execution-invalid: gradient |

The maximum eigenvalue is stable near `34.6487`. Independent NumPy
diagonalization reproduces both 48 by 48 spectra.

None of these positive spectra is promoted. Each run missed a locked common
gate.

## Why the coarse Hessian is not a Hessian

The quadratic polarity coat uses the cardinal quadratic B-spline

\[
B_2(r)=
\begin{cases}
3/4-r^2,&r\le 1/2,\\
\tfrac12(3/2-r)^2,&1/2<r<3/2,\\
0,&r\ge3/2.
\end{cases}
\]

It is `C1` but not `C2`: the second derivative jumps at half-integer knot
planes. The refined state lies only `9.465e-5` from its nearest knot. Therefore
the `2e-4` Hessian stencil used by FTD-0634/0635 crosses polynomial sectors.
This explains why the full-Hessian translation Rayleigh quotient did not equal
the FTD-0633 `1e-3` rigid-translation secant. For example, the axial direct
curvature changed from `0.0497788` at the coarse registered displacement to
`0.0543918` at `2e-4`.

FTD-0636 locked `h_H=4e-5`, less than half the knot clearance. At that scale
the required local identity closes:

\[
v_T^T H v_T \simeq K_{\mathrm{direct}}/16.
\]

All 48 knot-local eigenvalues remain positive. The run nevertheless remains
execution-invalid because its maximum 48-coordinate gradient is
`1.116e-8`, narrowly above the locked `1e-8` stationarity gate.

## Consequence for earlier mode language

FTD-0629 and the reduced Hessian portion of FTD-0633 used stencils that cross
the same B-spline knots. Their measured oscillations remain real
finite-amplitude, reversible responses of the selected action, but the phrase
"infinitesimal linear normal modes" is not established. Their frequencies are
amplitude- and sector-qualified secant predictions until a knot-local analytic
linearization is completed.

## Resolution by FTD-0637--0639

Another finite-difference tolerance change was not licensed. FTD-0637 instead
derived the analytic static gradient and Hessian of

\[
U(x)=U_{\mathrm{binding}}(x)+\beta U_{\mathrm{field}}[E_*(\rho(x))]
\]

within that sector. It confirms the small FTD-0636 residual force, while also
confirming that all 48 analytic eigenvalues are positive. FTD-0638 then removes
the residual by one full-space Newton step without leaving the sector, and
FTD-0639 qualifies the resulting state as a reversible common-action fixed
point. See `ANALYSIS_CONNECTED_BLOCK_ANALYTIC_STATIC_AND_DYNAMICAL_REST_v1.md`.

## Evidence boundary

This document's evidence supports only "no negative mode observed at the
tested finite-difference scales." The later analytic campaign establishes a
positive 48-dimensional basin within the occupied polynomial sector. The
selected action remains globally `C1`, not globally `C2`; any physical
small-oscillation statement must still state its knot sector and amplitude
domain.
