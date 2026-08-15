# SCOPE — The two-body Lorentz campaign v1

**Status:** `[DRAFT — AWAITING OWNER RATIFICATION]`; no LEDGER row is minted
by this document (row on ratification).
**Date:** 2026-08-15
**Position:** the queue item the FTD-1000 fold conditionally unblocked
(agreed queue of 2026-08-13: four-walls ✓ FTD-1002 → **two-body Lorentz** →
C3 retirement ✓ FTD-1004–1008). The carrier question is parked as a mapped
low-intensity no-go arc (INDEX status note, 2026-08-15).

## 1. The obligation

One-body Lorentz phenomenology is CLOSED (the 2026-08-08 reductions: free
sector exact with `|Δv/v| = (ka)⁴/3240`, k⁴-isotropic; radiative dim-4
closed three ways). The **two-body item is OPEN**: do two bound bodies in
relative motion behave Lorentz-consistently on the substrate — does the
composite's internal rate dilate as the adopted clock law demands, and do
its kinetic, binding, and boost responses cohere? The gate analysis
(`ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md`) leaves the standing
criterion: *kinetic, binding, stress-energy and boost generators should
arise from one interacting dynamics before covariance is claimed.*

## 2. What the record already settles

- **The two failure modes must not be conflated** (gate analysis §5):
  (i) *Newtonian nodes ⇒ no dilation* — a modelling error, **fixable by
  substitution** (constituents must carry the lattice dispersion, not a
  distance-potential kinematics; FTD-0812's Galilean result is the record
  of the error, not of the substrate); (ii) *ω inside the band ⇒ radiates* —
  genuine (C2), **harsher post-FTD-1003** (true edge π, not 1.2310).
- **Cone inheritance is established** (FTD-0813): the composite's limiting
  speed averages the constituents' cones. The open half is the **internal
  clock rate under boost** — precisely what the superseded clock-rate
  campaign (`ANALYSIS_COMPOSITE_CLOCK_DILATION_v1.md`) left "exploratory
  and inconclusive pending finite-volume, discretization and held-out-
  momentum controls."
- **The conditional footing**: CLK-1 lives inside FC-2 (FTD-1000). The
  campaign therefore does not require a native carrier and does not derive
  the γ-law; it **tests the adopted law's consistency** on composites whose
  parts obey substrate dynamics. A clean deviation is a *falsifier run*
  against FC-2's clock clause — Front A work, the sharpest kind.

## 3. Design skeleton (to be preregistered per stage)

- **S1 — instrument + controls.** Synthetic known-answer calibration:
  composites of imposed-γ constituents must reproduce the adopted law
  exactly; the FTD-0812 Galilean toy must reproduce T(v)=T(0). The three
  missing controls named by the superseded campaign (finite-volume,
  discretization, held-out momentum) are designed in from the start.
- **S2 — the measurement.** A bound two-constituent composite with
  lattice-dispersive parts, boosted across a preregistered momentum grid;
  measurand: internal beat rate T(v)/T(0) against the adopted quadratic
  budget, with the finite-horizon caveat declared (a band-embedded internal
  mode decays — Γ_E-fold ≈ 26.6 cycles for the recorded doublet — so the
  measurand is rate-while-coherent, windows preregistered). Platform: the
  engine via WSL2/GPU per house rules; Python only for quick checks.
- **S3 — booking.** Outcomes declared before S2 runs: consistency
  (conditional pass; the fold's clause survives its first substrate test),
  deviation (falsifier candidate against FC-2's clock clause — escalates to
  owner), or invalid (controls fail; nothing booked).

## 4. Boundaries

C2 is not voided by this campaign: a radiating internal mode is acceptable
for a finite-horizon *measurement* while remaining fatal for a *persistent
clock* — the distinction the gate analysis already draws. Nothing here
touches C3, the gearbox, G*, or any α-sector claim; no target constant
enters the design. Every stage carries its own fresh lock.
