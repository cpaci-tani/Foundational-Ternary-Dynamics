# FTD-0845 — Swap-parity phase readout and odd-pointer minimum v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 31/32]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked discriminator for the first local readout of the
FTD-0844 relative quartic carrier  
**Production impact:** none

## 1. Registered question

Can the relative critical-quartic clock be read by a local conservative
pointer without:

1. adding quadratic stiffness to the clock coordinate;
2. discarding the sign/sheet information lost by a symmetric square;
3. importing `G*`, a target period, an outcome, or a Born weight; or
4. hiding the readout work and backreaction?

The discriminator separates three claims:

- what a common/even pointer can observe;
- why a positive bilinear faithful pointer is incompatible with exact
  criticality; and
- the lowest-degree positive polynomial interaction that preserves the
  critical zero Hessian while exposing the signed relative coordinate to a
  covariant odd pointer.

## 2. Epistemic firewall

The exchange parity, polynomial class, continuous pointer type, masses, and
positive couplings are registered mathematical assumptions. An odd pointer
is a **new `[SELECTED reference type]`**, not something derived from the
production ternary state. The fact that the actual alphabet already contains
`+1` and `-1` does not supply the missing continuous-to-ternary record map.

Passing may establish an exact local reference transaction and a scoped
minimum-degree theorem. It cannot establish a production interaction, a
manifested record, irreversible memory, autonomous clock formation,
maintenance, a finite-tick `G*` cadence, Born frequencies, or actualization.
No numerical search, fitting, near-miss comparison, or target substitution is
permitted.

## 3. Frozen source inputs

| Input | SHA-256 |
|---|---|
| `THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md` | `64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0` |
| `THEOREM_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md` | `07BDB4CA22A655C378BCC4BA4B6A69830686200A4B4F59B19136363F5F4F6496` |
| `THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md` | `C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93` |
| `SPEC_SUBSTRATE_NATIVE_CLOCK_MINIMUM_v1.md` | `E5E21BCB0D9F16825ED4FEEE9B915E2835F16F9446F0D636C801A4316CB0D0C5` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The first four paths are relative to
`docs/theory/10_eft_program/derivations/native_time_carrier_programme/`,
except the minimum-clock specification, which is relative to
`docs/theory/10_eft_program/native_time_carrier_programme/`.

## 4. Frozen mathematics

### 4.1 Exchange parity and the common/even quotient

On one invariant polarized FTD-0844 site, write the relative canonical state
as `(q,p)`. Channel exchange acts by

\[
 S:(q,p)\mapsto(-q,-p).                         \tag{1}
\]

A common pointer `a` is exchange even. Any analytic position interaction
obeying

\[
 V_+(a,q)=V_+(a,-q)                            \tag{2}
\]

contains only even powers of `q`; its common force
`-partial_a V_+` is also even. It can therefore distinguish phase only on
the quotient `(q,p)~(-q,-p)`, not on the faithful full cycle.

The natural positive square-pointer control is

\[
 W_+(a,q)=\frac\kappa2(a-q^2)^2,
 \qquad \kappa>0.                              \tag{3}
\]

It has no `q^2` term at the origin and its pointer force at `a=0` is
`kappa q^2`. It reads the symmetric-square coordinate exactly and is blind to
`q -> -q`. This is a readout of the BCC-like quotient, not of the missing
orientation sheet.

### 4.2 Positive quadratic faithful-readout obstruction

For a signed pointer `r`, the general quadratic position potential is

\[
 V_2(r,q)=\frac a2r^2+b,rq+\frac c2q^2.       \tag{4}
\]

Positivity requires `a>=0`, `c>=0`, and `b^2<=ac`. Exact criticality of the
clock requires `c=0`. Therefore

\[
 c=0\quad\hbox{and}\quad V_2\ge0
 \quad\Longrightarrow\quad b=0.               \tag{5}
\]

Thus a nonzero positive bilinear signed readout necessarily supplies
quadratic clock stiffness. The positive lock `(r-q)^2` is an explicit
control: it contains a nonzero `q^2` term.

Every nonzero homogeneous odd-degree polynomial changes sign under total
inversion, so it cannot be globally nonnegative. Under the frozen class

- autonomous polynomial position interaction;
- global nonnegativity;
- joint odd covariance `(r,q)->(-r,-q)`;
- a zero clock Hessian at the origin; and
- nonzero pointer response to the sign of `q`,

degrees one and three fail positivity/parity, and degree two fails (5). The
first admissible degree is therefore four.

### 4.3 Selected degree-four odd pointer

Select an odd pointer `(r,pi)` transforming with the relative channel:

\[
 S:(r,\pi,q,p)\mapsto(-r,-\pi,-q,-p).          \tag{6}
\]

Use the local interaction

\[
 W_-(r,q)=\frac\kappa4(r-q)^4,
 \qquad \kappa>0.                              \tag{7}
\]

It is positive, invariant under (6), and has zero gradient and Hessian at
the origin. Its pointer force is

\[
 F_r=-\partial_r W_-=-\kappa(r-q)^3,
 \qquad F_r(0,q)=\kappa q^3.                  \tag{8}
\]

The pointer response therefore carries the signed sheet. With `r=pi=0`, its
force derivative for `q!=0` also contains the velocity datum

\[
 \dot F_r(0,q)=3\kappa q^2\frac p m.           \tag{9}
\]

At an exact crossing `q=0`, instantaneous force and its first derivative
vanish. The local history still distinguishes the crossing direction:

\[
 r^{(5)}(0)=\frac{6\kappa p^3}{M m^3}          \tag{10}
\]

for initial `r=pi=q=0`. Equation (10) is a history/readout statement, not an
instantaneous zero-crossing oracle.

### 4.4 Positive Hamiltonian and exact discrete transaction

Adopt the onsite reference Hamiltonian

\[
 H=\frac{p^2}{2m}+\frac{\pi^2}{2M}
   +\lambda q^4+\frac\kappa4(r-q)^4,
 \qquad m,M,\lambda,\kappa>0.                 \tag{11}
\]

It is positive and coercive. Hence every finite-energy orbit is bounded.
No spatial neighbor is read.

For

\[
 G(x_0,x_1)=(x_1^2+x_0^2)(x_1+x_0),           \tag{12}
\]

let `z_j=r_j-q_j` and use

\[
 \frac{q_1-q_0}{h}=\frac{p_1+p_0}{2m},
 \qquad
 \frac{r_1-r_0}{h}=\frac{\pi_1+\pi_0}{2M},   \tag{13}
\]

\[
 \frac{p_1-p_0}{h}
 =-\lambda G(q_0,q_1)+\frac\kappa4G(z_0,z_1), \tag{14}
\]

\[
 \frac{\pi_1-\pi_0}{h}
 =-\frac\kappa4G(z_0,z_1).                    \tag{15}
\]

The secant identities

\[
 G(x_0,x_1)(x_1-x_0)=x_1^4-x_0^4             \tag{16}
\]

and `z_1-z_0=(r_1-r_0)-(q_1-q_0)` prove exact conservation of
(11). More strongly, with

\[
 E_q=\frac{p^2}{2m}+\lambda q^4,
 \quad E_r=\frac{\pi^2}{2M},
 \quad E_I=\frac\kappa4z^4,                   \tag{17}
\]

the primitive transaction is

\[
 \Delta E_q=\frac\kappa4G_z\Delta q,
 \quad
 \Delta E_r=-\frac\kappa4G_z\Delta r,
 \quad
 \Delta E_I=\frac\kappa4G_z(\Delta r-\Delta q),\tag{18}
\]

whose sum is exactly zero. Readout work is therefore visible rather than
free.

The endpoint equations are symmetric. After eliminating the momenta, their
Jacobian is a positive mass matrix plus nonnegative rank-one terms because

\[
 \partial_{x_1}G(x_0,x_1)
 =3x_1^2+2x_0x_1+x_0^2
 =3(x_1+x_0/3)^2+2x_0^2/3\ge0.                \tag{19}
\]

Thus the endpoint map is strongly monotone and coercive: one next state
exists, it is unique, and the symmetric rule is reversible.

### 4.5 Orientation compliance boundary

The clock-only swept-area witness is no longer unconditionally negative.
Equations (13)--(15) give

\[
 \chi_q
 =-h\left[
   \frac\lambda2(q_1+q_0)^2(q_1^2+q_0^2)
   +\frac{(p_1+p_0)^2}{4m}
 \right]
 +\frac{h\kappa}{8}(q_1+q_0)G(z_0,z_1).       \tag{20}
\]

The final term is the exact backreaction torque. A sufficient per-step
orientation-compliance gate is that its absolute value be strictly smaller
than the positive bracket in (20). No nonzero `kappa` gives a universal
strict-sign theorem for unrestricted pointer states. Readout without energy
destruction exists; readout without any phase disturbance does not.

Setting `kappa=0` recovers the FTD-0844 onsite quartic recursion exactly and
leaves an inert zero pointer.

## 5. Frozen exact checks

The implementation must run exactly 32 checks:

1. all five source hashes;
2. exchange parity is an involution;
3. a generic degree-four common/even polynomial loses all odd powers of `q`;
4. its common force is even in `q`;
5. the square-pointer control (3) is nonnegative and exchange invariant;
6. (3) has zero clock Hessian at the origin;
7. its zero-pointer force is `kappa q^2`;
8. it is exactly blind to `q -> -q`;
9. the quadratic Hessian determinant is `ac-b^2`;
10. at `c=0` positivity forces `b=0`;
11. the positive harmonic difference control adds clock stiffness;
12. nonzero odd homogeneous degree cannot be globally nonnegative;
13. (7) has the registered joint-odd covariance;
14. (7) is nonnegative;
15. (7) has zero gradient and Hessian at the origin;
16. equation (8) holds exactly;
17. equation (9) holds exactly;
18. equation (10) holds exactly;
19. (11) is positive/coercive under positive parameters;
20. equation (16) holds for the clock quartic;
21. equation (16) holds for the interaction coordinate;
22. equations (13)--(15) conserve (11) exactly;
23. the three-account transaction (18) sums to zero;
24. the discrete gradient is endpoint symmetric;
25. the update is physically reversible;
26. equation (19) is nonnegative;
27. the eliminated endpoint Jacobian is positive definite;
28. strong monotonicity/coercivity gives one local next state;
29. the dependency radius is onsite and `kappa=0` recovers FTD-0844;
30. equation (20) is the exact clock swept-area identity;
31. no universal strict clock-orientation sign survives unrestricted
    backreaction, while the stated compliance inequality is sufficient; and
32. combined discriminator: common/even readout sees only the square quotient,
    positive bilinear faithful readout destroys criticality, and a selected
    odd pointer with a quartic difference is the degree-minimum positive local
    energy-closed faithful bridge in the frozen class.

## 6. Locked implementation

```text
scripts/proofs/proof_swap_parity_phase_readout.py
```

Script SHA-256:
`41E1D1E9043620D20E71A2B18EC72041D5BBC7298133F6C082F9FB877F58FB66`

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before the first execution. Run exactly:

```text
python scripts/proofs/proof_swap_parity_phase_readout.py
```

## 7. Outcomes

- **Outcome A — common/even faithful readout:** all gates pass and a positive
  common/even interaction distinguishes the full signed phase while retaining
  exact criticality. This would refute the registered parity boundary.
- **Outcome B — odd-pointer minimum:** all 32 gates pass. The common/even
  pointer reads only the square quotient; the positive quadratic faithful
  channel is obstructed; the degree-four odd pointer is local, reversible,
  bounded, and exactly energy closed, with explicit backreaction and an
  orientation-compliance boundary.
- **Outcome C — invalid:** any exact or source-hash gate fails without
  establishing Outcome A. Book no theorem and repair only under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run.

## 8. Recorded outcome

The first locked execution returned `31/32`. C9 compared the algebraically
identical factorizations

```text
kappa*(-a + q**2)**2/2
kappa*( a - q**2)**2/2
```

by SymPy structural equality. Their exact simplified difference is zero.
Every source hash and C6--C8/C10--C32 passed. The parent certificate is
invalid and books no theorem. No physical equation, coefficient, class,
gate, or expected outcome is changed under this lock; any repair requires a
fresh verifier-only preregistration.
