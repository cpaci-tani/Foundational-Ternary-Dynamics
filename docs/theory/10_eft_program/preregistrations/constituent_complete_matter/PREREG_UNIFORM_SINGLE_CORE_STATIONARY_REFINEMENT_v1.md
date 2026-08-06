# FTD-0612 — Uniform single-core stationary refinement v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** numerical refinement of the unchanged FTD-0611 selected action
**Production change:** forbidden
**Protocol lock:** `protocol_sha256=B0C93907D5EEB6BE96ED9BA485E2BC452E6180FE619533052A2D870C73B52002`

## 1. Frozen input and allowed repair

FTD-0611 found a fully covered nine-mode positive basin but failed its locked
gradient and 16-tick drift tolerances. Freeze its deterministic best state from
runner SHA-256
`CA6D26735D7984E6A539C1D965439AFE5D30E5C994A5BAAB0A70D09CCE525DC3`
and result-record SHA-256
`4B6079E8D28CCAEF55368DA5A9D8D6C5B8996EBF54AD4587C56601CA916AABCA`.

The only allowed change is numerical refinement of the same nine coordinates
in the same uniform-neutralizer energy. No start, charge, binding, field,
normalization, coat, basin, fibre, action, or state variable may change.

## 2. Registered refinement

Recompute the exact FTD-0611 16-start search and require the same best-energy
fingerprint within `1e-15`. Starting only from that best state, apply at most
eight damped Newton iterations.

- use a fourth-order central gradient with step `2e-4`;
- use the symmetric central Hessian with step `5e-4`;
- solve the nine-dimensional Newton system by pivoted elimination, requiring
  every pivot above `1e-8`;
- try damping factors `1,1/2,...,1/128` in order and accept the first strict
  energy decrease;
- stop when the fourth-order gradient infinity norm is at most `1e-11` or the
  Newton step infinity norm is at most `1e-12`;
- do not switch optimizer, add random perturbations, or loosen a gate.

Record every iteration, gradient, step, damping factor, and energy.

## 3. Refined static gates

At the returned state require:

- fourth-order gradient infinity norm at most `1e-10`;
- nine Hessian eigenvalues greater than `1e-6`;
- all 18 signed `1e-3` perturbations increase energy;
- direct Poisson, charge, Gauss, curl, and fast/direct energy residuals at most
  `1e-11`;
- integer `x/y/z` energy and translated-state covariance at `1e-12`.

Initialize the direct field and run 64 zero-momentum forward ticks followed by
64 state-only inverse ticks. Require every common-action gate at `1e-12`,
energy drift at `1e-10`, pair distances in `[0.5,2.0]`, multiplicity at most
two, centre displacement and centre-momentum change at most `1e-9`, and state
recovery at `1e-9`. Pseudomomentum remains recorded but ungated because the
uniform neutralizer is external.

## 4. Verdicts

- `REFINED_UNIFORM_SINGLE_CORE_STATIC_CONSTRUCTIVE`: fingerprint,
  deterministic refinement, nine-mode stability, direct-field, covariance,
  and 64-tick rest/inverse gates pass;
- `REFINED_UNIFORM_SINGLE_CORE_STATIC_CLOSED_NEGATIVE`: refinement and all
  numerical coverage complete, but a physical static/stability gate fails;
- `REFINED_UNIFORM_SINGLE_CORE_STATIC_NUMERICALLY_UNRESOLVED`: fingerprint,
  Newton solve, line search, derivative, field, covariance, transaction, or
  record coverage is incomplete.

A constructive result repairs only FTD-0611's numerical precision mismatch.
It licenses a later preregistered boost test of this one selected compact rest
state, not a particle, isolated charge, production ontology, pole, Lorentz
recovery, or unitarity.
