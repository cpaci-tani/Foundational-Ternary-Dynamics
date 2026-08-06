# FTD-0561 Audit — Periodic-Hop Source Multipole Hierarchy

**Status:** [THEOREM — MULTIPOLE HIERARCHY] + [CLOSED NEGATIVE — RIGID CHARGED EXTENSION CURE]
**Date:** 2026-07-26
**Verdict:** `HOP_SOURCE_MULTIPOLE_HIERARCHY_DERIVED`
**Production changes:** none

## Result

FTD-0560's point-source obstruction survives arbitrary fixed rigid spatial
extension whenever the source has nonzero net polarity.  The leading
slow-hop resonant amplitude is universal:

\[
 A_T=6\pi G_C|Q|/T^2+O(T^{-3}).
\]

If the microscopic source is neutral, the first nonzero axial multipole sets
the next power: dipole `T^-3`, quadrupole `T^-4`, and generally moment order
`m` gives `T^{-(m+2)}`.

## Locked campaign

| Diagnostic | Result |
|---|---:|
| profile/period/axis/polarity arms | 96/96 |
| maximum pole residual | 1.52655665886e-16 |
| maximum form-factor residual | 1.11022302463e-16 |
| minimum positive normalized forcing | 1.52675880724e-6 |
| maximum polarity-mirror residual | 0 |
| maximum cubic-covariance residual | 0 |
| point `T=256` asymptotic error | 0.00696666 |
| same-sign pair error | 0.00718800 |
| dipole error | 0.0136650 |
| quadrupole error | 0.0209003 |
| same-plane dipole axial residual | 0 |
| same-plane dipole oblique amplitude | 0.141480289 |

The independent Python verifier reproduces the hierarchy and registered
limits.

## Ontological consequence

“Matter is extended” is not by itself a solution.  For a microscopically
charged rigid object, the infrared field sees only total polarity at leading
order.  A genuine suppression mechanism begins with microscopic neutrality,
followed by dipole and higher-moment balance.

This makes an emergent-charge route structurally interesting: a carrier could
be microscopically neutral while presenting an effective long-range polarity
through its dressing.  That possibility is not established here; it is now a
precisely stated remaining branch.

**Subsequent closure:** FTD-0562 proves that no fixed nonzero finite rigid form
factor cancels the complete slow-hop surface for all sufficiently large
periods.  Neutrality remains a suppression mechanism, but no longer an exact
finite-profile cure.  Deforming/nonlinear emergent-charge carriers remain
open.

No toggle, scenario, force, damping mechanism, or ontology change is licensed.
