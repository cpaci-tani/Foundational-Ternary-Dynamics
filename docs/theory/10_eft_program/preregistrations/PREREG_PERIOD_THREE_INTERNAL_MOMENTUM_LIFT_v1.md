# FTD-0715 — Period-three internal-momentum lift v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Parents:** FTD-0713 and FTD-0714

## Question

FTD-0713 found a causal, zero-center internal deformation that makes the
prescribed moving source compatible with the locked two-tick field operator,
while FTD-0714 proved that the same deformation cannot be a two-tick
momentum-return orbit of the selected discrete-gradient matter kinematics.
Does the first unexcluded temporal family, a three-tick translated relative
orbit, admit an exact momentum lift without adding a primitive variable?

## Frozen construction

Load the 16 displacement vectors `delta_a` from the FTD-0713 state artifact
(SHA-256
`6C9B7684DBEB2976823B2A0B908407ED201253E4A6ED22D1F83B053712C4ACDF`).
For each labeled constituent prescribe the three consecutive displacements

\[
 u_{a0}=\tfrac13\hat x+\delta_a,\qquad
 u_{a1}=\tfrac13\hat x-2\delta_a,\qquad
 u_{a2}=\tfrac13\hat x+\delta_a.
\]

They sum to one site, while the relative shape follows the exact cycle
`0 -> delta -> -delta -> 0`. Because `sum_a delta_a=0`, the center advances
by exactly `1/3` site on every tick.

For each constituent solve for three periodic endpoint momenta
`(p_a0,p_a1,p_a2)`, with `p_a3=p_a0`, using only the frozen production
dispersion and discrete-gradient velocity

\[
 H(p)=\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p|^2},\qquad
 V(p,q)=\frac{C_{\rm SPEED}^2(p+q)}{H(p)+H(q)},
\]

so that

\[
 V(p_{a0},p_{a1})=u_{a0},\quad
 V(p_{a1},p_{a2})=u_{a1},\quad
 V(p_{a2},p_{a0})=u_{a2}.
\]

Initialize all three momenta at the exact uniform momentum corresponding to
speed `1/3`. Use a central finite-difference `9x9` Newton Jacobian with
`h=1e-6`, pivoted Gaussian elimination with pivot floor `1e-14`, at most 30
iterations, and strict-residual-decrease backtracking through `2^-12`. No
trajectory coordinate, momentum component, force, or residual may be fitted
outside this solve.

## Locked checks

The observer records and independently checks:

1. parent hashes and the exact 16-row displacement reconstruction;
2. maximum velocity residual `<=1e-12` for all 48 segments;
3. exact translated shape return and per-tick center residual `<=1e-13`;
4. every segment speed `<=C_SPEED` and every phase-shape edge deformation
   `<=0.10`;
5. nontrivial internal motion, defined by maximum segment difference `>=1e-3`;
6. exact three-tick labeled momentum return;
7. the discrete-gradient work identity
   `H(q)-H(p)=V(p,q) dot (q-p)` to `1e-12` on every segment;
8. per-constituent and total three-tick energy/impulse telescoping to `1e-12`;
9. momentum-solution covariance under all 24 proper cubic rotations and the
   direction mirror to `1e-10`.

The matter momentum may exchange a finite impulse with a future field solve on
individual ticks. Only the complete-cycle impulse is required to telescope in
this kinematic campaign.

## Verdicts

- `PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE` if every locked check passes;
- `PERIOD_THREE_KINEMATIC_LIFT_CLOSED_NEGATIVE` if the execution is valid but
  no locked root satisfies the algebraic, causal, or graph gates;
- `PERIOD_THREE_MOMENTUM_LIFT_EXECUTION_INVALID` for parent, reconstruction,
  solver, covariance, or provenance failure.

A constructive result proves only that period three removes the two-tick
kinematic obstruction using existing momentum variables. It does not prove
that the common action generates the orbit, that a compatible co-moving field
exists, or that the configuration is stable. The next required test is the
three-tick Fourier field/source solvability condition, followed by an atomic
matter-field action replay.
