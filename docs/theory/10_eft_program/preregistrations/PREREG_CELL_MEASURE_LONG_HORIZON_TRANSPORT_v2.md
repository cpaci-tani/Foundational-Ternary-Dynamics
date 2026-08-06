# FTD-0652 — Cell-measure long-horizon transport v2

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Parent:** FTD-0649; execution successor to FTD-0650/0651  
**Scope:** default-off CPU observer dynamics in the selected connected-block action

## Question

Does the FTD-0649 fixed-mass reciprocal action remain coherent, reversible,
and secularly mobile over one common physical horizon as resolution increases?
Do directional splitting and the declared spline-Poynting translation defect
decrease rather than move to another finite scale?

FTD-0650 could not evaluate this question because its locked matrix-free
instrument exceeded the execution budget before complete records were written.
FTD-0651 qualifies the cached exact-root implementation used here. No physical
question or gate changes.

## Frozen clock, ruler, and action

For width `w`,

\[
a_w=2/w,\qquad \ell_w=\tau_w=a_w,\qquad T_{\rm phys}=32,
\]

so forward tick counts are `n_w={32,48,64}` for `w={2,3,4}` and physical
displacement is `a_w` times lattice displacement. Each full forward history is
then inverted for the same number of ticks from state alone.

Retain exactly:

- `L=8w+1`, periodic boundary;
- `r_m=r_q=r_kappa=a_w^3`, `r_beta=a_w^-1` in every FTD-0649 occurrence;
- zero fractional phase and scaled minimum-energy longitudinal dressing;
- finite chart fibre;
- the exact FTD-0649 residual, root tolerance, and action gates;
- no transverse seed, reaction, collision, legacy force, external drive,
  post-step correction, or redressing between ticks.

## Solver and checkpoint change

Use the FTD-0651-qualified central-difference Jacobian with deterministic
Broyden reuse. Each history owns one forward cache and one reverse cache. A
stale cache may only trigger the existing exact central-difference rebuild.
Every accepted state is certified by the unchanged exact residual.

After each arm completes, write its full tick CSV and arm JSON to a temporary
path and rename them into a versioned checkpoint path only after closing the
files. Each checkpoint stores the protocol hash, arm label, coverage, and
verdict inputs. Final campaign outputs require all 30 checkpoints. Incomplete
checkpoints are ignored. Checkpointing observes state and cannot change update
ordering or supply history to reverse dynamics.

Record residual evaluations, Jacobian refreshes/reuses, and wall time in
addition to the FTD-0650 measurements. Performance is diagnostic, not a
physical acceptance gate.

## Locked arm matrix

At each width:

1. six primary arms: `v={0.01,0.04}` along `<100>`, `<110>`, and `<111>`,
   orientation axis zero;
2. one zero-launch control;
3. one sign mirror `v=-0.04<100>`;
4. two whole-state cubic controls for the high-speed parallel family:
   orientation/velocity `(1,+y)` and `(2,+z)`.

Total: 30 histories, 1,440 forward plus 1,440 reverse ticks. Launch momentum
comes from the scaled production dispersion. No momentum is copied between
widths.

## Locked measurements

Per tick record complete state hash, center, matter momentum, local and spline
field momenta, field/kinetic/binding energies, relative bond strain, site hops,
chart multiplicity, nonlinear diagnostics, and every common-action residual.

For launch unit vector `u`,

\[
d_\parallel=a_w u\cdot(X_T-X_0),\quad
d_\perp=a_w|(X_T-X_0)-u\,u\cdot(X_T-X_0)|,
\]

\[
\mu={d_\parallel\over vT_{\rm phys}},\qquad
D={|P_m(T)+P_{\rm spline}(T)-P_m(0)-P_{\rm spline}(0)|
\over\max(|P_m(0)|,10^{-15})}.
\]

Compute projected advance independently in four equal physical-time windows.

## Exact and coherence gates

Every history must satisfy:

1. every forward/reverse root converges and every state remains finite,
   graph-local, graph-connected, chart-valid, and constituent-complete;
2. every action residual is at most `1e-9` and causal excess at most `1e-12`;
3. maximum relative bond-length strain is at most `0.10`;
4. full state-only recovery is at most `1e-7`;
5. zero-control physical displacement is at most `1e-6`;
6. signed high-speed mirrors agree in displacement, final matter momentum,
   energy, and recovery within `1e-6`;
7. three high-speed parallel cubic copies agree after whole-state rotation
   within `1e-6`.

## Transport and resolution gates

A launched arm is persistent only if all four window advances are positive,
`mu>=0.50`, and `d_perp/(|v|T_phys)<=0.10`.

The constructive trend additionally requires:

1. all canonical `v=0.04` arms persistent at every width;
2. minimum high-speed mobility nondecreasing with width, with `1e-4` slack;
3. high-speed directional mobility span smaller at `w=4` than `w=2`;
4. maximum high-speed `D` smaller at `w=4` than `w=2`.

Low-speed arms classify the finite depinning bracket and need not persist.
Three widths do not establish a zero continuum intercept.

## Verdicts

- `CELL_MEASURE_LONG_HORIZON_IR_TREND_CONSTRUCTIVE` if all exact, coherence,
  control, transport, and resolution gates pass;
- `CELL_MEASURE_LONG_HORIZON_MIXED` if every history is exact, coherent, and
  reversible but any transport/resolution gate fails;
- `CELL_MEASURE_LONG_HORIZON_CLOSED` if roots execute but an action,
  coherence, control, or inverse gate fails;
- `CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID` if coverage, initialization,
  convergence, or record completeness prevents evaluation.

No arm, horizon, velocity, tolerance, scale, physical gate, or verdict may
change after execution. A constructive result licenses only a larger ladder
and pole preregistration. It does not establish native formation, reaction,
charge, gauge symmetry, quantum statistics, particle masses, Lorentz recovery,
unitarity, or production adoption.
