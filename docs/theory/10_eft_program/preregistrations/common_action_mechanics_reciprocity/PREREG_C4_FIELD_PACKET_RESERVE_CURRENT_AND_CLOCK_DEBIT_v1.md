# Preregistration — C4 field-packet reserve current and clock debit v1

**Date frozen:** 2026-08-24  
**Campaign status:** pre-execution lock  
**Ledger status:** no FTD identifier or claim row reserved  
**Production status:** no engine mutation authorized

## 1. Question

Can the already certified half-admitted C4 Maxwell carrier instantiate the
native nonnegative reserve density, Moore-local signed current,
phase-complete ownership, atomic debit, refill, and exact reverse transport
left open by FTD-0999, without reading a target coupling, clock energy,
amplitude, Born weight, or master root?

This campaign tests a finite carrier/interface theorem. It does **not** test
whether the common microscopic action selects the carrier, its C4 parity
schedule, its field metric, its absorption vertex, or the relative scale of
field and clock energy.

## 2. Frozen source chain

The proof may import only the exact finite definitions exposed by:

1. `proof_c4_phase_parity_half_admitted_two_polarization_carrier.py`;
2. `proof_c4_half_admitted_energy_current_momentum_boundary.py`;
3. `proof_cotangent_handed_directional_radiation_port.py`;
4. `proof_global_c3_cotangent_layer_hodge_maxwell_target.py`; and
5. the FTD-0999 scalar resource law as a comparison contract.

The implementation must record SHA-256 hashes of the four imported proof
sources. A later source-byte change invalidates the locked execution rather
than silently changing this result.

## 3. Frozen definitions

For each registered outgoing carrier, merge the two co-located C4 phase bands
within each handed flag. The prior theorem fixes eight groups with

\[
 h_a={1\over8},\qquad \sum_a h_a=1.                     \tag{P1}
\]

For tick `n`, define the microscopic reserve density

\[
 b_n(x)=\sum_a h_a\,\mathbf1[x_a(n)=x].                  \tag{P2}
\]

For every group transition `x -> y`, define the antisymmetric one-tick bond
current by adding `+h_a` to `J_n(x,y)` and `-h_a` to
`J_n(y,x)`. Holds contribute zero.

The current convention is outward-positive, so the pointwise continuity gate
is

\[
 b_{n+1}(x)-b_n(x)+\sum_yJ_n(x,y)=0.                    \tag{P3}
\]

For a finite domain `Omega`, inward boundary supply is

\[
 \Phi_n(\Omega)=-\sum_{x\in\Omega,y\notin\Omega}J_n(x,y), \tag{P4}
\]

and must equal the change of reserve inside `Omega`.

An atomic packet debit is a declared ownership swap of complete retained
packet identities between `reserve` and `clock-port`. The event is admitted
only when every requested identity is locally reserve-owned before mutation.
The inverse swaps the same identities back. No packet payload may be copied,
deleted, or synthesized by the ownership operation.

## 4. Exact acceptance gates

### G1 — frozen-source integrity

- The preregistration and imported source hashes are recorded.
- The proof performs no floating-point calculation, fit, sweep, or target
  comparison.

### G2 — positive phase-complete density

Across the full registered frame/chirality/phase/stage/orientation/parity
census:

- exactly eight groups exist;
- every group has energy `1/8`;
- `b_n(x)>=0` pointwise and `sum_x b_n(x)=1`;
- every group retains two C4-separated records and a unique handed flag; and
- the group update has an exact inverse.

### G3 — local discrete continuity

For every checked tick:

- every transition is a hold or one SC hop;
- `J(x,y)=-J(y,x)`;
- equation (P3) holds at every site in the finite support union;
- equation (P4) holds for coordinate half-spaces and the full support; and
- the six-tick first moment is the already certified `r`, hence mean reserve
  current `r/6`.

### G4 — atomic ownership and no double spend

For finite packet populations and every requested batch size:

- an underfunded batch fails before mutation;
- an admitted batch changes reserve/clock counts by `(-D,+D)`;
- packet identities and phase payloads are unchanged;
- inverse execution restores the entire owner map;
- disjoint debits commute; and
- overlapping same-tick debits are rejected atomically.

### G5 — FTD-0999 count law

For packet energy `Gamma`, incoming boundary packets `Phi`, source-released
packets `U`, and clock-debited packets `D`, the ownership counts must give

\[
 B_{n+1}=B_n+\Phi_n+U_n-D_n                              \tag{P5}
\]

in packet units and the same equation multiplied by `Gamma` in energy units.
Negative boundary flow must be represented as explicit reserve-to-environment
ownership, not a negative packet.

### G6 — scale-compliance boundary

If one maintained receiver clock quantum has action `I_*`, frequency
`omega_0`, and energy `e=omega_0 I_*`, then a debit of `d` complete field
packets is compatible exactly when

\[
 \omega_0 I_*=d\Gamma.                                  \tag{P6}
\]

The proof may derive the conditional identity

\[
 \chi_{\rm EM}={\Gamma\over I_*}={\omega_0\over d}       \tag{P7}
\]

but must state that neither `omega_0`, `d`, `Gamma/I_*`, nor alpha is fixed.
No comparison with `x_+`, CODATA, or any measured coupling is permitted.

## 5. Predeclared outcomes

- **Outcome A — full native reserve closure:** G1--G6 pass and the existing
  common action is shown to select the density, current, ownership transfer,
  and scale compliance without a new choice.
- **Outcome B — exact carrier/interface, action-scale open:** G1--G6 pass as
  finite theorems, but carrier admission, absorption, or equation (P6) remains
  selected rather than action-derived.
- **Outcome C — continuity/ownership failure:** the carrier fails at least one
  of G2--G5.
- **Outcome D — invalid execution:** a frozen source, implementation, or
  disclosure gate fails.

The expected result is not frozen. The first conforming execution determines
the outcome.

## 6. Explicit exclusions

Even Outcome A would not by itself establish nonlinear Maxwell dynamics,
Lorentz force, stable matter formation, gravity, Born/no-signalling,
biological memory, or framework completeness. Outcome B may narrow the common
action to one energy-compliance coefficient but cannot be reported as a
native alpha derivation.
