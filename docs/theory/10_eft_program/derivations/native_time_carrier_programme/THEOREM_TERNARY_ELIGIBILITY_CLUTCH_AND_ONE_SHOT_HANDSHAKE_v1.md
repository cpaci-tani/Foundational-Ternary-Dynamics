# THEOREM — Ternary eligibility clutch and one-shot handshake v1

**Identifier:** `FTD-0867`  
**Status:** `[THEOREM — CONDITIONAL ON SELECTED REFERENCE COMPONENTS]`  
**Certificate:** repaired FTD-0867 `40/40`; parent FTD-0866 remains
execution-invalid `39/40`  
**Date:** 2026-08-11

## 1. Scope

This theorem composes three already-booked reference components:

1. the FTD-0848 selected persistent ternary latch;
2. the FTD-0863 selected nonzero phase reference; and
3. the FTD-0865 imposed harmonic Hamiltonian exchange.

It proves a minimum reduced handshake from a retained ternary event label to a
one-cycle outgoing canonical signal. It does not derive those components from
the cubic substrate and does not supply an autonomous physical controller.

## 2. Minimum eligibility map

Let the retained latch record be

\[
 s\in\{-1,0,+1\}.                              \tag{1}
\]

Require a context-blind eligibility map which is invariant under event-sign
reversal, holds at zero, and exchanges at either nonzero latch state. Among
even polynomials of degree at most two, write

\[
 \epsilon(s)=a_0+a_2s^2.
\]

The conditions `epsilon(0)=0` and `epsilon(+/-1)=1` give

\[
 a_0=0,\qquad a_2=1,
\]

hence uniquely

\[
 \boxed{\epsilon(s)=s^2}.                      \tag{2}
\]

This symmetric square intentionally loses sign as a clutch command. It does
not erase the retained signed latch state `s` itself.

## 3. Hamiltonian clutch

Let `I` and `theta` be the clock action and phase, and let `I_c,I_r` be the
common and relative canonical-mode actions. Insert (2) into the selected
FTD-0865 Hamiltonian:

\[
 H_s=\omega I+\nu(I_c+I_r)
     +s^2\chi\bigl(1-\cos\theta\bigr)I_r.       \tag{3}
\]

For the registered harmonic ratios

\[
 \nu=\omega,\qquad \chi=\frac{\omega}{2},      \tag{4}
\]

one complete clock cycle gives the exact stroboscopic map

\[
 s=0:\quad (M,D)\mapsto(M,D),
\]

\[
 s=\pm1:\quad (M,D)\mapsto(D,M).               \tag{5}
\]

The scalar swap on only one canonical coordinate would be anti-symplectic;
(5) exchanges both coordinates of the complete matter and signal modes and is
symplectic.

Changing the clutch from `s_0` to `s_1` at fixed phase changes its interaction
energy by

\[
 \Delta H_{\rm clutch}
 =(s_1^2-s_0^2)\chi(1-\cos\theta)I_r.           \tag{6}
\]

At either cycle endpoint, `theta=0 mod 2pi`, equation (6) vanishes exactly.
Off phase, it generally does not vanish and must be booked as controller work.
The zero in (6) does not remove the separate acquisition, damping, barrier,
and bath accounts of the FTD-0848 latch.

## 4. Signed event export

Let `beta` be a nonzero phase reference and

\[
 f=\frac{J\beta}{|\beta|}.
\]

On the registered one-shot preparation domain, take

\[
 s=0:\quad B=0,\quad M=D=0,
\]

or

\[
 s\in\{-1,+1\},\quad B>0,\quad
 M=s\sqrt{2B}\,f,\quad D=0.                   \tag{7}
\]

If the reference reserve obeys

\[
 I_0>\frac{B}{2},                              \tag{8}
\]

then the active branch of (5) gives

\[
 (M,0)\mapsto(0,M).                            \tag{9}
\]

The outgoing signal therefore retains the declared event exactly:

\[
 \boxed{B=\frac{|D'|^2}{2}},\qquad
 \boxed{s=\operatorname{sign}(\beta\wedge D')}. \tag{10}
\]

Thus `epsilon=s^2` may forget sign at the clutch while the separate canonical
signal retains sign and energy. No second pulse of energy `B` may be added:
the energy is already present once in `D'`.

## 5. One-shot reset boundary

The active stroboscopic map `P` is an involution:

\[
 P^2=1.                                        \tag{11}
\]

Leaving `s=+/-1` active for a second cycle would move the signal back into the
matter mode. The reduced safe ordering is therefore

\[
 s\to s^2\to\text{one cycle}\to D'
 \to\text{gate-zero clutch release request}\to\text{latch reset}. \tag{12}
\]

After (9), resetting the local latch does not lose the declared macroscopic
pair `(s,B)`, because (10) recovers it from `(beta,D')`. It may lose within-
basin latch microdetails and any production information not encoded by (10).

Equation (12) is not an autonomous reset mechanism. It requires an
acknowledgement telling the controller that signal formation has completed;
FTD-0848's dissipative reset then still carries its own work and bath ledger.

## 6. Exact certificate and isolated witness

The frozen FTD-0866 parent certificate first returned `39/40`: its C14 SymPy
line omitted the parent Hamiltonian term `nu*I_r`. FTD-0867 froze exactly the
correction

```text
nu*I_c -> nu*(I_c+I_r)
```

in the certificate representation and changed nothing else. The repaired
certificate passed `40/40`.

The isolated implementation is:

- `engine/include/ftd/eft/ternary_eligibility_clutch.h`
- `engine/src/eft/ternary_eligibility_clutch.cpp`
- `engine/tests/test_ternary_eligibility_clutch.cpp`

It derives eligibility only from the local ternary latch, rejects
non-reference-orthogonal or sign-inconsistent preparations, inherits the
strict reserve gate, decodes `(s,B)` from the signal, and exposes a gate-zero
release request. It explicitly returns false for microscopic latch reset,
autonomous acknowledgement, clock synchronization, and cubic production
coupling. The focused CTest passes `1/1`.

## 7. Epistemic accounting

- **[THEOREM]** Equation (2) is the unique even degree-at-most-two clutch with
  the registered ternary values.
- **[THEOREM, CONDITIONAL]** Equations (5)--(12) follow exactly from the
  selected latch/reference and imposed FTD-0865 harmonic Hamiltonian.
- **[SELECTED]** The FTD-0848 loss-booked latch and FTD-0863 phase reference
  remain adopted reference components, not substrate derivations.
- **[IMPOSED]** The harmonic law and ratios (4) remain a reference realization.
- **[OPEN]** Native formation and maintenance of the phase reference.
- **[OPEN]** Autonomous acknowledgement and reset scheduling.
- **[OPEN]** Microscopic bath realization and exact reset work in the combined
  dynamics.
- **[OPEN]** Synchronization of local latch, exchange clock, and transport.
- **[OPEN]** Cubic-lattice production of the preparation (7).
- **[OPEN]** Dynamic compensation for the quartic load dependence proved in
  FTD-0865.
- **[OPEN]** Born recovery, Bell correlations, operational Lorentz hiding, and
  framework completeness.

## 8. Conclusion

The substrate programme no longer needs a separate binary eligibility type:
the existing ternary latch already contains the minimum clutch through its
even square. The sign that the square loses is retained by the outgoing
oriented canonical signal. What remains missing is not the algebraic clutch;
it is the physical acknowledgement/reset/synchronization mechanism that makes
the exact exchange autonomous and repeatable.
