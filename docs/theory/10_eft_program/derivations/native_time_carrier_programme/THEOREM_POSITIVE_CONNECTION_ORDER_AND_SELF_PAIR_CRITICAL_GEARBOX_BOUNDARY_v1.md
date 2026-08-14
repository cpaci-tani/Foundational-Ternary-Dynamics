# Theorem — Positive connection order and self-pair critical-gearbox boundary v1

**Identifiers:** `FTD-0902` parent; `FTD-0903` repaired proof of record  
**Date:** 2026-08-11  
**Status:** `[THEOREM — POSITIVE LINEARIZED-CONNECTION CLOCK-HESSIAN OBSTRUCTION]` +
`[THEOREM — EXACT REST-SECTOR SIGNED-SELF-PAIR QUARTIC GEARBOX]` +
`[BOUNDARY — C1/NOT-C2 ORIGIN AND MOVING-SECTOR QUADRATIC TERM]` +
`[BOUNDARY — ZERO SYMMETRIC-CYCLE NET TRANSPORT]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]`

## 1. General positive-connection obstruction

Let `C` be cyclic, let `D` be a clock coordinate, let `M` be positive
definite, and adopt the positive connection Hamiltonian

\[
H(C,D;P,\Pi)=\frac12(P-A(D))^TM^{-1}(P-A(D))
 +\frac{|\Pi|^2}{2m}+V(D),                              \tag{1}
\]

where `A(0)=0`. Suppose the uncoupled clock is critical at the origin,

\[
\nabla V(0)=0,
\qquad \nabla^2V(0)=0.                                  \tag{2}
\]

Writing `B=DA(0)`, the rest-sector clock Hessian is

\[
\boxed{\nabla_D^2H(0)=B^TM^{-1}B.}                      \tag{3}
\]

The right side is a positive Gram matrix. If `R^TR=M^{-1}`, then

\[
x^TB^TM^{-1}Bx=|RBx|^2\ge 0.                            \tag{4}
\]

Because `R` is invertible, (3) vanishes if and only if `B=0`. Therefore a
nonzero linearized positive connection cannot preserve an exact zero-Hessian
critical clock. At nonzero canonical common momentum,

\[
\nabla_DH(0)=-B^TM^{-1}P,                               \tag{5}
\]

so the same linearized connection also generically tilts the moving clock.
This strictly generalizes the FTD-0901 scalar result `gamma^2/M`.

This is an order obstruction, not a no-go for every connection. It permits a
connection with `DA(0)=0`, a context-blind clutch, a separate clock, or a
declared constrained system.

## 2. Registered signed-self-pair escape

Use the existing signed radial self-pair coordinate

\[
U(D)=|D|D.                                                \tag{6}
\]

It is odd, homogeneous of degree two, and orthogonally equivariant. It is
`C^1`, with `DU(0)=0`, but it is not `C^2` at the origin. For `D=rn`,
`r>0`,

\[
DU(D)=r(I+nn^T),                                         \tag{7}
\]

whose radial and tangential eigenvalues are `2r` and `r`. Thus the mixed
connection curvature of `A(D)\cdot dC` vanishes at the clock origin but is
nonzero away from it when `gamma != 0`.

Adopt the **[IMPOSED reference coupling law]**

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma U(D)\cdot\dot C-\lambda|D|^4,
\qquad M,m,\lambda>0.                                   \tag{8}
\]

Its momenta and positive Hamiltonian are

\[
P=M\dot C+\gamma U(D),
\qquad \Pi=m\dot D,                                     \tag{9}
\]

\[
H=\frac{|P-\gamma U(D)|^2}{2M}
  +\frac{|\Pi|^2}{2m}+\lambda|D|^4.                    \tag{10}
\]

The mechanical common momentum

\[
K=P-\gamma U(D)                                         \tag{11}
\]

obeys the exact endpoint gearbox identity

\[
\boxed{\Delta K=-\gamma\Delta U.}                       \tag{12}
\]

Equation (12) exchanges mechanical common impulse with the signed self-pair
while the canonical Noether momentum `P` remains fixed.

The certificate establishes (6) as the first registered quadratic
self-pair escape already available in the FTD corpus. It does **not** prove
uniqueness among every `C^1` non-polynomial or higher-order connection.

## 3. Exact rest-sector critical clock

In the canonical rest sector `P=0`, use `|U(D)|^2=|D|^4` and define

\[
\Lambda=\lambda+\frac{\gamma^2}{2M}>0.                 \tag{13}
\]

Then (10) reduces exactly to

\[
\boxed{H_{P=0}=\frac{|\Pi|^2}{2m}+\Lambda|D|^4.}       \tag{14}
\]

The connection adds no quadratic clock Hessian; it renormalizes the quartic
coefficient. Every linearly polarized invariant orbit therefore retains

\[
\boxed{TA=\sqrt\pi\,G^*\sqrt{\frac{m}{2\Lambda}}.}      \tag{15}
\]

Thus `G*` survives as the exact continuum quartic period factor. Its
dimensional scale changes through `Lambda`. Neither the continuum identity
nor the finite endpoint map derives an integer-tick `G*` cadence.

## 4. Moving and transport boundaries

For fixed nonzero `P` and `D=rn`, (10) contains

\[
-\frac{\gamma}{M}r^2P\cdot n.                           \tag{16}
\]

Generic moving sectors therefore acquire a direction-dependent quadratic
ray term. The exact quartic reduction (14) is a rest-sector theorem, not a
moving-clock theorem.

For a linearly polarized symmetric rest orbit,

\[
D(t+T/2)=-D(t),
\qquad U(t+T/2)=-U(t).                                  \tag{17}
\]

Since `M dot(C)=-gamma U` at `P=0`,

\[
\Delta C_T=-\frac\gamma M\int_0^T U(t)\,dt=0.          \tag{18}
\]

The registered mechanism is therefore a reversible oscillatory
energy/impulse gearbox. It does not derive net propulsion, matter
translation, absolute inertia, or a constituent.

## 5. Orientation and gamma

The complex structure `J^2=-I` and the two signs of the odd self-pair provide
orientation data. They do not fix the real positive magnitude `|gamma|`.

If one separately **selects** equal bare and connection quartic energies,

\[
\frac{\gamma^2}{2M}=\lambda,                            \tag{19}
\]

then

\[
|\gamma|=\sqrt{2M\lambda},
\qquad \Lambda=2\lambda.                               \tag{20}
\]

Equation (20) is conditional algebra. Condition (19) is not adopted here and
is not derived from `i`, discreteness, self-duality, or substrate dynamics.

```text
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
SELF_DUAL_EQUAL_PARTITION=NOT_ADOPTED
```

## 6. Exact discrete reference witness

The linearly polarized implementation composes the existing exact quartic
discrete-gradient recursion with coupling `Lambda`. If
`u_j=D_j|D_j|`, its common endpoint update is

\[
C_1-C_0=-\frac{h\gamma}{2M}(u_1+u_0).                  \tag{21}
\]

The repaired exact certificate and isolated C++ witness establish:

- exact rest-sector quartic energy conservation;
- exact mechanical impulse exchange (12);
- exact equal-and-opposite canonical channel impulses;
- exact reciprocal-carry composition after imposed `p_*` is supplied;
- signed-step endpoint reversal, including (21);
- orthogonal/cubic covariance of the vector law;
- the zero origin Jacobian and zero clock Hessian;
- the moving-sector coefficient (16); and
- the continuum beta/gamma identity (15).

The discrete map is not claimed to be exact continuum flow. It neither reads
`G*` nor encodes a target period.

## 7. Epistemic accounting

Theorem-grade conditional on the imposed connection law:

- the positive Gram obstruction (3)--(5);
- the `C^1`/not-`C^2` signed-self-pair geometry (6)--(7);
- the Legendre transform and exact positive rest-sector fold (10)--(14);
- mechanical/common and canonical/channel impulse conservation;
- reciprocal carry and signed-step reversal conditional on supplied `p_*`;
- the continuum `G*` factor at the renormalized quartic scale; and
- the moving-sector and zero-net-cycle boundaries (16)--(18).

Still open:

- substrate derivation and normalization of `A(D)=gamma|D|D`;
- the value of `gamma` and whether any self-dual balance is physical;
- physical identification of the common/relative coordinates and channels;
- `p_*`, the complete total-momentum map, carry energy, and absolute mass;
- a rectifier or phase clutch if net common transport is required;
- a moving clock that avoids the quadratic term (16);
- integer-tick `G*` cadence and preferred-tick hiding;
- production integration and stable constituent formation; and
- Born, Bell-laboratory, and operational Lorentz recovery.

No new selected type, adoption currency, target-coded weight, fitted
coupling, or production integration is added.

## 8. Certificate and repair provenance

The frozen FTD-0902 protocol SHA-256 is
`568F98C7AF01FC48DEAFEDC773FF33A129D089AFC606511C2D3C9F1C45D37061`.
Its frozen parent certificate SHA-256 is
`C56907311B93942ABD7CD3DA96882CDC811EA333526C11C30F9C7BE004EB107C`.

The first immutable execution passed `80/81`. C32 alone failed because a
simultaneous SymPy dictionary substitution did not expose `u^2` after
setting `P=0`. No theorem was booked from that invalid run.

FTD-0903 froze exactly one in-memory representation repair:

- repair protocol SHA-256
  `3C7E31BF8160EDCC8D8721EA0021A051AD5F5C502010FB1014D8DB1FBC03AFE7`;
- repair wrapper SHA-256
  `9F6BABDC7F8450A16834A8CA76200BB9C0F526CD9E1DA9B9E99509FF352CFA43`.

The wrapper changed only C32 from simultaneous to sequential substitution of
the same registered values. Its integrity gates passed and the inherited
certificate passed `81/81`. FTD-0903 is the repaired proof of record.

## 9. Isolated reference implementation

The fail-closed, linearly polarized witness is isolated under `ftd::eft`:

- `engine/include/ftd/eft/self_pair_connection_critical_gearbox.h`, SHA-256
  `038F48F4E99D3CD55CAE25CF09170670733057FF1A43279839D3C78B0DC74447`;
- `engine/src/eft/self_pair_connection_critical_gearbox.cpp`, SHA-256
  `D7502AE50056A1A4D18E8335750B861C98A44C4589F4736D5037374AC21E4B91`;
- `engine/tests/test_self_pair_connection_critical_gearbox.cpp`, SHA-256
  `F715C76FAD397993B9E7A6650FE9D99C85A0DB946E63F1DCEFAE5E3D183F6DB3`.

The pinned MSVC 14.44 build succeeds, the focused CTest passes `1/1`, and the
isolated actualization/EFT chain passes `28/28`. The implementation changes no
production `Voxel`, renderer, boundary, default toggle, or tick phase.

## 10. Next acceptance gate

Pre-register a context-blind rectification boundary. The test must decide
whether any locally readable, phase-dependent clutch can produce nonzero
cycle-averaged common transport while:

1. retaining exact signed-step reversibility on the enlarged controller
   state;
2. booking switching work and controller/backreaction energy;
3. preserving the rest clock within declared compliance tolerances;
4. remaining blind to `G*`, measurement context, outcome, and Born weights;
   and
5. not silently imposing `gamma`, `p_*`, mass, or a preferred direction.

Failure of a reversible rectifier favors a separate-clock architecture: keep
the critical `G*` oscillator as a local eligibility clock and couple it to a
distinct momentum gearbox through an explicitly paid controller.

```text
POSITIVE_LINEARIZED_CONNECTION_CLOCK_HESSIAN=B_TRANSPOSE_M_INVERSE_B
NONZERO_LINEARIZED_CONNECTION_PRESERVES_CRITICAL_QUARTIC=FALSE
SIGNED_SELF_PAIR_CONNECTION=IMPOSED_REFERENCE_LAW
SIGNED_SELF_PAIR_CONNECTION_REGULARITY=C1_NOT_C2_AT_ORIGIN
REST_SECTOR_CRITICAL_QUARTIC=EXACT
REST_SECTOR_CONTINUUM_GSTAR_FACTOR=EXACT
MECHANICAL_COMMON_IMPULSE=EXACTLY_EXCHANGED_WITH_SIGNED_SELF_PAIR
FULL_CYCLE_REST_SECTOR_NET_COMMON_DRIFT=ZERO_FOR_POLARIZED_SYMMETRIC_ORBIT
MOVING_SECTOR_EXACT_QUARTIC=FALSE_GENERICALLY
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
SELF_DUAL_EQUAL_PARTITION=NOT_ADOPTED
PHYSICAL_MOMENTUM_SCALE=OPEN
ABSOLUTE_MASS=NOT_DERIVED
INTEGER_TICK_GSTAR_CADENCE=OPEN
PRODUCTION_INTEGRATION=FORBIDDEN
NO_NEW_SELECTED_TYPE=TRUE
BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED
```
