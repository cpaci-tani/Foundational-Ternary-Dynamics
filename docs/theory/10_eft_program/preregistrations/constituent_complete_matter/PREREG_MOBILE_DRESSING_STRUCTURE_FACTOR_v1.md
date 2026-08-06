# FTD-0655 — Mobile dressing structure factor v1

**Status:** `[PRE-REGISTRATION — LOCK BEFORE IMPLEMENTATION/EXECUTION]`  
**Parent:** FTD-0654  
**Scope:** observer-only selected cell-measure common action; no production change

## 1. Question

Does the matched field-energy dressing travel as one coherent dynamical pattern
with the fixed-total-measure constituent core, or does the core move while the
field dressing dephases, diffuses, or remains tied to the lattice?

This is a necessary feasibility gate before a retarded pole campaign. It is
not itself a pole, particle, charge, photon, or Lorentz test.

## 2. Frozen dynamics

Use the unchanged FTD-0654 common action and exact cached-root solver with

\[
a=2/w,\qquad r_m=r_q=r_\kappa=a^3,\qquad r_\beta=a^{-1}.
\]

Keep `allow_shared_anchor_chart=true`, periodic `L=8w+1`, physical horizon
`T_phys=64`, exact forward/reverse evolution, and all FTD-0654 root, action,
causality, graph, chart, strain, energy, and inverse tolerances.

No force, stencil, normalization, field update, state variable, or accepted
root is changed. The new calculation is read-only.

## 3. Locked arms

For each `w={2,3,4}` run:

1. primary boosts `v=0.03` along `<100>`, `<110>`, and `<111>`;
2. a sign mirror `v=-0.03<100>`;
3. whole-state cubic images `+0.03<010>` and `+0.03<001>`.

Total: 18 histories and `64w` forward plus `64w` reverse ticks per history.

## 4. Observer definitions

For the launch direction `d`, use the first nonzero periodic wavevector

\[
k={2\pi\over L}d_{\rm integer},
\]

where `d_integer` is `(1,0,0)`, `(1,1,0)`, or `(1,1,1)` for the three
families and the corresponding rotated axial vector for cubic controls.

The constituent mass-density structure factor is

\[
F_m(t)=\sum_A m_A\exp[-i k\cdot(n_A+r_A)],
\]

using the exact effective position `site+remainder`.

The field-energy structure factor is the component-position-aware sum

\[
F_f(t)=\sum_\ell {r_\beta\over2}
 (E_\ell^2+B_\ell^2)\exp[-i k\cdot x_\ell],
\]

with electric components at face centres and magnetic components at edge
centres. No centered projection or constituent data enter `F_f`.

For each complex series independently:

1. unwrap `arg F(t)` by nearest `2pi` continuation;
2. fit `theta(t)=theta_0+s t` by unweighted least squares over all forward
   ticks, including the initial state;
3. define projected phase velocity `v_phase=-s/|k|`;
4. define phase RMS from that fit;
5. define amplitude coefficient of variation `CV=sd(|F|)/mean(|F|)`.

Define relative-phase RMS by fitting and removing one constant from
`unwrap(arg(F_f/F_m))`. Define direct kinematic velocity from the physical
centre displacement divided by `T_phys`.

## 5. Locked gates

Every arm must pass the inherited exact/coherence gates. For every arm:

- `mean(|F_m|)>1e-8` and `mean(|F_f|)>1e-12`;
- matter phase RMS `<0.10 rad`;
- field phase RMS `<0.20 rad`;
- matter amplitude `CV<0.10`;
- field amplitude `CV<0.20`;
- relative-phase RMS `<0.20 rad`;
- `|v_matter-v_center|/0.03 < 0.10`;
- `|v_field-v_matter|/0.03 < 0.10`.

For each width, mirror velocities must reverse sign with sum residual below
`1e-8`; the two cubic images must agree with the axial primary in matter and
field phase speed, phase RMS, amplitude CV, and relative-phase RMS to `1e-8`.

Across widths, the maximum matter/field velocity mismatch, maximum relative-
phase RMS, and maximum field-amplitude CV must each decrease strictly from
`w=2` to `w=3` to `w=4`.

## 6. Verdict map

- `MOBILE_DRESSED_STRUCTURE_FACTOR_CONSTRUCTIVE`: all exact, individual,
  symmetry, and width-trend gates pass.
- `MOBILE_CORE_FIELD_DRESSING_MIXED`: exact coherent matter motion passes and
  the matter structure factor passes, but any field, relative-phase, symmetry,
  or width-trend gate fails.
- `MOBILE_MATTER_STRUCTURE_FACTOR_CLOSED_NEGATIVE`: exact evolution completes
  but the matter structure factor or its centre-velocity agreement fails.
- `MOBILE_DRESSING_STRUCTURE_FACTOR_EXECUTION_INVALID`: coverage, inherited
  exactness, or record completeness fails.

The constructive verdict licenses a separately preregistered retarded
source-response pole campaign. It does not itself license particle, photon,
charge, common-cone, unitarity, or production language. A failed trend remains
a failed v1 conjunction; no threshold or observable is changed after inspection.
