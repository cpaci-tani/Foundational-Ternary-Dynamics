# FTD-0607 — Site-admissible compact matter motion v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only constrained-core and autonomous-motion discriminator
using the unchanged FTD-0601 common-action transaction, FTD-0602 minimum-energy
Gauss field, and FTD-0606 `SO(3) × strain` coordinates.
**Production change:** forbidden.
**Protocol lock:** `protocol_sha256=CA37FB9700A2416FE293B26A903A9DCA5233091C215E0AEB83D92BA802D871E9`

## 1. Ontological question

Does the one-label-per-site ternary state space contain a stable compact
constituent/field pattern that can autonomously translate through at least one
lattice cell under the same local transaction that transports its current and
balances its energy?

This protocol adds no constituent, occupancy channel, force, field component,
history variable, connection, phase, or production state. It tests whether
the current selected constituent ontology already has enough capacity.

## 2. Frozen configuration and hard site constraint

Use the unchanged FTD-0606 neutral pair at `L=17`, including its charges,
centres, body frame, `SO(3) × Sym(2)` coordinates, quadratic polarity coat,
quartic intratrimer binding, face-flux normalization, periodic Green kernel,
minimum-energy direct field, production dispersion, and common-action solver.

A trial configuration is admissible only when:

- all six constituent anchors are pairwise distinct;
- all remainders lie in the canonical half-open site chart;
- the minimum distance from any remainder component to a chart face is at
  least `5e-3` at a reported static minimum;
- `|h_i| <= 0.20`, the minimum eigenvalue of `I+H` is at least `0.70`, and all
  internal pair distances lie in `[0.5,2.0]`.

Duplicate anchors are rejected by the objective; they are not penalized with
a fitted energy. The physical static functional remains exactly

\[
 U_f(R,h)=V_{\rm bind}(h)
 +\beta\,\frac12\langle\rho(R,h),G_L\rho(R,h)\rangle.
\]

## 3. Constrained static campaign

For `f=j/32`, `j=0,...,31`, launch the same 24 proper-cubic orientation starts
at zero strain. A start that is not site-admissible is recorded and skipped.
Every admissible start uses deterministic six-dimensional Nelder-Mead with
coefficients `(1,2,1/2,1/2)`, orientation step `0.03`, strain step `0.01`,
1,500 objective evaluations, simplex diameter `1e-7`, and energy spread
`1e-14`. Every objective evaluation enforces the hard site constraint.

For each phase require:

- at least 12 admissible starts;
- at least 75% of admissible starts terminate;
- at least two terminated starts lie within `1e-10` energy of the best;
- the best candidate satisfies the chart-margin, strain, distance, and energy
  gates;
- all central tangent samples used by the `h=1e-4` gradient and `h=2e-3`
  Hessian remain site-admissible;
- gradient infinity norm is at most `5e-7`;
- the `6x6` tangent Hessian has no eigenvalue below `-5e-6` and all six
  eigenvalues exceed `1e-6`;
- the direct Gauss/curl/Green-energy gate is at most `1e-11`.

The phase-zero lowest-energy qualified candidate is selected before any
dynamical arm. No alternate candidate may be substituted after observing
motion.

## 4. Autonomous motion arms

Create two independent copies of the selected phase-zero state. Set every
constituent's initial momentum using the unchanged production map

\[
 p(v)=\gamma(v)M_{\rm INERTIAL}v
\]

for collective velocities `v=(1/64,0,0)` and `v=(1/32,0,0)`. The initial
electric field is the selected core's independently rebuilt minimum-energy
Gauss field and the initial magnetic half-field is zero. This initial-field
choice is reported as selected, not derived.

Run the unchanged FTD-0601 forward transaction for 128 and 64 ticks,
respectively. Then apply the unchanged state-only inverse transaction for the
same number of ticks, starting from the final state only.

For each arm require:

- every forward and reverse solve is valid and common-action qualified;
- continuity, Gauss, work, energy, and causal residuals remain at most
  `1e-12` per tick;
- all six anchors remain pairwise distinct at every stored state;
- each trimer's three internal distances remain in `[0.5,2.0]`;
- the two trimer centres remain separated within `0.25` of their initial
  separation;
- mean centre displacement along `x` is at least `75%` of the nominal
  ballistic displacement and transverse drift is at most `0.25`;
- at least six legitimate constituent anchor changes occur;
- cumulative total-energy drift is at most `1e-10`;
- full state recovery after reverse evolution is at most `1e-9`.

Repeat the first forward tick from the exact integer-translated phase-zero
state and require translated-state covariance at `1e-12`.

## 5. Verdicts

- `SITE_ADMISSIBLE_COMPACT_MATTER_MOBILE_CONSTRUCTIVE`: every static and
  dynamical gate passes in both velocity arms;
- `SITE_ADMISSIBLE_STATIC_CORE_DYNAMICS_CLOSED_NEGATIVE`: the constrained
  static campaign qualifies at all phases, but at least one autonomous-motion
  arm fails a registered dynamics gate;
- `SITE_ADMISSIBLE_STATIC_BRANCH_NOT_FOUND_IN_REGISTERED_SEARCH`: static
  search coverage qualifies, but at least one phase has no qualified stable
  site-admissible candidate;
- `SITE_ADMISSIBLE_COMPACT_MATTER_NUMERICALLY_UNRESOLVED`: start/optimizer,
  field, or record coverage is insufficient to apply an earlier verdict.

A negative dynamics verdict closes only the two registered collective-momentum
arms, selected zero-magnetic initial dressing, and compact trimer family. A
missing static branch is a numerical fact about the registered starts, not a
global no-go on `SO(3)`. No verdict licenses a physical particle, electron,
production toggle, scenario, electromagnetic ontology, pole, Lorentz
recovery, or unitarity claim.
