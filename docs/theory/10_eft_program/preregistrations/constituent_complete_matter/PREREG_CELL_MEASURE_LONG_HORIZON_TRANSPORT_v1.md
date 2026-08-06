# FTD-0650 — Cell-measure long-horizon transport v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Parent:** FTD-0649  
**Scope:** default-off CPU observer dynamics in the selected connected-block action

## Question

Does the FTD-0649 fixed-mass reciprocal action remain coherent, reversible,
and secularly mobile over one common physical horizon as resolution increases?
Do directional splitting and the declared spline-Poynting translation defect
decrease rather than merely move to a different finite scale?

This is a long-horizon transport discriminator. It is not a pole, Lorentz, or
production campaign.

## Frozen cross-resolution clock and ruler

For width `w`, retain the complete FTD-0649 action and define

\[
a_w=2/w,\qquad \ell_w=a_w,\qquad \tau_w=a_w.
\]

Thus the dimensionless lattice cone and a physical limiting speed are both
resolution-independent: one lattice cell and one engine tick acquire the same
relative physical scale. Engine `dt` remains exactly one; `tau_w` is the
cross-resolution interpretation, not a changed update equation.

The common physical horizon is `T_phys=32`. Therefore the forward tick counts
are exactly

\[
n_w=T_{\rm phys}/\tau_w=\{32,48,64\}
\]

for `w={2,3,4}`. Physical displacement is `a_w` times lattice displacement.
The full history is then inverted for the same number of ticks from state
alone.

## Locked action and initialization

- `L=8w+1`, periodic boundary;
- `r_m=r_q=r_kappa=a_w^3`, `r_beta=a_w^-1` everywhere qualified by
  FTD-0649;
- zero fractional phase and minimum-energy longitudinal dressing multiplied
  by `r_q`;
- finite chart fibre enabled;
- Jacobian-free Newton--Krylov solver, with the exact FTD-0649 residual and
  tolerances unchanged;
- no transverse seed, reaction, collision, legacy force, external drive,
  post-step correction, or redressing between ticks.

## Locked arm matrix

At each width:

1. six primary arms: speeds `v={0.01,0.04}` in each of
   `<100>`, `<110>`, and `<111>`, with unit direction vectors and object
   orientation axis zero;
2. one zero-launch control at orientation zero;
3. one sign-mirror control: `v=-0.04<100>` at orientation zero;
4. two whole-state cubic controls for the `v=0.04` parallel family:
   orientation/velocity `(1,+y)` and `(2,+z)`.

Total: 30 histories, 1,440 forward ticks and 1,440 reverse ticks. Runner
parallelism may change wall time but not arm contents, update ordering, or
results.

Launch momentum is computed from the scaled dispersion. No momentum is copied
between widths.

## Per-tick records

Record complete state hashes, center, matter momentum, local and spline field
momenta, field/kinetic/binding energies, maximum edge strain, site hops,
minimum chart margin or occupancy validity, nonlinear iterations and Krylov
matvecs, and every common-action residual.

For a launch unit vector `u`, define

\[
d_\parallel=a_w\,u\cdot(X_T-X_0),\qquad
d_\perp=a_w\left|(X_T-X_0)-u\,u\cdot(X_T-X_0)\right|,
\]

\[
\mu={d_\parallel\over vT_{\rm phys}},
\]

and the normalized spline-Poynting translation defect

\[
D={|P_{\rm matter}(T)+P_{\rm spline}(T)
      -P_{\rm matter}(0)-P_{\rm spline}(0)|
     \over \max(|P_{\rm matter}(0)|,10^{-15})}.
\]

Compute `d_parallel` independently in four equal physical-time windows.

## Exact/coherence gates

Every history must satisfy:

1. every forward/reverse root converges and every state remains graph-local,
   graph-connected, chart-valid, and finite;
2. every FTD-0649 action residual remains `<=1e-9` and causal-speed excess
   remains `<=1e-12`;
3. maximum relative bond-length strain remains `<=0.10` and no constituent is
   lost;
4. complete state-only recovery after the full inverse history is `<=1e-7`;
5. each zero control has final physical center displacement `<=1e-6`;
6. the signed high-speed pair mirrors center displacement, final total matter
   momentum, energy, and recovery within `1e-6`;
7. the three parallel cubic copies agree after whole-state rotation within
   `1e-6`.

## Transport and resolution gates

A launched arm is **persistent** only if:

- all four physical-time windows have positive projected advance;
- `mu>=0.50`;
- `d_perp/(|v|T_phys)<=0.10`.

The constructive infrared-trend conjunction additionally requires:

1. every `v=0.04` canonical arm is persistent at every width;
2. the minimum high-speed mobility over the three directions is nondecreasing
   from `w=2` to `w=3` to `w=4`, allowing only `1e-4` numerical slack;
3. the high-speed directional mobility span at `w=4` is strictly smaller than
   at `w=2`;
4. the maximum high-speed `D` over directions at `w=4` is strictly smaller
   than at `w=2`.

The `v=0.01` arms classify the finite depinning bracket but are not required
to be persistent. Three widths cannot establish a zero continuum intercept.

## Locked verdicts

- `CELL_MEASURE_LONG_HORIZON_IR_TREND_CONSTRUCTIVE` if every exact/coherence,
  control, transport, and resolution gate passes.
- `CELL_MEASURE_LONG_HORIZON_MIXED` if every history is exact, coherent, and
  reversible but at least one transport/resolution gate fails.
- `CELL_MEASURE_LONG_HORIZON_CLOSED` if roots execute but an action,
  coherence, control, or inverse gate fails.
- `CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID` if coverage, initialization,
  root convergence, or record completeness prevents evaluation.

No arm, horizon, velocity, tolerance, scale interpretation, or verdict may
change after execution.

## Consequence policy

A constructive result licenses a larger width/horizon ladder and finite-volume
pole preregistration. A mixed result retains exact extended matter but does not
license a pole. A closed result closes this selected cell-measure dynamical
family at the registered scope. None of the verdicts changes production or
establishes native formation, charge, gauge symmetry, quantum statistics,
particle masses, Lorentz recovery, or unitarity.
