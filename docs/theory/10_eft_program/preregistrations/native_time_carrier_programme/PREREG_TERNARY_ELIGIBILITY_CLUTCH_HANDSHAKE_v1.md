# PRE-REGISTRATION — Ternary eligibility clutch and one-shot handshake v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0866`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID 39/40]`  
**Parents:** `FTD-0848`, `FTD-0850`, `FTD-0852`, `FTD-0856`, `FTD-0863`,
`FTD-0865`

## 1. Question

Can the selected persistent ternary latch already constructed in FTD-0848
replace FTD-0865's frozen external eligibility value without adding another
controller type, while preserving signed event information through one exact
Hamiltonian exchange and honestly booking the reset boundary?

The registered composition must answer:

1. what map sends the ternary latch record to hold/exchange eligibility;
2. where the event sign remains after eligibility forgets it;
3. when switching the Hamiltonian clutch costs zero work;
4. why a reset is mandatory before a second cycle;
5. whether resetting the local latch loses the declared event information; and
6. which latch, synchronization, bath, and production debts remain.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md` | `1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md` | `95F39274E361868E039368AB149A9196F2008D2BB58CD5F0DAD0CD8F7E92110B` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md` | `7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md` | `8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` |
| `engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h` | `0BDEF8D6278FDF352F89C739F995F337B76AECC8C4FE716DF899B4058DE8A29E` |

Any mismatch gives Outcome C and books no theorem.

## 3. Minimum ternary clutch

Let the retained latch record be

\[
 s\in\{-1,0,+1\}.                              \tag{1}
\]

The hold/exchange eligibility is

\[
 \epsilon(s)=s^2.                              \tag{2}
\]

Equation (2) is the unique even degree-at-most-two polynomial satisfying
`epsilon(0)=0`, `epsilon(+/-1)=1`; degree zero cannot distinguish hold from
exchange. The symmetric square is used only for eligibility. The signed
record `s` remains retained and is not reconstructed from `epsilon`.

Insert (2) into the FTD-0865 reference Hamiltonian:

\[
 H_s=\omega I+\nu A+s^2\chi g(\theta)I_r,
 \qquad g(\theta)=1-\cos\theta.                \tag{3}
\]

At a gate zero `g(theta)=0`, changing `s_0` to `s_1` changes the clutch energy
by

\[
 \Delta H_{\rm clutch}
 =(s_1^2-s_0^2)\chi g(\theta)I_r=0.            \tag{4}
\]

Off the gate zero, equation (4) is generally nonzero and its opposite must be
booked as switch work. Zero clutch work does not make acquisition or physical
latch reset free; those remain governed by the FTD-0848 latch work/bath ledger.

## 4. Signed one-shot event preparation

Let `beta` be the nonzero FTD-0863 orientation reference, with

\[
 f=\frac{J\beta}{|\beta|}.                     \tag{5}
\]

On the declared event domain,

\[
 s=0:\quad B=0,\ M=0,
\]

or

\[
 s\in\{-1,+1\},\quad B>0,\quad
 M=s\sqrt{2B}\,f,\quad D=0.                   \tag{6}
\]

With the FTD-0865 winding and reserve `I_0>B/2`, one complete cycle gives

\[
 s=0:\ (M,D)\mapsto(M,D),
\]

\[
 s=\pm1:\ (M,D)\mapsto(0,M).                 \tag{7}
\]

The outgoing signal decodes the event exactly:

\[
 B=\frac{|D'|^2}{2},\qquad
 s=\operatorname{sign}(\beta\wedge D').       \tag{8}
\]

Thus the local latch may be reset to zero after the exchange without losing
the declared `(s,B)` information from the complete latch-plus-signal state.
Within-basin latch microdetails and other production labels are not recovered
by (8).

## 5. One-shot and reset boundary

The stroboscopic active swap `P` is an involution:

\[
 P^2=1.                                        \tag{9}
\]

If `s=+/-1` remains latched for a second cycle, the signal is swapped back into
matter. A one-shot event therefore requires an acknowledgement/reset before
the next harmonic cycle.

The safe order is:

\[
 \text{latch }s
 \to \epsilon=s^2
 \to \text{one full exchange cycle}
 \to \text{signal carries }(s,B)
 \to \text{gate-zero clutch release}
 \to \text{latch reset}.                       \tag{10}
\]

FTD-0848 supplies a selected damped latch, explicit switch-work account, bath
energy, and many-to-one basin quotient. It does not derive an autonomous
acknowledgement schedule or microscopic bath state. Equation (10) therefore
closes the reduced reference handshake, not autonomous production control.

The event energy `B` already resides in `D'`. Launching a second independent
odd pulse with energy `B` would double count. FTD-0852's odd history carrier
may be identified with the outgoing signal or fed by it, but it cannot be an
additional energetic copy.

## 6. Frozen checks

The exact certificate must run exactly 40 checks:

1--7. all frozen source hashes;
8. `epsilon(-1)=epsilon(+1)=1`, `epsilon(0)=0`;
9. eligibility is even;
10. `s^2` is the unique even polynomial of degree at most two with the
    registered values;
11. degree zero cannot implement hold and exchange;
12. `s^3=s` retains the signed ternary channel;
13. the eligibility square deliberately identifies opposite signs;
14. inserting `s^2` gives equation (3);
15. the phase gate vanishes at both cycle endpoints;
16. clutch switching work is exactly zero at a gate zero;
17. off-phase clutch switching has the exact nonzero work ledger (4);
18. `s=0` selects the identity branch;
19. `s=+/-1` selects the same full-mode swap branch;
20. the active swap is symplectic;
21. the active swap is an involution;
22. the registered signed matter preparation has energy `B`;
23. the active output places that complete mode in the signal;
24. signal energy after exchange is exactly `B`;
25. oriented area has sign `s`;
26. equation (8) recovers `B`;
27. equation (8) recovers both signs;
28. the reduced `(s,B)->D'` map is injective for `B>0`;
29. resetting the local latch after exchange retains declared event recovery;
30. retaining the active latch for a second cycle swaps the signal back;
31. reset before signal formation would erase the only local sign input;
32. event energy is counted once in the outgoing signal;
33. the FTD-0848 latch minima are energy degenerate;
34. the zero-coupling continuous reset crosses the exact positive latch barrier;
35. FTD-0848 books damping and switch work separately;
36. its bath scalar does not retain microscopic erased details;
37. FTD-0850 proves current production is not the selected latch;
38. the C++ Hamiltonian interface still declares dynamic eligibility absent;
39. no setting, outcome weight, Born target, or `G*` cadence enters the clutch;
40. the combined verdict retains autonomous acknowledgement/reset,
    microscopic bath, clock synchronization, cubic/production coupling, Born,
    Bell, Lorentz hiding, and completeness as open.

## 7. Locked implementation and outcomes

The unrun certificate is

```text
scripts/proofs/proof_ternary_eligibility_clutch_handshake.py
```

- **Outcome A — autonomous closed handshake:** all gates pass and the existing
  latch supplies a self-triggered reversible acknowledgement/reset with no
  additional history or bath state.
- **Outcome B — exact reduced clutch plus reset boundary:** all 40 gates pass.
  `s^2` closes latch-derived eligibility and the signal retains `(s,B)`, while
  one-shot acknowledgement/reset remains a selected open-system schedule and
  production remains incomplete.
- **Outcome C — invalid:** any source or exact gate fails without establishing
  Outcome A. Book no theorem.

Expected result: Outcome B. This expectation is frozen before the first run.

No numerical search, fitted threshold, extra event-energy copy, outcome/Born
target, remote setting, production mutation, biological identification, or
whole-framework completeness claim is permitted.

## 8. Recorded outcome

The first locked execution returned `39/40`. All seven source hashes and every
clutch-value, gate-zero work, symplectic, event-energy, sign-decoder, reset,
source-boundary, and scope-firewall check passed. C14 failed.

The failure is substantive at the certificate-expression level, not structural
equality. The script encoded

```text
H = omega*I + nu*A + s^2*chi*g(theta)*I_r
```

with `A` algebraically independent of `I_r`, but expected
`dH/dI_r = nu + s^2*chi*g(theta)`. Its actual derivative is
`s^2*chi*g(theta)`. The intended FTD-0865 parent Hamiltonian is
`omega*I + nu*(I_c+I_r) + s^2*chi*g(theta)*I_r`.

FTD-0866 remains execution-invalid and books no theorem. A fresh repair must
freeze the exact missing `nu*I_r` correction and change nothing else.
