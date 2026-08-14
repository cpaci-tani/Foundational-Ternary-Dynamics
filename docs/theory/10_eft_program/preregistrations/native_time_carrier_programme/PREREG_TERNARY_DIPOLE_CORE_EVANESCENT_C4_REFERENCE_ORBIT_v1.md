# FTD-0922 — Ternary-dipole-core evanescent `C4` reference orbit v1

**Identifier:** `FTD-0922`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact outside-band resolvent, compact ternary dipole source,
source-locked order-four field orbit, tail and field-work ledgers, and the
source-autonomy boundary; no numerical search and no engine change

## 1. Question

FTD-0921 leaves a conservative existing-ontology route: abandon exact compact
field support while retaining a compact ternary source core. Since the exact
one-tick order-four stiffness `kappa=2` lies above the free C18 band
`[0,16/9]`, does a compact ternary dipole source generate a rigorously
localized evanescent field profile? Can four rotated source snapshots drive
an exact `C4` field recurrence, and what does the exact source-work ledger
say? Which part remains externally imposed?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md` | `5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8` |
| `AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md` | `033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08` |
| `THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md` | `CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285` |
| `THEOREM_MOORE_COATED_COMPACT_HODGE_PREIMAGE_AND_LIVE_CURRENT_SCAFFOLD_TRILEMMA_v1.md` | `26992693A73CBC956F50CEDA35F481F5658014D57E50EC9D931874D6D1171FB1` |

The certificate fails closed on source drift.

## 3. Frozen outside-band resolvent

Let

\[
 K=-C_{\rm WAVE}^2\Delta_{18},
 \qquad C_{\rm WAVE}^2={1\over3}.
\]

FTD-0919 gives

\[
 K=K^T,
 \qquad
 \sigma(K)\subset[0,16/9].
\]

At `kappa=2`, define

\[
 R_2=(2I-K)^{-1}.
\]

The certificate must prove the convergent operator identity

\[
 \boxed{
 R_2={1\over2}\sum_{m=0}^{\infty}\left({K\over2}\right)^m,}
\]

with

\[
 \left\|{K\over2}\right\|={8\over9}<1,
 \qquad
 \|R_2\|={1\over2-16/9}={9\over2}.
\]

If compact `q` has support `S` and `P_{>=r}` projects outside C18 graph
distance `r-1` from `S`, finite propagation of `K^m` gives

\[
 P_{\ge r}K^m q=0\quad(m<r).
\]

Therefore

\[
 \boxed{
 \|P_{\ge r}R_2q\|_2
 \le {9\over2}\left({8\over9}\right)^r\|q\|_2.}
\]

This is a rigorous localization bound, not a fitted decay constant. It does
not claim compact support or optimal pointwise asymptotics.

## 4. Frozen ternary dipole core

On the uncontained cubic lattice, define

\[
 s_0=\delta_{e_x}-\delta_{-e_x}.
\]

Let `S` be a right-handed quarter-turn about `z`, acting on both coordinates
and vector components, and define

\[
 s_n=S^n s_0.
\]

Then

\[
 s_{n+2}=-s_n,
 \qquad
 s_{n+4}=s_n.
\]

Every snapshot is exactly ternary, neutral, and supported on two sites. Set
the snapshot velocity to zero and use only the coded electric source

\[
 U_n=-G_C\nabla_c s_n.
\]

For `G_C=1`, the certificate must verify directly that `grad_c s_0` has
vector support on exactly eleven sites and

\[
 \|\nabla_c s_0\|_2^2={7\over2}.
\]

The zero-velocity choice is a frozen reference actuator. Because `s_n`
changes while `j_n=s_nv_n=0`, it does not satisfy a source continuity law and
is not a production movement mechanism.

## 5. Frozen evanescent profiles

Define

\[
 q_n=G_C\nabla_c s_n,
 \qquad
 F_n=R_2q_n.
\]

Then

\[
 (2I-K)F_n=q_n,
\]

so

\[
 \boxed{U_n=(K-2I)F_n.}
\]

Since `K`, central gradient, and `R_2` commute with cubic rotations,

\[
 F_{n+1}=SF_n,
 \qquad
 F_{n+2}=-F_n,
 \qquad
 F_{n+4}=F_n.
\]

The certificate must prove `F_0` and `F_1` are nonzero, orthogonal, and have
equal norm. It must verify these relations both abstractly and on an exact
rational `L=4` periodic witness. The periodic witness is a consistency check,
not the uncontained localization proof.

## 6. Frozen exact kick--drift orbit

Define the pre-kick wave velocity

\[
 P_n=F_n+F_{n+1}.
\]

The production-order driven kick--drift is

\[
 P_{n+1}=P_n-KF_n+U_n,
 \qquad
 F_{n+1}=F_n+P_{n+1}.
\]

Using `U_n=(K-2I)F_n`, the certificate must prove both equations exactly:

\[
 P_{n+1}=F_{n+1}-F_n=F_{n+1}+F_{n+2},
\]

\[
 F_n+P_{n+1}=F_{n+1}.
\]

Thus the source-locked reference field has exact order four. Its modal
circulation on the orthonormal plane spanned by `F_0,F_1` must be nonzero and
constant through all four ticks.

This is a maintained driven orbit. It is not a free eigenmode, so it does not
contradict FTD-0919.

## 7. Frozen midpoint source-work ledger

FTD-0576 gives the exact field-invariant source work for kick `U_n`:

\[
 W_n=\left\langle U_n,{P_n+P_{n+1}\over2}\right\rangle.
\]

For the frozen orbit,

\[
 {P_n+P_{n+1}\over2}=F_{n+1}.
\]

Let `A=K-2I`. It is self-adjoint and commutes with `S`. On the doublet,
`S^T=-S`. Therefore `AS` is skew and

\[
 \boxed{
 W_n=\langle AF_n,SF_n\rangle=0}
\]

on every tick. The certificate must verify four exact zero-work arms on the
rational periodic witness.

This is a field-side result: the maintained return is radial and the orbit
increment tangential. It does not account for the work required to change
`s_n`, erase/source records, enforce zero velocity, or react the field impulse
back onto the source.

## 8. Frozen source-autonomy failure control

Because `v_n=0`,

\[
 j_n=s_nv_n=0.
\]

But

\[
 s_{n+1}-s_n\ne0.
\]

Therefore the frozen source snapshots fail the central-continuity equation

\[
 s_{n+1}-s_n+\operatorname{div}_c j_n=0.
\]

The certificate must register this failure exactly. The theorem may book an
evanescent source-locked reference orbit and zero field-side maintenance work,
but it must leave source motion, continuity, reaction, switching work,
formation, and autonomy open.

## 9. Outcome rules

- **Outcome A — evanescent reference orbit with autonomy boundary:** the
  resolvent/tail theorem, compact ternary source, exact `C4` kick--drift,
  conserved circulation, and zero field-side work pass, while the frozen
  continuity control fails exactly. Book the scoped reference construction.
- **Outcome B — no localized orbit:** the outside-band inverse, tail bound,
  exact recurrence, or work identity fails. Book no positive reference result.
- **Outcome C — invalid execution:** any source lock, exact count, rational
  witness, production marker, or scope firewall fails. Book no theorem.

## 10. Required certificate gates

The exact certificate must cover:

1. all frozen hashes;
2. exact C18 band gap at `kappa=2` and Neumann/tail constants;
3. dipole ternarity, neutrality, rotation, support, source support, and norm;
4. exact `L=4` C18 stiffness matrix and invertibility of `2I-K`;
5. rotation covariance, antipodes, orthogonality, and equal norms;
6. all four kick--drift transitions;
7. nonzero constant modal circulation;
8. all four exact midpoint source-work zeros;
9. exact nonzero source-continuity residual;
10. production source and kick--drift markers;
11. unchanged engine/type/import status; and
12. no `G*`, gamma, Born/Bell, context, measurement, fit, sweep, near-miss,
    or formula-substitution read.

## 11. Frozen scope ceiling

Success does not derive autonomous source motion, a continuity-compatible
current, reciprocal source reaction, source switching energy, a positive
source battery, formation, perturbation recovery, mobility, physical scale,
the `G*` gearbox, gamma, Born frequencies, Bell correlations, or preferred
tick hiding. `kappa=2` is chosen because it is the exact order-four control
outside the proved band, not because it numerically matches an observation.
