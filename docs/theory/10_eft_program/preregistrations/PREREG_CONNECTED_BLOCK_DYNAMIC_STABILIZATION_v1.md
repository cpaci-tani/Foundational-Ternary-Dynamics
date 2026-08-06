# FTD-0625 — Connected-block dynamic stabilization v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0624 JSON SHA-256
`55D34381B4968653740DF57A0F2330A3D175CC2CFD52012A2C4657D601825653`  
**Scope:** existing-variable circulation feasibility before any new occupancy
primitive  
**Date:** 2026-07-27

## 1. Question

FTD-0624 shows that the smooth static minimum of the connected neutral block is
an opposite-polarity ternary occupancy conflict. This campaign asks the least
expensive next question:

> Can zero-total-momentum circulation already present in constituent phase
> space keep the connected pattern inside the admissible ternary chart long
> enough to license a dedicated periodic-orbit search?

This is a feasibility test, not a stability or particle claim. Failure closes
the registered rigid-circulation family only. Success licenses a longer locked
periodic/Floquet campaign; it does not itself establish a bound state.

## 2. Frozen action

Use the unchanged FTD-0622–0624 selected action:

- `L=17`, `w=2`, 16 exact `+1/-1` constituents, 72 reference-Moore bonds;
- body orientation and translation axis `x` in base arms;
- initial phase `f=1/2-1/64`;
- `kappa=1`, `dt=1`, `C_SPEED=1/sqrt(3)`;
- minimum-energy initial Gauss field and zero initial magnetic field;
- unchanged production dispersion, quadratic coat, face current, matched
  face/edge update, interaction normalization, common-action tolerance
  `1e-10`, solve tolerance `2e-11`, and 48-iteration limit.

No damping, reaction, annihilation, exclusion impulse, graph rewiring,
neutralizer, legacy force, fitted coefficient, field correction, or production
change is admitted.

## 3. Derived circulation launch

For effective constituent positions `X_a` and centre `X`, define a rigid
zero-total-momentum circulation about the base `z` axis,

\[
p_a(A,\sigma)=\sigma A\,\hat z\times(X_a-X),\qquad \sigma\in\{-1,+1\}.
\]

The centred block makes `sum_a p_a=0` exactly. Let

\[
K(A)=\sum_a\left[
\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p_a(A,+)|^2}
-E_{\rm REST}\right].
\]

Let `B_x=C_x/16` be the FTD-0624 parallel Peierls barrier. Determine the unique
nonnegative amplitudes `A_1,A_4` from the monotone equations

\[
K(A_1)=B_x,\qquad K(A_4)=4B_x.
\]

The amplitudes are model-internal energy normalizations, not fits. Solve them
to residual `<=1e-13` by a deterministic bracketed root. No other amplitude may
be substituted.

## 4. Locked arms

Run 16 forward steps followed by 16 state-only reverse steps for:

1. zero-circulation near-half control;
2. `+A_1` and `-A_1`;
3. `+A_4` and `-A_4`;
4. cyclic rotations of the `+A_1` and `+A_4` arms, mapping body/phase axis
   `x -> y` and circulation axis `z -> x`.

Total: seven arms, 224 registered transactions. No failed arm may be replaced.

## 5. Observers

At every forward tick record:

- centre, total matter momentum, and centre-subtracted shape RMS;
- binding strain, matter/field/total energy, and common-action residuals;
- constituent anchors, remainders, and minimum chart margin
  `m=min_(a,i)(1/2-|r_ai|)`;
- all opposite-polarity anchor conflicts;
- internal angular momentum
  `L_int=sum_a (X_a-X) cross p_a`;
- phase distance to the half-cell surface;
- local and spline translation-reaction defects.

The chart margin is diagnostic only. Exact unique ternary projection remains
the admissibility gate.

## 6. Exact and trajectory gates

Every executed step must pass the unchanged continuity, Gauss, work, energy,
causality, graph, site-projection, and common-action gates. Across every
qualified arm require:

- total-energy drift `<=1e-9`;
- shape RMS `<=0.05` cell;
- maximum squared-edge strain `<=0.10`;
- state-only recovery `<=1e-8`;
- initial total matter momentum `<=1e-14`;
- circulation-energy root residual `<=1e-13` for nonzero arms.

Signed partners must agree in scalar energy, shape, strain, chart-margin, and
collision histories within `1e-8`; their internal angular momenta must reverse
within `1e-8`. Cyclic controls must rotate all vector histories and agree in
scalar histories within `1e-8`.

## 7. Feasibility discriminator

Let `m_0` be the minimum chart margin achieved by the zero-circulation control
over its valid forward history. A registered amplitude family is a candidate
stabilizer only if both signs and its cyclic control:

1. complete all 16 forward and reverse steps with no occupancy conflict;
2. keep the centre within `2 epsilon=1/32` of the positive half-cell phase;
3. achieve minimum chart margin at least
   `m_0+epsilon/10 = m_0+1/640`;
4. retain the sign of the initial axial internal angular momentum at tick 16;
5. remain below the coherence and exactness gates in section 6.

The registered margin increment is fixed before execution. It prevents calling
mere survival or numerically indistinguishable delay “stabilization.”

## 8. Verdicts

- `RIGID_CIRCULATION_DYNAMIC_STABILIZATION_FEASIBLE`: at least one complete
  amplitude family passes every exact, symmetry, inverse, and feasibility gate.
  This licenses a separately locked 128-tick periodic-orbit/Floquet search.
- `RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE`: the action is
  executable and controls are valid, but neither registered amplitude family
  passes the feasibility conjunction.
- `CONNECTED_DYNAMIC_STABILIZATION_EXECUTION_INVALID`: initialization,
  amplitude normalization, required controls, or record integrity fails before
  the dynamical comparison can be evaluated.

No outcome promotes a production toggle, scenario, stable particle, angular-
momentum quantum number, spin, statistics, annihilation rule, gauge ontology,
or normal-mode pole.

## 9. Escalation rule

If rigid circulation closes negative, do not tune amplitudes or add a contact
force post hoc. The next explicit fork is:

1. an atomic reaction transaction that converts an opposite-polarity conflict
   into outgoing field state while auditing invertibility; or
2. a newly priced occupancy/temporal-phase or topological species fibre that
   prevents the conflict by construction.

The existing `J_L/J_R` dual substrate is not such a fibre: it supplies two
field registers, not two ternary occupancy slots.
