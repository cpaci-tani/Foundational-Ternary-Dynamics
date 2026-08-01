# Audit — connected-block fixed-mass refinement obstruction v1

**Ledger ID:** FTD-0647  
**Verdict:** `FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED`  
**Production changed:** no

## Audited claim

The claim is deliberately narrow: the current connected-block action, with
unchanged per-constituent production dispersion and unchanged nonnegative
binding/field sectors, has no fixed-mass width limit.

## Independent checks

1. The preregistration hash is
   `5D3A8E64750936A1A437C4F743777297977AA0E6BEBAC241F8FF46BD647706D9`.
2. The block constructor supplies exactly `N=2w^3` signed constituents.
3. `h(p)>=E_REST` follows directly from the production dispersion.
4. The Moore binding functional is a positive coefficient times a sum of
   squares.
5. The matched curl Fourier norm is bounded by `2*sqrt(3)`. At
   `lambda=1/sqrt(3)` the staggered modified field energy is positive
   semidefinite.
6. The mapped field-work coefficient is positive.
7. All 12 locked `w={1,2,3,4}` by orientation arms pass. Worst cubic
   scalar-energy residual is `4.5608e-15`.
8. The independent proof validates both run-of-record hashes and every arm.

## Scope control

The result does not say extended matter is impossible, and it does not prove
that a physical particle must have infinitely many constituents. It closes
only the claim that increasing `w` under the frozen action gives fixed-mass
copies of one object.

The result also distinguishes an energy-zero convention from inertia. An
`N*E_REST` subtraction alone cannot remove the `N*M_INERTIAL` collective
kinetic curvature.

## Correct program consequence

The next fixed-mass family must preregister a scale-dependent cell/action
measure, a collective graph mass, a dynamical background contribution, or a
finite-carrier depinning mechanism. Such a repair is permitted as a new
selected candidate. It is not a derivation from the five postulates and must
not modify the locked FTD-0646/0647 records.
