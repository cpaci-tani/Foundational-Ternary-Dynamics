# FTD-0847 — Loss-booked ternary phase latch v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked discriminator for converting the FTD-0846 odd
pointer into a persistent ternary phase record  
**Production impact:** none

## 1. Registered question

Can a context-blind local mechanism convert the continuous odd-pointer
history into one persistent record `s in {-1,0,+1}` while:

1. retaining exchange covariance;
2. booking pointer backreaction, damping export, and gate-switching work;
3. identifying exactly where information becomes lossy;
4. remaining deterministic and onsite per global tick; and
5. reading no `G*`, target period, measurement context, outcome, or Born
   weight?

The mechanism must distinguish finite-time dissipative dynamics from the
many-to-one record quotient. Damping may contract phase-space volume, but a
smooth finite-time flow does not by itself justify an information-erasure
claim.

## 2. Epistemic firewall

The continuous latch coordinate, its potential, scale, mobilities/damping,
coupling schedule, and basin convention are `[SELECTED reference types]`.
The theorem may prove their mathematical minimum and exact accounting inside
the registered class. It cannot derive them from primitive ternarity or
production dynamics.

The actual state alphabet already contains `-1,0,+1`; this supplies the
codomain, not the transition law. Passing cannot establish a Born rule,
actualization selector, thermodynamic Landauer bound, biological mechanism,
production toggle, or finite-tick `G*` cadence. No numerical search, fit,
near-miss comparison, or target substitution is permitted.

## 3. Frozen source inputs

| Input | SHA-256 |
|---|---|
| `THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md` | `D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB` |
| `docs/theory/02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md` | `50CB845B2CB3874028A9C49C36141EB061785E6160F7880C361A21526C3461C0` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The first path is relative to
`docs/theory/10_eft_program/derivations/native_time_carrier_programme/`.

## 4. Frozen mathematics

### 4.1 Minimum symmetric ternary latch

Let `x` be a continuous exchange-odd latch coordinate and let `A,beta>0`.
Select

\[
 V_T(x)=\beta x^2(x^2-A^2)^2.                 \tag{1}
\]

An even polynomial with isolated nondegenerate minima at `-A,0,+A` needs at
least two intervening maxima. Its derivative therefore needs at least five
distinct real roots, so the potential degree is at least six. Equation (1)
attains that lower bound.

Exactly,

\[
 V_T'(x)=2\beta x(x^2-A^2)(3x^2-A^2).         \tag{2}
\]

The minima are `0,+/-A`; the barriers are at `+/-A/sqrt(3)`, with

\[
 E_b=V_T(A/\sqrt3)=\frac{4\beta A^6}{27}.     \tag{3}
\]

For the undriven damped latch, the three open attraction basins are separated
by those barriers. Define the actual record quotient

\[
 \rho(x)=
 \begin{cases}
 -1,&x<-A/\sqrt3,\\
 0,&|x|\le A/\sqrt3,\\
 +1,&x>A/\sqrt3.
 \end{cases}                                  \tag{4}
\]

The equality cases are assigned to `0` as a deterministic convention. The
map is odd and many-to-one.

### 4.2 Exact deterministic acquisition threshold

During acquisition, an odd pointer `r` tilts the latch by `-f x`, with
`f=g r`. The positive central barrier disappears if `f` exceeds the maximum
of `V_T'` on `[0,A/sqrt(3)]`. Put

\[
 z_* = \frac{6-\sqrt{21}}{15},
 \qquad x_*=A\sqrt{z_*}.                       \tag{5}
\]

Then

\[
 F_c=V_T'(x_*)
 =\frac85\beta A^5\sqrt{z_*}(1-2z_*).         \tag{6}
\]

If `f>F_c` is maintained until the latch crosses `+A/sqrt(3)`, the central
positive-side metastable obstruction is absent; the negative statement is
obtained by parity. Pulse duration remains a registered control. Equation
(6) is a deterministic threshold, not a probability.

### 4.3 Pointer--latch energy and switching work

After the FTD-0846 clock--pointer transaction is decoupled, use the local
pointer--latch potential

\[
 U_g(r,x)=\frac\alpha4r^4+V_T(x)-g\,r x,
 \qquad \alpha>0.                              \tag{7}
\]

For bounded `g`, (7) is coercive. Both `r` and `x` are exchange odd, so
`U_g(-r,-x)=U_g(r,x)`.

Changing `g_0` to `g_1` at fixed `(r,x)` changes system energy by

\[
 \Delta_g U=-(g_1-g_0)rx.                     \tag{8}
\]

Introduce a controller-work account

\[
 \Delta W_{\rm ctrl}=(g_1-g_0)rx,             \tag{9}
\]

so the switch satisfies `Delta_g U+Delta W_ctrl=0`. The account may be
positive or negative: the controller can supply or recover work.

### 4.4 Exact discrete damping and bath ledger

For one constant-`g` tick, let

\[
 Q=(r,x),\qquad P=(\pi,p_x),
 \qquad M={\rm diag}(M_r,M_x),
 \qquad \Gamma={\rm diag}(\gamma_r,\gamma_x), \tag{10}
\]

with all masses and damping coefficients positive. Define the average-vector-
field discrete gradient

\[
 \overline\nabla U_g(Q_0,Q_1)
 =\int_0^1\nabla U_g((1-\xi)Q_0+\xi Q_1)\,d\xi.\tag{11}
\]

It is an exact polynomial and obeys

\[
 \overline\nabla U_g\cdot(Q_1-Q_0)
 =U_g(Q_1)-U_g(Q_0).                           \tag{12}
\]

Use

\[
 \frac{Q_1-Q_0}{h}=M^{-1}\frac{P_1+P_0}{2},  \tag{13}
\]

\[
 \frac{P_1-P_0}{h}
 =-\overline\nabla U_g
  -\Gamma M^{-1}\frac{P_1+P_0}{2}.           \tag{14}
\]

For

\[
 H_g=\frac12P^TM^{-1}P+U_g(Q),                \tag{15}
\]

equations (12)--(14) give

\[
 \Delta H_g
 =-\frac1h(Q_1-Q_0)^T\Gamma(Q_1-Q_0).        \tag{16}
\]

Define the exported bath-energy increment

\[
 \Delta B=\frac1h(Q_1-Q_0)^T\Gamma(Q_1-Q_0)
 \ge0.                                        \tag{17}
\]

Then

\[
 \boxed{\Delta(H_g+B)=0}                      \tag{18}
\]

for every constant-coupling tick. Equations (9) and (18) close the full
piecewise-constant acquisition schedule.

### 4.5 Local well-posedness condition

For `|g|<=g_max`, the Hessian obeys

\[
 \nabla^2U_g\succeq-LI,
 \qquad
 L=g_{\max}+\frac{14}{5}\beta A^4.            \tag{19}
\]

The lower bound follows from

\[
 V_T''(x)+\frac{14}{5}\beta A^4
 =30\beta(x^2-2A^2/5)^2\ge0                  \tag{20}
\]

and `2|g r x|<=g_max(r^2+x^2)` at Hessian level. The symmetric part of the
eliminated endpoint Jacobian is bounded below by

\[
 \delta I,
 \qquad
 \delta=\frac{2M_{\min}}h+\gamma_{\min}
        -\frac{hL}{2}.                         \tag{21}
\]

The registered compliance condition is `delta>0`. Under it the endpoint
residual is strongly monotone and coercive, so every tick has exactly one
next state. All dependencies are onsite.

### 4.6 Persistence and the exact location of loss

After `g=0`, if the latch lies strictly inside one basin and its mechanical
energy is below `E_b`, equation (16) prevents it from crossing a barrier.
The ternary record is persistent while that inequality holds.

The damped continuous vector field has phase-volume divergence

\[
 -\frac{\gamma_r}{M_r}-\frac{\gamma_x}{M_x}<0.\tag{22}
\]

This is contraction, not by itself finite-time non-injectivity. The explicit
lossy map is (4): for example `x=+A/(2sqrt(3))` and
`x=-A/(2sqrt(3))` have equal latch energy and both map to `s=0`. The reduced
record `(s,B)` therefore does not reconstruct the discarded pointer/latch
microstate. This is the registered mathematical meaning of
**unactualization**: irrelevant within-basin detail is not retained in the
actual record.

A scalar bath-energy account closes energy but is not a microscopic bath
state and does not restore injectivity. No universal Landauer cost follows
without additionally selecting a thermal bath, temperature, and erasure
protocol.

## 5. Frozen exact checks

The implementation must run exactly 30 machine gates. The three source hashes
are counted separately so the list below matches the executable one-for-one:

1. the FTD-0846 theorem source hash;
2. the FTD-0395 irreversibility source hash;
3. the production `Voxel` source hash;
4. an even quartic cannot have the five distinct extrema required for three
   nondegenerate minima;
5. (1) is an even nonnegative degree-six polynomial;
6. equation (2) is exact;
7. `0,+/-A` are minima and `+/-A/sqrt(3)` are maxima;
8. equation (3) is exact;
9. the undriven force points into the three registered basins;
10. `z_*` obeys the stationary equation and lies in `(0,1/3)`;
11. `x_*` is the unique maximum of `V_T'` in the positive central interval;
12. equation (6) is exact;
13. exceeding `F_c` removes the registered central-side stationary barrier;
14. (7) is exchange invariant and coercive for bounded `g`;
15. equation (20) is exact;
16. equation (19) is a valid Hessian lower bound;
17. the AVF gradient obeys the exact chain identity (12);
18. the AVF gradient is endpoint symmetric;
19. equations (13)--(14) imply the energy decrement (16);
20. the bath increment (17) is nonnegative;
21. equation (18) closes the constant-`g` tick;
22. equations (8)--(9) close every coupling switch;
23. the endpoint-Jacobian lower bound is (21);
24. `delta>0` gives one local next state;
25. the record quotient takes only `-1,0,+1` and is odd;
26. energy below (3), together with the non-increasing mechanical-energy
    identity, certifies basin persistence after decoupling;
27. equation (22) is the exact negative phase-volume divergence, with no
    finite-time erasure claim attached to contraction alone;
28. the quotient (4) is many-to-one even at equal latch energy;
29. the registered dynamical expressions read no Born target, measurement
    context, outcome target, `G*`, or temperature; a scalar bath ledger closes
    energy but supplies neither the discarded microstate nor a Landauer bound;
30. combined discriminator: the selected sextic latch is degree-minimum in
    the frozen symmetric polynomial class, the damped discrete tick plus
    switching ledger is deterministic/local/energy closed, and the ternary
    basin quotient is the explicit lossy record step.

## 6. Locked implementation

```text
scripts/proofs/proof_loss_booked_ternary_phase_latch.py
```

Frozen implementation SHA-256:

```text
8C0D60C2B0624FC58BA00B9B4A76DA1B641C37D9E4873D991D9ED89CD30103CE
```

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_loss_booked_ternary_phase_latch.py
```

## 7. Outcomes

- **Outcome A — production-native ternary latch:** all gates pass and the
  frozen production state transition already implements (1)--(22).
- **Outcome B — exact selected lossy latch:** all 30 gates pass. The reference
  latch is degree-minimum, deterministic, local, persistent under the barrier
  condition, and energy/work closed. Its latch type, potential, controller,
  and basin quotient remain selected; production realization and Born/selector
  coupling remain open.
- **Outcome C — invalid:** any exact or source-hash gate fails without
  establishing Outcome A. Book no theorem and repair only under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run.

## 8. Recorded outcome

The first locked execution passed all three source hashes and C4--C24, then
aborted at C25 before returning a check count. SymPy could not determine the
truth value of the relational expression

```text
-2*A < -sqrt(3)*A/3
```

despite the frozen assumption `A>0`. C25--C30 were therefore not evaluated.
This is a verifier defect in the sample implementation of `rho`, not a failed
mathematical discriminator. The run is nevertheless invalid by the frozen
Outcome C rule. No theorem is booked from FTD-0847; any repair must use a
fresh lock and may alter only the exact symbolic comparison inside `rho`.
