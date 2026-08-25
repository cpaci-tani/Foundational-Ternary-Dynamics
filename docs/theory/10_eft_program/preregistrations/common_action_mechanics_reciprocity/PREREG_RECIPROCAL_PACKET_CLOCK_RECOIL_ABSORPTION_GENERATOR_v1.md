# Preregistration — Reciprocal packet/clock/recoil absorption generator v1

**Date frozen:** 2026-08-24  
**Campaign status:** pre-execution lock  
**Ledger status:** no FTD identifier or claim row reserved  
**Production status:** no engine mutation authorized

## 1. Question

Does one local type-2 discrete generating function exist that absorbs a
complete C4 field packet into a maintained body clock, transfers its declared
translation charge to matter, preserves exact total energy and symplectic
structure, retains an exact inverse, and exposes rather than hides the
remaining field/clock/recoil scale compliance?

The test is conditional on a complete incoming packet of energy `d Gamma`, a
declared packet translation charge `p`, a regular body clock pair
`(theta,I)`, and a material translation pair `(X,P)` with positive kinetic
energy `K(P)`. It does not assume that the current microscopic action derives
`Gamma`, `p`, `omega`, `K`, or the absorption trigger.

## 2. Frozen source chain

The certificate may import or compare only:

1. `proof_c4_field_packet_reserve_current_and_atomic_clock_debit.py`, SHA-256
   `F58075539C396815F3942A70EE58A17AC04F139B4E205514666858D284CEADAB`;
2. `proof_clocked_remainder_recoil_noether_boundary.py`, SHA-256
   `B73C82F9853123732A539E86760D14C4B8DE9DADBFA6CA633649D5A998434C44`;
3. `proof_cotangent_charged_pole_reciprocal_alpha_measurement_protocol.py`,
   SHA-256
   `F903FF32FE4E38BB4EA5BFD6907A79DFA00C5F524E4B4624309C23D03A65EF87`;
   and
4. this preregistration's pre-execution hash.

No numerical search, target comparison, root substitution, or empirical
coupling may enter.

## 3. Frozen candidate generator

Let one admitted absorption batch contain `d` complete packets, total field
energy

\[
 E_F=d\Gamma,\qquad d\in\mathbb N_{>0},\quad\Gamma>0,    \tag{P1}
\]

and total declared field translation charge $p\in\mathbb R^3$. The body has
clock pair $(\theta,I)$ with Hamiltonian $\omega I$, $\omega>0$, and
translation pair $(X,P)$ with differentiable positive kinetic energy $K(P)$.

Freeze the type-2 generating function

\[
 \boxed{
 F_2(\theta,X;I',P')
 =\theta I'+X\cdot(P'-p)
 -{\theta\over\omega}
 \left[d\Gamma+K(P'-p)-K(P')\right].}                  \tag{P2}
\]

The packet ownership branch is `field -> absorbed-history/clock-port`; the
complete packet identities, phases, route, `d`, and `p` remain in retained
history until inverse emission.

## 4. Exact acceptance gates

### G1 — frozen integrity and target blindness

- All frozen hashes match.
- The certificate is exact symbolic/rational code only.
- No alpha, master-root, CODATA, or desired clock value is an input.

### G2 — canonical map from one generator

Differentiating equation (P2) must give

\[
 P'=P+p,\qquad \theta'=\theta,                          \tag{P3}
\]

\[
 I'=I+{d\Gamma+K(P)-K(P+p)\over\omega},                \tag{P4}
\]

\[
 X'=X-{\theta\over\omega}
 [\nabla K(P)-\nabla K(P+p)].                            \tag{P5}
\]

The Jacobian must preserve the canonical symplectic matrix. Equation (P5)
may not be dropped away from the registered seam.

### G3 — exact energy and translation-charge exchange

For arbitrary admissible variables,

\[
 \omega I+K(P)+d\Gamma
 =\omega I'+K(P').                                     \tag{P6}
\]

If the incoming field owns translation charge $p$ and the absorbed field owns
zero, then

\[
 P+p=P'                                                \tag{P7}
\]

must preserve total declared translation charge. No identification of $p$
with raw $E\times B$ is permitted.

### G4 — seam locality, admission, and inverse

At the clock crossing $\theta=0$, equation (P5) must give $X'=X$. The branch
is admitted only when equation (P4) leaves $I'\ge0$ and all requested packet
identities are field-owned. Underfunding or missing ownership fails before
mutation. Algebraic inversion must recover $(\theta,I,X,P)$ and restore the
same packet histories.

### G5 — quadratic recoil specialization

For

\[
 K(P)={|P|^2\over2m},\qquad m>0,                        \tag{P8}
\]

the clock-energy gain must be

\[
 \omega(I'-I)
 =d\Gamma-{2P\cdot p+|p|^2\over2m}.                    \tag{P9}
\]

At rest this partitions packet energy into clock energy plus recoil energy.
For a momentum-neutral counterpropagating batch, $p=0$ and the entire
$d\Gamma$ funds clock action.

### G6 — scalar gravity-source continuity

If the scalar gravity constraint reads the complete local energy owner rather
than a sector-specific label, equation (P6) must make its $T_{00}$ source
continuous through absorption. The certificate must explicitly leave the
tensor stress handoff, constraint propagation, and nonlinear gravity open.

### G7 — coupling compliance, not prediction

If one absorbed batch creates one clock action quantum $I_*$ from a rest body,
then

\[
 \omega I_*=d\Gamma-{|p|^2\over2m}.                    \tag{P10}
\]

Together with $\chi_{\rm EM}=\Gamma/I_*$, the allowed conditional relation is

\[
 \boxed{
 \chi_{\rm EM}
 ={\omega\over d-|p|^2/(2m\Gamma)}.}                   \tag{P11}
\]

For $p=0$, this reduces to $\chi_{\rm EM}=\omega/d$. No value of any symbol
may be chosen or compared with experiment.

## 5. Predeclared outcomes

- **Outcome A — native absorption vertex:** every gate passes and equation
  (P2), including its packet trigger and coefficients, is forced by the
  existing microscopic action without a new selection.
- **Outcome B — exact selected common-action vertex:** every mathematical,
  ownership, and disclosure gate passes, but the generator, packet momentum,
  trigger, or scale compliance remains selected.
- **Outcome C — canonical/energy failure:** G2--G6 cannot all hold for the
  frozen generator.
- **Outcome D — invalid execution:** a hash, source, or disclosure lock fails.

## 6. Explicit exclusions

Passing this protocol does not derive the field packet, stable matter, the
clock frequency or inertia, spin-2 gravity, lensing, native Born preparation,
multipartite no-signalling, alpha, or framework completeness. It tests one
reciprocal interaction vertex and its honest boundary.
