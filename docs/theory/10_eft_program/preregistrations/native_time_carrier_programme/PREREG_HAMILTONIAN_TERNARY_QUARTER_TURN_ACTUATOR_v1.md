# FTD-0873 — Hamiltonian ternary quarter-turn actuator v1

**Identifier:** `FTD-0873`  
**Status:** `[PRE-REGISTRATION — LOCKED/PRE-RUN]`  
**Date:** 2026-08-11  
**Parents:** `FTD-0836`, `FTD-0865`, `FTD-0867`, `FTD-0871`, `FTD-0872`  
**Production status:** unchanged; isolated imposed reference Hamiltonian only

## 1. Registered question

FTD-0872 proves the exact actual-layer permutation

\[
 R(s,o)=(-o,s),
\]

but deliberately leaves physical actuation and controller work open. Can that
permutation be lifted to the minimum autonomous continuous Hamiltonian phase
space, with an exact transient clock-action ledger and without identifying the
quartic `G*` calendar with a load-bearing actuator?

The registered result is conditional on one imposed harmonic carrier/clock
Hamiltonian and one imposed amplitude embedding. It is not a P1--P5 derivation,
a native production mechanism, a one-shot scheduler, or a `G*` synchronization
theorem.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md` | `779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CLOCK_GATED_HAMILTONIAN_EXCHANGE_AND_QUARTIC_LOAD_BOUNDARY_v1.md` | `FFC0E39CC2C87FE73DC3C931302FE32EB5493E6AFB426CFA5BF97624DA3917D1` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md` | `6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md` | `F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md` | `898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317` |
| `engine/include/ftd/eft/clock_gated_hamiltonian_exchange.h` | `0BDEF8D6278FDF352F89C739F995F337B76AECC8C4FE716DF899B4058DE8A29E` |
| `engine/include/ftd/eft/oriented_ternary_quarter_turn.h` | `46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1` |

Any mismatch invalidates the run. The certificate may read only these sources
and this protocol. It may not alter a source, tune a coefficient after seeing
an outcome, or infer a production result from a reference Hamiltonian.

## 3. Frozen Hamiltonian class

Embed the actual ternary pair into one ordered canonical carrier mode by

\[
 z=(p,q)=a(s,o),\qquad a>0,
 \qquad A=\frac{p^2+q^2}{2}
 =\frac{a^2}{2}(s^2+o^2).                     \tag{1}
\]

Let `(theta,I)` be an independent clock pair with `{theta,I}=1`. For frozen
eligibility `e in {0,1}` and orientation `sigma in {-1,+1}`, register

\[
 H_{e,\sigma}
 =\Omega I+\nu A
  +e\sigma\kappa(1-\cos\theta)A,             \tag{2}
\]

with

\[
 \Omega>0,\qquad \nu=\Omega,
 \qquad \kappa=\frac{\Omega}{4}.             \tag{3}
\]

The ordering `(p,q)` is deliberate. Hamilton's equations give

\[
 \dot p=-[\nu+e\sigma\kappa(1-\cos\theta)]q,
 \qquad
 \dot q=[\nu+e\sigma\kappa(1-\cos\theta)]p. \tag{4}
\]

Thus positive `sigma` realizes the FTD-0872 orientation rather than its
inverse. No numerical tolerance or target probability appears in (1)--(4).

## 4. Frozen exact solution

Starting at a gate zero `theta=0`, one clock cycle has

\[
 T=\frac{2\pi}{\Omega},\qquad \theta(T)=2\pi. \tag{5}
\]

The carrier action is constant and the clock action is

\[
 I(\theta)=I_0
 -e\sigma\frac{\kappa}{\Omega}A(1-\cos\theta). \tag{6}
\]

Consequently

\[
 H_{e,\sigma}=\Omega I_0+\nu A                \tag{7}
\]

at every phase. The carrier angle is

\[
 \Phi_{e,\sigma}
 =2\pi\frac{\nu}{\Omega}
 +e\sigma,2\pi\frac{\kappa}{\Omega}
 =2\pi+e\sigma\frac{\pi}{2}.                \tag{8}
\]

Therefore the stroboscopic branches are

\[
 e=0:\ z\mapsto z,
 \qquad
 e=1,\sigma=+1:\ z\mapsto Rz,
 \qquad
 e=1,\sigma=-1:\ z\mapsto R^{-1}z.          \tag{9}
\]

The endpoint carrier energy is

\[
 E_{\rm carrier}=\nu A
 =\frac{\nu a^2}{2}(s^2+o^2).                \tag{10}
\]

Equation (10) supplies an explicit physical scale only conditional on the
imposed `a` and `nu`; it does not derive either scale from the substrate.

The maximum absolute clock-action excursion and reference-energy exchange are

\[
 |\Delta I|_{\max}=e\frac{A}{2},
 \qquad
 |\Delta E_{\rm ref}|_{\max}
 =e\frac{\Omega A}{2}.                        \tag{11}
\]

The interaction carries the opposite transient amount. At the endpoint,
`I(T)=I_0`, the interaction vanishes, and the total energy residual is zero.
A conservative bidirectional positive-action reserve is `I_0>A/2`.

Changing `e sigma` at phase `theta` costs the exact switching energy

\[
 \Delta H_{\rm switch}
 =\Delta(e\sigma)\kappa(1-\cos\theta)A.       \tag{12}
\]

It vanishes at a preregistered gate zero, not at an arbitrary phase.

## 5. Registered certificate gates

The source-locked certificate must report exactly forty-eight checks.

### Provenance

- **C1--C7:** the seven source hashes match section 2.
- **C8:** this protocol hash matches the pre-run lock embedded in the frozen
  certificate before its first execution.

### Hamiltonian lift

- **C9:** the positive-amplitude ternary embedding is injective.
- **C10:** equation (1) reproduces `A=a^2 Q/2` on all nine ternary pairs.
- **C11:** the registered quarter-turn obeys `R^2=-I`.
- **C12:** the carrier Hamilton equations have generator proportional to `R`.
- **C13:** `theta=Omega t` is the exact clock solution.
- **C14:** `A` is invariant under the exact carrier flow.
- **C15:** equation (6) solves the clock-action equation.
- **C16:** substitution of (6) makes the total Hamiltonian phase independent.
- **C17:** one cycle returns the clock phase to a gate zero.
- **C18:** equation (8) is the exact integrated carrier angle.
- **C19:** `nu/Omega=1` makes the inactive branch identity.
- **C20:** `kappa/Omega=1/4` makes the forward active branch `R`.
- **C21:** the reverse active branch is `R^-1`.
- **C22:** the inactive branch fixes all nine embedded ternary pairs.
- **C23:** the forward branch maps all nine embeddings exactly as FTD-0872.
- **C24:** the reverse branch maps all nine embeddings by the exact inverse.
- **C25:** ready emission sends `(s,0)` to `(0,s)` for all ternary `s`.
- **C26:** reciprocal absorption sends `(0,s)` to `(s,0)` for all ternary `s`.

### Energy and work ledger

- **C27:** carrier action is unchanged in all three branches.
- **C28:** endpoint carrier energy `nu A` is unchanged.
- **C29:** the physical record-energy coefficient is the explicitly imposed
  positive scale `nu a^2/2`.
- **C30:** the maximum absolute clock-action excursion is `e A/2`.
- **C31:** the maximum absolute reference-energy exchange is `e Omega A/2`.
- **C32:** the maximum absolute interaction energy equals that exchange.
- **C33:** the reference action returns exactly at the endpoint.
- **C34:** the interaction energy vanishes exactly at the endpoint.
- **C35:** the endpoint total-energy residual is zero.
- **C36:** switching eligibility/orientation at a gate zero costs zero in the
  registered interaction-energy account.
- **C37:** the same nontrivial switch away from gate zero generally has
  nonzero cost.
- **C38:** `I_0>A/2` is a sufficient positive-action reserve for either
  orientation.

### Minimum and boundary

- **C39:** one carrier pair plus one independent clock pair gives four
  continuous phase-space dimensions.
- **C40:** within the registered independent-phase-gate class, fewer than four
  dimensions cannot contain both pairs.
- **C41:** repeating the active forward cycle gives `R^2=-I`, not one-shot
  hold.
- **C42:** because the label norm remains nonzero after emission, norm-only
  eligibility cannot stop the next cycle.
- **C43:** the protocol contains `HARMONIC_ACTUATOR_STATUS=IMPOSED_REFERENCE`.
- **C44:** the protocol contains `DYNAMIC_ONE_SHOT_SCHEDULER=OPEN`.
- **C45:** the protocol contains `PRODUCTION_COUPLING=NONE`.
- **C46:** the protocol contains `GSTAR_ROLE=SEPARATE_CALENDAR_NOT_ACTUATOR`.
- **C47:** the protocol contains `BORN_BELL_STATUS=UNTOUCHED`.
- **C48:** the terminal verdict is emitted only if C1--C47 all pass.

## 6. Frozen interpretation

If all forty-eight gates pass, the permitted result is:

- **[THEOREM, CONDITIONAL]** the FTD-0872 ternary quarter-turn has an exact
  autonomous Hamiltonian lift on one carrier pair plus one clock pair;
- **[THEOREM]** the reference clock/inter-action energy loan is explicit,
  bounded, and returned at a complete gate cycle;
- **[THEOREM]** the minimum positive winding in the registered class is
  `nu=Omega`, `kappa=Omega/4`;
- **[IMPOSED]** the harmonic Hamiltonian and amplitude embedding set the
  physical scale and are not substrate derivations;
- **[CLOSED NEGATIVE]** repeating the active map or using invariant norm alone
  supplies an autonomous one-shot scheduler;
- **[OPEN]** gate-zero eligibility acquisition/release, backpressure-safe
  handoff, native carrier/clock formation, protected cubic transport,
  production coupling, robustness, and synchronization to the separate
  quartic `G*` calendar.

The result consumes the already-booked `SEL-CA-PHASE-RAIL` reference type. It
does not add a selected type or retire the native-production debt.

## 7. Frozen outcome rule

- **Outcome A:** `48/48`; book the scoped conditional theorem and an isolated
  `ftd::eft` reference witness.
- **Outcome B:** all provenance checks pass but any mathematical gate fails;
  book the counterexample and no theorem.
- **Execution invalid:** a source/hash mismatch, exception, wrong check count,
  or scope-marker failure; preserve the run and preregister any repair.

No post-run coefficient change, tolerance change, source substitution, or
scope promotion is permitted.

## 8. Scope markers

```text
HARMONIC_ACTUATOR_STATUS=IMPOSED_REFERENCE
DYNAMIC_ONE_SHOT_SCHEDULER=OPEN
PRODUCTION_COUPLING=NONE
GSTAR_ROLE=SEPARATE_CALENDAR_NOT_ACTUATOR
BORN_BELL_STATUS=UNTOUCHED
```

## 9. Pre-run lock

The exact SHA-256 of this byte-frozen protocol must be embedded in the
certificate and recorded in the preregistration manifest before first
execution. Later outcome prose must not alter this evidence hash.
