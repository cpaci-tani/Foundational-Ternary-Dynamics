# Metastable matter-object predicate contract v1

**Ledger ID:** FTD-0743  
**Status:** `[SCOPE / INSTRUMENT CONTRACT — NO EXISTENCE CLAIM]`  
**Date:** 2026-07-29  
**Governing charter:** FTD-0740  
**Roadmap baseline:** FTD-0742  
**Production status:** unchanged

## 1. Purpose

FTD currently has favorable finite-horizon histories, exact reciprocal
transactions, and local robustness results. Those facts do not by themselves
define a matter family. This contract fixes what a future M3 classifier must
mean before its numerical scales or tolerances are chosen.

The contract prevents four circular moves:

1. recognizing an object only after inspecting its future trajectory;
2. changing the family boundary until a preferred history survives;
3. calling every field value inside a selected radius part of the object;
4. replacing an open family by a finite list of favorable states.

This contract itself ran no numerical campaign. FTD-0739 subsequently closed
M1 at its registered selected scope. The unresolved quantities in Section 8
may be fixed only through M2 identification of the environmentally bound field
component; the FTD-0739 radius-12 tail gate alone cannot define it.

## 2. State, evolution, and equivalence

In a declared finite-volume research sector, let

\[
X=(s,C,F)\in\Omega_L,
\qquad T_L:\Omega_L\longrightarrow\Omega_L
\]

be the complete instantaneous state and the selected complete tick. On the
candidate sector, `T_L` must be single-valued. If reverse claims are made, its
restriction to that sector must also be state-only invertible.

Let

\[
\mathcal G_L=\mathcal T_L\rtimes\mathcal O_{\rm cube}^{+}\times\Pi
\]

contain periodic or integer translations as appropriate, proper cubic
rotations, and polarity/graph-preserving constituent relabellings. A separate
polarity-conjugation operation is a covariance test, not an identification of
positive and negative objects.

The candidate family lives in the quotient `Omega_L/G_L`. Translation of an
object, rotation of the whole preparation, or consistent relabelling cannot
change family membership.

## 3. Required instantaneous observer

A future classifier must first freeze an observer

\[
\mathfrak O_\theta(X)
 =\big(a(X),K(X),q(X),F_{\rm b}(X),F_{\rm o}(X),F_{\rm bg}(X),
       \boldsymbol\mu(X)\big),
\]

with the following meanings.

- `a(X)` is an equivariant object centre or anchor.
- `K(X)` is a bounded core support selected from the present state.
- `q(X)` is an internal quotient coordinate or complete family feature.
- `F_b(X)` is the candidate bound/co-moving field component.
- `F_o(X)` is detached outgoing field.
- `F_bg(X)` is the declared control/background component.
- `mu(X)=(mu_1,...,mu_r)` is a vector of signed membership margins.

Every output is a deterministic function of the current complete state. A
stored route label, launch time, seed name, future minimum, fitted worldline,
or forward replay tape is not an admissible observer input.

The field decomposition must reconstruct the declared excess field exactly,

\[
F-F_{\rm control}=F_{\rm b}+F_{\rm o}+F_{\rm bg},
\]

under its registered convention. Spatial location alone cannot define these
terms. In particular, “near” is not equivalent to bound, “far” is not
equivalent to radiation, and “trailing” is not equivalent to a wake.

## 4. State-only family predicate

For fixed predeclared parameters `theta`, define

\[
P_\theta(X)=1
\quad\Longleftrightarrow\quad
X\in\mathcal S_\theta
\ \text{and}\ 
\min_j\mu_j(X)>0,
\]

where `S_theta` is the declared topology, event, and field-representation
sector. The strict inequalities give a nonzero classifier margin; equality is
a boundary state and does not count as certified membership.

The one-object family is

\[
\mathcal M_{\theta,L}
 =\{[X]_{\mathcal G_L}:P_\theta(X)=1\}.
\]

The predicate must satisfy exact or tolerance-qualified symmetry covariance,

\[
P_\theta(gX)=P_\theta(X),\qquad g\in\mathcal G_L,
\]

and its scalar margins must be invariant or permuted consistently. A
polarity-conjugate preparation must receive the conjugate classification.

Negative core energy may be one component of `mu` for the current compact-pair
branch. It cannot be the sole predicate and is not a universal definition of
matter.

## 5. Persistence and metastability

For a state in the family, define the forward exit time

\[
\tau_\theta(X)
 =\inf\{k\ge 0:P_\theta(T_L^kX)=0\},
\]

with `tau=infinity` if no exit occurs. For a registered relative-open
neighborhood `U subset M_{theta,L}` in the quotient metric of the declared
hybrid-state sector, define

\[
\tau_{\min}(U)=\inf_{X\in U}\tau_\theta(X).
\]

A finite-horizon M3 certificate requires a declared `U` with nonzero radius,
strict initial margins, and `tau_min(U)>H` for a horizon `H` preceding any
boundary or environmental return. A finite sample can supply numerical
evidence for that statement only over its registered sample; it cannot prove
the universal infimum.

The following statuses must remain distinct:

1. **trajectory survival:** one state remains classified through `H`;
2. **sampled robustness:** every registered perturbation remains classified;
3. **finite-time open neighborhood:** regularity plus strict margins proves a
   nonzero neighborhood for the tested finite horizon;
4. **volume-stable metastability:** the neighborhood and survival lower bound
   do not collapse over the registered volume/horizon ladder;
5. **invariant family:** `T(M)=M` is proved, rather than sampled.

Only levels 3--5 qualify M3, and the exact achieved level must be stated.

## 6. Formation, identity, and decay times

When `T` is state-only invertible, entry and exit are properties of complete
states rather than hidden history:

\[
\begin{aligned}
\operatorname{enter}(X)&=[P_\theta(X)=1\land
 P_\theta(T^{-1}X)=0],\\
\operatorname{exit}(X)&=[P_\theta(X)=1\land
 P_\theta(TX)=0].
\end{aligned}
\]

This does not prove that native dynamics ever enter the family. It only makes
formation and decay well posed once the family and complete action exist.

For deterministic evolution, a decay rate is not an intrinsic random number.
It requires either a declared ensemble measure, a coarse-graining, or a
controlled resonance/spectral construction. The exact deterministic
observable is the exit-time function `tau_theta(X)`.

## 7. Environmental and volume compatibility

The object predicate must not depend on periodic return or a quotient-wide
preparation. For nested volumes whose causal diamonds agree, the embedding
`iota_{L,L'}` must give the same classification and local margins before first
possible exterior contact:

\[
P_{\theta,L}(X)=P_{\theta,L'}(\iota_{L,L'}X).
\]

At minimum, the M3 campaign must track with increasing causal buffer:

- the core support and its localization margin;
- the bound-field contribution and its co-motion defect;
- detached outward energy/current and first-passage times;
- inward return through every registered enclosing surface;
- the minimum family margin and perturbation radius.

Environmental support does not automatically disqualify matter. It changes
the persistence class. The observer must distinguish at least:

1. **autonomous metastable object:** after its preparation transient, no
   nonvanishing inward sustaining flux is required over the registered causal
   horizon;
2. **throughput-maintained object:** a localized state-identifiable family is
   maintained by balanced inflow, internal conversion, and outflow, as in the
   abstract dynamics of a flame or radiating star;
3. **constraint-maintained object:** an external pressure, membrane, trap, or
   surrounding medium supplies a persistent boundary condition;
4. **transient localization:** concentration disperses when the launching
   disturbance leaves and has no state-identifiable family.

Classes 2 and 3 can qualify as environmentally sustained matter patterns if
the sustaining environment and its exact ledger are explicit. They cannot by
themselves establish an isolated particle ontology. At a larger scale the
object plus its sustaining environment may form another complete relational
system, so the distinction is hierarchical rather than a claim that one
boundary is ontologically absolute.

In a globally reversible complete dynamics, an apparent driven attractor for
the subsystem is compatible with information and energy being carried by the
environment. The full-state map remains subject to the declared inverse and
ledger tests.

## 8. Parameters intentionally unresolved until M1/M2

This contract does not yet choose:

1. the bound-field projector or nonlinear separator;
2. the core and shell radii;
3. the complete quotient metric and component weights;
4. membership-margin scales;
5. the admissible perturbation measure;
6. the volume/horizon ladder;
7. an ensemble measure for lifetime statistics;
8. a multi-object segmentation or counting rule.

Choosing these now would use quotient-dressed histories to define the object
that the finite-support campaign is supposed to discover. After M1/M2, each
choice must come from an exact identity, an independently calibrated control,
or a documented discovery/validation split. The held-out validation arms may
not alter `theta`.

## 9. Mandatory negative controls

The eventual classifier must reject or separately classify:

- source-free dispersing field packets;
- a static imposed source with no autonomous matter transaction;
- transient graph contact without energetic formation;
- a favorable trajectory sustained by periodic return;
- a core boost whose dressing remains behind;
- two separated objects mistaken for one diffuse object;
- a temporary near-field concentration with no open state neighborhood;
- a saved route label that cannot be reconstructed from current state.

Controls are evaluated by the same observer and tolerances as candidate
histories.

## 10. Anti-circularity and preregistration rules

Before the first M3 validation trajectory is inspected, the campaign must
freeze:

- the complete state representation and permitted sector;
- `a`, `K`, `q`, the field decomposition, every margin, and `P_theta`;
- symmetry and polarity actions;
- the quotient metric, perturbation set, volumes, horizons, and controls;
- numerical tolerances and convergence requirements;
- the verdict map below.

It may not use future samples to determine membership, minimize retrospectively
over unrecorded routes, exclude unfavorable phases after execution, alter the
energy zero, or replace failed controls with new controls under the same
identifier.

## 11. Verdict map

| outcome | permitted statement |
|---|---|
| observer/state reconstruction fails | candidate state or instrument is incomplete |
| symmetry, control, or volume-causality gate fails | classifier invalid at registered scope |
| only isolated trajectories survive | trajectory evidence; no matter family |
| sampled neighborhood survives but no theorem applies | finite sampled robustness only |
| strict margins plus regularity prove a finite-time open neighborhood | finite-time selected matter family |
| neighborhood and lifetime margins stabilize over increasing causal buffers | selected metastable matter candidate |
| exact invariance is proved | invariant family in the declared selected dynamics |

No row licenses physical particle, charge, species, mass, spin, statistics, or
Lorentz claims.

## 12. Consequence for the roadmap

M3 is no longer the vague instruction to “find something stable.” Its target
is a state-only, symmetry-covariant open family with a nonzero lifetime margin
that survives environmental and volume controls. The current record has
finite-time ingredients but not the complete predicate, bound-field separator,
or volume-stable certificate.

FTD-0739 subsequently closed M1 constructively at its selected scope;
FTD-0745's first M2 continuation failed its overextended shell-arrival horizon.
After the bounded CUDA arithmetic repair, FTD-0753 then closed a fresh
`L=321`, tick-312 three-ray causal-horizon witness constructively: radius 48 is
reached at tick 297 and remains outward before tick-313 contact. This is
positive M2 evidence but does not supply the unique state-only field separator
required by this contract.

FTD-0754 has now completed the licensed discovery action constructively. Its
state-only observer selects a finite-support minimum-energy Gauss dressing and
splits the centered residual readout into outgoing characteristic and
incoming-plus-radial background, while preserving the FTD-0753 histories
exactly. The result remains discovery evidence only. FTD-0755 is the licensed
next action: all new perturbation and volume histories remain unseen until its
separate validation protocol freezes `theta`. M3 remains open until held-out
validation supports a nonzero finite-time open neighborhood at the exact
achieved scope. The nonzero bound--residual interference measured in FTD-0754
was subsequently decomposed by the FTD-0754B analytic addendum into primitive
support-boundary exchange, face-centering correction, and integer-time
magnetic-readout correction. The centered total may not be used as a predicate
margin or silently assigned to either object or environment energy. FTD-0755
must keep internal energy, primitive boundary exchange, centered-readout
corrections, and environmental energy separate, and must freeze a support
radius ladder or prove a unique selected support before execution.

The exploratory interpretation and its decisive state-fibre tests are recorded
in
[EXPLR_RECURSIVE_MATTER_DERIVATION_AFTER_FTD0754_v1.md](EXPLR_RECURSIVE_MATTER_DERIVATION_AFTER_FTD0754_v1.md),
with the exact selected-observer ledger in
[THEOREM_STATE_ONLY_BOUNDARY_ENERGY_LEDGER_v1.md](../10_eft_program/derivations/THEOREM_STATE_ONLY_BOUNDARY_ENERGY_LEDGER_v1.md).
Those additions promote no M3 or particle claim and do not alter this
contract's held-out firewall.
