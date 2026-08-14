# PRE-REGISTRATION — Signal-acknowledged two-stroke reset v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0868`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID, ABORTED AFTER C26]`  
**Parents:** `FTD-0848`, `FTD-0865`, `FTD-0867`

## 1. Question

Can the outgoing local signal produced by FTD-0867 serve as its own
target-blind acknowledgement, allowing one harmonic supercycle to exchange an
event, reset the persistent latch, export the signal, and return the local
controller to a ready state without an externally timed reset command?

The registered discriminator must separate four claims:

1. exact stroboscopic acknowledgement from physical signal completion;
2. exact finite-time reset of the continuous latch coordinate;
3. controller-work, bath, clock-reserve, and event-energy ledgers; and
4. local reference recursion from protected cubic production transport.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md` | `1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md` | `6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D` |
| `engine/include/ftd/eft/ternary_eligibility_clutch.h` | `C53ED1A7FCFF54E4236D2353CA319BCE61EC459C1A7A90F2069C01145256FE43` |

Any mismatch gives Outcome C and books no theorem.

## 3. Two-stroke harmonic supercycle

Let `theta` advance uniformly with `dot(theta)=omega>0` over one supercycle
`0<=theta<=2pi`. Use the exchange window

\[
 g_E(\theta)=
 \begin{cases}
 1-\cos(2\theta),&0\le\theta\le\pi,\\
 0,&\pi\le\theta\le2\pi.
 \end{cases}                                      \tag{1}
\]

It is nonnegative, has zeros at `0,pi,2pi`, and

\[
 \int_0^\pi g_E(\theta)\,d\theta=\pi.           \tag{2}
\]

On the first stroke select the reference Hamiltonian

\[
 H_E=\omega I+2\omega(I_c+I_r)
     +s^2\omega g_E(\theta)I_r.                  \tag{3}
\]

The common mode winds by `2pi` and the relative mode receives one additional
`pi`, so the complete matter and signal modes swap at `theta=pi`. On the
second stroke the interaction vanishes and both modes wind by another `2pi`,
so their midpoint values return unchanged at `theta=2pi`.

For the active empty-signal event, `I_r=B/2`. The exact clock action is

\[
 I(\theta)=I_0-s^2g_E(\theta)I_r,               \tag{4}
\]

hence

\[
 I_{\min}=I_0-2I_r=I_0-B,
 \qquad I(\pi)=I(2\pi)=I_0.                     \tag{5}
\]

This compressed exchange therefore requires the stricter reserve `I_0>B`.
It is not the FTD-0865 full-cycle law and is booked as a new **[IMPOSED
reference controller waveform]**.

## 4. Signal-as-acknowledgement

At the midpoint define the even local completion predicate

\[
 a=s^2\,
   \mathbf1\!\left[|M(\pi)|^2/2=0\right]
   \mathbf1\!\left[|D(\pi)|^2/2>0\right].       \tag{6}
\]

For the registered active preparation, the exact swap gives `M(pi)=0` and
`|D(pi)|^2/2=B>0`, so `a=1`. For the no-event state `s=0,M=D=0`, `a=0`.
No sign, event-energy target, outcome weight, Born probability, measurement
setting, remote context, or `G*` cadence enters (6).

The signal remains local throughout the reset stroke. It is therefore both
the retained event record and the temporary acknowledgement token; no extra
acknowledgement bit is selected.

## 5. Smooth finite-time obstruction

Let an autonomous reset ODE be `dot(x)=F(x)` with `F(0)=0` and `F` locally
Lipschitz. If a nonzero trajectory reached `x(T)=0` at finite `T`, uniqueness
of the initial-value problem through `(T,0)` would make it the identically-zero
solution in a neighbourhood of `T`, including backwards. Repetition would
contradict the nonzero initial state. Therefore a smooth locally unique
autonomous attraction cannot perform exact finite-time reset.

At least one of the following is required: a nonsmooth/non-Lipschitz law, an
explicit many-to-one quotient, a timed forcing switch, or an additional
retained controller state. Ordinary exponential damping is only asymptotic.

## 6. Selected finite-time dry-friction reset

For the first reference realization, assume the acknowledged latch begins at
`x=sA`, `s=+/-1`, and switch at `theta=pi` from the ternary potential

\[
 V_T(x)=\beta x^2(x^2-A^2)^2
\]

to the even cusp potential

\[
 V_R(x)=\kappa|x|,\qquad\kappa>0.               \tag{7}
\]

Use the overdamped maximal-monotone inclusion

\[
 \gamma\dot x\in-\kappa\,\partial|x|.          \tag{8}
\]

With the sticking selection at zero, its exact solution is

\[
 x(t)=s\max\!\left(A-\frac{\kappa}{\gamma}t,0\right),
 \qquad
 T_R=\frac{\gamma A}{\kappa}.                  \tag{9}
\]

The reset stroke lasts `pi/omega`; therefore compliance is

\[
 \kappa\ge\frac{\gamma A\omega}{\pi}.          \tag{10}
\]

Equality is the minimum registered reset force. At the end, `x=0`, the
FTD-0848 basin quotient is exactly zero, and switching back to `V_T` costs
zero because `V_R(0)=V_T(0)=0`.

Switching on at `x=sA` raises system potential by `kappa A`; the controller
reservoir supplies that amount. During (8), the bath receives

\[
 \Delta B_R=\int_0^{T_R}\frac{\kappa^2}{\gamma}\,dt
            =\kappa A.                         \tag{11}
\]

Thus the reset system returns to zero energy, the controller reservoir loses
`kappa A`, the bath gains `kappa A`, and their exact sum closes. This is a
selected scalar loss ledger, not a microscopic thermal bath or Landauer law.

## 7. Export and ready state

After reset, swap the complete local signal mode into an initially empty
export port `E`. This reciprocal full-mode swap preserves action and gives

\[
 (s,x,M,D,E)=(\pm1,\pm A,M_s,0,0)
 \longmapsto(0,0,0,0,M_s).                     \tag{12}
\]

The local site is ready for a later event, while the export port retains

\[
 B=|E|^2/2,\qquad s=\operatorname{sign}(\beta\wedge E). \tag{13}
\]

Equation (12) is a reference output-port handoff, not protected cubic
propagation. Native transport must separately move and protect `E` before the
port is reused.

## 8. Frozen checks

The certificate must run exactly 44 checks:

1--4. all frozen source hashes;
5. exchange-window endpoint zeros;
6. first-stroke nonnegativity identity;
7. exact first-stroke gate area;
8. reset-stroke interaction is zero;
9. first-stroke common winding is `2pi`;
10. first-stroke extra relative winding is `pi`;
11. the active first stroke is the complete-mode swap;
12. the inactive first stroke is identity;
13. second-stroke common winding is `2pi`;
14. second-stroke relative extra winding is zero;
15. the second stroke preserves the midpoint modes;
16. empty-signal relative action is `B/2`;
17. equation (4) satisfies the clock-action equation;
18. clock action returns at midpoint and endpoint;
19. the exact minimum action is `I_0-2I_r`;
20. the event reserve is `I_0-B`;
21. strict reserve requires `I_0>B`;
22. maximum interaction energy is `omega B`;
23. maximum reference-energy loan is `omega B`;
24. the active midpoint acknowledgement is one;
25. the no-event acknowledgement is zero;
26. acknowledgement is even under sign reversal;
27. partial transfer does not satisfy the exact completion predicate;
28. the midpoint signal retains both signs;
29. a smooth exponential representative is nonzero at every finite time;
30. the cusp reset potential is even and positive away from zero;
31. equation (9) satisfies (8) before arrival for both signs;
32. equation (9) reaches zero at `T_R`;
33. zero is an admissible sticking solution;
34. the reset-window duration is `pi/omega`;
35. equation (10) is the exact compliance threshold;
36. the minimum registered force finishes at the endpoint;
37. switch-on system energy is `kappa A`;
38. integrated bath export is `kappa A`;
39. switch-off energy at zero vanishes;
40. controller/reset/bath ledger closes exactly;
41. reset leaves event energy in the signal unchanged;
42. the full-mode export swap is symplectic, involutive, and energy preserving;
43. the final local state is ready and the export decodes `(s,B)`;
44. autonomous reference acknowledgement is not promoted to a smooth reset,
    microscopic bath, protected cubic transport, production, `G*`, Born/Bell,
    Lorentz, biological, or completeness result.

## 9. Locked implementation and outcomes

The unrun certificate is

```text
scripts/proofs/proof_signal_acknowledged_two_stroke_reset.py
```

- **Outcome A — smooth autonomous closure:** all gates pass and a locally
  Lipschitz autonomous attraction performs exact finite-time reset. This would
  refute the registered uniqueness obstruction.
- **Outcome B — nonsmooth two-stroke reference closure:** all 44 gates pass.
  Signal completion supplies exact acknowledgement; the selected cusp law
  closes reset in finite time with explicit work/bath cost; an empty export
  port makes the local reference recursively ready. Smooth reset and physical
  protected production transport remain closed/open as stated.
- **Outcome C — invalid:** any source or exact gate fails without establishing
  Outcome A. Book no theorem; any repair requires a fresh lock.

Expected result: Outcome B. This expectation is frozen before first execution.

No numerical search, fitted tolerance, target cadence, event-energy copy,
Born/outcome weight, remote setting, production mutation, or whole-framework
completeness claim is permitted.

## 10. Recorded outcome

The first locked execution recorded 26 checks through C26: 25 passed and C21
failed. It then aborted while constructing C27, before C27--C44 ran.

- C21 compared SymPy's exact relational result `B < I_0` to the equivalent set
  object `Interval.open(B,oo)` by structural equality.
- C27 attempted `int(BooleanTrue)`, which this SymPy/Python combination rejects;
  the exact Boolean must first pass through Python `bool`.

All four source hashes and C5--C20 plus C22--C26 passed. FTD-0868 remains
execution-invalid and books no theorem. FTD-0869 may repair only these two
verifier representations under a fresh lock.
