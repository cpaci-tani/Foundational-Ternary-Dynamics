# FTD-0902 — positive connection order and self-pair critical-clock gearbox boundary v1

**Identifier:** `FTD-0902`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Production status:** unchanged

## 1. Question

Is the FTD-0901 critical-clock detuning specific to the linear connection
`A(D)=gamma D`, or is it forced by every positive smooth cyclic connection
with nonzero linearized curvature? What is the lowest-order positive
connection that can retain an exact rest-sector quartic clock while still
exchanging mechanical common impulse?

The registered candidate uses the existing signed self-pair coordinate

\[
U(D)=|D|D,
\qquad |U(D)|^2=|D|^4,                                   \tag{1}
\]

and the imposed connection

\[
A(D)=\gamma U(D).                                        \tag{2}
\]

This is an **[IMPOSED reference coupling law]** on existing selected
common/relative variables. It is not a production interaction, a new selected
type, or a derivation of `gamma`.

## 2. Frozen sources

| source | SHA256 |
|---|---|
| `DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md` | `07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496` |
| `THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `THEOREM_QUARTIC_RELATIVE_IMPULSE_RECIPROCAL_CARRY_GEARBOX_BOUNDARY_v1.md` | `E044129DB0E28DCCE3723D77027E5A652EC7A668C0DD73AD17C77E74FA7F4F6C` |
| `THEOREM_COMMON_RELATIVE_CONNECTION_AND_MOMENTUM_GEARBOX_BOUNDARY_v1.md` | `3E2895157741C19DC8603E92E31A71933BFDAAF5B35062DFCE2F92404F8B9542` |
| `native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |

Any source-hash mismatch invalidates the certificate.

## 3. General positive-connection obstruction

Let `C in R^r` be cyclic, `D in R^s` the clock coordinate, `M` a positive
definite common kinetic metric, and `A(D)` a `C^1` connection with `A(0)=0`.
At fixed canonical common momentum `P`, the positive connection Hamiltonian is

\[
H=\frac12(P-A(D))^T M^{-1}(P-A(D))
  +\frac{|\Pi|^2}{2m}+V(D).                              \tag{3}
\]

Assume the uncoupled clock is critical at the origin:

\[
\nabla V(0)=0,
\qquad \nabla^2V(0)=0.                                   \tag{4}
\]

Writing `B=DA(0)`, the certificate must prove at `P=0`

\[
\nabla^2_D H(0)=B^T M^{-1}B.                             \tag{5}
\]

Because `M^{-1}` is positive definite, (5) is positive semidefinite and
vanishes if and only if `B=0`. Hence every nonzero **linearized** connection
curvature detunes the critical clock. This is the general form of the
FTD-0901 `gamma^2/M` result.

At nonzero `P`, the origin has gradient

\[
\nabla_D H(0)=-B^T M^{-1}P,                              \tag{6}
\]

so a linear connection also generically tilts a moving clock.

The theorem is local and order-sensitive. It does not forbid a connection
whose first derivative vanishes at the clock origin, a pulsed clutch, a
separate clock, a constrained gauge system, or a declared non-positive
effective description.

## 4. Lowest-order self-pair escape

The radial signed self-pair (1) is odd, homogeneous of degree two, and
equivariant under every orthogonal transformation, hence under the cubic
group. It is `C^1` with

\[
DU(0)=0.                                                  \tag{7}
\]

For `D=r n`, `r>0`, `|n|=1`,

\[
DU(D)=r(I+n n^T),                                        \tag{8}
\]

with radial eigenvalue `2r` and tangential eigenvalue `r`. Thus the
connection curvature vanishes at the origin but is nonzero away from it for
`gamma != 0`. The price is explicit: `U` is `C^1` but not `C^2` at `D=0`.

Adopt the reference Lagrangian

\[
L=\frac M2|\dot C|^2+\frac m2|\dot D|^2
  +\gamma U(D)\cdot\dot C-\lambda|D|^4,
\qquad M,m,\lambda>0.                                    \tag{9}
\]

Its momenta and Hamiltonian are

\[
P=M\dot C+\gamma U(D),
\qquad \Pi=m\dot D,                                     \tag{10}
\]

\[
H=\frac{|P-\gamma U(D)|^2}{2M}
  +\frac{|\Pi|^2}{2m}+\lambda|D|^4.                     \tag{11}
\]

The mechanical common momentum is

\[
K=P-\gamma U(D),
\qquad \Delta K=-\gamma\Delta U.                        \tag{12}
\]

In the exact rest sector `P=0`, define

\[
\Lambda=\lambda+\frac{\gamma^2}{2M}>0.                  \tag{13}
\]

Then

\[
H_{P=0}=\frac{|\Pi|^2}{2m}+\Lambda|D|^4.                \tag{14}
\]

Equation (14) is an exact positive critical quartic, not merely a small-
amplitude approximation. Every linearly polarized invariant sector therefore
has

\[
T A=\sqrt\pi\,G^*\sqrt{\frac{m}{2\Lambda}}.             \tag{15}
\]

The factor `G*` survives; the dimensional period scale is renormalized by the
connection energy.

## 5. Gamma and self-dual balance

The identity `J^2=-I` and the oddness of `U` fix orientation data, not the
positive magnitude `|gamma|`. The certificate must retain

```text
I_SUPPLIES_ORIENTATION=TRUE
GAMMA_MAGNITUDE_DERIVED_FROM_I=FALSE
```

One possible extra condition is equal self-dual energy partition,

\[
\frac{\gamma^2}{2M}=\lambda.                             \tag{16}
\]

Conditional on **selecting** (16),

\[
|\gamma|=\sqrt{2M\lambda},
\qquad \Lambda=2\lambda.                                \tag{17}
\]

Equation (17) is a theorem given the selected equality (16). It is not a
derivation from `i`, discreteness, or the current substrate. No self-dual
balance is adopted by this protocol and no selection currency changes.

## 6. Moving-sector and transport boundaries

For nonzero fixed `P` and `D=r n`, the connection contribution contains

\[
-\frac{\gamma}{M}r^2 P\cdot n.                           \tag{18}
\]

Thus the exact quartic reduction is rest-sector only. Generic moving sectors
acquire a direction-dependent quadratic term, even though `DU(0)=0`.

For a linearly polarized rest-sector orbit,

\[
D(t+T/2)=-D(t),
\qquad U(t+T/2)=-U(t).                                   \tag{19}
\]

Hence the common drift integrates to zero over a full cycle at `P=0`:

\[
\Delta C_T=-\frac\gamma M\int_0^T U(t)dt=0.             \tag{20}
\]

The self-pair connection is an exact oscillatory impulse/energy gearbox, not
yet net propulsion, matter translation, absolute inertia, or a physical
total-momentum map.

## 7. Registered discrete rest-sector witness

At `P=0`, reuse the exact vector/scalar quartic discrete-gradient step with
coupling `Lambda` from (13). For the scalar polarized certificate, define
`u_j=D_j|D_j|` and add the symmetric common update

\[
C_1-C_0=-\frac{h\gamma}{2M}(u_1+u_0).                   \tag{21}
\]

The frozen certificate must prove:

- exact relative/common Hamiltonian conservation through (14);
- exact mechanical endpoint identity (12);
- exact equal-and-opposite canonical channel impulses and FTD-0897 carry
  composition after imposed `p_*` is supplied;
- exact endpoint-exchange/`h -> -h` reversal of the augmented map;
- cubic/orthogonal covariance of `U` and the vector law;
- exact continuum identity (15);
- the linearized-connection no-go (5) and the self-pair evasion (7)--(14);
- the moving-sector and zero-net-cycle boundaries (18)--(20); and
- all gamma, scale, mass, production, Born, and cadence firewalls.

The finite discrete map is not exact continuum flow. `G*` is not read by the
map, and no integer-tick recurrence is inferred from (15).

## 8. Outcome map

- **Outcome A:** the general positive linearized-connection obstruction and
  the rest-sector signed-self-pair escape both close exactly. Book the
  self-pair connection as an imposed reference gearbox; retain gamma,
  self-dual balance, moving-sector clock, net transport, scale, mass,
  production, and finite-tick cadence as open.
- **Outcome B:** a frozen source already derives (2), gamma, or the balance
  (16) from production substrate dynamics. Identify it explicitly before any
  promotion.
- **Outcome C:** any positivity, order, self-pair, conservation, reversal,
  covariance, period, or scope gate fails. Book no theorem.
- **Execution invalid:** any source hash, protocol hash, or terminal firewall
  fails.

## 9. Post-certificate implementation

Only after a passing locked certificate, add an isolated `ftd::eft`
rest-sector analyzer. It must compose the existing quartic recursion using
`Lambda`, derive `U`, common displacement, mechanical impulse, channel
increment, and reciprocal carry, and audit reversal, connection curvature,
zero clock Hessian, continuum period factor, and the moving-sector boundary.
It must fail closed on nonfinite input, nonpositive masses/coupling/tolerance
or scale, zero step, child-recursion/carry failure, chart overflow, endpoint
mismatch, or inverse failure.

## 10. Next acceptance gate

If Outcome A holds, decide between two sharply distinct physical programmes:

1. derive the signed self-pair connection and its normalization in substrate
   variables, then add a distinct rectifier/phase clutch if net common
   transport is required; or
2. keep the critical `G*` clock separate from a linear connection gearbox and
   close the autonomous controller/backreaction ledger between them.

Neither route may read `G*`, context, outcome, or a Born weight. Both must
recover the same physical total-momentum map and inertia before production.

## 11. Scope firewall

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

The exact SHA256 of this protocol and its certificate must be entered in the
preregistration manifest before first execution.
