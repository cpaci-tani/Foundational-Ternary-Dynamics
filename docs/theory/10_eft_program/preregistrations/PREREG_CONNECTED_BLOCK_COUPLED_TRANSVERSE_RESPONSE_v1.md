# FTD-0642 — Connected-block coupled transverse response v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Matter parent:** FTD-0640
`CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE`  
**Field parent:** FTD-0641
`CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CONSTRUCTIVE`  
**Scope:** release the exact-center matter coordinates under registered
transverse face/edge perturbations and classify the resulting tangent response  
**Date:** 2026-07-27

## 1. Question

When an independently qualified transverse field mode is placed on the exact
dressed object and all 48 constituent coordinates are released, does the same
common action produce a reversible linear coupled response? Is the response
weakly hybridized, strongly hybridized, effectively decoupled, or unstable?

This is not yet a pole-extraction campaign. It measures finite-volume tangent
hybridization and supplies the inputs needed to design one.

## 2. Locked arms

Use the FTD-0638 orientation-zero center at `L=17`. Use the `n=1` canonical
wavevectors `<100>=(1,0,0)`, `<110>=(1,1,0)`, and `<111>=(1,1,1)`, with both
nonvanishing edge-potential polarizations. For each of these six modes run:

- full amplitude `+1e-7`;
- half amplitude `+5e-8`;
- sign mirror `-1e-7`.

This gives 18 arms. Set initial matter momenta to zero. Evolve every arm for
256 full common-action forward ticks and 256 state-only reverse ticks, using
the observer-only exact-residual Jacobian cache qualified beside FTD-0640.
In parallel, evolve the same initial transverse perturbation through the
source-free matched field map as a bare-field reference.

## 3. Locked observables

At every forward tick record:

- the coupled electric projection `q_c` onto the initial transverse face mode;
- the bare source-free projection `q_0`;
- electric leakage orthogonal to that face mode;
- all 48 projections onto the FTD-0640 analytic matter basis;
- constituent coordinate RMS, center displacement, full-state excursion,
  common-action residual, energy drift, chart sector, fibre multiplicity,
  separation, and site hops.

Define

\[
D_{\rm field}=\frac{\|q_c-q_0\|_2}{\|q_0\|_2},
\qquad
L_{\rm field}=\frac{\|E_c-q_c e_0\|_2}
{\|q_c e_0\|_2},
\]

and matter response

\[
R_m=\sqrt{\frac1{256}\sum_t\sum_{a=1}^{48}Q_a(t)^2}.
\]

## 4. Exact and linear-response gates

Require all 18 arms to:

- initialize, complete, and invert;
- remain in the starting spline sector with zero site hops, anchor
  multiplicity `<=8`, and same-anchor separation `>=0.9`;
- keep center displacement and full-state excursion `<=1e-3`;
- keep common-action residual `<=1e-10`, energy drift `<=1e-12`, and inverse
  recovery `<=1e-10`.

For each full/half/sign triple require:

- `R_m(full)>1e-9` to count as detected coupling;
- `R_m(full)/R_m(half)` in `[1.8,2.2]`;
- full/half coupled-field phase difference `<=1%`;
- full/sign coupled-field phase difference `<=1%`;
- normalized signed matter-modal trajectory residual `<=10%`;
- normalized signed field-projection trajectory residual `<=10%`.

For a weakly hybridized response additionally require every full arm to have
an identifiable finite recurrence phase, relative phase shift from the FTD-0641
bare phase `<=5%`, `D_field<=0.25`, and `L_field<=0.25`.

## 5. Verdicts

- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_WEAK_HYBRID_CONSTRUCTIVE` if exact,
  coupling, scaling/sign, and weak-hybridization gates all pass;
- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_STRONG_HYBRID_CONSTRUCTIVE` if exact
  coupling and linear scaling/sign pass but any weak-hybridization gate fails;
- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_DECOUPLED` if all exact gates pass and
  every full-arm `R_m<=1e-9`;
- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_MIXED` if exact gates pass but detected
  response fails amplitude or sign linearity;
- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_CLOSED_NEGATIVE` if a valid arm leaves
  the registered bounded/sector envelope or fails state-only inversion;
- `CONNECTED_BLOCK_COUPLED_TRANSVERSE_EXECUTION_INVALID` for provenance,
  eigensystem, coverage, initialization, solver, or output failure.

No outcome establishes a photon, asymptotic scattering state, continuum pole,
physical charge, common cone, or Lorentz recovery.

## 6. Artifacts

Produce one focused CTest, arm/tick CSV and JSON summary, independent
certificate, analysis/audit, and synchronized canonical records. Production
remains unchanged.
