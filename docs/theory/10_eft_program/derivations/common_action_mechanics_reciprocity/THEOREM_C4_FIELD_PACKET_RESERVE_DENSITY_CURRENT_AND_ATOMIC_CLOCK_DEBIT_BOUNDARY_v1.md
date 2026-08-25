# C4 field-packet reserve density, current, and atomic clock-debit boundary v1

**Date:** 2026-08-24

**Status:** **[THEOREM, CONDITIONAL — POSITIVE PHASE-COMPLETE FIELD-PACKET RESERVE DENSITY]** +
**[THEOREM — EXACT POINTWISE DISCRETE CONTINUITY AND MOORE-LOCAL CURRENT]** +
**[THEOREM — ATOMIC WHOLE-PACKET DEBIT/REFILL/INVERSE]** +
**[THEOREM, CONDITIONAL — FTD-0999 RESOURCE LAW REALIZED IN PACKET UNITS]** +
**[THEOREM, CONDITIONAL — FIELD/CLOCK SCALE-COMPLIANCE IDENTITY]** +
**[OUTCOME B — EXACT CARRIER/INTERFACE; COMMON ACTION AND SCALE OPEN]**

**Production status:** unchanged

**Ledger status:** no FTD claim row minted

**Locked preregistration:**
[PREREG_C4_FIELD_PACKET_RESERVE_CURRENT_AND_CLOCK_DEBIT_v1.md](../../preregistrations/common_action_mechanics_reciprocity/PREREG_C4_FIELD_PACKET_RESERVE_CURRENT_AND_CLOCK_DEBIT_v1.md),
pre-execution SHA-256
`A76FA492E9B8DB022F0F708ABBC94EFD4F9372062E91C4D464A9D00568D81C80`.

**Exact certificate:**
[proof_c4_field_packet_reserve_current_and_atomic_clock_debit.py](../../../../../scripts/proofs/proof_c4_field_packet_reserve_current_and_atomic_clock_debit.py)
performs 2,046,451 exact checks. It exhausts all 24 ordered SC frames, both
propagation branches, all four C4 phase origins, all twelve internal stages,
both charge orientations, both parity schedules, and six successive ticks. It
also exhausts finite packet-debit populations and exact integer resource-law
fixtures. No floating-point value, fitted coefficient, measured coupling,
master root, or target clock energy enters.

---

## 1. The FTD-0999 hardware question

FTD-0999 proves that coherent clock growth must obey

\[
 B_{n+1}=B_n+\Phi_n+U_n-D_n,                             \tag{1}
\]

with atomic admission and retained inverse history. Its boundary is explicit:
a scalar $B_n$ is bookkeeping, not an owned phase-complete physical reserve.

The existing half-admitted C4 Maxwell carrier supplies a finite candidate. It
already has:

- a selected positive energy metric;
- eight phase-paired microscopic energy groups;
- local reversible hold/SC-hop dynamics;
- an exact two-polarization $c_{\rm eff}=1/6$ cone; and
- retained C4 phase and handedness on every group.

This theorem proves that those data instantiate every kinematic and ownership
clause of equation (1) in whole-packet units. It does not prove that the common
action selects the carrier, absorbs it into a clock, or fixes the field/clock
energy conversion.

---

## 2. Nonnegative microscopic reserve density

Under the selected C4-trivial field metric, the sixteen records form eight
phase-paired handed groups with

\[
 h_a={1\over8},\qquad h_a>0,qquad \sum_{a=1}^8h_a=1.    \tag{2}
\]

Let $x_a(n)$ be the position of group $a$ at global tick $n$. Define

\[
 \boxed{
 b_n(x)=\sum_{a=1}^8h_a\,\mathbf1[x_a(n)=x].}           \tag{3}
\]

Then, pointwise and exactly,

\[
 \boxed{b_n(x)\ge0,qquad \sum_xb_n(x)=1.}              \tag{4}
\]

The sum in equation (4) is over the finite support of the realized packet; no
completed infinite lattice is invoked.

Each group retains two co-located records whose C4 phases differ by two and a
unique cotangent handed flag. The movement permission is common to both phase
bands. Thus the density does not discard the information needed to reconstruct
the microstate: it is a readout of an explicitly retained phase-complete
carrier.

---

## 3. Exact Moore-local current

For a one-tick group transition $x_a(n)\to x_a(n+1)$, add $+h_a$ to
$J_n(x,y)$ and $-h_a$ to $J_n(y,x)$. A hold contributes zero. Consequently

\[
 J_n(x,y)=-J_n(y,x).                                     \tag{5}
\]

Every nonzero transition is one SC hop, so $J_n$ is strictly within the Moore
cone. Direct counting of arrivals and departures gives the pointwise identity

\[
 \boxed{
 b_{n+1}(x)-b_n(x)+\sum_yJ_n(x,y)=0.}                   \tag{6}
\]

This is a finite discrete continuity theorem, not a continuum approximation.

For any finite domain $\Omega$, define inward boundary supply by

\[
 \Phi_n(\Omega)
 =-\sum_{x\in\Omega,\ y\notin\Omega}J_n(x,y).           \tag{7}
\]

Summing equation (6) over $\Omega$ cancels internal antisymmetric currents and
gives

\[
 \boxed{
 \sum_{x\in\Omega}[b_{n+1}(x)-b_n(x)]=\Phi_n(\Omega).}  \tag{8}
\]

The certificate verifies equation (8) on the full finite support and every
coordinate half-space induced by every checked transition.

The six-tick first moment is

\[
 \sum_{j=0}^{5}\sum_{x,y}(y-x)J_{n+j}^{+}(x,y)=r,       \tag{9}
\]

where $J^+$ denotes the actual directed group transfers before
antisymmetrization. Hence the transported reserve current is the previously
certified

\[
 \boxed{J_E={r\over6}.}                                 \tag{10}
\]

The raw $E\times B=r/2$ readout remains three times equation (10) and is not
silently relabeled as energy current or canonical momentum.

---

## 4. Phase-complete inverse transport

The one-tick carrier map advances the cotangent flag and C4 phase and either
holds or performs one SC hop. From the output flag and phase, the previous
permission and tangent are unique. Therefore every group transition in
equation (6) has an exact inverse.

The density/current readout alone is not asserted to be invertible. The
physical carrier retains:

\[
 (\text{packet identity},\ \text{position},\ \text{flag},\
 \text{two C4 phases},\ \text{orientation},\ \text{stage}). \tag{11}
\]

Reverse transport acts on equation (11), not on the scalar density. This is
the required distinction between ontic ownership and an energy summary.

---

## 5. Atomic whole-packet debit

Let a finite set of complete packet identities have explicit owners such as
`environment`, `reserve`, `clock-port`, and `source`. A declared batch $S$ is
admitted only if every identity in $S$ is locally reserve-owned before any
mutation. The forward operation changes

\[
 (\mathrm{reserve},P_a)\longmapsto(\mathrm{clock\mbox{-}port},P_a),
 \qquad a\in S,                                         \tag{12}
\]

where $P_a$ is the complete phase payload in equation (11). The inverse
changes the same owners back.

Equation (12) proves:

- packet identity and phase payload are unchanged;
- a batch of size $D$ changes owner counts by $(-D,+D)$;
- an underfunded batch fails before mutation;
- disjoint batches commute;
- overlapping same-tick batches cannot spend the same identity twice; and
- inverse execution restores the complete owner map.

This is an ownership permutation, not erasure or copying. Selecting which
local identities constitute $S$ remains a routing/controller question.

---

## 6. Physical realization of the FTD-0999 count law

Let $B_n$ be the number of locally reserve-owned packets. Let:

- $\Phi_n=N_{\rm in}-N_{\rm out}$ count explicit boundary ownership
  transfers;
- $U_n$ count explicit source-to-reserve packet transfers; and
- $D_n$ count reserve-to-clock-port transfers.

Packet-number conservation gives

\[
 \boxed{B_{n+1}=B_n+\Phi_n+U_n-D_n.}                    \tag{13}
\]

If one complete packet has physical energy $\Gamma>0$, multiply equation
(13) by $\Gamma$ to obtain the energy form of FTD-0999. A negative $\Phi_n$
is an explicit reserve-to-environment transfer; it is never represented by a
negative-energy packet.

Thus the C4 field carrier supplies a concrete finite witness for the reserve
density, current, ownership, causal boundary flow, atomic debit, and reverse
transport demanded by FTD-0999.

This closure is conditional because $H_F=1$ and the half-admission schedule
are selected field structures, and because no common action yet derives the
absorption/clock-port transfer in equation (12).

---

## 7. Field/clock scale compliance

The finite packet makes the remaining scale question unusually sharp. Let one
maintained receiver clock quantum carry action $I_*$ and frequency
$\omega_0$, so its energy is

\[
 e=\omega_0I_*.                                         \tag{14}
\]

If an admitted receiver is funded by $d\in\mathbb N_{>0}$ complete field
packets, exact energy compliance requires

\[
 \boxed{\omega_0I_*=d\Gamma.}                           \tag{15}
\]

The reciprocal-alpha protocol defines

\[
 \chi_{\rm EM}={\Gamma\over I_*}.                       \tag{16}
\]

Combining equations (15)--(16) gives the conditional common-action identity

\[
 \boxed{\chi_{\rm EM}={\omega_0\over d}.}               \tag{17}
\]

With $c_{\rm eff}=1/6$, this would imply

\[
 \alpha_{\rm native}
 ={3\omega_0\over2\pi d}.                               \tag{18}
\]

Equations (17)--(18) are not coupling predictions. The common action has not
derived $\omega_0$, the packet multiplicity $d$, $\Gamma/I_*$, or the
absorption vertex. No value is compared with the master root or experiment.
The equations expose the exact compliance that a one-action construction must
force if electromagnetic packets physically maintain matter clocks.

---

## 8. Contribution to the one-action chain

The currently certified causal chain can now be written

\[
 \begin{aligned}
 &\text{phase-compatible histories}
 \longrightarrow \text{manifestation/current/stress source}\\
 &\longrightarrow \text{phase-complete Maxwell packet}
 \longrightarrow \text{positive causal reserve current}\\
 &\longrightarrow \text{atomic clock-port debit}
 \longrightarrow \text{maintained matter-clock energy}.
 \end{aligned}                                           \tag{19}
\]

The same manifestation stress moment already supplies the selected scalar/STF
gravity reference-action source, while the contextual Poincare map supplies
the prepared physical event frequency. Equation (19) therefore connects
manifestation, electromagnetic transport, and clock/matter maintenance by one
owned energy path rather than parallel ledgers.

It is still not one native action. The arrows in equation (19) presently mix
theorem-grade finite maps with selected field metric, movement admission,
absorption, gravity readouts, and prepared contextual routing.

---

## 9. Epistemic disposition

### Established exactly within the selected carrier

- nonnegative finite reserve density;
- exact pointwise discrete continuity;
- signed Moore-local current and finite-domain flux;
- phase-complete inverse transport;
- atomic whole-packet debit, refill, and double-spend exclusion;
- exact realization of the FTD-0999 law in packet units; and
- the scale-compliance identity (17).

### Still selected or open

1. derivation of the C4-trivial field metric from the microscopic action;
2. action selection of phase-parity half-admission;
3. nonlinear protection and scattering of complete packets;
4. native derivation of the selected reciprocal absorption vertex;
5. derivation of $\omega_0$, $d$, $\Gamma$, and $I_*$;
6. field Noether momentum, recoil, and Lorentz force;
7. coupling of the same finite action to the scalar/STF gravity constraints;
8. autonomous formation of the contextual Born history bank; and
9. production realization, robustness, and hiding of the global tick.

The preregistered outcome is therefore **Outcome B**: the field packet closes
the physical reserve carrier/interface at exact finite level, while action
selection and scale compliance remain open.

---

## 10. Next locked discriminator

Construct one local reversible absorption generator whose stationary map:

1. moves a complete incoming C4 packet into clock-port ownership;
2. increases the common body-clock action by exactly $d\Gamma/\omega_0$;
3. writes equal-and-opposite field/material translation charge;
4. preserves the source, aperture, route, and inverse history;
5. couples the same event stress to the scalar/STF gravity constraint sector;
6. does not read a desired outcome, amplitude, coupling, or clock phase; and
7. reduces to equation (17) without an independently chosen scale ratio.

A pass would be the first actual common interaction vertex joining field
energy, matter time, recoil, and gravity sourcing. Failure would identify the
minimal additional type or coefficient that the one-action programme must
price.

### Subsequent reciprocal-absorption result (2026-08-24)

The preregistered
[reciprocal packet/clock/recoil successor](THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
constructs one selected type-2 generator that transfers a complete packet
batch into clock action and material recoil. It preserves the canonical
two-form, total energy, declared translation charge, retained packet history,
and the exact inverse. For quadratic material energy,

\[
 \omega\,\Delta I
 =d\Gamma+K(P)-K(P+p),                                  \tag{20}
\]

so a rest absorption obeys

\[
 d\Gamma=\omega\,\Delta I+{|p|^2\over2m}.               \tag{21}
\]

This closes the reference absorption map, not its native origin. The trigger,
packet momentum $p$, inertia $m$, clock frequency $\omega$, field/action
scale, finite ternary realization, tensor-stress handoff, and nonlinear
gravity remain selected or open. The successor therefore does not promote
this theorem beyond Outcome B.
