# FTD-0424 — Auxiliary pole and universal-counterterm gate

**Date:** 2026-07-22  
**Status:** `[INFRASTRUCTURE — scheme-carrying pole contract]` + `[DERIVED — one-calibration threshold arithmetic in FTD-0419 scheme]` + `[SCOPED CLOSED NEGATIVE — fixed-eta threshold invariance in that scheme]` + `[OPEN — gauge-independent on-shell coefficient]`  
**Verdict:** `ONE-CALIBRATION-SURROGATE-FAILS-THRESHOLD; PHYSICAL-ON-SHELL-GATE-OPEN`

## 1. What is implemented

`PoleMatchResult` now requires volume, momentum, gauge parameter, infrared
prescription, masses, species count, fit window, and renormalization condition.
`CountertermTrajectory` accepts exactly one calibration, rejects off-shell or
gauge-dependent references, and predicts every later residual with the same
stored `eta`.

This prevents the FTD-0419 coefficient from silently calibrating the physical
trajectory: FTD-0419 is not an on-shell result and is explicitly rejected by
the interface.

## 2. Computable threshold surrogate

In the frozen one-loop step scheme,

$$
\frac{\delta_{\rm match}(N_f)}{g^2}
=M-\frac12N_f P,
$$

where deterministic extrapolation gives

$$
M=-0.3122568143,\qquad P=+0.0294244847.
$$

Fix the single permitted counterterm at `N_f=1`:

$$
\frac{\eta}{g^2}=-(M-P/2)=+0.3269690566.
$$

Without retuning, the first species threshold predicts

$$
\frac{\delta_{\rm residual}(2)}{g^2}=-\frac P2
=-0.0147122424.
$$

Under the already selected `g^2=alpha_FTD` translation this is
`-1.074e-4`; FTD-0416's optimistic `1/137^3` attraction leaves `4.18e-11`,
more than `4e4` above the declared `1e-15` surrogate tolerance.

Thus a single fixed counterterm does not remain cancelled when the active
fermion multiplicity changes in this scheme. Sector/threshold retuning would
violate the preregistered one-calibration budget.

## 3. Scope ceiling

The calculation above is scheme-specific and not an on-shell result. It does
not satisfy the planned `xi={0.5,1,2}` physical pole comparison. A massless
charged field also requires an explicit infrared/infraparticle prescription
before “the fermion pole” is a well-defined observable. The current Euclidean
code has no real-time analytic-continuation or spectral reconstruction
contract, so quoting gauge-independent pole speeds now would be fabricated.

The physical classification therefore remains open. The narrower automatic
claim—one `eta` cancels every active-species threshold in the already frozen
step scheme—is closed negative.

**Artifacts:** `engine/include/ftd/eft/pole_matching.h`,
`engine/tests/test_pole_matching_contract.cpp`,
`scripts/proofs/proof_lorentz_universal_counterterm.py`, and
`scripts/proofs/_lorentz_universal_counterterm.csv`.
