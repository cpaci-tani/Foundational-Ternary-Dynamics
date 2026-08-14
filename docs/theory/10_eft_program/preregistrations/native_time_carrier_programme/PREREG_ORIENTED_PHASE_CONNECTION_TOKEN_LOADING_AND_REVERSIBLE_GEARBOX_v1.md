# Pre-registration — Oriented phase-connection token loading and reversible gearbox v1

**Identifier:** `FTD-0962`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Question

Can an oriented local clock traversal itself load the existing signed history
port and align a controller without a discontinuous `sign(Pi)` detector,
phase erasure, target-coded weight, external work pulse, or reset?

The candidate must close:

1. a positive autonomous Hamiltonian;
2. exact reciprocal source reaction;
3. speed-independent oriented token loading;
4. active controller alignment by complete phase-state export;
5. exact reverse recovery;
6. endpoint energy and finite-reserve/backpressure accounting; and
7. the conditional relation between the critical-quartic `G*` cadence and
   the gearbox holonomy.

It may not call selected internal modes or a selected connection profile
substrate-derived.

## 2. Frozen sources

| Source | Frozen SHA-256 |
|---|---|
| `THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md` | `8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33` |
| `THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md` | `73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98` |
| `THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` |
| `THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md` | `746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E` |
| `THEOREM_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_AND_COMPACT_CARRY_BOUNDARY_v1.md` | `85FC00E7B613894D5CD18276947C4A3BAD0B08CC8C8323996012B6EF8EE79514` |

No engine, production tick, constant, toggle, selector, Born law, or ontology
type may change under this protocol.

## 3. Frozen canonical architecture

Use the clock pair `(delta,Pi)` and four already-priced complete canonical
modes:

- token battery `B=(b_q,b_p)`;
- outgoing signed record `D=(d_q,d_p)`;
- target controller reference `C=(c_q,c_p)`; and
- aligned reserve/return reference `R=(r_q,r_p)`.

Define the two commuting exchange generators

\[
 G_T=b_qd_p-d_qb_p,
 \qquad
 G_C=r_qc_p-c_qr_p,
 \qquad G=G_T+G_C,                                    \tag{1}
\]

and positive mode actions

\[
 A_T={|B|^2+|D|^2\over2},
 \qquad
 A_C={|C|^2+|R|^2\over2}.                            \tag{2}
\]

Let `calA(delta)` be a selected real `C1` phase-connection profile whose
support lies inside one declared crossing chart and whose positive traversal
has

\[
 \int_{delta_-}^{delta_+}calA(delta)d delta={pi\over2}. \tag{3}
\]

The frozen compact-chart witness is, for `b>0`,

\[
 calA_b(delta)=
 \begin{cases}
 {15pi\over32b}\left(1-{delta^2\over b^2}\right)^2,
     &|delta|\le b,\\
 0,&|delta|>b.
 \end{cases}                                          \tag{3a}
\]

It must be `C1` at `+/-b` and have integral `pi/2`. Other profiles are outside
this first reference certificate.

With `M,nu_T,nu_C>0` and `V(delta)>=0`, freeze

\[
 \boxed{
 H_{\rm conn}=
 {K^2\over2M}+V(delta)+nu_T A_T+nu_C A_C,
 \qquad K=Pi+calA(delta)G.}                           \tag{4}
\]

Equation (4), including the complete square, is the candidate. A merely
linear term `Pi calA G/M` without `calA^2G^2/(2M)` is not admissible as the
same positive reciprocal law.

## 4. Frozen preparation and interpretation

At the entrance where `calA(delta_-)=0`, require

\[
 B=(a,0),\quad D=(0,0),\quad a>0,                    \tag{5}
\]

and

\[
 R=(r,0),\quad r>0,                                   \tag{6}
\]

where `R` is aligned with the already-selected catalytic phase frame. `C` is
arbitrary admitted controller state. The token energy is

\[
 epsilon_{\rm tok}=nu_Ta^2/2.                         \tag{7}
\]

The forward crossing is the traversal `delta_- -> delta_+`; the reverse
crossing traverses the same chart in the opposite direction. `D` may be handed
to the FTD-0961 existing oriented rail only after the forward endpoint is
reached.

## 5. Frozen gates

### G1 — Source and marker integrity

All hashes and scope markers must pass. No numerical fitting, floating
tolerance, near-miss search, or formula-substitution discovery is permitted.

### G2 — Exchange algebra

The certificate must prove:

\[
 \{G_T,G_C\}=0,
 \quad \{G,A_T\}=\{G,A_C\}=0,                         \tag{8}
\]

and derive the exact mode flow of `G`:

\[
 \begin{aligned}
 D(alpha)&=D_0\cos alpha+B_0\sin alpha,\\
 B(alpha)&=B_0\cos alpha-D_0\sin alpha,\\
 C(alpha)&=C_0\cos alpha+R_0\sin alpha,\\
 R(alpha)&=R_0\cos alpha-C_0\sin alpha.
 \end{aligned}                                        \tag{9}
\]

It must verify symplecticity, determinant one, exact inverse, and preservation
of both positive actions.

### G3 — Reciprocal connection equations

For equation (4), prove

\[
 \dot delta=K/M,
 \qquad
 \dot Pi=-V'(delta)-{K\over M}calA'(delta)G,
 \qquad \dot G=0,                                     \tag{10}
\]

and hence

\[
 \boxed{\dot K=-V'(delta).}                            \tag{11}
\]

The mechanical source trajectory therefore obeys the uncoupled natural-well
equation even when `G_C` is nonzero. The reciprocal reaction is not absent: it
is the exact canonical/mechanical difference

\[
 Pi=K-calA(delta)G.                                    \tag{12}
\]

At chart endpoints, `calA=0`, so canonical and mechanical momenta coincide.

### G4 — Oriented holonomy and speed independence

After removing the common free rotations generated by `nu_T A_T+nu_C A_C`,
the exchange angle must satisfy

\[
 \dot alpha=calA(delta)\dot delta,
 \qquad
 \boxed{alpha=\int calA(delta)d delta.}                \tag{13}
\]

Equation (13) must remain exact for arbitrary admitted crossing speed and
nonzero `G_C`. The forward chart gives `alpha=+pi/2`; the reverse chart gives
`alpha=-pi/2`.

### G5 — Forward token load and controller alignment

On (5)--(6), the forward map must give

\[
 B'=(0,0),\qquad D'=(a,0),                             \tag{14}
\]

and

\[
 C'=R=(r,0),qquad R'=-C.                              \tag{15}
\]

Thus the outgoing record is the `+1` ternary orientation in the selected
phase frame, the target controller is at gate zero in that frame, and the
complete old controller state survives in the return mode. A fresh negative
traversal from the same ready preparation must instead emit `D'=(-a,0)` and
place the target at the inverse-oriented gate `C'=(-r,0)`.

The negative gate is not to be relabelled as the same forward gate. In the
closed recursive sequence it is the inverse/unactualization stroke.

### G6 — Reverse recovery and self-dual recursion

Starting from the forward endpoint (14)--(15), a reverse traversal through the
same chart must recover exactly

\[
 (B,D,C,R)_{\rm final}=(B,D,C,R)_{\rm initial}.        \tag{16}
\]

The map must be time-reversal compatible: reversal flips `Pi` and all mode
momenta, hence `G->-G`, `K->-K`, while equation (4) remains invariant.

### G7 — Positive energy, reserve, and backpressure

Equation (4) must be nonnegative. It must conserve total Hamiltonian exactly,
preserve the two action sums, transfer the fixed token energy (7) from `B` to
`D`, and exchange rather than erase controller energy/state.

The forward semantic gate requires a nonzero battery, blank output, and
nonzero aligned reserve. An occupied output or absent reserve is exact
backpressure: the Hamiltonian remains reversible, but the result is not a
fresh one-shot record and must not be accepted as one. Repetition in one
direction requires fresh/recycled batteries and aligned reserves or must fail
closed.

### G8 — Exact phase-reset no-go and export witness

Any differentiable map that sends a controller phase coordinate to one
constant on an open set while retaining no output dependence on the old phase
has a zero Jacobian row and cannot be a symplectic diffeomorphism. Equation
(15) avoids this by moving the complete old controller mode into `R'=-C`.

This is reversible phase replacement, not erasure and not a contraction of an
open phase set into one state.

### G9 — Critical-quartic `G*` specialization

Conditional on a maintained fixed-amplitude critical quartic clock, let
`Theta_*` be its lifted action-angle phase and choose

\[
 calA_*(delta)d delta=dTheta_*                         \tag{17}
\]

inside the registered quadrant chart. Then one quadrant has holonomy
`Delta alpha=Delta Theta_*=pi/2`. Retain the exact period law

\[
 T_* A=\sqrt{pi}\,G^*\sqrt{m/(2lambda)}.              \tag{18}
\]

The certificate must derive only the conditional cadence

\[
 omega_*={2pi\over T_*}
 ={2pi A\over\sqrt{pi}G^*}\sqrt{2lambda/m},
 \qquad t_{\rm quadrant}=T_*/4.                       \tag{19}
\]

This separates roles exactly:

- `G*` fixes the maintained clock's temporal traversal rate;
- the normalized connection fixes the quarter-turn holonomy; and
- the signed path orientation fixes forward versus inverse operation.

Equation (19) is conditional use of the existing quartic law, not a derivation
of the connection from CM arithmetic or from production substrate dynamics.

### G10 — Scope and stop conditions

The result must retain as open:

- derivation of `calA`, the complete-square coupling, and all four internal
  modes from production fields;
- formation, replenishment, protection, routing, collision handling, and
  recycling of token batteries and aligned reserves;
- a proof that the production lattice exposes the required local clock
  coordinate and conjugate mechanical momentum;
- maintained-amplitude feedback, work, dissipation, and perturbation recovery;
- one-way unactualization/loss rather than exact reverse recovery;
- full nonlinear repeated-cycle stability and positive attraction export;
- CM-prime selection of the connection normalization;
- Born/Bell recovery, operational Lorentz hiding, and completeness.

Any target probability, future crossing, Born weight, fitted physical scale,
or outcome-coded connection is a failure. Any claim that (4) is already a
production law is a failure.

## 6. Frozen outcomes

- **Outcome A:** all exact gates pass and the connection/modes are also shown
  to be already generated by unchanged production dynamics.
- **Outcome B:** the exact positive connection witness, reciprocal reaction,
  oriented token loading, reversible controller alignment, inverse, and
  conditional `G*` cadence all pass, while native production/formation and
  connection selection remain open.
- **Outcome C:** the exact reference construction fails a mathematical,
  energy, inverse, or scope gate.
- **Outcome D:** source-integrity or certificate failure prevents
  classification.

The frozen expected classifier is Outcome B. Reference success cannot count as
substrate evidence.
