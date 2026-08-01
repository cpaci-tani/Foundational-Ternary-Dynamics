# FTD-0654 — Out-of-sample mobility convergence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`  
**Parents:** FTD-0652/0653  
**Scope:** selected cell-measure action; new velocities and doubled horizon

## Question

Do the FTD-0652 anisotropy and target-centred mobility improvements survive on
unseen launch velocities and a doubled physical horizon, or were they specific
to the inspected `v={0.01,0.04}`, `T_phys=32` matrix?

## Frozen dynamics and scales

Retain the complete FTD-0652 action, cell factors, periodic volumes, finite
chart fibre, cached exact-root solver, independent forward/reverse caches,
atomic checkpoints, and all exact/coherence tolerances.

Use `a=2/w`, `ell_w=tau_w=a`, but set the new common physical horizon to
`T_phys=64`. Therefore widths `w={2,3,4}` run exactly `{64,96,128}` forward
ticks and the same number in state-only reverse.

## Locked arms

At each width:

1. six new primary arms: `v={0.02,0.03}` along `<100>`, `<110>`, `<111>`;
2. one zero control;
3. one sign mirror `v=-0.03<100>`;
4. two whole-state cubic controls at `v=0.03`.

Total: 30 histories and 5,760 forward/reverse ticks. No FTD-0652 primary
velocity is reused.

## Exact/coherence gates

Retain verbatim the FTD-0652 root, action `1e-9`, causal `1e-12`, strain
`0.10`, inverse `1e-7`, zero `1e-6`, mirror `1e-6`, cubic `1e-6`, chart,
graph, finite-state, and constituent-completeness gates.

A primary arm is persistent only if all four equal physical-time windows
advance, `mu>=0.50`, and transverse drift ratio is at most `0.10`.

## Prospective convergence gates

For each new speed independently define

\[
E_w(v)=\max_d|\mu_w(d,v)-1|,
\quad A_w(v)=\max_d\mu_w(d,v)-\min_d\mu_w(d,v),
\quad D_w(v)=\max_d D_w(d,v).
\]

The normalized-target conjunction passes only if, for both velocities:

1. all nine width/direction arms are persistent;
2. `E_4(v)<E_3(v)<E_2(v)`;
3. `A_4(v)<A_3(v)<A_2(v)`;
4. `D_4(v)<D_2(v)`.

Also report, without using it to rescue the normalized gate, the common-target
interval formed by the three width-four direction values. If the normalized
gate fails but directional span still shrinks and all width-four intervals for
both speeds overlap one positive interval, classify a **renormalized-common-
mobility candidate**. This is a finite-width classification, not an intercept
fit or pole.

## Verdicts

- `OUT_OF_SAMPLE_NORMALIZED_MOBILITY_CONSTRUCTIVE` if every exact/coherence and
  normalized-target gate passes;
- `OUT_OF_SAMPLE_RENORMALIZED_MOBILITY_MIXED` if exact/coherence gates pass,
  normalized target fails, but the declared common-positive interval condition
  passes;
- `OUT_OF_SAMPLE_MOBILITY_MIXED` if exact/coherence gates pass but neither
  convergence conjunction passes;
- `OUT_OF_SAMPLE_MOBILITY_CLOSED` if any physical exact/coherence/control gate
  fails;
- `OUT_OF_SAMPLE_MOBILITY_EXECUTION_INVALID` if coverage or records are
  incomplete.

No threshold, target, velocity, horizon, or verdict may change after execution.
Even a constructive result licenses only a separately preregistered pole
campaign; it does not establish a particle, formation, charge, Lorentz
recovery, or production adoption.
