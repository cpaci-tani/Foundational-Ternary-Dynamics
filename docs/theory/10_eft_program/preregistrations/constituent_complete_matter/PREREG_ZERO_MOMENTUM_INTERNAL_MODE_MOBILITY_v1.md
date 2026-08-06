# FTD-0615 — Zero-momentum internal-mode mobility v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only active-internal-mode discriminator
**Production change:** forbidden

## 1. Frozen parent state and energy scale

Reconstruct only the exact FTD-0612 uniformly neutralized compact fixed point.
Require the rest-energy fingerprint `0.0015517955076684577`, its nine positive
modes, and its 64-tick rest/inverse gate.  Require the FTD-0614 run-of-record
SHA-256
`8A2866361FAECED8358DD8BB59A62F01CA583273D62235436A0600796520BA45`
and use its smallest registered locally relaxed selected-path barrier

```text
Delta_ref = 0.00011302707069732617.
```

Keep the constituents, charges, uniform compensator, binding, field,
dispersion, shared-anchor fibre, action, solver, and tolerances unchanged.

## 2. Complete zero-centre-momentum internal tangent basis

At the refined rest state, construct six nine-component constituent-momentum
patterns from the already registered orientation/strain coordinates:

1. infinitesimal rigid rotations about laboratory `x`, `y`, and `z`;
2. the three symmetric in-plane strain tangents `e0e0`, `e0e1+e1e0`, and
   `e1e1` of the body frame used by FTD-0606/0612.

Subtract each pattern's constituent mean and normalize it so

```text
sum_a |u_a|^2 = 1,       sum_a u_a = 0.
```

Require zero-sum and normalization residuals at most `1e-12`.  Require the
six-pattern Gram matrix to have six eigenvalues greater than `1e-8`.  No mode
may be selected or omitted after seeing its trajectory.

## 3. Locked excitation arms

For every mode, both momentum signs, and excitation ratios `1` and `4`, solve
the unique nonnegative amplitude `A` satisfying

```text
sum_a [H(A u_a)-E_REST] = ratio * Delta_ref,
H(p)=sqrt(E_REST^2+C_SPEED^2 |p|^2).
```

The energy-normalization residual and initial total matter momentum must each
be at most `1e-12`.  This gives 24 arms.  Evolve each for 128 forward ticks and
then 128 state-only inverse ticks.  Supply no centre boost, external packet,
force, damping, trajectory record, or forward branch data to the inverse.

For every tick record common-action residual, total-energy drift, centre,
centre momentum, constituent anchor changes, pair distances, and field-plus-
matter pseudomomentum defect.

## 4. Classification gates

An arm has **base coverage** only if:

- all 256 transactions complete;
- every common-action gate is at most `1e-12`;
- total-energy drift is at most `1e-10`;
- state-only recovery is at most `1e-9`;
- maximum anchor multiplicity is at most two.

An arm is **intact** if all pair distances stay in `[0.5,2.0]`.

An arm is an **active internal walker** only if it has base coverage, remains
intact, has net centre displacement at least `0.75` cell, maximum centre
excursion at least `1.0` cell, and at least three constituent anchor changes.
This is a selected lattice-walker criterion, not a particle or inertia claim.

An arm is **bounded** if it has base coverage, remains intact, maximum centre
excursion is below `0.5`, and net centre displacement is below `0.25`.
Complete arms outside both cells are reported without forced classification.

## 5. Verdicts

- `ZERO_MOMENTUM_INTERNAL_WALKER_CONSTRUCTIVE`: all arms have base coverage
  and at least one arm is an active internal walker;
- `REGISTERED_INTERNAL_MODES_NO_STABLE_WALKER`: all arms have base coverage
  and no arm is an active internal walker; bounded, intermediate, and broken-
  geometry counts are reported separately;
- `ZERO_MOMENTUM_INTERNAL_MODE_MOBILITY_NUMERICALLY_UNRESOLVED`: any mode-basis,
  excitation, solver, action, energy, inverse, or record coverage is missing.

A constructive arm would show that the existing constituent phase space can
convert an internal phase into translation without a new primitive.  A
negative verdict closes only these six linear tangent families at the two
locked energies; it does not exclude nonlinear breathers, mixed modes, or an
extended carrier.  No physical particle, production rule, scenario, force,
mass, pole, Lorentz, or unitarity claim is licensed.

**Protocol lock:** `protocol_sha256=1F8B86C20FFAC79381F2DA4B69085E5DC4B360BFAC379281D3F272C87387104B`
