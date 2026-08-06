# FTD-0659 — Native excited matter clock v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Production status:** unchanged; observer-only selected-action campaign  
**Parent:** FTD-0640 JSON SHA-256
`AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A`

## 1. Question

Does the first non-rigid analytic matter eigenspace carry a robust,
state-functional action--angle phase under the already selected reversible
common action?

Success establishes a native **excited matter clock**. It does not establish
an intrinsic rest clock, a quantum phase, a rest-mass frequency, natural mode
occupation, a particle pole, or a production primitive.

## 2. Candidate selection

Use the exact FTD-0638/0639 dressed center and the FTD-0640 analytic Hessian at
`L=17`. The parent spectrum fixes six soft lattice-dressed rigid modes followed
by a stiffness gap of `221.67476`. The first group after that gap is the
two-dimensional degenerate internal eigenspace at parent indices `{6,7}`.
No other mode may be substituted after execution.

Because an individual eigenvector inside a degenerate eigenspace is
basis-dependent, the observable must use both coordinates. For mass-normalized
modal position and canonical momentum vectors

\[
q=(q_6,q_7),\qquad p=(p_6,p_7),
\]

and their common analytic frequency `omega`, define

\[
I={|p|^2+\omega^2|q|^2\over2\omega},
\qquad
Z=(\omega^2|q|^2-|p|^2)-2i\omega q\cdot p,
\qquad
\Theta=\arg Z.
\]

For a linearly polarized harmonic orbit, `Z=2 omega I exp(2 i theta)`.
`Theta` is therefore a basis-independent doubled phase. It is invariant under
orthogonal changes of basis in the doublet and undefined when `Z=0`; the test
must not manufacture a phase at zero amplitude.

## 3. Locked initial conditions

For each of the two independently stored cyclic orientations:

- polarizations `u={(1,0),(0,1),(1,1)/sqrt(2)}` in the local doublet basis;
- maximum constituent-displacement amplitudes
  `A_max={2e-6,4e-6,8e-6}`;
- oscillator quadratures `theta_0={0,pi/2,pi,3pi/2}`;
- generalized initial data
  `q=A u cos(theta_0)`, `p=-omega A u sin(theta_0)`, where `A` is fixed by
  `A_max` and the chosen polarization's maximum constituent component;
- `256` forward ticks followed by `256` state-only inverse ticks.

This gives `2*3*3*4=72` nonzero arms. Add one exact dressed-rest
zero-amplitude arm per orientation, for `74` total arms. No field packet,
force, reaction, collision, legacy force branch, parameter fit, or production
change is allowed.

## 4. Locked observers

At every accepted complete state record:

- the doublet projections `q` and `p`;
- action `I`, complex support `|Z|`, doubled phase `Theta`, and unwrapped phase;
- per-step doubled-phase advance and its residual from
  `2 phi`, where `phi=2 atan(omega/2)` is the independently predicted discrete
  phase of the parent common-action linearization;
- leakage into all non-target matter-mode groups;
- complete common-action residual, total-energy drift, centre drift, sector,
  chart multiplicity/separation, hops, and state-only inverse recovery.

The runner must emit versioned JSON plus arm- and tick-level CSV records.

## 5. Locked gates

### 5.1 Provenance and execution

- parent result hash and constructive verdict match;
- the selected eigenspace is exactly parent group `{6,7}`, with relative
  eigenvalue splitting below `1e-9` and the preceding stiffness gap above
  `100`;
- all `74` arms initialize and complete all forward/reverse ticks;
- every step passes the existing common-action gates with maximum residual
  `<=1e-10`;
- total-energy drift `<=1e-12`, inverse recovery `<=1e-10`, centre drift
  `<=1e-4`, no hops, unchanged sector, multiplicity `<=8`, and shared-anchor
  separation `>=0.9` whenever present.

### 5.2 Clock gates

For every nonzero arm:

- maximum relative action drift `<=0.02`;
- minimum linear-polarization support `|Z|/(2 omega I) >=0.90`;
- mean doubled-phase advance differs from `2 phi` by at most `2%`;
- phase-step RMS about the fitted mean is `<=0.05` radians;
- non-target group leakage is `<=0.10`.

Across controls:

- measured phase advance varies by at most `0.5%` across amplitudes;
- initial action scales as amplitude squared within `2%`;
- after subtracting their registered initial doubled phases, quadrature phase
  histories agree within RMS `0.05` radians;
- polarization and cyclic-orientation phase advances agree within `0.5%`;
- orientation spectra agree within `1e-9`.

For both zero-amplitude controls, `I<=1e-20` and `|Z|<=1e-20`; phase must be
reported as undefined and must not enter a fitted slope.

## 6. Outcome map

- **`NATIVE_EXCITED_MATTER_CLOCK_CONSTRUCTIVE`:** every provenance,
  execution, phase, robustness, covariance, and zero-control gate passes.
- **`NATIVE_EXCITED_MATTER_CLOCK_MIXED`:** exact reversible bounded execution
  passes but one or more clock/robustness gates fail.
- **`NATIVE_EXCITED_MATTER_CLOCK_CLOSED_NEGATIVE`:** the selected excitation
  cannot execute as an exact reversible bounded common-action history.
- **`NATIVE_EXCITED_MATTER_CLOCK_EXECUTION_INVALID`:** provenance, coverage,
  eigenspace identification, or record completeness fails.

No failed threshold may be changed in v1. Any repair is a new versioned and
pre-registered candidate.
