# ANALYSIS — Nonlinear delta-IND v2

**LEDGER:** FTD-0396.  
**Lock:** `preregister-nonlinear-delta-ind-v2`, commit `8b6003d3`; preregistration SHA256 `5179b24c480cf89be6fb4b0e4fc6df2a72fdb995c2180e94f697244532394385`.  
**Instrument:** `scripts/proofs/proof_nonlinear_delta_ind_v2.py`, SHA256 `f4624e066131fe43f775b8d334d06893d7af0d978e5e07b8b0673215c08c94f5`.  
**Verdicts:** `N_bounded = BLOCKED-ESCAPE`; `N_unrestricted = BLOCKED-ESCAPE`.  
**Epistemic status:** [SCOPED NO-GO + EXPLICIT UNDERDETERMINATION]. No delta valuation was run.

## 1. Correctness and adequacy

The verifier ran twice from the tagged execution source with exit 0 and byte-identical output. It used Python 3.13.12 on Windows 11 and only `fractions.Fraction`; it imported neither engine code nor a physics constant.

Both frozen spec-level transition anchors passed exactly:

- Genesis: `J=(5/2,0,0)` is strictly above rational threshold `3/2`. Conditional on the frozen accepted branch, the threshold drain gives `J'=(1,0,0)`, kinetic drain `1/3` gives `Jdot'=(1/2,-1/3,1/6)`, and the negative polarity signal gives state `-1`. Equality at threshold and an unaccepted branch both remain void and undrained.
- Annihilation: remainder `3/4` plus velocity `1/2` at `dt=1` carries `1/4` after the unit jump. Contact with the opposite state resets both states and both remainders to zero. Each frozen rational flux is redistributed in exact `1/6` shares. The same-sign negative control cannot enter the annihilation transition.

These anchors establish that the definition contains two real nonlinear rule forms. They do not establish an engine trajectory, universality, or closure properness.

## 2. `N_bounded`: why the event budget does not close the proof

For an event alphabet of size `A` and event budget `B`, the undecorated type strings are finite: `sum_{b=0}^B A^b`. That fact is too weak for the v1 period proof. The lock correctly defines an effective transcript to include the event tick, lattice offset, comparison result, and algebraic local record, because those data select the linear segments being composed.

With polynomial horizon `H(L)`, already `B=1` leaves `H(L)+1` possible tick labels at one fixed site. The verifier recomputed counts `4,6,10,18,34` for `H(L)=L` at `L=3,5,9,17,33`. Thus the event-count assumption supplies no `L`-independent finite upper bound on the space-time-decorated transcript space.

This is a scoped no-go for the proposed implication only:

> `B` fixed independently of `L` does not by itself imply a finite effective branch class, so it does not by itself preserve the v1 Fourier-period/valuation upper bound.

No unbounded family of *realized* FTD event histories was constructed. Consequently this is not an IMPROPER/universality result. A replacement characterization of threshold-time/location dependence was not proved. By the frozen precedence, the bounded rung is **BLOCKED-ESCAPE**.

## 3. `N_unrestricted`: expressive power remains undecided

The exact anchors demonstrate branching and collision. Those ingredients are not a universal-computation embedding. Conversely, no monotone invariant, finite-state quotient, entropy bound, or other structural obstruction was proved that rules universality out when nonlinear activity grows with `L` and the horizon.

The execution therefore supplies neither branch required to decide properness. Under the lock, absence of an embedding cannot be called non-universality, and countability cannot rescue a closure that may generate arbitrary computable target values. The unrestricted rung is **BLOCKED-ESCAPE**.

## 4. Outcome precedence and standing claims

Neither rung fired IMPROPER because no universality/dense-computability witness exists. Neither fired REFUTED because no delta construction was attempted, and REFUTED is barred before properness. Neither fired PROVEN or PROVEN-CONDITIONAL because the characterization gate failed. BLOCKED-ESCAPE is therefore the unique valid outcome for each rung.

The v1 delta valuation was intentionally not run. FTD-0369's linear-sector theorem remains exactly as scoped; it has not been extended to nonlinear dynamics and has not been weakened within its original domain. Under both v2 outcomes:

- `x_+=1/alpha` remains [STRONGLY MOTIVATED CONJECTURE].
- MC-T4.3 remains [FOUNDATIONAL OBSTRUCTION].
- FC-W remains [AXIOM].
- Programmable generation, if later shown, would not be physical forcing.

## 5. Reproduction record

Command:

```text
python scripts/proofs/proof_nonlinear_delta_ind_v2.py
```

Execution HEAD `8b6003d324f01da0a71f1f7a001abf26837a38fd`; tagged source; duplicate output identical. Raw record: `engine/results/nonlinear_delta_ind_v2_2026-07-20/verifier.txt`.

Any future attack must be a new lock. For the bounded rung it must characterize the effective threshold-time/location sequence, not merely count event types. For the unrestricted rung it must supply either a real native universal-computation embedding or a real structural obstruction. This v2 arc itself is closed as explicit underdetermination.
