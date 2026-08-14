# FTD-0849 — Production ternary-latch equivalence discriminator v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 28/30]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked comparison of the production CPU
genesis/evaporation transaction with the FTD-0848 selected reference latch  
**Production impact:** none

## 1. Registered question

Does the frozen production CPU `phase_write` path already realize an
equivalent of the FTD-0848 loss-booked ternary latch, or does it contain only
some of the required architecture?

Equivalence requires all of the following:

1. a ternary retained record and signed acquisition rule;
2. a context-complete deterministic local transition;
3. a strict post-acquisition persistence region, not merely long expected
   lifetime;
4. an exact event-level energy/work/export ledger; and
5. an explicit many-to-one reduced record step whose discarded information is
   not mislabeled as damping.

The test is classification, not a request to retrofit the engine. Existing
genesis/evaporation rules are evaluated as written. No threshold, toggle,
probability ramp, seed, drain, or state rule may be tuned.

## 2. Epistemic firewall

This discriminator may prove an exact statement about the frozen production
map and its equivalence or non-equivalence to the FTD-0848 reference class. It
cannot prove that the selected FTD-0848 sextic latch is physically correct,
derive a Born rule, interpret the production exponential ramp as quantum
probability, or infer thermodynamic Landauer cost.

The engine's index/tick/seed-keyed pseudorandom draw makes the complete program
deterministic when those variables are retained. If they are omitted from a
local state description, the genesis rule is not one-valued. This distinction
must be stated explicitly; stochastic effective behavior is not a violation
of P5 and is not automatically a Born selector.

## 3. Frozen sources

| Input | SHA-256 |
|---|---|
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `engine/src/render_bridge.cpp` | `BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724` |
| `engine/include/ftd/term_toggles.h` | `2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `THEOREM_LOSS_BOOKED_TERNARY_PHASE_LATCH_v1.md` | `1C1BE138260B4CD3B639F7B6E1DB9E78886B2CCC9E6C0388CFC83E0D0FE073CA` |
| `THEOREM_GENESIS_ACTION_OBSTRUCTION.md` | `877ACAA8C859DFE065120543B8FBC7862BD619AFCB57A4B7CD6D214A6CA18055` |
| `THEOREM_GENESIS_RESERVOIR_DILATION.md` | `565BCD17963322349D5D136E40DE11BF2268677A1CF8D1EED062818EA0E6BFBC` |
| `THEOREM_GENESIS_NATURAL_EXTENSION.md` | `2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076` |

The four theorem paths are relative to their respective directories under
`docs/theory/10_eft_program/derivations/`. The executable freezes their full
repository-relative paths.

## 4. Frozen production map

### 4.1 Acquisition

For the single-substrate branch, a void site is eligible when

\[
 |J|=k_g+x>k_g,
 \qquad x>0.                                  \tag{1}
\]

Acceptance uses

\[
 p(x)=1-e^{-x/k_m}.                           \tag{2}
\]

For every finite positive `x`, `0<p(x)<1`. The accepted update is

\[
 |J|'=x,
 \qquad |W|'=(1-d)|W|,                       \tag{3}
\]

and the state sign is selected from the post-write divergence:

\[
 s=\begin{cases}+1,&D>0,\\-1,&D\le0.\end{cases}                \tag{4}
\]

Equation (4) is odd for nonzero `D` but has a selected negative tie at `D=0`.
The dual branch instead assigns a positive tie and changes `s` without a
matching `J/W` drain.

The single-branch quadratic withdrawal is

\[
 \Delta H_J=k_gx+\frac{k_g^2}{2},
 \qquad
 \Delta H_W=\left(d-\frac{d^2}{2}\right)|W|^2.                \tag{5}
\]

It varies with the incoming overshoot and wave energy. Equation (3) is radial
subtraction, not relaxation into one post-event well.

### 4.2 Retention and evaporation

For an unlocked manifested site, production computes an unsigned local energy
`E>=0` and uses

\[
 q(E)=e^{-E/K_M^2}K_{\rm evap}\,d\tau.         \tag{6}
\]

For finite `E` and positive rate/proper-time factors, `q(E)>0`. Hence no
finite-energy unlocked `+1` or `-1` sector is a strict invariant basin for all
selector phases. `locked=true` suppresses evaporation, but this is an explicit
Boolean retention type, not an energy barrier derived from genesis.

An accepted evaporation applies

```text
s -> 0
particle_id -> -1
spin -> 0
color -> 0
```

without compensating `J/W` changes. Distinct signed preimages therefore
collapse to one post-event record. This is a genuine many-to-one loss step.

### 4.3 Ledger scope

The production energy ledger aggregates field, wave, particle kinetic, and an
optional strong term. It records tick-to-tick residuals as cumulative injection
or dissipation. It does not consume genesis/evaporation event records and has
no event-level bath state or switch-work account. Its own source says the
selective-damping expected rate is only an approximation.

Thus a decreased total can be observed, but the discarded event microstate and
the branchwise transaction are not recovered. The FTD-0569/0570 exact natural
extensions show what an explicit environment would need to add; that added
environment is not production.

## 5. Frozen 30 gates

The executable must run exactly 30 checks:

1. the nine source hashes, counted separately;
10. production writes only the registered ternary state labels in the audited
    genesis/evaporation path;
11. genesis requires `state==0` and a strict magnitude threshold;
12. nonzero divergence polarity is odd, while the exact zero tie is selected
    and branch-dependent;
13. (2) rises strictly from zero to one, so finite positive excess has
    `0<p<1`;
14. acceptance reads the index/tick/seed selector state;
15. the accepted single-branch flux map leaves exactly the excess `x`;
16. equation (5) is exact;
17. the withdrawn energy is input dependent and cannot be one fixed ternary
    state quantum;
18. the dual branch changes state without the single-branch drain;
19. the two branches therefore do not share one event-level latch transaction;
20. evaporation reads unsigned local energy and is sign blind;
21. (6) is positive at every finite energy under its positive factors, so an
    unlocked manifested label has no strict invariant basin;
22. `locked` suppresses evaporation as an explicit Boolean control;
23. evaporation collapses distinct signed records to zero;
24. the erased labels are not transferred into the frozen continuous voxel
    fields;
25. the aggregate energy ledger consumes no genesis/evaporation event state and
    has no event-level bath/controller account;
26. the selective-damping expected rate is explicitly approximate;
27. the audited production path contains no FTD-0848 latch coordinate,
    sextic potential, AVF transaction, or controller-work type;
28. the frozen transition formulas read no measurement context, outcome target,
    Born weight, `G*`, or target cadence;
29. production passes the ternary/sign/loss fragments but fails strict unlocked
    persistence and exact event-level ledger equivalence; and
30. combined discriminator: the frozen production path is a selected noisy
    ternary-memory/open-system rule, not the FTD-0848 loss-booked latch.

## 6. Outcomes

- **Outcome A — production equivalent:** all five equivalence requirements in
  Section 1 pass. The current path may be promoted as a production realization
  of the FTD-0848 class.
- **Outcome B — partial ternary/open-system witness:** the ternary codomain,
  signed acquisition fragment, and many-to-one loss are present, but strict
  unlocked persistence or exact event-level energy/export closure fails. Book
  a scoped closed negative for current production equivalence and retain the
  explicit environment/new-latch branches.
- **Outcome C — invalid:** a source hash or exact gate fails. Book no physical
  verdict and repair only under a fresh lock.

The expected result is Outcome B. No production change is authorized.

## 7. Locked implementation

```text
scripts/proofs/proof_production_ternary_latch_equivalence.py
```

Frozen implementation SHA-256:

```text
BABEB15BEB639D947F664D05972D38E9246CAFBDDB5908FD79479D5894A491B9
```

The script hash and this protocol's pre-run hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before first execution. Run exactly:

```text
python scripts/proofs/proof_production_ternary_latch_equivalence.py
```

## 8. Recorded outcome

The first locked execution returned `28/30`. All nine source hashes and
C10--C18/C20--C29 passed. C19 asked SymPy whether the unsimplified
difference-of-squares `field_withdrawal` was positive; SymPy returned an
undecided result even though C16 had already proved exactly

```text
field_withdrawal = k_g*x + k_g^2/2 = k_g*(x+k_g/2) > 0.
```

C30 inherited the C19 failure. This is a verifier defect, so the run is
invalid by Outcome C and books no production verdict. Any repair must use a
fresh lock and may change only C19's positivity evaluation; C30 may change
only through the inherited check state.
