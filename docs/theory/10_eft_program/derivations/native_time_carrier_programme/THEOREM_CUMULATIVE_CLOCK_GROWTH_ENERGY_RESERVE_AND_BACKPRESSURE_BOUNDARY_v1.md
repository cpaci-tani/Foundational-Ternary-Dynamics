# Theorem — Cumulative clock-growth energy reserve and backpressure boundary v1

**Identifier:** `FTD-0998` / repaired execution `FTD-0999`  
**Date:** 2026-08-12  
**Status:** `[THEOREM — UNIQUE BATCH RESOURCE BALANCE]` +
`[THEOREM — CUMULATIVE FINITE-RESERVE AND AVERAGE-POWER BOUNDS]` +
`[THEOREM — ATOMIC SHARED-RESERVE BACKPRESSURE]` +
`[THEOREM — JOINT-WORK / INDEPENDENT-FRONTIER DISTINCTION]` +
`[THEOREM — MOORE-CAUSAL SUPPLY DELAY]` +
`[THEOREM, CONDITIONAL — EXACT HISTORY-COMPLETE INVERSE]` +
`[BOUNDARY — SCALAR BALANCE IS NOT PHASE-COMPLETE HARDWARE]` +
`[OPEN — NATIVE RESERVOIR / CURRENT / OWNERSHIP / REPLENISHMENT / PRODUCTION]`  
**Parent:** `FTD-0997`

## Result

The minimum many-event resource law for coherent clock growth is fixed by
energy conservation.

At tick `n`, let `F_n` be an accepted receiver batch, let

\[
 D_n=\sum_{y\in F_n}e_{y,n},\qquad e_{y,n}>0,          \tag{1}
\]

be its total maintained-clock demand, let `B_n>=0` be the usable reserve
already inside the batch's causal ownership domain, let `Phi_n` be signed net
energy entering through that domain's boundary, and let `U_n=U(F_n)>=0` be
the exact release from the local formation/source sector.

If each common/relative catalyst is restored to its initial energy, it has
zero net contribution. Exact conservation then uniquely forces

\[
 \boxed{B_{n+1}=B_n+\Phi_n+U_n-D_n.}                  \tag{2}
\]

The entire batch is admitted only if

\[
 \boxed{B_n+\Phi_n+U_n\ge D_n.}                      \tag{3}
\]

Otherwise it fails closed before any state mutation, or a separately chosen
subbatch is recomputed and tested.

Summing accepted transactions gives the exact cumulative identity

\[
 \boxed{
 \sum_{n<T}D_n
 =B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}                    \tag{4}
\]

For an identical per-site clock energy `e` and
`N_add(T)=sum_{n<T}|F_n|`,

\[
 \boxed{
 eN_{\rm add}(T)
 =B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}                    \tag{5}
\]

Thus a finite closed reserve cannot fund indefinite positive-energy growth.
On the quiescent FTD-0997 seam, where `U_n=0`, and with no boundary inflow,

\[
 \boxed{
 N_{\rm add}(T)\le
 \left\lfloor{B_0\over e}\right\rfloor.}            \tag{6}
\]

If long-time rates exist, indefinite homogeneous growth requires

\[
 \boxed{\bar P\ge e v_g,}                             \tag{7}
\]

where `v_g` is the coherent-site growth rate and `bar P` is the average
causally delivered formation-plus-boundary power.

The theorem separates catalyst from fuel:

> The common/relative port can transmit and restore phase-bearing
> organization. Every additional maintained clock-energy share must still be
> supplied by a causally available source.

## Certificate of record

- Parent protocol:
  [`PREREG_CUMULATIVE_CLOCK_GROWTH_ENERGY_RESERVE_AND_BACKPRESSURE_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_CUMULATIVE_CLOCK_GROWTH_ENERGY_RESERVE_AND_BACKPRESSURE_v1.md),
  SHA-256
  `6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052`.
- Immutable parent proof:
  [`proof_cumulative_clock_growth_energy_reserve_and_backpressure.py`](../../../../../scripts/proofs/proof_cumulative_clock_growth_energy_reserve_and_backpressure.py),
  SHA-256
  `E8257678700C732214D1A44E69FF5FCBEB31696BB86E6A2F5DB8F611534CD6F0`.
- First locked execution: `89/91`; every mathematical and physical gate
  passed, while two dependent source-census markers failed because normalized
  C++ comments retained an intervening `//` token.
- Repair protocol:
  [`PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_REPAIR_v2.md),
  SHA-256
  `28525592D68887E4795B9E4F9664565C72969DD7828EE22F68970D1C2173EB70`.
- Repair wrapper:
  [`proof_cumulative_clock_growth_energy_reserve_and_backpressure_v2.py`](../../../../../scripts/proofs/proof_cumulative_clock_growth_energy_reserve_and_backpressure_v2.py),
  SHA-256
  `3FC2DA55EF1DAC8C48CE65AF5B76870981B75F15C191C45D6B62BCD6306961E3`.
- Final execution: inherited `74/74` computational, `17` disclosure
  (blended headline `91/91`), plus repair integrity `11/11`,
  **Outcome B — exact causal resource law / native reservoir open**.

No production file was changed by this campaign.

## 1. Why equation (2) is unique

Let the reserve change by an initially unknown `Delta B_n`. The new receivers
gain `D_n`; the local source loses `U_n`; the external boundary environment
loses the signed inflow `Phi_n`; and the restored catalyst changes by zero.
The closed-completion energy difference is

\[
 \Delta H_{\rm closed}
 =D_n+\Delta B_n-U_n-\Phi_n.                           \tag{8}
\]

Setting equation (8) to zero has the unique solution

\[
 \Delta B_n=\Phi_n+U_n-D_n,                           \tag{9}
\]

which is equation (2). Positivity of the post-transaction reserve is exactly
equation (3).

The result does not depend on a particular battery coordinate. It follows
from the declared sector boundary and per-receiver energy. Selecting a
physical representation of the reserve is a separate problem.

## 2. One credit, one owner

`U_n` and `Phi_n` refer to different ledger interfaces.

- `U_n` is energy lost by the local formation/source sector and credited to
  the transaction.
- `Phi_n` is signed net energy lost by the exterior and delivered across the
  causal boundary.
- `D_n` is energy retained by the new coherent receiver clocks.
- `B_{n+1}-B_n` is the remaining internal reserve change.

If a physical energy packet crosses the outer boundary and is later released
by a local source, it must change ownership at a declared interface; it cannot
be booked simultaneously in both `Phi_n` and `U_n`. Equation (8) is exact
only under that non-overlapping sector partition.

This is why the law is ledgerable: every term names a state difference or a
signed boundary transfer, the balance telescopes, and the inverse restores
the named owner.

## 3. Joint formation work

For a general batch, `U_n` is the exact joint before-minus-after source energy

\[
 U_n=U(F_n).                                           \tag{10}
\]

One-site formation releases are not automatically additive. If two sites
change a shared bond or source coordinate, a naive sum can count the same
energy twice.

For a Moore-independent frontier, FTD-0996 proves disjoint changed-edge
supports. On that branch only,

\[
 \boxed{U(F_n)=\sum_{y\in F_n}U_{y,n}.}               \tag{11}
\]

Overlapping events must be evaluated as one joint transaction or placed in
nonoverlapping sublayers. Choosing a subbatch changes both `D(F_n)` and
`U(F_n)`; neither may be inherited from the rejected batch without
recalculation.

## 4. Cumulative and finite-resource bounds

Iterating equation (2) gives

\[
 B_T=B_0+\sum_{n<T}(\Phi_n+U_n-D_n),                  \tag{12}
\]

and rearrangement gives equation (4). Since `B_T>=0`,

\[
 \sum_{n<T}D_n
 \le B_0+\sum_{n<T}(\Phi_n+U_n).                      \tag{13}
\]

For homogeneous demand, equation (13) gives equations (5)--(6).

More generally, if every maintained coherent site has additive energy at
least `e_min>0` and every other Hamiltonian sector is nonnegative, a finite
closed total energy obeys

\[
 N_{\rm coherent}\le
 \left\lfloor{H_{\rm tot}\over e_{\min}}\right\rfloor. \tag{14}
\]

Equation (14) is conditional on that positive additive Hamiltonian. The
production energy diagnostic deliberately omits rest offsets and does not
yet instantiate this bound.

## 5. Open growth and average power

With finite `B_0` and positive `e`, equation (13) implies that unbounded
`N_add(T)` needs unbounded cumulative supply

\[
 S(T)=\sum_{n<T}(\Phi_n+U_n).                          \tag{15}
\]

If the limits

\[
 v_g=\lim_{T\to\infty}{N_{\rm add}(T)\over T},
 \qquad
 \bar P=\lim_{T\to\infty}{S(T)\over T}               \tag{16}
\]

exist, divide equation (13) by `T` and let `T` grow. The finite initial
reserve contributes zero rate, proving equation (7).

An unchanged finite catalyst contributes zero net energy per completed
cycle. It cannot replace `S(T)`. Nor may an unbounded initial `B_0` be hidden
inside the word “substrate”; that would be an explicitly infinite primitive,
not a derived local mechanism.

## 6. Atomic concurrency and backpressure

A shared reserve is one transactional object. For

\[
 B_n=e,
 \qquad D_n=2e,
 \qquad \Phi_n=U_n=0,                                 \tag{17}
\]

each of two independent stale reads sees enough energy for one receiver, but
the joint post-reserve would be `-e`. Therefore equation (3) must be evaluated
atomically for the whole shared-reserve batch.

If the events instead have genuinely disjoint local reserves, each domain
may test

\[
 B_{y,n}+\Phi_{y,n}+U_{y,n}\ge e_{y,n}.               \tag{18}
\]

The disjoint local updates sum to equation (2). FTD-0985's phase-completeness
lower bound still applies: independent physical reserve differences need
their conjugate relative phases or an equivalent local canonical field.

Backpressure means one of two explicit outcomes:

1. reject the full batch without partial mutation; or
2. choose and recompute an admissible subbatch.

There is no third outcome in which all receiver records appear while the
missing energy is left as an unowned diagnostic residual.

## 7. Moore-causal supply and exact inverse

If usable reserve is stored graph distance `d` from a prospective event and
one update propagates at most radius `r`, the earliest funding tick is

\[
 \boxed{N_{\min}=\left\lceil{d\over r}\right\rceil.}  \tag{19}
\]

Global tick order may align eligibility windows. It cannot move energy
through equation (19) faster than the substrate cone.

For an accepted event whose complete signed history is retained, reverse
execution uses

\[
 \boxed{B_n=B_{n+1}-\Phi_n-U_n+D_n.}                  \tag{20}
\]

It then restores the local source, exterior boundary, catalyst, receiver
occupancy, and reserve in reverse order. Applying equation (20) to equation
(2) gives `B_n` identically, and last-in/first-out reversal telescopes to
`B_0`.

A cumulative scalar total is insufficient to identify the inverse. Distinct
local source/boundary histories may have the same sum. The orientation,
ownership, source, boundary, port, and occupancy records must remain in the
closed state until they are exported or uncomputed.

## 8. What the law says physically

The self-dual common/relative architecture supplies a plausible division of
labor:

- the common mode carries the maintained body clock;
- the relative mode can act as the catalytic phase-bearing transfer port;
- local formation work and boundary current supply the receiver energy; and
- reserve/backpressure decides whether a complete transaction is possible.

This is “more than matter” only in the precise dynamical sense of maintained
organization plus throughput. It is not a new substance, a biological
identification, or a consciousness theorem.

`G*` remains the selected critical-quartic calendar factor. It can determine
the cadence inherited by an admitted receiver; it supplies no term in
equations (2)--(7). The clock tells the transaction when. The resource law
decides whether it can be paid for.

## 9. Production boundary

Frozen production does not implement this theorem.

- `Voxel` has no clock-growth reserve or phase-complete reserve owner.
- single-substrate genesis uses a selected field/wave drain that is already
  documented as not being an exact common-action latent-heat identity;
- dual-substrate genesis has no matched drain;
- genesis admission does not atomically compare joint receiver demand with a
  causal local budget;
- no local replenishment, routing, source-reaction, or inverse-growth
  transaction exists; and
- the production drift ledger is rest-offset-free and interaction-incomplete.

The current engine may be used later as a measurement target. It is not
evidence that equations (2)--(20) have been physically realized.

## 10. Epistemic disposition

Established — every "Established" bullet below is conditional on the
declared `B`/`Phi`/`U`/`D` sector partition (see "Selected/conditional"
immediately following); that conditionality is stated once here rather than
repeated in each tag, but it is not optional and should be read into all six:

- **[THEOREM, CONDITIONAL]** conservation uniquely forces equation (2) for
  the declared sectors;
- **[THEOREM, CONDITIONAL]** positivity forces the batch gate (3);
- **[THEOREM, CONDITIONAL]** equations (4)--(7) give exact cumulative,
  finite-reserve, and average-power bounds;
- **[THEOREM, CONDITIONAL]** a shared reserve requires atomic admission and
  overlapping supports require joint work evaluation — this is the growth
  law's instance of the double-spend obstruction FTD-0985 already
  established for the same-tick concurrent-work setting, restated here for
  the accumulating-reserve setting rather than freshly derived;
- **[THEOREM, CONDITIONAL]** remote supply obeys the Moore delay (19) — the
  same causal-locality bound FTD-0985 already proved (`ceil(d/r)` ticks to
  reach distance `d`), restated here for the growth setting; and
- **[THEOREM, CONDITIONAL]** complete signed history makes the full sequence
  exactly reversible.

Selected/conditional:

- the positive per-site clock energies `e_y` and their additivity;
- the sector partition defining `B`, `Phi`, and `U`;
- the existence of a positive reserve domain; and
- a phase-complete local representation capable of realizing the scalar
  identity.

Still open:

- derive a substrate-native reserve density and causal energy current;
- identify the existing common/relative variables that own that reserve, or
  price a new adopted type;
- derive reserve charging, local transport, atomic ownership, replenishment,
  backpressure, and history export;
- connect that current to the FTD-0997 refill without target-energy leakage;
- prove finite-tick stability, moving-front behavior, CPU/CUDA parity, and
  operational hiding;
- derive critical quarticity, clock energy scale, amplitude, and maintenance;
  and
- recover Born/Bell physics, mass, Lorentz symmetry, biology, consciousness,
  or framework completeness.

No production integration follows.

## 11. Next discriminator

The next missing dynamics is no longer an energy-accounting equation. It is
the local current that realizes it:

> Can the existing relative/environmental field carry a nonnegative reserve
> density and signed Moore-local current satisfying a discrete continuity
> equation, phase-complete ownership, atomic debit, exact refill, and reverse
> transport—without reading a target clock energy or adding a hidden infinite
> source?

A positive answer would begin to derive the gearbox between the substrate's
clock hardware and its fuel supply. A negative answer would price the reserve
field as an additional adopted physical type.

### Subsequent unbooked carrier result (2026-08-24)

The preregistered
[C4 field-packet reserve-current successor](../common_action_mechanics_reciprocity/THEOREM_C4_FIELD_PACKET_RESERVE_DENSITY_CURRENT_AND_ATOMIC_CLOCK_DEBIT_BOUNDARY_v1.md)
answers the finite carrier/interface part conditionally. Under the already
selected C4-trivial field metric and phase-parity half-admission, the outgoing
Maxwell packet has a pointwise nonnegative finite density, exact Moore-local
antisymmetric current, finite-domain boundary flux, retained phase-complete
inverse, and atomic whole-packet ownership debit. Packet counts realize
equation (2) exactly, with $\Phi$ represented by explicit boundary crossings.

The locked census passes 2,046,451 exact checks, Outcome B. It does not promote
this row or close the native-action boundary: carrier metric/admission,
absorption into the common clock, and field/clock scale compliance remain
selected. The successor sharpens the next gate to a local reciprocal
absorption generator. If $d$ packets of energy $\Gamma$ maintain one clock
quantum $\omega_0I_*$, the required but unforced compliance is

\[
 \chi_{\rm EM}={\Gamma\over I_*}={\omega_0\over d}.       \tag{21}
\]

No value of either side is derived or compared with a target.

### Subsequent unbooked reciprocal-absorption result (2026-08-24)

The preregistered
[packet/clock/recoil absorption successor](../common_action_mechanics_reciprocity/THEOREM_RECIPROCAL_PACKET_CLOCK_RECOIL_ABSORPTION_GENERATOR_AND_GRAVITY_SOURCE_BOUNDARY_v1.md)
selects one type-2 generating function that realizes the admitted packet debit
as physical clock action plus material recoil. It is exactly symplectic,
energy conserving, translation-charge exchanging, and history invertible.
For a quadratic body,

\[
 \omega\Delta I=d\Gamma+K(P)-K(P+p).                   \tag{22}
\]

Complete local energy ownership is therefore continuous across the seam,
which conditionally preserves the scalar gravity source. The locked result is
Outcome B and remains unbooked. It does not promote FTD-0999: the trigger,
canonical packet momentum, inertia, clock/action scale, finite microscopic
lift, tensor stress, and native action remain selected or open.
