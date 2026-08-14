# FTD-0850 — Production ternary-latch boundary

**Status:** `[ENGINE FACT — SOURCE-LOCKED PRODUCTION MAP]` +
`[THEOREM — CONDITIONAL PRODUCTION EQUIVALENCE CLASSIFICATION]` +
`[CLOSED NEGATIVE — CURRENT UNLOCKED GENESIS/EVAPORATION AS THE FTD-0848 LATCH]` +
`[PARTIAL POSITIVE — TERNARY CODOMAIN, NONZERO ODD SIGN, AND MANY-TO-ONE LOSS]` +
`[OPEN — NATIVE STRICT BARRIER, EVENT RESERVOIR, OR EXPLICIT OPEN-SYSTEM ADOPTION]`  
**Date:** 2026-08-10  
**Programme row:** `FTD-0850`  
**Invalid parent:** FTD-0849, `28/30`; unsimplified positivity query at C19,
with dependent C30 failure; no verdict booked  
**Repair protocol:**
[`PREREG_PRODUCTION_TERNARY_LATCH_EQUIVALENCE_CERTIFICATE_REPAIR_v2.md`](../../preregistrations/native_time_carrier_programme/PREREG_PRODUCTION_TERNARY_LATCH_EQUIVALENCE_CERTIFICATE_REPAIR_v2.md),
pre-run SHA-256
`72F9F9E5DA8EF9F57CE579DCB715E8744ACCB154B80E47E8D5FC4D431FC26968`  
**Repaired certificate:**
[`proof_production_ternary_latch_equivalence_v2.py`](../../../../../scripts/proofs/proof_production_ternary_latch_equivalence_v2.py),
SHA-256
`376606CAB83B9B7A35B324054F4958AB94DAFFB076A7A699711AB7A596095391`,
`30/30 PASS`  
**Production impact:** none

## 0. Result

The frozen production CPU `phase_write` path contains three important pieces
of the FTD-0848 architecture:

1. the actual record alphabet is exactly `{-1,0,+1}`;
2. genesis writes a signed manifested record, odd in nonzero divergence; and
3. evaporation maps distinct signed records to `0`, giving a genuine
   many-to-one loss event.

So production already contains an **actualization/unactualization skeleton**.
It is not, however, the FTD-0848 loss-booked latch.

Two exact failures decide the classification:

- every finite-energy unlocked manifested record has a nonzero evaporation
  hazard, so there is no strict invariant post-acquisition basin; and
- the energy ledger is aggregate rather than event-level, while the single
  and dual genesis branches do not share one energy transaction. There is no
  microscopic bath/controller record that carries the withdrawn energy and
  erased labels.

The honest production description is therefore a selected noisy ternary
memory/open-system rule. It may have long lifetimes, and the complete program
is deterministic when its seed/index/tick selector state is retained. Neither
fact turns it into a closed stable latch.

## 1. Exact acquisition map

In the single-substrate branch, write the eligible incoming magnitude as

\[
 |J|=k_g+x,
 \qquad x>0.                                  \tag{1}
\]

The acceptance ramp is

\[
 p(x)=1-e^{-x/k_m}.                           \tag{2}
\]

It satisfies `p(0)=0`, `p'(x)=e^{-x/k_m}/k_m>0`, and
`lim_{x->infinity}p(x)=1`. Thus `0<p(x)<1` for every finite positive excess.
The source keys the draw by seed, site index, and global tick. The program is
deterministic on that complete state; the reduced local `(J,W,s)` description
is not one-valued without the selector state.

Conditioned on acceptance, production applies

\[
 |J|'=x,
 \qquad |W|'=(1-d)|W|.                       \tag{3}
\]

The field update is radial subtraction, not convergence to a fixed well. It
retains the incoming overshoot exactly.

For single-substrate polarity,

\[
 s(D)=\begin{cases}+1,&D>0,\\-1,&D\le0.\end{cases}             \tag{4}
\]

Hence `s(-D)=-s(D)` for nonzero `D`, while the exact zero tie is selected as
negative. The dual branch selects the opposite zero tie. Production therefore
has a real signed acquisition fragment, but no branch-independent odd rule at
the fixed point.

## 2. No common event-level energy transaction

The single-branch quadratic withdrawal is

\[
 \Delta H_J=k_gx+\frac{k_g^2}{2}
 =k_g\left(x+\frac{k_g}{2}\right)>0,          \tag{5}
\]

\[
 \Delta H_W=\left(d-\frac{d^2}{2}\right)|W|^2.                 \tag{6}
\]

It depends on the incoming excess and wave energy, so it is not one fixed
state quantum. The dual branch invokes the same state-manifestation helper
without either drain. Therefore the two branches cannot be restrictions of
one currently implemented event-level bath/controller transaction.

The aggregate production ledger computes a tick-level total from field, wave,
particle kinetic, and optional strong channels. It accumulates unexplained
residual as injection or dissipation. It does not consume the genesis or
evaporation event journal, and it contains no controller-work or microscopic
bath state. Its selective-damping expectation is explicitly approximate.

This does not mean energy disappears from a complete physical universe. It
means the current production state has not represented the receiver. FTD-0569
and FTD-0570 already prove that a reversible natural extension requires
exported branch history and added reservoir primitives; those are not present
in production.

## 3. No strict unlocked basin

For an unlocked manifested site, the production evaporation hazard is

\[
 q(E)=e^{-E/K_M^2}K_{\rm evap}\,d\tau.         \tag{7}
\]

For finite `E` and positive `K_evap,d tau`, equation (7) is strictly positive.
Consequently there exists selector state that erases either sign in one tick;
no finite-energy unlocked `+1` or `-1` sector is invariant for every complete
state. Large energy makes the record metastable, not mathematically stable.

Setting `locked=true` prevents evaporation and movement. That supplies exact
retention by an explicit Boolean control. It is not a barrier derived from the
genesis dynamics, and it books no acquisition or release work. The natural
mechanism question therefore cannot be closed by pointing to `locked`.

## 4. The genuine lossy step

Evaporation applies

```text
s -> 0
particle_id -> -1
spin -> 0
color -> 0
```

without writing those labels into `J`, `W`, velocity, or remainder. Choose two
preimages with identical continuous fields but opposite state/sign metadata.
They map to the same post-event voxel. This is exact many-to-one
unactualization, consistent with FTD-0395/0425/0567.

The missing piece is not irreversibility. Production has that. The missing
piece is a closed account of **where the discarded distinguishing information
and event energy go**.

## 5. Comparison with FTD-0848

| Requirement | Production result |
|---|---|
| ternary retained record | pass |
| signed acquisition | pass for nonzero divergence; zero tie selected |
| context-complete determinism | pass only when seed/index/tick selector state is retained |
| strict unlocked persistence basin | fail at every finite energy |
| exact event-level energy/work/export ledger | fail in the current state type |
| explicit many-to-one reduced record | pass via evaporation |
| context/Born/`G*` target blindness | pass for the audited formulas |

Because strict persistence and event-level closure are necessary, the current
map is not equivalent to the FTD-0848 latch. This is a scoped closed negative
for the frozen production path, not a universal no-go for ternary memory.

## 6. Certificate record

FTD-0849 returned `28/30` because C19 asked positivity of an unsimplified
difference of squares. FTD-0850 changed only C19 to use the exact C16
factorization and returned:

```text
FTD-0849 production ternary-latch equivalence: 30/30 PASS
PRODUCTION_HAS_TERNARY_SIGNED_ACQUISITION_AND_MANY_TO_ONE_LOSS_FRAGMENTS
UNLOCKED_FINITE_ENERGY_RECORD_HAS_NO_STRICT_INVARIANT_BASIN
NO_EXACT_EVENT_LEVEL_BATH_OR_CONTROLLER_LEDGER_IS_IMPLEMENTED
CURRENT_GENESIS_EVAPORATION_IS_NOT_THE_FTD0848_LOSS_BOOKED_LATCH
VERDICT=OUTCOME_B_PARTIAL_TERNARY_OPEN_SYSTEM_WITNESS
FTD-0850 CERTIFICATE_REPAIR_ONLY_C19_EXACT_POSITIVE_FACTORIZATION
```

## 7. The next natural mechanism

The search no longer needs another threshold curve. The minimum missing
dynamics are:

1. a native continuous odd coordinate with a restoring barrier, or a proven
   movement-enabled matter--field composite whose invariant regions play the
   same role;
2. an event reservoir/history carrier that receives both branchwise energy and
   the distinguishing information removed by evaporation; and
3. an exact local transaction joining that carrier to `s` without reading a
   measurement context, outcome, Born weight, `G*`, or target cadence.

FTD-0781 already localizes the only untested smooth native nonlinearity to the
movement-enabled coupled matter--field sector. That sector is therefore the
next derivation target. Alternatively, v2 may explicitly adopt the FTD-0848
latch plus reservoir as a selected open-system extension; it must then pay
that type cost rather than call it emergent.
