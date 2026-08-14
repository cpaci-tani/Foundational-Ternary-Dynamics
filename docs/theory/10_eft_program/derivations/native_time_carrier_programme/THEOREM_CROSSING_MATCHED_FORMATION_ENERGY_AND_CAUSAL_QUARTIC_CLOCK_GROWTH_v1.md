# Theorem — Crossing-matched formation energy and causal quartic-clock growth v1

**Identifier:** `FTD-0995/0996`  
**Date:** 2026-08-12  
**Status:** `[THEOREM, CONDITIONAL — NECESSARY/SUFFICIENT FORMATION-CLOCK COMPLIANCE]` +
`[THEOREM, CONDITIONAL — EXACT LOCAL COHERENT GROWTH AND INVERSE]` +
`[THEOREM — MOORE-CAUSAL FRONT / INDEPENDENT-FRONTIER WORK ADDITIVITY]` +
`[THEOREM, CONDITIONAL — CRITICAL-QUARTIC G* CADENCE INHERITANCE]` +
`[THEOREM, CONDITIONAL — WORK MISMATCH DETUNES THE QUARTIC CLOCK]` +
`[BOUNDARY — AUTONOMOUS MATCHING, ROBUSTNESS, CONTROLLER, AND PRODUCTION OPEN]`  
**Parent:** `FTD-0993/0994`

## Result

The minimum exact local clock-growth law is a crossing-matched energy
transaction. It does not copy an arbitrary canonical state. It admits a new
site only when the local work released by forming that site already equals
the donor clock's energy at a kinetic crossing.

In mass-normalized coordinates, let an occupied donor `x` obey

\[
 q_x=0,qquad p_x\ne0,qquad
 \sigma=\operatorname{sgn}(p_x),                         \tag{1}
\]

and let an adjacent prospective receiver `y` obey `q_y=p_y=0`. Let `W_y` be
the exact FTD-0992 occupancy-flip work and define the positive released work

\[
 U_y=-W_y>0.                                             \tag{2}
\]

The FTD-0994 local Cartesian seed gives

\[
 q_y'=0,qquad p_y'=\sigma\sqrt{2U_y}.                  \tag{3}
\]

Define the local compliance scalar

\[
 \boxed{C_{xy}=2U_y-p_x^2.}                             \tag{4}
\]

Then, on the admitted sign branch,

\[
 \boxed{C_{xy}=0
 \quad\Longleftrightarrow\quad
 (q_y',p_y')=(q_x,p_x).}                                \tag{5}
\]

Equation (5) is the law. It is necessary and sufficient within the registered
seed class. No coefficient was fitted. With kinetic energy `p^2/(2m)`, the
same statement is

\[
 p_y'=\sigma\sqrt{2mU_y},
 \qquad
 \boxed{C_{xy}^{(m)}=2mU_y-p_x^2=0.}                   \tag{6}
\]

The energy ledger is exact if the membrane/source sector loses the same
`U_y`:

\[
 \Delta H_{\rm membrane}=-U_y,
 \qquad
 \Delta H_{\rm receiver}=+U_y,
 \qquad
 \Delta H_{\rm total}=0.                               \tag{7}
\]

At a later identical crossing, the inverse `-sigma` shear clears the
receiver and the reverse occupancy flip restores the membrane energy. The
orientation/aperture record must remain available until that reversal.

For a connected component with identical Cartesian state at every occupied
site, the FTD-0990 occupancy Laplacian annihilates the uniform vector. Every
site therefore follows the same onsite Hamiltonian equation. An admitted
receiver begins in precisely that same state, so the enlarged uniform
manifold is invariant. Repeating local admitted transactions grows exact
coherence causally, never faster than the Moore graph distance from the seed.

For the selected critical quartic onsite Hamiltonian

\[
 h_4(q,p)={p^2\over2m}+\lambda q^4,                     \tag{8}
\]

equation (5) transfers the amplitude and orientation as well as phase. Hence
the receiver inherits

\[
 \boxed{TA=\sqrt\pi G^*\sqrt{m\over2\lambda}}          \tag{9}
\]

without the admission law reading `G*`, a period target, or a target phase.
This is a conditional physical gearbox from local formation energy to an
already-selected quartic calendar. It is not a derivation of critical
quarticity or `G*`.

## Certificate of record

- Parent protocol:
  [`PREREG_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md),
  SHA-256
  `B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F`.
- Immutable parent proof:
  [`proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth.py`](../../../../../scripts/proofs/proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth.py),
  SHA-256
  `17DE90F5BBEFD1BDEFC22AACB236C024FBE8446BD5DE765AA7F95B79EDD87574`.
- First locked execution: `84/88`; all new physical/mathematical gates
  passed, while four verifier representations failed.
- Repair protocol:
  [`PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md),
  SHA-256
  `854C1EDA934DA8CDFA1B0C2649EF9CE2A20C4D6A30731D28A2C285BDB7379554`.
- Repair wrapper:
  [`proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth_v2.py`](../../../../../scripts/proofs/proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth_v2.py),
  SHA-256
  `9104D6F3FD842C8BF09C7F35BC080BCCCA96EFCC0F4022CAAAF9DF3846B130E2`.
- Final execution: inherited `88/88` plus repair integrity `17/17`,
  **Outcome B — exact compliance-surface growth / autonomous matching open**.

## 1. Necessity and sufficiency

Write `p_x=sigma v` with `v>0`. Equation (3) agrees with the donor exactly
when

\[
 \sigma\sqrt{2U_y}=\sigma v.
\]

Because both magnitudes are positive, squaring introduces no extra admitted
branch and gives

\[
 2U_y=v^2=p_x^2,
\]

which is equation (4). Conversely, equation (4) and the retained sign give
equation (5). The four exclusions are exact:

- `U_y<=0` supplies no positive real clock energy;
- `p_x=0` supplies no crossing orientation;
- the wrong sign produces the opposite momentum-axis point; and
- `C_xy!=0` produces a nonzero amplitude mismatch.

The gate compares two current local scalars. It reads no future profile,
remote body aggregate, completed target state, measurement context, setting,
outcome, probability, Born weight, or `G*` value.

## 2. Exact energy and inverse

At the registered zero receiver seam, equation (3) gives

\[
 {p_y'^2\over2}=U_y.
\]

Together with the occupancy-flip loss `-U_y`, this proves equation (7). At a
later matching crossing, applying the opposite generator gives

\[
 p_y''=p_y'-\sigma\sqrt{2U_y}=0.                       \tag{10}
\]

The reverse occupancy flip costs `+U_y`, exactly receiving the returned clock
energy. This is an environment-complete inverse only if the aperture sign,
occupancy transition, work value, and other source variables needed by the
FTD-0994 generator are retained.

The simple ledger is a crossing theorem. Away from a zero receiver momentum,
the clock-energy change contains

\[
 U_y+\sigma p_y\sqrt{2U_y},                             \tag{11}
\]

and cannot be booked as `U_y` alone.

## 3. Why the coherent manifold persists

For occupied component `S`, the membrane stiffness has incidence form

\[
 K_m=B^TG_mB.
\]

Every incidence row annihilates `1_S`, so

\[
 K_m\mathbf1_S=0.                                      \tag{12}
\]

If all sites share `(q,p)`, every membrane difference and bond current
vanishes. For any identical onsite law

\[
 h(q,p)={p^2\over2}+V(q),
\]

all sites then obey

\[
 \dot q=p,
 \qquad
 \dot p=-V'(q).                                        \tag{13}
\]

Equation (5) inserts the receiver with the same initial data. Uniqueness of
the onsite flow keeps the enlarged component on the uniform manifold for as
long as the occupancy and identical onsite law remain fixed.

This is conservative locking in the exact-manifold sense. It is not
attraction: a perturbed site is not proved to return to the manifold.

## 4. Causal frontier and concurrency

Each growth event touches a prospective site and its Moore neighborhood. By
induction, after `r` admitted local events along a path, a coherent descendant
can lie no farther than Moore graph distance `r` from the initial seed.

Simultaneous growth can preserve the exact work ledger when the prospective
frontier `F_n` is Moore-independent. No two sites of `F_n` then share a C18
bond, their changed-edge sets are disjoint, and FTD-0992 gives

\[
 \boxed{W_{F_n}=\sum_{y\in F_n}W_y.}                   \tag{14}
\]

A coordinate-parity color class supplies a deterministic reference witness.
Choosing which class fires at which crossing is still controller/scheduling
structure. The theorem derives the conditional transaction and causal bound,
not that physical scheduler.

## 5. The conditional G* gearbox

For equation (8), a turning amplitude `A` has energy

\[
 E=\lambda A^4.
\]

At `q=0`,

\[
 |p|=\sqrt{2mE}=\sqrt{2m\lambda}\,A^2.                 \tag{15}
\]

The normalized quartic-shell coordinates are

\[
 x={q\over A},
 \qquad
 y={p\over\sqrt{2m\lambda}A^2}.
\]

At the admitted crossing they equal `(0,sigma)` for both donor and receiver.
Consequently both occupy the same oriented point of

\[
 y^2=1-x^4,
\]

with the same amplitude. Their real-oval traversal and the oriented map to
the conductor-32 CM curve are therefore identical, conditional on FTD-0827.
Equation (9) is inherited because the state is inherited. Nothing in
equations (1)--(7) evaluates or targets `G*`.

This closes one part of the previously missing gearbox:

\[
 \text{matched local formation work}
 \Longrightarrow
 \text{same quartic state}
 \Longrightarrow
 \text{same oriented G* calendar}.                     \tag{16}
\]

The first arrow is conditional on exact compliance; the second is the prior
quartic/CM theorem. Neither arrow derives the clock hardware from production.

## 6. Mismatch and harmonic control

Let the receiver work differ by the positive ratio

\[
 r={U_y\over E_x}.
\]

For the quartic law,

\[
 {A_y\over A_x}=r^{1/4},
 \qquad
 \boxed{{T_y\over T_x}=r^{-1/4}.}                      \tag{17}
\]

Thus a generic formation-work mismatch immediately becomes cadence detuning.
The exact law must fail closed, export and repair the mismatch through a
separately derived positive local port, or relinquish exact quartic
coherence.

An identical harmonic onsite clock has amplitude-independent period. It may
retain equal phase after an amplitude mismatch. That control demonstrates
why the compliance requirement is specifically load-bearing for the
amplitude-dependent critical quartic clock rather than for every possible
clock.

## 7. What the theorem did and did not naturalize

Established:

- **[THEOREM, CONDITIONAL]** equation (4) is necessary and sufficient for
  exact receiver-state inheritance in the FTD-0994 seed class;
- **[THEOREM, CONDITIONAL]** the membrane-to-clock energy ledger and exact
  same-crossing inverse close;
- **[THEOREM, CONDITIONAL]** the enlarged uniform manifold remains invariant
  under the FTD-0990 membrane and identical onsite flow;
- **[THEOREM]** independent-frontier work is additive and exact coherence
  grows inside the Moore causal cone;
- **[THEOREM, CONDITIONAL]** exact quartic state inheritance carries the same
  amplitude, orientation, `G*` period, and CM normalization without a target
  read; and
- **[THEOREM, CONDITIONAL]** equation (17) is the exact critical-quartic
  mismatch detuning law.

Still selected/open:

- why the physical formation work should land on `C_xy=0` rather than miss
  it;
- any autonomous local feedback, tolerance band, mismatch export, attraction,
  perturbation recovery, or finite-capacity backpressure;
- the source of positive `U_y`, the selected FTD-0990 coupling, receiver
  formation, aperture hardware, and frontier scheduler;
- native critical quarticity, `m`, `lambda`, amplitude selection, and
  maintenance;
- a finite-tick realization of the continuum `G*` period;
- production genesis/evaporation, moving boundaries, CPU/CUDA parity, and
  operational hiding; and
- Born/Bell recovery, probability, mass, Lorentz recovery, biology,
  consciousness, or framework completeness.

No production integration follows.

## 8. Next discriminator

The next question is no longer “how can a phase be copied?” It is:

> Does the existing self-dual common/relative membrane architecture force
> `2mU_y=p_x^2` at a natural crossing, or must a controller measure/export the
> mismatch?

That test must derive `U_y` and `p_x` from one local Hamiltonian rather than
declare their equality, include a positive mismatch port and backreaction,
and fail closed if the result merely tunes a coefficient to the compliance
surface.
