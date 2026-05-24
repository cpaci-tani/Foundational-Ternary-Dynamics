# Plan 05 — Optional Dashboard and Scenario Integration

## Objective

After the C++ modules and docs pass, add optional dashboard views/scenarios for demonstration.

This is intentionally second wave. Do not begin before Plans 01–04 are green.

## Scenario A — Branch Holonomy Torus

Visual goal:

Show how an anti-periodic branch twist removes the zero mode and opens the finite branch gap.

### Proposed UI elements

- Toggle: `branch_holonomy_x`
- Display:
  - \(H_x,H_y,H_z\)
  - exact gap \(\lambda_{\min}\)
  - \(4\sin^2(\pi/2N)\)
  - comparison to \(\pi^2/N^2\)
- Visual:
  - torus/wrap sheet indicator
  - spectrum bars for untwisted vs twisted

### Implementation locations

Likely locations based on project atlas:

- JS scenario: `engine/web/js/bridge/scenarios/`
- Toggle defaults: `engine/web/js/config/toggles.js`
- C++ scenario if needed: `engine/src/scenarios/`
- WASM binding only if exposing new C++ API to dashboard.

### Rule

Prefer JS-only demonstration first. Do not expose C++ API unless the visualization needs live engine data.

## Scenario B — Z3 Color Center Closure

Visual goal:

Show \(q\bar q\), \(qqq\), and non-neutral open flux cases.

### UI elements

- Charge set selector:
  - single \(q\)
  - \(q\bar q\)
  - \(qqq\)
  - \(qq\)
- Display:
  - total center charge mod 3
  - neutral/non-neutral badge
  - toy flux energy \(E=\sigma_c L\)
  - open penalty if non-neutral

### Rule

Label the panel explicitly:

"Center-closure scaffold, not full QCD confinement."

## Scenario C — Generation Graph

Visual goal:

Show the candidate \(\Gamma_F(d)\) family and \(d_U=3,d_D=2\) overlap.

### UI elements

- \(d\) selector
- Edge weights:
  - \(q^{d+1}\)
  - \(1\)
  - \(q^d\)
- Loop phase:
  - \(\phi=\pi+\pi/d\)
- Matrix display:
  - \(|U_U^\dagger U_D|\)

### Rule

Label explicitly:

"Candidate reconstruction. Not theorem-forced CKM."

## Playwright tests

Add smoke tests only:

- scenario loads
- values render
- toggles do not crash
- no console errors

Do not add numeric physics claims in Playwright unless values come from exact formulas.

## Acceptance criteria

- No production tick change unless intentional.
- Existing dashboard tests pass.
- New scenario labels are honest.
- No hidden claims in user-facing copy.
