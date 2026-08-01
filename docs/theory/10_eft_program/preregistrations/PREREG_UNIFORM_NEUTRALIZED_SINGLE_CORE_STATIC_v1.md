# FTD-0611 — Uniform-neutralized single-core static state v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** default-off observer-only compact-matter existence discriminator
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=45FC3250CE24A236EBC231DAD9AA171CADFD754FA8289601892B73C107279B69`

## 1. Candidate family

Search for a genuine rest state of one charge-`+1` trimer in the periodic
uniform neutralizer `rho_bg=-1/L^3` at `L=17`. This is the prerequisite exposed
by FTD-0610. Do not reuse the extracted phase-15 field or call a trajectory
from its nonstationary state a boost.

Retain exactly the selected FTD-0600/0606 matter variables and action:

- three constituent records with charges `(+1,+1,-1)`;
- continuous effective positions `anchor+remainder`;
- the default-off multiplicity-two chart fibre;
- the exact quartic distance binding, quadratic polarity coats, matched
  face/edge field normalization, and production dispersion.

Parameterize the nine position degrees of freedom by collective translation
`t in R^3`, global `SO(3)` orientation, and the symmetric three-component
in-plane strain used by FTD-0606. The reference offsets are the exact positive
trimer offsets inherited from the phase-15 family. Require internal distances
in `[0.5,2.0]`, strain components at most `0.20`, and strain minimum eigenvalue
at least `0.70`.

## 2. Energy and registered search

For each candidate, derive its coat density `rho_c`. The uniform background
removes only the zero mode, so evaluate the exact minimum longitudinal energy
as

```text
E(t,R,S) = V_bind(R,S) + beta/2 sum_xy rho_c(x) G_L(x-y) rho_c(y),
```

where `G_L` is independently verified as the zero-mean periodic Green kernel.
This expression must agree with a direct Poisson initialization of
`rho_c-1/L^3` at the final minimum.

Run 16 deterministic nine-dimensional Nelder-Mead starts: all eight
translation shifts in `{0,1/2}^3`, each with the identity and cyclic
`(x,y,z)->(y,z,x)` proper-cubic orientation. Start with zero strain. Permit at
most 2,500 evaluations per start. Lock termination at simplex diameter
`1e-7` and energy spread `1e-14`. Do not add starts, change basins, or select a
different charge assignment after execution.

Require all 16 starts to be admissible, at least 12 to terminate with a valid
minimum, and at least two returned minima within `1e-10` of the best energy.

## 3. Stationarity and stability gates

At the best returned minimum:

- central finite-difference gradient infinity norm at most `1e-8`;
- all nine translation/orientation/strain Hessian eigenvalues greater than
  `1e-6`;
- all 18 signed coordinate perturbations of magnitude `1e-3` increase energy;
- total charge, Poisson, Gauss, curl, and fast/direct energy residuals at most
  `1e-11`;
- integer translations along `x`, `y`, and `z` preserve energy and the direct
  field at `1e-12` after translating back.

These conditions distinguish an interior stable rest solution from a saddle,
flat direction, chart artifact, or merely periodic copy.

## 4. Transactional rest gate

Initialize the direct minimum-energy electric field, zero magnetic half-field,
and zero constituent momenta. Run 16 common-action forward ticks followed by
16 state-only inverse ticks with `allow_shared_anchor_chart=true`.

Require every solver and common-action gate at `1e-12`, energy drift at
`1e-10`, internal distances in `[0.5,2.0]`, maximum anchor multiplicity at most
two, centre displacement and centre-momentum change at most `1e-8`, and
state-only recovery at `1e-9`. Record pseudomomentum but do not gate it because
the uniform neutralizer is external.

## 5. Verdicts

- `UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_CONSTRUCTIVE`: search coverage,
  differential stability, direct-field, covariance, and transactional rest
  gates all pass;
- `UNIFORM_NEUTRALIZED_COMPACT_STATIC_CLOSED_NEGATIVE`: search coverage is
  complete and every returned candidate is fully evaluable, but no candidate
  passes the registered stationarity/stability/rest conjunction;
- `UNIFORM_NEUTRALIZED_SINGLE_CORE_STATIC_NUMERICALLY_UNRESOLVED`: search,
  differential, field, covariance, transaction, or record coverage is
  incomplete.

A constructive verdict licenses only a selected compact rest state under an
external uniform periodic neutralizer. Its slow and fast boosts require a new
locked campaign. It does not derive a production particle, isolated charge,
closed momentum channel, statistics, a pole, Lorentz recovery, or unitarity.
