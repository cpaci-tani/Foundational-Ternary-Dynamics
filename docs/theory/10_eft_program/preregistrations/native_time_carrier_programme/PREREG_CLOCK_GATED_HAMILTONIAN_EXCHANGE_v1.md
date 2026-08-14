# PRE-REGISTRATION — Clock-gated Hamiltonian exchange v1

**Date locked:** 2026-08-11  
**Identifier:** `FTD-0864`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXECUTION INVALID 39/40]`  
**Parents:** `FTD-0840`, `FTD-0846`, `FTD-0856`, `FTD-0858`, `FTD-0863`

## 1. Question

Can the ideal FTD-0863 matter/signal exchange be lifted to a closed autonomous
Hamiltonian flow in which the clock phase supplies the pulse, the reference
backreaction is booked exactly, and a complete cycle realizes the FTD-0856
hold/swap branches without external time dependence?

The test must also determine whether the same minimal construction can use a
nonlinear quartic `G*` clock as a **load-blind** exact swap controller for more
than one event energy.

The registered class is deliberately narrow:

1. one canonical clock action-angle pair `(theta,I)`;
2. one full canonical matter mode `M` and one full canonical signal mode `D`;
3. an onsite phase gate `g(theta)=1-cos(theta)`;
4. a quadratic common/relative-mode Hamiltonian; and
5. a frozen eligibility sector `epsilon in {0,1}`.

The eligibility value is not called a dynamical controller. Promoting it
requires its own state, conjugate/work account, and transition law.

## 2. Frozen sources

| Source | SHA256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md` | `C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_SWAP_PARITY_PHASE_READOUT_AND_ODD_POINTER_MINIMUM_v1.md` | `D73693F364A83D468AC76F3165411784610965A66ACC7BD1E7CE3766A3D267AB` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md` | `5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md` | `06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md` | `8BD6BB16999E91A72CADBA991A215F56A3E3E13816073E39B36F9EB51FD5FE33` |
| `engine/include/ftd/eft/catalytic_phase_reference.h` | `25C094B166DE32894A2FB4F0B0BCEE7A68AB279AB8C7D3BA48D4CAEE2BD4B9AB` |

Any mismatch gives Outcome C and books no theorem.

## 3. Symplectic minimum

The scalar open-gate map used in the FTD-0856 barrier is

\[
 S=\begin{pmatrix}0&1\\1&0\end{pmatrix}.                 \tag{1}
\]

It has `det(S)=-1` and `S^T J S=-J`; therefore it is not the time map of a
Hamiltonian flow on one canonical pair. The minimum faithful Hamiltonian lift
uses two full modes

\[
 M=(q_m,p_m),\qquad D=(q_d,p_d),                         \tag{2}
\]

for which the simultaneous mode swap has determinant `+1` and is symplectic.
Introduce common and relative modes

\[
 C=\frac{M+D}{\sqrt2},\qquad R=\frac{M-D}{\sqrt2},       \tag{3}
\]

with actions

\[
 I_c=\frac{|C|^2}{2},\qquad I_r=\frac{|R|^2}{2},\qquad
 A=I_c+I_r=\frac{|M|^2+|D|^2}{2}.                       \tag{4}
\]

The swap is exactly `C -> C`, `R -> -R`.

## 4. Frozen harmonic reference Hamiltonian

For `omega,nu,chi>0` and frozen `epsilon in {0,1}`, adopt the reference law

\[
 H_\epsilon
 =\omega I+\nu(I_c+I_r)
  +\epsilon\chi g(\theta)I_r,
 \qquad g(\theta)=1-\cos\theta.                         \tag{5}
\]

This is an **[IMPOSED reference Hamiltonian law]**, not a substrate
derivation. It is autonomous: global tick or external time does not occur in
the Hamiltonian.

Hamilton's equations give

\[
 \dot\theta=\omega,\qquad
 \dot I=-\epsilon\chi\sin\theta\,I_r,
 \qquad \dot I_c=\dot I_r=0.                            \tag{6}
\]

Starting at `theta=0`,

\[
 I(\theta)=I_0-epsilon\frac\chi\omega I_r
                 (1-\cos\theta).                       \tag{7}
\]

The interaction therefore borrows reference action during the pulse and
returns it after a complete cycle. Since `0<=g<=2`, physical nonzero reference
action throughout the active cycle requires

\[
 I_0>\frac{2\chi}{\omega}I_r.                           \tag{8}
\]

For emission from `D=0` with matter energy `B=|M|^2/2`, `I_r=B/2`.

Over `T=2pi/omega`, the common mode accumulates phase
`2pi nu/omega` and the relative mode has the additional pulse area

\[
 \Xi=\frac\chi\omega\int_0^{2\pi}(1-\cos\theta)d\theta
    =\frac{2\pi\chi}{\omega}.                           \tag{9}
\]

The active branch is an exact swap and the inactive branch an exact hold when

\[
 \frac\nu\omega\in\mathbb Z,
 \qquad \frac{2\chi}{\omega}\in2\mathbb Z+1.           \tag{10}
\]

The minimum positive choice is `nu=omega`, `chi=omega/2`. It gives
`I_min=I_0-I_r`, so an emitted load requires `I_0>B/2`.

At the endpoints the gate energy vanishes and `I(T)=I_0`. During the pulse,
the change `omega Delta I` is exactly offset by
`chi g(theta) I_r`; controller work is visible as internal interaction-energy
exchange rather than silently omitted.

## 5. Nonlinear-clock discriminator

Replace `omega I` by a differentiable clock Hamiltonian `K(I)` with
`K'(I)>0`. Energy closure gives

\[
 K(I(\theta))+\chi g(\theta)I_r=K(I_0).                 \tag{11}
\]

The mixing angle becomes

\[
 \Xi(I_r)=\int_0^{2\pi}
 \frac{\chi g(\theta)}{K'(I(\theta;I_r))}\,d\theta.     \tag{12}
\]

Implicit differentiation yields

\[
 \frac{\partial I}{\partial I_r}
 =-\frac{\chi g}{K'},\qquad
 \frac{d\Xi}{dI_r}
 =\chi^2\int_0^{2\pi}
   \frac{g(\theta)^2K''(I)}{K'(I)^3}\,d\theta.          \tag{13}
\]

Hence `dXi/dI_r>0` for a strictly convex clock (`K''>0`) and a nontrivial
pulse. A fixed coupling cannot produce the same exact swap angle for two
different loads in this class.

For the pure quartic oscillator, homogeneity gives

\[
 K(I)=cI^{4/3},\qquad c>0,                              \tag{14}
\]

so `K'>0` and `K''>0`. The `G*` traversal factor fixes the quartic waveform's
period relation inside the adopted clock; it does not cancel the load
dependence in (13).

This is a scoped obstruction, not a no-go for compensated reservoirs,
isochronized nonlinear clocks, fixed-load sectors, weak-coupling approximations,
or larger controller systems.

## 6. Frozen exact checks

The implementation must run exactly 40 checks:

1--6. all frozen source hashes;
7. the scalar swap has determinant `-1`;
8. the scalar swap reverses the two-dimensional symplectic form;
9. the full two-mode swap is symplectic and determinant `+1`;
10. the common/relative transform is orthogonal;
11. the common/relative transform is symplectic;
12. the physical and common/relative action ledgers agree;
13. the harmonic reference phase advances uniformly;
14--16. `I_r`, `I_c`, and total matter/signal action are first integrals;
17. equation (7) solves the exact reference-action equation;
18. the complete Hamiltonian energy is constant under equation (7);
19. equation (8) is the exact active-cycle reserve bound;
20. an empty signal gives `I_r=B/2`;
21. the gate is a nonnegative square with zero endpoints and maximum two;
22. the exact full-cycle gate area is `2pi`;
23. the common-mode phase is `2pi nu/omega`;
24. the additional relative phase is `2pi chi/omega`;
25. equation (10) makes common and relative endpoint phases `0` and `pi`
    modulo `2pi`;
26. the minimum active winding produces the exact full-mode swap;
27. the minimum inactive winding produces identity;
28. the active map emits `M -> D` from an empty signal;
29. the same active map absorbs `D -> M` from empty matter;
30. signal energy after emission equals the initial matter energy;
31. reference action returns exactly after a full cycle;
32. interaction energy vanishes at both endpoints;
33. generic active loads produce nonzero transient reference backreaction;
34. the Hamiltonian is invariant under canonical time reversal;
35. the harmonic pulse area is load independent;
36. equation (11) is the nonlinear clock energy ledger;
37. the first formula in (13) follows by implicit differentiation;
38. the second formula in (13) is strictly positive for `K''>0`;
39. `K=cI^(4/3)` is increasing and strictly convex for `I>0`; and
40. the scope firewall retains dynamic eligibility, production coupling,
    compensation, cubic propagation, Born/Bell, and a native `G*` gearbox as
    open while rejecting a universal load-blind quartic swap in this class.

## 7. Locked implementation and outcomes

The unrun exact certificate is

```text
scripts/proofs/proof_clock_gated_hamiltonian_exchange.py
```

- **Outcome A — universal nonlinear controller:** all source and construction
  gates pass but equation (13) vanishes for the quartic clock across varying
  nonzero loads. This would refute the registered convex-clock boundary.
- **Outcome B — exact harmonic lift plus quartic boundary:** all 40 gates pass.
  The autonomous harmonic reference realizes exact hold/swap with a transient
  reserve ledger, while the registered minimal quartic controller has strictly
  load-dependent pulse area.
- **Outcome C — invalid:** any source or exact gate fails without establishing
  Outcome A. Book no theorem; repair only under a fresh lock.

The expected result is Outcome B. This expectation is frozen before the first
execution.

No numerical parameter search, near-miss scan, target-frequency fit, Born
weight, Bell setting, outcome, production code, biological interpretation, or
completeness claim is permitted.

## 8. Recorded outcome

The first locked execution returned `39/40`. All six source hashes and every
construction, conservation, reserve, swap, and nonlinear-clock gate passed.
C34 compared two algebraically identical Hamiltonians by structural equality:

```text
I*omega + I_r*chi*epsilon*(1-cos(theta)) + nu*(I_c+I_r)
I*omega - I_r*chi*epsilon*(cos(theta)-1) + nu*(I_c+I_r)
```

Their simplified difference is exactly zero, but their expression trees differ.
The parent script and its pre-run hash remain frozen. FTD-0865 locks the sole
verifier-only repair. No theorem is booked from FTD-0864.
