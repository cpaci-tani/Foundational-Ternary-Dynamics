# FTD-0998 — Preregistration: cumulative clock-growth energy reserve and backpressure v1

**Identifier:** `FTD-0998`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — NOT YET EVIDENCE]`  
**Expected classifier:** **Outcome B — exact causal resource law / native reservoir open**

## 1. Question

FTD-0997 supplies a conditional catalytic clock-growth transducer. On its
compliant surface the relative port returns to its initial clock-energy state,
but the source loses one clock-energy share for every newly coherent site.
The catalyst transfers organization; it does not manufacture its replacement
energy.

This discriminator asks for the exact many-event resource law:

1. what reserve update is forced by conservation for one accepted batch;
2. what cumulative identity and finite-reserve bound follow over many ticks;
3. what causal supply rate is necessary for indefinite coherent growth;
4. how a shared reserve must admit concurrent events without double spending;
5. how overlapping formation supports differ from Moore-independent supports;
6. what local-delay and exact-reversal laws follow; and
7. whether frozen production already contains the required reserve,
   replenishment, ownership, and backpressure dynamics.

The campaign derives a necessary energy-and-admission law. It does not claim
that a passive scalar account is a phase-complete canonical mechanism. No
engine or production mutation is authorized.

## 2. Frozen sources

| source | SHA-256 |
|---|---|
| `THEOREM_COMMON_RELATIVE_CATALYTIC_CLOCK_GROWTH_AND_QUIESCENT_SEAM_BOUNDARY_v1.md` | `9418AA0841B3122A65B3276525A7B9DEDE89C31FEA563AC4055B8F50EF262110` |
| `THEOREM_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md` | `68087ED4B410AF54571D61E6F8C7ABEFA694E29E0889ADC2286CC45BFEB70C0F` |
| `THEOREM_GLOBAL_AGGREGATE_WORK_AND_LOCAL_CONCURRENCY_OWNERSHIP_BOUNDARY_v1.md` | `1CF020D3AA4EB78746C8CF7B932B3AB27E265E173E7F81524CF2A4547A38FA91` |
| `THEOREM_LOCAL_CANONICAL_WORK_PORT_AND_C18_FACTOR_EVENT_BOUNDARY_v1.md` | `3BF425E7F826844BDD1F87ACA3B57EE9A26704996CC8A6F7781C683477D3B994` |
| `THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md` | `AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |

Any mismatch invalidates execution. A repair must preserve this protocol and
the first certificate byte-for-byte.

## 3. Registered batch and energy sectors

At global tick `n`, let `F_n` be a preregistered batch of prospective coherent
receivers and let

\[
 k_n=|F_n|,
 \qquad
 D_n=\sum_{y\in F_n}e_{y,n},
 \qquad e_{y,n}>0.                                    \tag{1}
\]

`D_n` is the total receiver-clock demand. The homogeneous critical-clock
branch has `e_{y,n}=e`, hence `D_n=k_ne`.

Let:

- `B_n>=0` be the usable reserve already inside the batch's causal ownership
  domain before admission;
- `Phi_n` be signed net energy entering that domain through its causal
  boundary during the registered transaction;
- `U_n=U(F_n)` be the exact energy released by the local formation/source
  sector for the whole batch; and
- `B_{n+1}` be the reserve after an accepted transaction.

On the forward growth branch, `U_n>=0`. `Phi_n` may be signed, but only energy
that has reached the local ownership domain may enter the same-tick admission
test.

The catalytic common/relative port starts and ends with the same energy on a
compliant event. Its net contribution to the batch energy difference is
therefore zero. The new receivers contribute `+D_n`, the formation/source
sector contributes `-U_n`, and the external boundary environment contributes
`-Phi_n`.

## 4. Forced one-step balance and admission

Exact conservation of the declared closed completion requires

\[
 D_n+(B_{n+1}-B_n)-U_n-\Phi_n=0.                     \tag{2}
\]

Solving for the only unknown scalar reserve change gives the unique balance

\[
 \boxed{B_{n+1}=B_n+\Phi_n+U_n-D_n.}                 \tag{3}
\]

The positive-domain admission law is

\[
 \boxed{B_n+\Phi_n+U_n\ge D_n.}                      \tag{4}
\]

If equation (4) fails, the complete batch fails closed before any receiver,
source, catalyst, reserve, or history state is mutated. A controller may
instead choose an explicitly registered admissible subbatch, but it must then
recompute the exact joint formation release for that subbatch.

Equation (3) is an energy identity, not a license to represent `B_n` by an
unpaired mutable scalar. FTD-0982/0985 still require a complete local
canonical work pair or an equivalent phase-complete field per independently
owned concurrent batch.

## 5. Joint formation work and independent-frontier corollary

For a general batch, the formation release is the exact before-minus-after
source energy

\[
 U_n=U(F_n).                                           \tag{5}
\]

If two prospective sites share changed bonds or source support, separately
computed one-site values may count the same source energy twice. Therefore
no unrestricted identity `U(F_n)=sum_y U_y` is registered.

For a Moore-independent frontier, FTD-0996 proves disjoint changed-edge
supports and exact additivity. Only on that branch may one write

\[
 \boxed{U(F_n)=\sum_{y\in F_n}U_{y,n}.}               \tag{6}
\]

Overlapping events must be compiled as one joint transaction or scheduled in
nonoverlapping sublayers.

## 6. Cumulative identity and finite-resource bounds

For accepted ticks `n=0,...,T-1`, summing equation (3) must telescope to

\[
 \boxed{
 B_T=B_0+\sum_{n<T}(\Phi_n+U_n-D_n).}                 \tag{7}
\]

Equivalently,

\[
 \boxed{
 \sum_{n<T}D_n
 =B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}                    \tag{8}
\]

Because `B_T>=0`,

\[
 \boxed{
 \sum_{n<T}D_n
 \le B_0+\sum_{n<T}(\Phi_n+U_n).}                    \tag{9}
\]

For homogeneous energy `e>0` and
`N_add(T)=sum_{n<T}k_n`, equation (8) becomes

\[
 \boxed{
 eN_{\rm add}(T)
 =B_0-B_T+\sum_{n<T}(\Phi_n+U_n).}                   \tag{10}
\]

On the FTD-0997 quiescent seam, `U_n=0`. In a closed no-inflow domain
`Phi_n=0`, so

\[
 \boxed{
 N_{\rm add}(T)\le\left\lfloor{B_0\over e}\right\rfloor.} \tag{11}
\]

More generally, if every coherent site has additive energy at least
`e_min>0` and every other energy sector is nonnegative, a closed finite total
energy `H_tot` implies the conditional occupancy bound

\[
 N_{\rm coherent}\le
 \left\lfloor{H_{\rm tot}\over e_{\min}}\right\rfloor. \tag{12}
\]

Equation (12) is not a claim about the current production rest-offset-free
ledger. It applies only to a model that actually includes the positive
per-site maintained-clock energy in its total Hamiltonian.

## 7. Indefinite-growth and average-power condition

If `B_0` is finite, equation (9) shows that `N_add(T)->infinity` requires the
cumulative causal supply

\[
 S(T)=\sum_{n<T}(\Phi_n+U_n)                           \tag{13}
\]

to be unbounded. A hidden infinite preloaded reserve is not an allowed
explanation.

If the asymptotic limits exist,

\[
 v_g=\lim_{T\to\infty}{N_{\rm add}(T)\over T},
 \qquad
 \bar P=\lim_{T\to\infty}{S(T)\over T},              \tag{14}
\]

then the homogeneous law gives the necessary average-power inequality

\[
 \boxed{\bar P\ge e v_g.}                             \tag{15}
\]

This is a necessary resource condition, not a sufficiency theorem for stable
growth, replication, life, cognition, or consciousness.

## 8. Concurrency, ownership, and causal delay

For one shared reserve, the entire batch must pass equation (4) atomically.
Independent event checks against the same pre-update balance can double spend.
The exact symbolic counterexample is

\[
 B_n=e,
 \quad k_n=2,
 \quad \Phi_n=U_n=0.                                  \tag{16}
\]

Each isolated demand `e` appears affordable, while the joint demand `2e`
violates equation (4).

For genuinely independent local reserves, every event domain must instead
satisfy

\[
 B_{y,n}+\Phi_{y,n}+U_{y,n}\ge e_{y,n}                \tag{17}
\]

and retain its own phase-complete ownership state. Summing the disjoint local
balances then reproduces equation (3).

If usable reserve is stored at graph distance `d` from an event and the
substrate update radius is `r`, it cannot fund that event earlier than

\[
 N_{\min}=\left\lceil{d\over r}\right\rceil           \tag{18}
\]

ticks. Global update order can synchronize eligibility but cannot transport
energy outside the Moore cone.

## 9. Exact inverse

An accepted forward batch carries the complete record

\[
 \mathcal R_n=(F_n,D_n,U_n,\Phi_n,
 \text{orientation, source, boundary, and port history}).   \tag{19}
\]

After undoing receiver formation, source release, and boundary inflow in
reverse order, the reserve update is

\[
 \boxed{B_n=B_{n+1}-\Phi_n-U_n+D_n.}                  \tag{20}
\]

Substitution of equation (3) into equation (20) must recover `B_n` exactly.
Reversing all accepted batches in last-in/first-out order must recover `B_0`,
every source and boundary energy, every catalyst, and the original receiver
occupancies. A scalar cumulative total without the signed local history is
insufficient to define that inverse.

## 10. Exact gates

### G1 — source lock and production census

- all eight frozen hashes match;
- the sources contain catalytic source expenditure, independent-frontier
  additivity, local phase-complete ownership, finite positive-reserve
  backpressure, and exact reversal precedents;
- production genesis lacks the registered batch reserve, exact joint-demand
  admission, causal ownership, replenishment, and inverse transaction; and
- the production drift ledger explicitly omits rest energy and remains an
  observational account rather than the registered growth budget.

### G2 — forced one-step law

Prove equations (2)--(4), uniqueness of the reserve change, cancellation of
the restored catalyst from the net ledger, and fail-closed positivity for
heterogeneous and homogeneous demands.

### G3 — cumulative and finite-resource laws

Prove equations (7)--(12) by exact telescoping/induction. Include zero-event,
single-event, quiescent, finite-reserve exhaustion, and heterogeneous-demand
cases.

### G4 — indefinite-growth power condition

Prove that finite total causal supply implies finitely many positive-energy
additions and derive equation (15) whenever the registered limits exist.
Reject an unchanged finite catalyst or hidden infinite initial reserve as a
source of unbounded receiver energy.

### G5 — concurrency and joint-work discipline

Prove the atomic batch gate, the symbolic double-spend counterexample,
disjoint-local-reserve factorization, equation (6) only for independent
frontiers, and the requirement to compute one joint `U(F_n)` on overlapping
supports.

### G6 — causality and inverse

Prove equation (18), equations (19)--(20), exact last-in/first-out recovery,
and the necessity of retaining signed source/boundary/port history.

### G7 — interpretation firewalls

Explicitly reject promotion to:

- a phase-complete reservoir derived from a scalar balance;
- a native reserve, inflow, ownership, routing, scheduler, or replenishment
  law;
- free energy from the catalytic common/relative port;
- an unrestricted additive one-site work law for overlapping supports;
- production energy conservation, genesis/evaporation completion, or engine
  integration;
- a derivation of `e`, critical quarticity, amplitude, `G*`, mass, or clock
  maintenance;
- Born/Bell, measurement, Lorentz hiding, biology, consciousness, or
  framework completeness.

No fit, numerical near-miss search, parameter scan, formula substitution, or
engine mutation is permitted.

## 11. Classifier

- **Outcome A — native causal growth reservoir:** all gates pass and frozen
  production already contains the complete local reserve, joint admission,
  causal replenishment, backpressure, ownership, and exact inverse.
- **Outcome B — exact causal resource law / native reservoir open:** G2--G6
  pass, giving the necessary conservation, finite-capacity, concurrency,
  causal-delay, and reversal laws, while the phase-complete substrate
  reservoir and production implementation remain selected/absent.
- **Outcome C — passive bookkeeping only:** a cumulative scalar account can
  be written but positivity, joint admission, causal ownership, or exact
  reversal fails.
- **Outcome D — invalid:** a source hash or exact gate fails.

Outcome B is expected. Outcome A is forbidden unless the frozen production
sources contain the complete mechanism.
