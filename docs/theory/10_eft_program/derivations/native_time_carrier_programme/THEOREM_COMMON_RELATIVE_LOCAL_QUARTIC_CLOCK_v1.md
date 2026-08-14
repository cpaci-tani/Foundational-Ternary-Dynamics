# FTD-0844 — Common/relative local quartic clock carrier

**Status:** `[THEOREM — EXACT SELECTED TWO-CHANNEL CONSTRUCTION]` +
`[THEOREM — P4-LOCAL POSITIVE-SECTOR ENERGY CLOSURE]` +
`[THEOREM — SINGLE-SITE RELATIVE QUARTIC CARRIER]` +
`[SELECTION/OPEN — RANK-ONE CROSS-GRADIENT, FORMATION, READOUT, MAINTENANCE, AND CADENCE]`  
**Date:** 2026-08-10  
**Programme row:** `FTD-0844`  
**Invalid parent:** FTD-0843, `26/28`; unsimplified matrix-equality verifier
defect; no theorem booked  
**Repair protocol:**
[`PREREG_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_CERTIFICATE_REPAIR_v2.md),
pre-run SHA-256
`B3045E5CDD0DEA22F6AFE9CA7379D1D5A458EA9A99CCCD89A1A4E2FC12B16FED`  
**Repaired certificate:**
[`proof_common_relative_local_quartic_clock_v2.py`](../../../../../scripts/proofs/proof_common_relative_local_quartic_clock_v2.py),
SHA-256
`A2AEF445E7C9260CE0A546A0041A87744007F3A747CCD907EC5DBF92578EAC41`,
`28/28 PASS`  
**Production impact:** none

## 0. Result

There is a minimal exact two-channel repair of the FTD-0842 single-field
obstruction.

Let the **common** channel propagate with the source-free production wave
tick, and let the **relative** channel carry only the local self-pair quartic
recursion. Choose the channel cross-gradient at the positive-semidefinite
rank-one boundary so that spatial stiffness acts on the common field and
vanishes exactly on the relative field.

Then:

- the common production tick preserves its exact quadratic tick invariant;
- every relative site preserves its positive quartic energy exactly;
- the sum of those sector ledgers is exactly conserved;
- the update reads at most one Moore shell;
- one excited relative site stays compactly supported at one site; and
- every fixed relative polarization reduces to the FTD-0840 scalar quartic
  carrier and has the exact continuum `G*` period factor.

This is the simplest stable recursive system found in the programme that is
simultaneously local, bounded, oriented, and energy closed at selected
reference scope.

It is not production-native. The frozen dual core propagates both left and
right channels separately, equivalent to zero cross-gradient. The required
rank-one cross-gradient is a new selection. Perfect sector decoupling also
makes the relative clock invisible to the propagating/actual channel, so
formation and readout are the next dynamical problem.

## 1. Common and relative variables

At each site let `L,R in R^3` and their momenta be `P_L,P_R`. Define the
orthogonal chart

\[
C=\frac{L+R}{\sqrt2},
\qquad
D=\frac{L-R}{\sqrt2},                           \tag{1}
\]

\[
P_C=\frac{P_L+P_R}{\sqrt2},
\qquad
P_D=\frac{P_L-P_R}{\sqrt2}.                    \tag{2}
\]

The transform preserves the kinetic norm:

\[
|P_L|^2+|P_R|^2=|P_C|^2+|P_D|^2.               \tag{3}
\]

This is a mathematical two-channel decomposition. “Left/right brain” is an
intuition for division of labor, not evidence for a biological or neural
identification.

## 2. The unique positive soft-relative boundary

For an edge difference `dL,dR`, the general swap-symmetric quadratic channel
energy is

\[
E_e=\frac a2(|dL|^2+|dR|^2)+b\,dL\cdot dR.     \tag{4}
\]

Using (1),

\[
\boxed{
E_e=\frac{a+b}{2}|dC|^2
+\frac{a-b}{2}|dD|^2.}                          \tag{5}
\]

The channel metric has eigenvalues `a+b` and `a-b`. Nonnegative edge energy
requires

\[
|b|\le a.                                       \tag{6}
\]

Exact softness of the relative channel requires

\[
a-b=0
\quad\Longleftrightarrow\quad
b=a.                                            \tag{7}
\]

At (7),

\[
E_e=a|dC|^2,                                    \tag{8}
\]

and the channel metric is

\[
a\begin{pmatrix}1&1\\1&1\end{pmatrix},        \tag{9}
\]

with eigenvalues `2a,0`. The null eigenvector is `(1,-1)`, precisely the
relative mode. Thus (7) is the unique positive-semidefinite boundary point
that retains common propagation and makes the relative mode exactly soft.

This uniqueness is conditional on the two-channel, quadratic,
swap-symmetric edge class (4). It does not derive why nature selects the
boundary value.

## 3. Common propagating ledger

Let `K=-C_WAVE^2L_18`. Evolve the common field by the source-free production
unit tick

\[
P_{C,1}=P_{C,0}-KC_0,
\qquad
C_1=C_0+P_{C,1}.                                \tag{10}
\]

FTD-0574 proves that (10) preserves

\[
H_C=\frac12\langle P_C,P_C\rangle
+\frac12\langle C,KC\rangle
-\frac12\langle P_C,KC\rangle.                 \tag{11}
\]

On a `K` eigenmode with eigenvalue `k`, the metric is

\[
G_k=\begin{pmatrix}k&-k/2\\-k/2&1\end{pmatrix},
\qquad
\det G_k=k(1-k/4).                              \tag{12}
\]

It is positive definite for `0<k<4`. The FULL production stencil has
`k<=16/9`, so every nonzero common mode is inside the positive region. The
spatial constant coordinate is the familiar zero-mode degeneracy; it does
not affect the positive relative clock energy below.

## 4. Relative local quartic ledger

At every site independently, use the FTD-0841 unit-step recursion

\[
D_{1i}-D_{0i}=\frac{P_{D,1i}+P_{D,0i}}2,        \tag{13}
\]

\[
P_{D,1i}-P_{D,0i}
=-\lambda(|D_{1i}|^2+|D_{0i}|^2)(D_{1i}+D_{0i}),
\qquad\lambda>0.                                \tag{14}
\]

Each implicit solve is onsite and globally unique for that site. It
preserves

\[
H_D=\sum_i\left(\frac12|P_{D,i}|^2
+\lambda|D_i|^4\right)                         \tag{15}
\]

exactly and has the strict FTD-0840/0841 swept-area orientation off the
origin.

Because (10) and (13)--(14) are decoupled,

\[
\boxed{H_{\rm ledger}=H_C+H_D}                  \tag{16}
\]

is exactly conserved. Equation (16) is a sum of two exact discrete sector
invariants. It is not yet proved to arise from one common discrete action or
one conventional endpoint Hamiltonian; that is part of the adoption price.

## 5. Locality and compact support

The common update reads `K C_0`, hence one production 18-point/Moore shell.
The relative update reads only `(D_i,P_{D,i})` at the same site. Transforming
between `(L,R)` and `(C,D)` is onsite. Therefore the complete selected tick
has dependency radius one Moore shell.

Prepare

\[
C=P_C=0,
\qquad
D_{i_0}=qe,
\qquad
P_{D,i_0}=pe,                                   \tag{17}
\]

with fixed unit vector `e`, and set every other relative site to zero.
Uniqueness makes the zero sites remain zero. The support of the carrier is
exactly the one-site body `{i_0}` for every tick.

The polarized sector is invariant because its force is radial. On (17),

\[
H_D=\frac{p^2}{2}+\lambda q^4.                  \tag{18}

The continuum period is therefore

\[
TA=\frac{\sqrt\pi G^*}{\sqrt{2\lambda}}.        \tag{19}

`G*` is not present in the update. It is the exact shape factor of the
continuum generator selected by (18). The finite-step discrete-gradient map
is not exact quartic Hamiltonian flow, so (19) is not an integer-tick period
or gate-cadence theorem.

## 6. Why this evades FTD-0842

FTD-0842 placed positive spatial stiffness and onsite quarticity on the same
field. A bounded profile then necessarily paid quadratic gradient energy.

FTD-0844 separates the responsibilities:

```text
common mode C:    spatial propagation + production tick invariant
relative mode D:  local quartic recurrence + positive onsite energy
```

The relative body is compact because it has no edge stiffness, while total
energy remains nonnegative because the channel metric is rank-one positive
semidefinite rather than sign-indefinite. No global simultaneous solve is
needed; the only implicit roots are independent onsite solves.

This is exactly the structural content of the “two sides” intuition: not two
copies doing the same thing, but two complementary modes doing different
jobs.

## 7. Production and operational boundaries

The frozen dual production path computes separate `lap_L` and `lap_R` and
updates both wave channels. In the class (4), this is `b=0`, so common and
relative modes have equal spatial stiffness. Production has neither the
rank-one value `b=a` nor the relative onsite quartic.

The selected construction also contains an operational tension:

- exact decoupling protects the relative energy and one-site support;
- the same decoupling prevents the common field, state field, matter, or an
  observer from reading the relative phase.

A physical clock needs a local readout interaction. Such an interaction must
exchange information and generally energy. It must be designed so that:

1. total energy/work remains closed;
2. support and orientation survive for a preregistered horizon;
3. the interaction is context blind and P4-local;
4. no target period, `G*`, outcome, or Born weight is read; and
5. turning the readout off recovers (10)--(16) exactly.

Formation is separate: nothing here makes a nonzero relative clock arise
from a generic production state. Maintenance is unnecessary for the isolated
selected carrier but may become necessary once readout/backreaction is added.

## 8. Certificate record

The original FTD-0843 certificate returned `26/28` because SymPy structural
matrix equality did not simplify two algebraically identical matrices. The
parent is preserved as invalid. FTD-0844 changed only C14 and dependent C28
to exact entrywise simplified-difference comparisons. It returned:

```text
FTD-0843 common-relative local quartic clock: 28/28 PASS
RANK_ONE_COMMON_PROPAGATION_LEAVES_EXACT_LOCAL_RELATIVE_SOFT_MODE
DECOUPLED_COMMON_TICK_AND_RELATIVE_QUARTIC_ENERGIES_CLOSE_EXACTLY
SELECTED_TWO_CHANNEL_CARRIER_IS_POSITIVE_AND_P4_LOCAL
PRODUCTION_CROSS_GRADIENT_FORMATION_READOUT_AND_FINITE_TICK_CADENCE_OPEN
FTD-0844 CERTIFICATE_REPAIR_ONLY_C14_C28_EXACT_SIMPLIFIED_DIFFERENCE
```

## 9. Non-claims and next gate

FTD-0844 establishes an exact selected carrier architecture. It does not
derive the cross-gradient, `lambda`, biological hemispheres, a production
toggle, a single common action, spontaneous formation, physical readout,
Born frequencies, actualization, or finite-tick `G*` synchronization.

The next narrow discriminator is **readout without destruction**: add the
lowest-degree P4-local common--relative exchange, book its energy current, and
test whether it can expose the relative phase while retaining bounded support
and orientation without target coding.

