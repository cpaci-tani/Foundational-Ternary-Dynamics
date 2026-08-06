# FTD-0622 — Connected Moore-block common action

**Status:** `[SELECTED DYNAMICS] + [THEOREM — CONDITIONAL EXACT
COMMON-ACTION IDENTITIES] + [MEASURED — CONSTRUCTIVE ONE-STEP RESPONSE AND
POSITIVE PRELIMINARY INFRARED TREND]`  
**Protocol:** `PREREG_CONNECTED_MOORE_BLOCK_COMMON_ACTION_v1.md`, SHA-256
`7E09ADBC2A16513DD3495BB117015F574E150F7B8BA5632C03BC96783AFE00AF`  
**Parent record:** FTD-0621, SHA-256
`D6ED6A0BF3C9B351ED59E4B16C0FD82430A4713B4ED06B0092F9BDCBB4026383`  
**Verdict:** `CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_POSITIVE`  
**Production status:** unchanged

## 1. Result

The exact integer block-bipole family of FTD-0621 admits the registered
runtime-size generalization of the constituent-complete common action.  At
widths `w=1,2,3`, every occupied ternary site is one explicit constituent,
and initial Moore neighbours are joined by local quartic binding edges.  All
13 registered forward transactions and all 13 state-only inverse
transactions pass.

The construction uses no rigid centre coordinate, fractional primitive
polarity, stationary compensator, independent-copy superposition, legacy
force branch, or post-hoc energy correction.  Motion, derived face current,
Maxwell update, field recoil, and edge binding are solved simultaneously.

## 2. Conditional exact identities

For the selected state and action, the following identities are analytic
consequences of the endpoint discrete-gradient construction:

1. every constituent segment satisfies exact discrete continuity;
2. the aggregate current preserves Gauss when inserted into the matched field
   update;
3. the endpoint dispersion gradient obeys
   `Delta H = v_bar dot Delta p`;
4. magnetic impulse performs zero scalar work;
5. the edge discrete gradient gives binding-energy change equal to negative
   binding work, while edge impulses sum to zero;
6. electric current work is matter-energy gain and field-energy loss;
7. reversing the same implicit transaction reconstructs the earlier state
   from the later state and fixed action data.

These are conditional theorems about the selected common action.  They do not
derive that action, its bond graph, or `kappa=1` from the five FTD postulates.

## 3. Registered measurements

For the primary `x` orientation, the fractional-response values are:

| width | constituents | edges | `Pi_parallel` | `D_parallel` | `Pi_transverse` | `D_transverse` |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 2 | 1 | `0.2811369406` | `0.5551175951` | `0.09650416302` | `0.1743524897` |
| 2 | 16 | 72 | `0.04004290983` | `0.07053780525` | `0.01882540306` | `0.03283883734` |
| 3 | 54 | 365 | `0.01370536723` | `0.02386621687` | `0.007285465576` | `0.01265382491` |

Here

\[
\mathcal D = \frac{C_{\rm SPEED}
|\Delta P_{\rm matter}+\Delta P_{\rm spline}|}{E_{\rm field,0}}.
\]

Both `Pi` and `D` decrease strictly in both registered phase classes.  This is
the preregistered positive preliminary trend.  It is not an asymptotic fit or
a zero-intercept result.

All integer-phase controls have centre displacement at or below
`1.78e-15`.  The worst aggregate common-action residual is
`8.71117414314e-12`, worst state-only recovery is
`1.96593048923e-14`, and worst cyclic-covariance residual is
`2.48048077664e-12`.  All are inside their preregistered gates.

## 4. Ontological consequence

This result removes one specific objection to extended matter: exact ternary
site values do not prevent a many-site object from exchanging current,
energy, and recoil with one field in a reversible local transaction.  A
fractional density envelope is unnecessary as primitive ontology; the only
fractional density used here is the deterministic compact coupling image of
integer constituents at subcell positions.

The response also supports a concrete mechanism for continuum-like behaviour:
collective extension can reduce the *relative* sensitivity of a material
pattern to lattice phase and reduce the dimensionless continuous-translation
defect at the same time.  This is a finite-width observation, not yet an
infrared theorem.

The object carries more state than the ternary snapshot alone.  Constituent
momenta and the frozen local bond graph are part of its selected material
state.  The graph therefore acts as relational memory.  FTD-0622 does not
derive that memory from native event rules and does not show that a graph can
form, reconnect, collide, or decay.

## 5. What is not established

- The widths do not describe the same fixed-mass object: constituent count,
  rest energy, field energy, and bond count all increase with `w`.
- Absolute Peierls barriers remain positive; the one-step response is not a
  gapless translation mode.
- No long-time coherent motion, stationary dressed state, decay law, or
  collision channel has been measured for this family.
- Three widths do not establish that `D` tends to zero.
- The spline momentum remains a diagnostic, not a derived exact coupled
  Noether charge.
- The construction is not a physical electron, electromagnetic charge,
  `U(1)`, matter pole, Lorentz recovery, unitarity result, or production rule.

## 6. Next discriminator

FTD-0623 must test whether the same connected object survives repeated
common-action updates.  It must separate rest stability, driven coherent
translation, internal deformation, bond failure, chart failure, and numerical
failure.  Any comparison across widths must report extensive energy and
constituent count alongside normalized observables.  No conclusion about a
particle or infrared limit is licensed unless repeated dynamics remains
coherent.

## 7. Reproducibility

- header SHA-256:
  `9EB68CA30D27D405366181AF2878C24E8F9E402324DC937A89708B9B1DE4CAD2`
- source SHA-256:
  `27DB8D0042024900F0130FFEA5C4864580627C6539B601D1BAE72520AFE96379`
- test SHA-256:
  `87AEFF7854FFC5BBBC1CBBBB50600CA81E1EA4DE0EB3B9E1E14052E3503452BC`
- JSON SHA-256:
  `6ED5287FB9AD84BACED79885E24E2352FE05CA82FA77636DD968297D6DF73396`
- CSV SHA-256:
  `81E989AE992CF0D00A7FCB54118883DD0779E1A0FC03AF6D839F665CF230E49A`
- independent certificate SHA-256:
  `6F4A16CC70ECA31E6A772893C8875C8C304146CC0FB921E37CCA2DC826618DCE`

The focused CTest `connected_moore_block_common_action` passes in
`193.77 s`; the independent certificate passes `32/32` checks.
