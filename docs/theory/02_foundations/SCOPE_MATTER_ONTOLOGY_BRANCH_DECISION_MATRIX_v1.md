# Matter ontology branch decision matrix v1

**Ledger ID:** FTD-0744  
**Status:** `[SCOPE / DECISION CONTRACT — NO NEW PHYSICS CLAIM]`  
**Date:** 2026-07-29  
**Governing charter:** FTD-0740  
**Object predicate contract:** FTD-0743  
**Production status:** unchanged

## 1. Purpose

The matter program currently uses a selected complete state `(s,C,F)` while
asking whether `C` and the face/edge representation are fundamental, derived,
redundant, or temporary resolution variables. This document fixes how that
question will be decided.

Its purpose is to prevent two opposite errors:

- protecting the native `(s,J)` ontology by hiding information in observers or
  replay histories; and
- adding constituent phase, connection, or topology merely because one
  numerical candidate failed.

No branch is promoted here. The matrix states the evidence required to remain
on, descend from, or escalate beyond each branch.

## 2. Exact state-sufficiency discriminator

Let an extended state space `Omega_e` have deterministic tick `T_e`, and let a
proposed reduced representation be

\[
\pi:\Omega_e\longrightarrow\Omega_r.
\]

Examples include forgetting constituent labels, projecting face/edge fields
to site-centred `J`, or forgetting a connection while retaining its field
strength.

### 2.1 Reconstruction

The reduced representation is **reconstructive** on a candidate family `M` if
there is a map `R` such that

\[
R(\pi X)=X\quad\text{modulo declared gauge/equivalence},
\qquad X\in\mathcal M.
\]

Then the forgotten variables are a derived coordinate chart on `M`. This is
the strongest native-reduction outcome, but it is not necessary for dynamical
sufficiency.

### 2.2 Behavioral congruence

The fibers of `pi` are **forward congruent** on `M` if

\[
\pi X=\pi Y
\quad\Longrightarrow\quad
\pi T_eX=\pi T_eY
\]

for every `X,Y` in `M`. They are **two-sided congruent** when the corresponding
condition also holds for `T_e^{-1}`. Declared observables must additionally be
constant on each fiber.

If the fibers are congruent but nontrivial, the forgotten variables are
behaviorally redundant labels or gauge data at the tested scope. The reduced
tick

\[
T_r(\pi X)=\pi T_eX
\]

is well defined even though `R` is not unique.

Every congruence claim must state its domain and horizon. A horizon-`H` claim
requires equal reduced iterates through every `0<=k<=H` while the histories
remain in the declared comparison sector. One-step congruence extends by
induction only when the comparison domain is forward invariant and the
one-step condition holds on every encountered fiber.

### 2.3 State incompleteness

The reduced representation is **dynamically incomplete** if there exist
`X,Y in M` with

\[
\pi X=\pi Y,
\qquad
\pi T_eX\ne\pi T_eY,
\]

or with equal reduced states but different declared present observables. Such
a witness proves that no single-valued Markov tick on the committed reduced
state reproduces the extended dynamics at that scope.

This is the exact trigger for additional state information. It does not force
the current C++ type: a smaller internal phase, current-cycle label, finite
memory coordinate, connection, or another injective equivalent may suffice.

### 2.4 History does not erase the price

If a finite reduced-state history

\[
(\pi X_n,\pi X_{n-1},\ldots,\pi X_{n-m})
\]

restores a single-valued tick, then a finite Markov embedding exists. If those
past values must be stored to predict the future, they are state information
in the operational ontology. Calling them “history” rather than “state” does
not make the price disappear.

## 3. Representation ladder

| branch | committed state | interpretation | evidence required to retain it | escalation trigger |
|---|---|---|---|---|
| N0 — native process | `(s,J)` with production-local update | matter is a native localized state--flux process | M1--M3 family plus a closed state-only native tick and observables | noncongruent-fiber witness licenses missing-state analysis; absence of a robust family routes to closure/redesign, not automatic enlargement |
| N1 — reconstructed chart | `(s,J)` ontic; `C,F_face/edge` derived | constituents and matched fields are reversible coordinates on native histories | injective reconstruction modulo declared equivalence, with commuting dynamics and observables | reconstruction noninjectivity together with behavioral noncongruence |
| E1 — explicit constituent phase space | `(s,C,F)` | constituent positions/momenta carry genuine additional state | M1--M6 selected object plus M7 noncongruence/minimality evidence that no smaller native or injective equivalent is sufficient | remaining transaction nonuniqueness, phase comparison, or holonomy dependence |
| E2 — phase/connection extension | `(s,C,A,E)` or equivalent | oriented phase transport is physical state; field strength is derived curvature | observables or dynamics distinguish equal field strengths with different phase/holonomy sectors | periodic phase/topological-sector evidence that noncompact connection cannot represent |
| E3 — compact/protected extension | compact connection, constraint, or order parameter | protected defect/quantized holonomy supports sectors | exact compactness, quantization, or defect necessity plus a robust localized family | absence of such sectors or failure of localization despite the extension |
| C — candidate closed | no adequate localized family in registered repair class | current dynamics does not found particle-like matter | repeated M1--M3 failure after predeclared admissible repairs | a genuinely new versioned dynamics with new evidence |

Environmentally sustained and constraint-maintained objects from FTD-0743 are
persistence classes, not automatically new microscopic state branches. Their
environment must be included in the ledger at the scale where closure is
claimed.

## 4. Native/chart/explicit-state decision

After an M3 family exists, the Track-B reduction proceeds in this order.

### Test R1 — reconstruction

Attempt a symmetry-respecting map

\[
R:(s,J)\longrightarrow(C,F)
\]

on the family and its registered perturbation neighborhood. Require
translation, cubic, polarity, and relabelling covariance and commutation with
both forward and reverse ticks.

- **Pass:** `C,F` are a derived chart on the tested family.
- **Nonunique but congruent:** quotient the redundancy; no new primitive is
  licensed.
- **Noncongruent witness:** continue to R2.

### Test R2 — minimal missing-information rank

For equal native projections with different extended futures, determine the
smallest variable that separates all observed fiber classes. Candidates must
be tested in increasing price order:

1. deterministic current-cycle selector derived from present state;
2. finite local memory coordinate;
3. constituent internal phase/action coordinate;
4. noncompact link connection;
5. compact connection or constrained order parameter.

The chosen variable must close forward selection, reverse reconstruction,
locality, energy/current accounting, and the same covariance gates. A variable
that labels the training examples but does not produce one common tick fails.

### Test R3 — cross-resolution commutation

For resolutions `a` and `a/b`, freeze coarse-graining maps `B_b` and require

\[
B_bT_{a/b}^{,b_t}X
 \simeq T_aB_bX
\]

for the registered physical time correspondence `b_t`, together with
convergence of object membership, centre, energy, current, lifetime margin,
and other dimensionless observables. If constituent number or coordinates
change but these commute, constituents may be resolution cells rather than
fundamental individuals.

A failure of one coarse map does not prove fundamentality. Repeated failure
over a registered admissible map class, with stable physical observables only
in the extended variables, is the relevant evidence.

## 5. Common-action branch decision

A selected binding force is not enough. For configuration variables `x`, its
work one-form

\[
\omega=\sum_i F_i(x)\cdot dx_i
\]

must be closed, `d omega=0`, on a simply connected region before it can arise
from a position-only potential there. Velocity-dependent or implicit forces
must satisfy the corresponding discrete variational/Helmholtz integrability
conditions for one common action.

The outcomes are:

- **integrable with the existing field:** derive the action and retain the
  branch;
- **nonintegrable only after projecting out field state:** restore the field or
  its exact memory equivalent; no new force is needed;
- **nonintegrable in the complete committed state:** the selected force is an
  imposed effective law, not an ontological derivation;
- **integrability restored by one independently necessary phase variable:**
  test the enlarged branch as a fresh candidate;
- **restored only by sector-specific correction terms:** close explanatory
  unification at that scope.

Post-hoc force amplification is not an admissible branch transition.

## 6. Connection trigger

Let `A` be a candidate link connection and `F=dA` its noncompact field
strength schematic. Equal `F` can correspond to inequivalent flat connection
or holonomy sectors on a nontrivial domain. A connection becomes
ontologically necessary only if at least one declared observable or future
transaction distinguishes such states:

\[
F(A_1)=F(A_2),
\qquad
\mathcal O(A_1)\ne\mathcal O(A_2)
\quad\text{or}\quad
\pi T(A_1)\ne\pi T(A_2).
\]

Examples of admissible discriminators are local phase comparison,
gauge-covariant canonical momentum, Wilson-loop response, or
Aharonov--Bohm-type transport. Merely drawing flux lines, using magnetic
language, or failing a force fit does not trigger a connection.

A **compact** connection requires more: periodic phase, quantized holonomy,
large-gauge identification, or protected defect sectors must be present in the
actual state/action. Compactness may not be inferred from the desired charge
quantization.

## 7. Charge does not choose the branch by itself

Primitive polarity, a reaction-free transported sign, and a
reaction-complete additive charge are different notions. The FTD-0421
nullspace-zero result applies only to its registered additive feature basis; it
neither proves a universal charge no-go nor licenses a connection.

The branch order for charge is:

1. obtain the qualified M6 event alphabet;
2. solve its exact invariant problem on a predeclared feature/function class;
3. derive the local current for any invariant;
4. test whether connection/phase state is needed to make the event action
   single-valued and gauge covariant.

Adding `U(1)` notation cannot manufacture a conserved quantity. Conversely,
a polarity-mediated effective electromagnetic response may exist without a
reaction-complete microscopic charge; it must be described at that weaker
status.

## 8. Failure-routing matrix

| observed failure | licensed conclusion | unlicensed conclusion | next branch action |
|---|---|---|---|
| one M1 preparation fails | that preparation/action conjunction fails | matter needs a new primitive | execute only predeclared alternative preparations or close the candidate |
| no M3 family after registered repair class | current selected dynamics lacks a robust object | connection state will fix localization | close candidate C or introduce a separately motivated new dynamics |
| same `(s,J)`, different `C`, same reduced future/observables | `C` is redundant on that fiber | native ontology is incomplete | quotient/test more fibers |
| same `(s,J)`, different reduced future | native state is incomplete at that scope | current `C` type is uniquely fundamental | determine minimal missing-information rank |
| selected object works but reconstruction is injective | `C` is a chart on the family | constituents are fundamental | remain N1 and test cross-resolution |
| complete-state binding force is nonintegrable | force is imposed/effective | add a multiplier | derive a new common action or close explanatory claim |
| equal field strengths yield different physical phase transport | field strength alone is incomplete | compact `U(1)` is already proved | test E2 noncompact connection first |
| reaction invariant search is null on one basis | that basis is closed negative | charge cannot emerge | wait for M6 alphabet; expand only prospectively justified classes |
| M3--M6 pass but no positive physical pole exists | selected classical matter pattern may exist | particle has been derived | retain classical ontology; close particle interpretation |

## 9. Adoption gates

No ontology branch becomes production or a sixth postulate merely because it
is constructive. Adoption requires:

1. M1--M3 object existence at minimum;
2. exact or convergence-controlled ledgers and symmetries;
3. a branch-comparison audit using the sufficiency discriminator above;
4. explicit accounting of every new state type and selected parameter;
5. an owner decision separate from the research result.

Particle-language adoption additionally requires the downstream M5--M9
evidence appropriate to the claim.

## 10. Current branch status

- N0 is a target, not established: no native finite-support M3 object exists.
- N1 is open: no reconstruction of selected `C,F` from `(s,J)` is proved.
- E1 is the current selected research instrument and is constructive over
  finite registered sectors. Its long-horizon host/device discrepancy is now
  localized by FTD-0751 to source-free fused-versus-unfused field arithmetic,
  not to the matter root or current as the first cause. FTD-0752 closes that
  bounded arithmetic check with exact dynamic parity, so M2/M3 testing may
  resume; E1 is not adopted ontology.
- E2 is motivated only as a possible response to transaction/holonomy
  incompleteness; its necessity is unproved.
- E3 has no present compactness or protected-defect evidence.
- Candidate closure remains a legitimate result if M1--M3 fail across the
  registered repair class.

The engine remains paused. This decision contract changes no dynamics,
scenario, toggle, default, primitive, or physics status.
