# FTD-0840 — Native pair-energy recursion and cadence boundary v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 24/24]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked Hamiltonian, discrete-gradient, orientation,
stability, and production-boundary certificate  
**Production impact:** none

## 1. Registered question

Can the already identified real canonical lift `(q,p)` support the simplest
stable recursive realization of the FTD-0839 square-field mechanism without
inserting `G*` or reading a target period?

More precisely, adopt the signed self-pair

\[
u=q|q|
\]

and put an ordinary positive quadratic energy on it. Does this give

1. an exact self-dual energy in paired coordinates;
2. a deterministic, reversible, energy-closed discrete recursion;
3. an orientation that survives the square quotient because the unsquared
   lift remains available;
4. the continuous critical-quartic `G*` period law; and
5. an exact finite-tick `G*` cadence in the production substrate?

## 2. Epistemic firewall

This is an exact symbolic discriminator. It performs no numerical period
search, parameter fit, formula substitution, near-miss comparison, or
post-execution choice. The positive constants `m` and `lambda`, the self-pair
map, and the discrete-gradient update are registered model inputs. Their
mathematical consequences may be theorem-grade; their physical adoption is
not thereby derived from P1--P5.

The certificate must keep four statements separate:

- the production free-field modal chart already provides a target-blind
  canonical pair;
- the pair-energy coupling is not present in the frozen production sources;
- the proposed recursion is exactly conservative and stable once adopted;
- the continuum generator has the `G*` shape factor, while the finite-step
  recursion is not the exact Hamiltonian time-`h` flow and therefore does not
  by itself establish an integer `G*` gate cadence.

No successful check licenses a Born rule, actualization selector, localized
clock, maintained nonzero shell, or production integration.

## 3. Frozen source inputs

The certificate must fail closed unless these SHA-256 values match:

| Input | SHA-256 |
|---|---|
| `engine/include/ftd/eft/native_modal_phase_action.h` | `C1E9D5C1944E66D7601D193DC77A39980EBA24B84A41F7D752A3A363910060B6` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md` | `07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/DERIV_QUARTIC_CLOCK_CM_GEARBOX_v1.md` | `1B969544B065D576523235F40A20918C22E0C55978E52282E2FC623385BC2FDF` |

## 4. Frozen mathematics

### 4.1 Self-pair energy

For `m>0` and `lambda>0`, define

\[
H(q,p)=\frac{p^2}{2m}+\lambda q^4,
\qquad
u=q|q|,
\qquad
y=\frac{p}{\sqrt{2m\lambda}}.
\]

Then

\[
\boxed{H=\lambda(u^2+y^2)}.
\]

The energy is quadratic and exchange-symmetric in the derived pair `(u,y)`,
while the retained unsquared coordinate `q` records which sheet the square
alone would forget. Hamilton's equations are

\[
\dot q=\frac pm,
\qquad
\dot p=-4\lambda q^3.
\]

Away from `q=0`, with `r=|q|`, they induce

\[
\frac d{dt}\begin{pmatrix}u\\y\end{pmatrix}
=2r\sqrt{\frac{2\lambda}{m}}
\begin{pmatrix}0&1\\-1&0\end{pmatrix}
\begin{pmatrix}u\\y\end{pmatrix}.
\]

Thus the pair radius is conserved and the forward orientation is fixed. The
rate slows to zero at the sheet crossing; that nonuniform cadence is the
source of the lemniscatic period.

### 4.2 Continuous period

For amplitude `A>0`, the quarter-cycle quadrature is

\[
\frac T4
=\sqrt{\frac{m}{2\lambda}}\frac1A
\int_0^1\frac{dx}{\sqrt{1-x^4}}
=\sqrt{\frac{m}{2\lambda}}\frac1{4A}
B\!\left(\frac14,\frac12\right).
\]

Since

\[
B\!\left(\frac14,\frac12\right)
=\sqrt\pi\frac{\Gamma(1/4)}{\Gamma(3/4)}
=\sqrt\pi G^*,
\]

the registered continuum law is

\[
\boxed{TA=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}}.
\]

### 4.3 Exact discrete recursion

For fixed `h>0`, define `(q_1,p_1)` implicitly from `(q_0,p_0)` by

\[
\frac{q_1-q_0}{h}=\frac{p_1+p_0}{2m},                    \tag{1}
\]

\[
\frac{p_1-p_0}{h}
=-\lambda\left(q_1^3+q_1^2q_0+q_1q_0^2+q_0^3\right).    \tag{2}
\]

The quartic divided difference in (2) is frozen; it may not be replaced after
execution. Eliminating `p_1` yields a cubic `f(q_1)=0` whose derivative is

\[
f'(q_1)=\frac{2m}{h}
+h\lambda(3q_1^2+2q_1q_0+q_0^2)>0.
\]

The polynomial has opposite limits at `+/- infinity`, so the next state is
globally unique. The step is deterministic despite being implicit.

The exact energy difference factors as

\[
H_1-H_0
=\frac{(p_1-p_0)(p_1+p_0)}{2m}
+\lambda(q_1-q_0)
(q_1^3+q_1^2q_0+q_1q_0^2+q_0^3)=0.
\]

The method is self-adjoint under endpoint exchange and `h -> -h`, and its
physical time reverse is obtained by `p -> -p` and reversing the endpoints.

### 4.4 Discrete orientation and stability

With midpoint variables `qbar=(q_1+q_0)/2` and
`pbar=(p_1+p_0)/2`, freeze the swept-area witness

\[
\chi_h
=\bar q(p_1-p_0)-\bar p(q_1-q_0).
\]

Equations (1)--(2) imply

\[
\chi_h=-h\left[
\frac\lambda2(q_1+q_0)^2(q_1^2+q_0^2)
+\frac{(p_1+p_0)^2}{4m}
\right].
\]

For `h,m,lambda>0`, this is strictly negative on every nonzero step. Equality
would force `q_1=-q_0` and `p_1=-p_0`; equations (1)--(2) then force the
origin. Exact energy conservation confines every positive-energy orbit to a
compact shell, proving bounded recurrence and Lyapunov stability of the
origin/energy sublevel sets. It does not establish pointwise Lyapunov
stability of every positive-energy phase point or create asymptotic
attraction to one preferred shell.

### 4.5 Finite-tick control

At the turning point `(q_0,p_0)=(A,0)`, the registered method has the exact
series

\[
q_1=A-\frac{2\lambda A^3}{m}h^2
+\frac{6\lambda^2A^5}{m^2}h^4+O(h^6),
\]

\[
p_1=-4\lambda A^3h
+\frac{12\lambda^2A^5}{m}h^3+O(h^5).
\]

The exact Hamiltonian flow instead has coefficients `2` and `8` in the
corresponding `h^4` and `h^3` terms. Therefore the recursion is not the exact
time-`h` flow at finite `h`. The continuum `G*` period law is not an exact
integer-tick cadence theorem for this map.

## 5. Frozen exact checks

The implementation must run exactly 24 checks:

1. all six frozen source hashes;
2. production contains the registered target-blind canonical modal pair;
3. the frozen production energy ledger contains only its registered
   quadratic field/wave contribution and no pair-energy channel;
4. `u^2=q^4`;
5. `H=lambda(u^2+y^2)`;
6. Hamilton's equations and cubic force;
7. the induced oriented pair flow;
8. conservation of the pair radius;
9. strict sign of the continuous swept-area current away from zero;
10. quartic quarter-period reduction to `B(1/4,1/2)/4`;
11. beta/Gamma identity `B(1/4,1/2)=sqrt(pi)G*`;
12. the full continuum period-amplitude law;
13. quartic divided-difference factorization;
14. positive-definite identity for the scalar-root derivative;
15. global uniqueness of the implicit next state;
16. positive Jacobian determinant;
17. exact discrete energy conservation;
18. self-adjoint endpoint/step reversal;
19. physical momentum time reversal;
20. exact discrete swept-area factorization;
21. strict discrete orientation off the origin;
22. origin as the only fixed point and compact-shell bounds;
23. correct continuum generator but exact finite-step mismatch;
24. the combined discriminator: exact stable recursion and continuum `G*`
    shape pass; production pair coupling and exact finite-tick `G*` cadence
    remain absent/open.

No check may be removed, reinterpreted, or tolerance-relaxed after execution.

## 6. Locked implementation

```text
scripts/proofs/proof_native_pair_energy_recursion.py
```

Script SHA-256:
`6300D46B59D505165CCDBC1FF3F634FA213327A6E378F9595E58EF79D4A922F8`

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before the first execution. Run exactly:

```text
python scripts/proofs/proof_native_pair_energy_recursion.py
```

## 7. Outcomes

- **Outcome A — exact native finite-tick gearbox:** the production source
  already contains the pair coupling and the registered finite-step map is
  the exact quartic Hamiltonian flow with a derived `G*` tick cadence.
- **Outcome B — exact recursive bridge, cadence still open:** all 24 checks
  pass. The adopted pair energy supplies a unique, reversible,
  energy-conserving, oriented, stable recursion. Its continuum generator has
  the exact `G*` period factor. The coupling is absent from production and the
  finite-step map is not the exact flow, so physical adoption, localization,
  maintenance, and tick cadence remain open.
- **Outcome C — invalid:** any exact or source-hash check fails without
  establishing Outcome A. Book no theorem and repair under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run
and does not weaken any gate.

## 8. Recorded outcome

The first locked execution returned `24/24 PASS`. Registered Outcome B is
selected:

```text
SIGNED_SELF_PAIR_GIVES_QUADRATIC_SELF_DUAL_ENERGY
DISCRETE_RECURSION_UNIQUE_REVERSIBLE_ENERGY_CLOSED_AND_ORIENTED
CONTINUUM_GSTAR_SHAPE_FACTOR_EXACT
PRODUCTION_PAIR_COUPLING_AND_FINITE_TICK_GSTAR_CADENCE_OPEN
```

The exact recursion closes its own energy without a bath and retains one
orientation on every nonzero step. The production pair coupling, localized
physical realization, dissipative maintenance, and exact global-tick cadence
remain open. See
[`THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md).

## 9. Post-certificate isolated implementation

After Outcome B was booked, the unchanged recursion was implemented as the
header-only selected reference
`engine/include/ftd/eft/native_pair_energy_recursion.h` with focused test
`engine/tests/test_native_pair_energy_recursion.cpp`. Their SHA-256 values are
`81B4941B...371E5A` and `F0D2BFD7...142F7`. Release CTest
`native_pair_energy_recursion` passes `1/1`, including 20,000-step energy-shell
confinement, both reversal laws, strict nonzero orientation, and fail-closed
solver controls. This implementation was not part of the pre-run theorem lock
and does not alter its outcome or promote the coupling to production-native.
