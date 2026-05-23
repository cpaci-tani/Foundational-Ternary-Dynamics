# Pre-Registration -- MC-T4.3 ARC-B1 Observable-Selection Readout (v1)

**Tag:** [PRE-REGISTRATION] -- this document locks the *design* of the
ARC-B1 closure attempt against MC-T4.3 (the central foundational
obstruction in the FTD framework, per `SPEC_DOCTRINE_LEDGER.md` v1.4
§14 Phase-2 Priority 0). It contains **no result**. All three pre-blessed
outcomes -- FOUND / UNDERDETERMINED / CLOSED-NEGATIVE -- are admissible;
the closure attempt's verdict is genuinely open, and the prior-favoured
outcome is CLOSED-NEGATIVE.

**Date:** 2026-05-23
**Hash-lock target tag:** `preregister-alpha-readout-observable-selection-v1`
**LEDGER row reservation:** FTD-0198 (provisional; confirm next-free
identifier against `../07_assessment/LEDGER.md` at hash-lock; current
top per audit 2026-05-23 is FTD-0197).
**Supersedes:** none -- first pre-registration against the MC-T4.3
closure contract.
**Companion docs:** `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`
(FTD-0152, the MC-T4.3 contract this pre-reg formalizes a closure
attempt against); `../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` §§2, 10.1
(Candidate B row and closed-negative reminders);
`../01_reference/SPEC_DOCTRINE_LEDGER.md` §§0.1, 5, 13.5, 14 (tag
conventions, "earn the map" framing, hardening targets);
`../01_reference/SPEC_FQCR.md` §§2-3 (FQCR Model V transfer/readout
operator the construction must contact -- as derived, not imported);
`../01_reference/SPEC_ALGEBRAIC_SPINE.md` §§2, 5 (master quadratic;
coefficient 16 as derivation target, not insertion);
`../09_mathematical/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md` (FTD-0073
mode-erasure capstone -- the no-go any candidate must escape);
`../02_foundations/FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129, the
4-leg empirical diagnostic the closure must survive);
`../10_eft_program/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md`
(FTD-0186 -- the v1->v2 cautionary precedent for the §11 protocol);
`../02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5
(what "v1 falsifier fires" means in practice);
methodological templates `../08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md`
and `../08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md`.

> **Pre-registration discipline.** Sections §§2-9 are committed before
> the closure attempt is run. After commit: SHA256 -> `REF_PREREGISTER_MANIFEST.md`,
> git tag applied. Any post-hoc edit to §§2-9 invalidates v1; a v2 is
> required before the closure attempt is run or re-run. The closure
> attempt's result lands in a separate doc (`FOUND_*`, `AUDIT_*`, or
> `AUDIT_*_CLOSED_NEGATIVE.md`), never as edits to this file.

**Purpose.** Lock, *before* any closure-attempt construction, (a) what
would count as an ARC-B1 observable-selection readout admissible under
the `SPEC_ALPHA_READOUT_CONTRACT.md` ARC tuple, (b) what would
**falsify** any candidate closure mechanism, and (c) the banned-moves
list that catches re-entries of the eleven closed-negative
alpha-derivation routes that precede this attempt. This pre-registration
is the anti-laundering instrument for the most epistemically dangerous
attempt in the FTD program -- the one where target-knowledge is highest
and the temptation to engineer toward it is therefore largest.

---

## §1 -- Context and doctrine

**MC-T4.3 status** (verbatim from `../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md`
§10.1, as of 2026-05-23): MC-T4.3 is tagged `[FOUNDATIONAL OBSTRUCTION]`,
effort code FO, and named in `../01_reference/SPEC_DOCTRINE_LEDGER.md`
§14 Phase-2 as **Priority 0** -- the **CENTRAL FOUNDATIONAL OBSTRUCTION**
of the FTD program. Closing MC-T4.3 (positively or negatively) is the
single largest framework advance currently available.

**Prior-favoured outcome.** Eleven independent alpha-derivation routes
precede this attempt and have closed negative (cataloged in
`../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` §2 closed-negative list
and the §13 closed-negative reminders of `SPEC_DOCTRINE_LEDGER.md`):
the four classical/action/gauge-field channels of
`../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §4, the site-local
Clifford route (`DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md`, FTD-0073),
the L2 substitution-identity route (FTD-0094), the lattice Z-factor
reading (FTD-0116), the renormalization-group-step characteristic
polynomial route (FTD-0050), the monomial look-elsewhere scan
(FTD-0097), the Langevin-equipartition route (calibration-conditional),
and the dimension-dependent 1/sqrt(d) reading (no substrate
justification). The structural narrowing this produces is decisive: the
surviving mechanism must be **non-site-local**, **non-action-level**,
**non-classical-gauge**, **non-numerology**. The prior on CLOSED-NEGATIVE
for any further attempt is high; the value of pre-registering this
attempt is precisely in making whichever verdict lands rigorous.

**Doctrine clause this serves.** CLAUDE.md goal-clause 2: "Derive
everything we can from a discrete ontology -- **and rigorously establish
what we cannot.**" A clean CLOSED-NEGATIVE here closes a recognized
FOUNDATIONAL OBSTRUCTION and becomes load-bearing input to Path II
(the FTD-0186 boundary theorem v2). A FOUND verdict would clear
the Priority-0 hardening target and is eligible (via a separate ARC-3
ratification document, per `SPEC_ALPHA_READOUT_CONTRACT.md` §7) for
upgrading FTD-0013's tag from `[STRONGLY MOTIVATED CONJECTURE]` to
`[DERIVED]`. No outcome of this attempt directly promotes or demotes
any LEDGER claim -- ratification is downstream and separately governed.

**Three-layer framing** (cited verbatim from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`
§1): "It must connect three layers: (1) Algebraic layer: G*,
coefficient 16, master quadratic, FQCR branch. (2) Readout layer: a
rule selecting a public/measurable observable from the finite substrate.
(3) Operational layer: a measurement protocol corresponding to
electromagnetic coupling. Failure at any layer keeps the claim at
[STRONGLY MOTIVATED CONJECTURE]." ARC-B1 attacks layer (2)
constructively (selecting non-site-local observables from the §4
catalog) and layer (3) operationally (exhibiting a measurement
protocol). Layer (1) is the derivation target, not an input.

**Why ARC-B1 specifically, not ARC-A/C/D.** Among the four candidate
mechanism classes of `SPEC_ALPHA_READOUT_CONTRACT.md` §5, ARC-B1
(Observable-Selection Readout) is the narrowest unclosed class
consistent with the surviving search space: ARC-A (boundary-condition)
carries high risk of hidden coefficient insertion equivalent to alpha;
ARC-C (quantization rule) is structurally vulnerable to reducing to
imported QED normalization; ARC-D (discrete-native measurement) is
engine-resourced and presupposes the engine-bridge work of FTD-0110
(Path IV in the present session's plan). ARC-B1 admits a desk attack
on non-site-local FTD-native observables that escapes FTD-0073's
site-local closure by construction. If this attempt closes negative,
the surviving search space narrows further to ARC-A / ARC-C / ARC-D
and each receives its own pre-registration.

---

## §2 -- The question (LOCKED)

**Q-ARC-B1.** Does there exist an ARC tuple
`(P, A_obs, O_EM, R, C)` (per D1) such that:

1. `A_obs` is built from the FTD-native non-site-local observables of
   the §4 frozen catalog (closed flux-loops / Wilson-loop-style
   readouts, plaquette bivectors, bilinear link observables,
   boundary-to-boundary transfer observables, reflexive projections);

2. the characteristic or fixed-point equation of the transfer/readout
   operator `T_O` derived from `A_obs` reproduces the master quadratic
   `x^2 - 16 (G*)^2 x + 16 (G*)^3 = 0` (FTD-0001) -- equivalently the
   FQCR Model V branch quadratic at `t = 1` (per `../01_reference/SPEC_FQCR.md`
   §2 Prop 5) -- and is *derived* from the observable algebra of
   item 1, not inserted as scaffold;

3. `O_EM` is operationally tied to a charge / scattering /
   field-strength measurement protocol per D3 -- a configurational
   preparation `P` inside the FTD substrate generates a measurable
   response whose readout has the form of an electromagnetic
   observable;

4. the dominant root `x_+` (rather than `x_-`) is selected as the
   electromagnetic readout by an explicit FTD-internal mechanism (per
   D4: positivity, stability, dominance under the readout flow, or
   measurement accessibility) -- *not* by appeal to empirical
   alpha-matching;

5. `C` is a calibration discipline under which the readout
   `R(O_EM(P)) = 1 / x_+` is dimensionless or calibration-invariant;

and is the construction admissible under the hard exclusion rules of
`../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §3 (cited there in
full; mirrored as §7 F-a..F-j and §8 banned moves of this
pre-registration)?

The verdict is genuinely open. All three §6 outcomes are pre-blessed.

---

## §3 -- Definitions (LOCKED)

- **D1 -- ARC tuple.** Verbatim from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`
  §2: `ARC = (P, A_obs, O_EM, R, C)`, where `P` is a preparation class
  (which FTD configurations or boundary conditions count as charge-like
  test systems); `A_obs` is the admissible observable algebra (finite,
  gauge-invariant, translation/O_h-compatible functionals); `O_EM` is
  the electromagnetic measurement functional (what a charge / scattering
  / field-strength measurement reads); `R` is the readout map (how
  `O_EM` returns a dimensionless inverse coupling); `C` is the
  calibration discipline (which dimensional or unit conventions are
  used, and why the result is dimensionless or calibration-invariant).
  The proposal passes the admissibility gate (D5) only if all five
  elements are stated before any physical target value is checked.

- **D2 -- Site-local vs non-site-local observable.** A *site-local*
  observable is one built only from the state field `s` (or the flux
  field `J`) evaluated at single voxels and its values at single sites
  -- the class closed-negative for Clifford embeddings by FTD-0073
  (`DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md`). A *non-site-local*
  observable is built from bilinears spanning at least two distinct
  lattice sites, from plaquette / link / loop combinations, from
  boundary-to-boundary transfer kernels, or from reflexive projections
  of a finite observable algebra to a public channel. **The ARC-B1
  construction MUST be non-site-local from its first definitional
  step.** Site-locality at any stage is an automatic falsifier (F-e
  fires irrevocably).

- **D3 -- Operational electromagnetic measurement.** An operational
  protocol satisfies D3 iff (a) a configurational preparation `P`
  inside the FTD substrate is specified -- which voxel / link / loop
  configurations constitute a charge-like test system; (b) the
  preparation generates a measurable response whose extracted scalar
  has the form of a charge-charge, charge-current, or field-strength
  readout (not merely a mathematical eigenvalue); (c) the protocol
  states what "measurement apparatus" means inside the substrate (a
  probe configuration, a finite-block transfer observation, a boundary
  trace at fixed source separation, or analogous); and (d) the protocol
  is reproducible from the locked specification without further free
  choices. An observable that is "merely the thing whose eigenvalue is
  `x_+`" without (a)-(d) fails D3 (F-d fires).

- **D4 -- Dominant-branch selection rule.** A rule that singles out
  `x_+` (rather than `x_-`) on FTD-internal grounds, stated and
  justified *before* the numerical value of either root is checked.
  Admissible grounds: (i) positivity / sign of `R(O_EM)` derivable from
  `A_obs` structure; (ii) stability under perturbations of the
  preparation `P`; (iii) dominance / spectral-largest-magnitude of `T_O`;
  (iv) accessibility under the operational protocol of D3 (the
  measurement reads the dominant branch by construction); (v)
  uniqueness as a fixed point of the readout flow. Inadmissible grounds:
  any rule that requires the numerical value of `x_+` or `1/x_+` to be
  known before formulating the rule (F-c fires).

- **D5 -- Admissibility gate.** A proposed closure passes the
  admissibility gate iff: D1's five elements are stated; the §7
  falsifier rules F-a through F-j do not fire on the stated tuple
  (mechanical per-falsifier check, §9 step 8); the §8 banned moves are
  not invoked (mechanical per-move check, §9 step 9); the §4 frozen
  catalog is the only source of observable primitives. Failing the
  gate -> the proposal does not proceed to step 10 numerical
  comparison; verdict is per §6.

- **D6 -- Closure verdict.** A construction earns the FOUND verdict
  (§6) iff every step of its derivation trace -- from §4 frozen catalog
  primitives, through `A_obs`, through `T_O`, through the dominant-branch
  selection rule, through the operational protocol, to the readout
  `R(O_EM(P)) = 1/x_+` -- is `[THEOREM]`/`[DERIVED]`-grade; the §7
  falsifiers do not fire; and the §8 banned moves are not invoked.
  UNDERDETERMINED and CLOSED-NEGATIVE are defined per §6. PARTIAL
  results corresponding to ARC-0..ARC-3 status levels per
  `SPEC_ALPHA_READOUT_CONTRACT.md` §7 are recorded as
  sub-classifications of UNDERDETERMINED, not as a separate verdict;
  this preserves the project's three-outcome pre-registration
  convention.

---

## §4 -- Admissible search space (LOCKED)

**The frozen FTD-native observable catalog.** ARC-B1 may draw **only**
on observables constructed from the following primitives. Any
construction depending on a primitive **not reducible to this catalog**
is a `[CONJECTURE -- new postulate]` and **does not** support a FOUND
verdict (§6). The closure attempt may use catalog items 1-7 as
construction inputs; items 8 and 9 are listed for derivation-target
discipline (see notes).

1. **The state field `s : Z[i]^3 -> {-1, 0, +1}`** and its finite
   differences (linear functionals of `s` and its first-order
   neighbour differences). Source: `SPEC_FTD.md` postulate-1
   (discrete-state postulate).

2. **The flux field `J`** (continuous vector field on the lattice) and
   the **dual substrate decomposition `(J_L, J_R)`** -- the chirality
   modes -- together with the parity-conjugate `phi = J_L - J_R`.
   Source: `../02_foundations/FOUND_FORCE_STRUCTURE.md`.

3. **Bilinear link observables.** Two-site bilinears of the form
   `s_i s_j`, `J_i . J_j`, `s_i J_j` for `i, j` distinct sites within
   the Moore neighbourhood. Non-site-local by construction. Sources:
   `SPEC_FTD.md` (Moore-neighbourhood postulate),
   `../02_foundations/FOUND_FORCE_STRUCTURE.md` (flux bilinears).

4. **Plaquette bivectors.** Closed 2-cycle (face) observables: products
   of four-link signed traces around an elementary face of the cubic
   sub-lattice or the BCC sub-lattice; permitted Clifford-projected
   combinations that respect non-site-locality (a bivector is a 2-form
   and therefore non-site-local by construction, escaping the FTD-0073
   site-local Clifford no-go). Source: `../01_reference/SPEC_FQCR.md`
   §6 (bivector frame); `../01_reference/SPEC_DOCTRINE_LEDGER.md` §7
   (bivector references). FTD-0073 site-local caveat applies: the
   bivector observable must be non-site-local in its first
   construction step.

5. **Wilson-loop-style closed flux-loop traces.** Oriented products
   around closed loops in the lattice; standard transverse / longitudinal
   projection allowed; trace functionals over loop classes. Source:
   `../02_foundations/FOUND_FORCE_STRUCTURE.md` plus standard
   lattice-gauge-theory technique.

6. **Boundary-to-boundary transfer observables.** Propagator-style or
   transition-amplitude functionals from one face of a finite L^3 block
   to the opposite face; transfer-matrix eigenvalues of operators
   constructed from items 1-5. Source: standard transfer-matrix
   framework adapted to the FTD lattice.

7. **Reflexive projections.** Projections of the finite observable
   algebra onto a "public" measurement channel, per
   `../01_reference/REF_REFLEXIVITY_VOCABULARY.md` and the math-first
   ontology of `../01_reference/SPEC_MATH_FIRST_ONTOLOGY.md`.

8. **(Target-not-input.) The FQCR Model V transfer/readout operator
   `T_O`** per `../01_reference/SPEC_FQCR.md` §2 Prop 5 and
   `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6 ARC-B1 target:
   admissible *only* when derived from the observable algebra
   (items 1-7), not when imported as construction scaffold. Importing
   `T_O` and noting its eigenvalue equation is the master quadratic is
   a §8 banned move and a §7 F-j firing.

9. **(Targets-not-inputs.) The G* algebraic identity (FTD-0001), the
   master quadratic (FTD-0001), and the coefficient `16 = |Aut(E)|^2`
   for the lemniscatic curve `y^2 = x^3 - x` (FTD-0006 / FTD-0007).**
   These are the derivation targets the construction must reproduce,
   not inputs to insert. The combinatorial fact `|Aut(E)| = 4` is
   admissible as a counting input only because it is a finite
   combinatorial invariant independent of any electromagnetic-coupling
   target; using it to derive the coefficient 16 of the master
   quadratic is consistent with the F-j rule provided the derivation
   of the polynomial form is forward (from `A_obs` and `T_O`) rather
   than reverse-engineered.

**Admissibility constraint.** Any observable not built from items 1-7,
or any computation that uses items 1-9 in a way that hides any of the
forbidden inputs of §7 F-a (numerical values of the fine-structure
coupling or empirical comparators) under a definition, calibration
declaration, or selection argument, is OUT OF SCOPE for this attempt.
Such moves are candidates for separate v2 pre-registrations with
corrected scope.

---

## §5 -- Benchmark (LOCKED): the MC-T4.3 contract

**The desired theorem shape** (verbatim from
`../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §1):

> Alpha Readout Theorem (target, not established). From FTD primitives
> plus an explicitly stated readout rule, construct a dimensionless
> operational electromagnetic coupling `alpha_read` such that
> `alpha_read = 1/x_+`, where `x_+` is the dominant root of the master
> quadratic / FQCR transfer operator. The construction must not use
> physical alpha or CODATA values as input, and it must explain why
> charge measurements access this readout rather than another
> distinguished algebraic number.

**The ARC-B1 target** (verbatim from
`../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6):

> Construct a finite-block observable family `O_N` and transfer/readout
> operator `T_O` satisfying:
> (1) `O_N` is built from FTD-native fields (`J`, `s`, finite differences,
> loops, plaquettes, or boundary traces);
> (2) `O_N` is invariant under translations, cubic symmetry, and gauge
> redundancies relevant to the chosen field representation;
> (3) the characteristic or fixed-point equation of `T_O` is the master
> quadratic or FQCR Model V branch at `t = 1`;
> (4) the dominant eigenvalue is selected by positivity / stability /
> accessibility, not by empirical matching;
> (5) the measurement interpretation says how a charge-like preparation
> reads `1/x_+`.

**Status levels** (verbatim from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`
§7):

| Level | Meaning | Claim impact |
|---|---|---|
| ARC-0 | Tuple `(P, A_obs, O_EM, R, C)` stated and passes exclusion rules | Work package admissible |
| ARC-1 | Mathematical readout theorem proved inside FTD/FQCR structures | Upgrades mechanism, not yet physics |
| ARC-2 | Operational protocol tied to charge/scattering/field measurement | Candidate physical readout |
| ARC-3 | Measurement or derivation returns `1/x_+` without target input | FTD-0013 (`x_+ = 1/alpha`) eligible for tag upgrade |
| ARC-N | Mechanism fails a hard exclusion or falsifier | Preserve as closed-negative provenance |

**No tag changes occur before ARC-3.** Any hypothetical FOUND verdict
in this attempt opens a separate ratification pass; this pre-reg
neither promotes nor demotes any LEDGER claim.

**The 4-leg empirical diagnostic the closure must survive** (per
`../01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` §10.1 cross-reference to
`../02_foundations/FOUND_STRUCTURAL_DECOUPLING.md`, FTD-0129): the
construction must not re-invoke or reduce to any of the four
closed-negative channels -- FTD-0004 (Phase G static V(r)), FTD-0005
(Phase J partition function / L=2 action), FTD-0125 (Phase I dynamical
V(r)), FTD-0126 (Phase II matter-sector vertex). Re-invocation without
an explicit mechanism-class change is F-g firing.

**Load-bearing reading of the benchmark.** The benchmark is *the
contract*, not merely the numerical value `1/alpha = 137.0...`
The numerical match is performed only at §9 step 10 -- AFTER the
admissibility gate (D5), the falsifier checklist (§7), and the
banned-moves checklist (§8). A construction that produces the right
number through a route that fires a falsifier or invokes a banned move
is NOT a FOUND verdict; it is a CLOSED-NEGATIVE for that route plus
an F-i look-elsewhere violation. The discipline of measuring the
construction's compliance with the contract takes precedence over
the discipline of comparing its output to the number. **This is the
single most load-bearing methodological rule of this pre-registration.**

---

## §6 -- The three pre-registered outcomes (LOCKED)

The closure attempt returns exactly one verdict.

### FOUND

An ARC tuple `(P, A_obs, O_EM, R, C)` satisfying §2 (1)-(5) is
exhibited, with a construction trace from the §4 frozen catalog such
that every load-bearing step is `[THEOREM]` / `[DERIVED]` from the
catalog or from pre-existing FTD theorems (no new postulate). No §7
falsifier fires. No §8 banned move is invoked. The numerical
comparison `R(O_EM(P_canonical)) = 1/x_+` matches to within a stated
precision floor (the pre-registered floor is relative error < 10^(-4)
at finite `L` sufficient for stability per FTD-0107 G2 protocol, or
stricter if the construction's analytical form permits an exact
identity).

Tag consequences:
- The ARC tuple is tagged `[DERIVED]` iff every step is `[THEOREM]`-grade
  from the catalog; `[SELECTION]` otherwise.
- FTD-0013 (`x_+ = 1/alpha`, currently `[STRONGLY MOTIVATED CONJECTURE]`)
  becomes *eligible* for upgrade to `[DERIVED]` via a separate
  ratification document and an explicit ARC-3 promotion per
  `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §7. No tag move
  occurs by editing this pre-reg or its result document. Ratification
  may be deferred indefinitely; eligibility does not force the upgrade.
- Closes the Priority-0 hardening target of `SPEC_DOCTRINE_LEDGER.md`
  §14 Phase 2 and W-CRIT-1 / W-CRIT-2 cross-references.

### UNDERDETERMINED

A candidate construction is admissible (passes the §5 admissibility
gate, no §7 falsifier fires, no §8 banned move is invoked) but at
least one of the following holds:
- the dominant-branch selection rule (D4) is not unique -- multiple §4
  catalog readings give comparable selection mechanisms, or `x_+` and
  `x_-` remain equally admissible electromagnetic readouts;
- a partial structural result is reached (corresponding to ARC-1 of
  `SPEC_ALPHA_READOUT_CONTRACT.md` §7) without an operational protocol
  (D3 incomplete);
- an operational protocol is exhibited (corresponding to ARC-2)
  without the readout returning `1/x_+` (numerical comparison fails
  outside the precision floor, but no falsifier fires);
- the construction depends on a choice among catalog primitives that
  is itself unforced.

Tag: `[SELECTION PRINCIPLE -- open]` for the partial elements; or
`[CONJECTURE]` if the partial result is more conjectural. The audit
records exactly which proof obligations remain and which ARC-0..ARC-3
sub-level the attempt reached. FTD-0013 status unchanged.

### CLOSED-NEGATIVE

Either:
(a) every candidate construction in the §4 frozen catalog has a §7
falsifier firing; or
(b) every candidate requires a new postulate outside the §4 catalog
(violating the admissibility gate); or
(c) the best candidate's `R(O_EM(P))` is provably NOT `1/x_+` -- the
construction yields a different distinguished algebraic number, or
yields the master quadratic but only via a route already closed by
FTD-0050 / FTD-0073 / FTD-0094 / FTD-0116 / FTD-0097 / FTD-0035 /
FTD-0004 / FTD-0005 / FTD-0125 / FTD-0126.

Tag: `[CLOSED NEGATIVE]`.

Consequences:
- Joins the closed-negative provenance set
  (`SPEC_OPEN_MATH_BY_SECTOR.md` §13 closed-negative reminders).
- Becomes **load-bearing input for Path II** (the FTD-0186 boundary
  theorem v2 pre-registration): a CLOSED-NEGATIVE here establishes
  that the *observable-selection mechanism class* is closed for the
  alpha-readout problem, narrowing the surviving search space to
  ARC-A / ARC-C / ARC-D. Each receives its own pre-registration; this
  attempt does not pre-commit those.
- Does NOT promote, demote, or re-tag FTD-0013, FTD-0001, FTD-0006, or
  any other LEDGER claim. The spine is untouched.
- Per CLAUDE.md goal-clause 2 ("rigorously establish what we cannot
  derive"), this is a recognized **deliverable**, not a failure. It
  maps a precise boundary of what the discrete ontology determines in
  the electromagnetic sector.

---

## §7 -- The falsifier (LOCKED)

A candidate construction is **falsified** -- moves to UNDERDETERMINED
or CLOSED-NEGATIVE per §6 -- if any of the following rules fire. Each
F-rule is stated so that mechanical checking by a reviewer is possible.

- **F-a.** The observable algebra `A_obs` or the readout `R` requires
  inserting a numerical value of the fine-structure coupling, the
  reciprocal coupling, the value `137.036...`, a CODATA constant, or
  any quantity whose only role is to encode the fine-structure
  coupling (e.g., a QED Schwinger coefficient, a measured anomalous
  magnetic moment value, a measured QED scattering normalization).

- **F-b.** The construction contains a free parameter `lambda` whose
  only role is to be set equal to the fine-structure coupling (or to
  `1/x_+`) for the numerical result to hold. Includes parameters
  disguised as calibration choices, boundary conditions, or
  normalization constants whose value is unconstrained by the §4
  catalog.

- **F-c.** D4 dominant-branch selection fails: no FTD-internal rule
  distinguishes `x_+` from `x_-` as the electromagnetic readout. If
  both roots are equally accessible under the proposed measurement
  protocol with no positivity / stability / accessibility / dominance
  asymmetry derivable from the §4 catalog, the construction fails.
  (Parallel to PREREG_COLOUR_SINGLET_RANK §7 F-f construction.)

- **F-d.** D3 operational-measurement specification fails: `A_obs` is
  mathematically well-defined but no protocol is exhibited under which
  an FTD substrate preparation generates a measurable response whose
  readout has the form of an electromagnetic observable. The
  observable is "merely the thing whose eigenvalue is `x_+`" --
  the immediate falsifier explicitly named in
  `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §5B.

- **F-e.** The construction reduces to or depends on a site-local
  0-form state-field readout in a Clifford module on a finite block.
  Closed-negative per FTD-0073 mode-erasure capstone
  (`../09_mathematical/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md`).
  Firing of this falsifier is automatic and irrevocable -- no v2
  reformulation can revisit site-local Clifford embeddings.

- **F-f.** The construction's `R(O_EM)` is equivalent to a standard
  QED definition or normalization (the textbook scattering vertex,
  the textbook one-loop magnetic-moment coefficient, the textbook
  fine-structure normalization rewritten in different notation) with
  no FTD-substrate origin for the normalization rule. The immediate
  falsifier for Candidate C of `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md`
  §5C, applied here to Candidate B.

- **F-g.** The construction is a relabelling of a previously
  closed-negative route -- FTD-0050 (renormalization-group-step
  characteristic polynomial), FTD-0073 (site-local Clifford),
  FTD-0094 (L2 substitution identity), FTD-0116 (Z-factor reading),
  FTD-0097 (look-elsewhere / monomial scan), FTD-0035 (Mechanism gamma
  for `a_phys`), or the 4-leg empirical diagnostic of FTD-0004 /
  FTD-0005 / FTD-0125 / FTD-0126 (Phase G static V(r) / Phase J
  partition function / Phase I dynamical V(r) / Phase II Wilson-Dirac
  vertex) -- with no identified mechanism-class change. Parallel to
  the `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §3 hard
  exclusion rule "reuses an already closed-negative action-level path
  without changing the mechanism class."

- **F-h.** The construction's dimensional analysis depends on a
  calibration declaration (`a_phys = ell_P`, `K_B`, `t_phys`, or a
  chosen physical scale) where the contract requires a dimensionless
  or calibration-invariant ratio. The hard exclusion rule of
  `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §3.

- **F-i.** The construction was selected from a search over a
  parameter family containing `1/x_+` as one outcome among many,
  without the look-elsewhere penalty of FTD-0097 being recorded and
  the selection rule being independently a-priori justified. Includes
  ad-hoc selection of any exponent quadruple, coefficient, or
  observable form by near-miss to the empirical fine-structure value.
  Anti-target from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6
  ARC-B1 anti-targets.

- **F-j.** The master quadratic, the FQCR Model V branch quadratic, or
  the coefficient 16 is treated as an *input* to `T_O` rather than a
  *derived* consequence of the observable algebra structure of §4
  items 1-7. The construction must derive (not import) the polynomial
  form from §4 catalog primitives. Anti-target from
  `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6 ARC-B1
  anti-targets: "start from the master quadratic and reverse-engineer
  `T_O`." **This is the highest-risk falsifier**: importing item 8 of
  the §4 catalog as scaffold and noting its eigenvalue equation is
  precisely the reverse-engineering pattern that F-j prohibits.

**Any single F-rule firing -> the candidate proceeds to UNDERDETERMINED
(if a §6 PARTIAL/UNDERDETERMINED reading remains available given the
firing) or CLOSED-NEGATIVE per §6 outcome (c).**

**Operational note.** F-a through F-j are stated so a reviewer can
mechanically check each. The closure attempt's §9 step 8 MUST include
an explicit per-falsifier checklist as its mandatory step.

---

## §8 -- Banned moves / anti-laundering (LOCKED)

These are process rules a closure attempt MUST NOT invoke. They are not
falsifier rules (which test the *content* of a candidate); they are
discipline rules that prevent re-entry of closed-negative patterns and
preserve the difference between a derivation and a substitution
identity.

- **No CODATA or fine-structure value appears anywhere in the
  construction**, including comments, units, calibration declarations,
  motivation prose, or selection arguments. Numerical comparison to
  `1/x_+` is performed only at §9 step 10, after the construction is
  complete and the admissibility gate (D5) has passed. Mentions of the
  fine-structure value are admissible only in §5 (benchmark / comparator
  side) and §7 (falsifier rules naming what to exclude).

- **No new free integer, exponent, coefficient, or finite group
  introduced to make the construction succeed.** The §4 catalog is
  frozen. Anything outside it is `[CONJECTURE -- new postulate]` and
  cannot support FOUND. Parallel to PREREG_FINITE_NEUTRAL_LOCK §8.

- **No reverse-engineering from `x_+` back to `T_O`.** The forward
  direction (§4 catalog -> `A_obs` -> `T_O` -> characteristic
  equation -> `x_+`) is the only admissible derivation order.
  Anti-target from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6.

- **No appeal to "the master quadratic is FTD's central algebraic
  content, therefore it must show up in `T_O`."** The master quadratic's
  appearance in `T_O` must be *derived* from the observable algebra's
  structural properties, not asserted. Discipline anchor: this is
  exactly the FTD-0050 closed-negative pattern, generalized.

- **No QED formula imports as scaffold.** No textbook electromagnetic
  vertex, no textbook anomalous-moment formula, no textbook beta
  function, no textbook scattering normalization appears as a definition
  or normalization rule. If a QED formula must be cited, it appears only
  in the comparator (the experimental target side, §5), not in the
  construction.

- **No site-local Clifford embedding** (FTD-0073 closure). No use of
  single-voxel 0-form state-field readout in a Clifford module. The
  observable MUST be non-site-local from the first step (D2).

- **No `g_c` insertion.** `g_c` is `[PARAMETRIC]` per FTD-0031 / FTD-0093.
  Inserting `g_c` and computing the electromagnetic coupling as a
  standard function of `g_c` is the FTD-0050 / FTD-0093 pattern; it is
  an F-g firing and a banned move.

- **No "visual" or "geometric analogy" as measurement rule.** D3
  requires an operational protocol, not a structural resemblance.
  Anti-target from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6.

- **No identification of `x_+` with the reciprocal fine-structure
  coupling before deriving the readout.** The closure attempt may not
  assume the identification; the entire point is to *earn* it.
  Anti-target from `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §6.

- **No retroactive editing of this pre-reg.** If a definition (D1-D6)
  or a falsifier (F-a..F-j) is found defective during the closure
  attempt, the response is a v2 pre-registration, not an edit to v1.
  Parallel to the FTD-0186 v1->v2 precedent recorded in
  `../02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5,
  which is the load-bearing example for the entire pre-reg framework.

- **No spine tag moves before ARC-3.** The closure attempt does not
  promote, demote, or re-tag FTD-0013, FTD-0001, FTD-0006, or any
  other LEDGER claim before reaching ARC-3 per
  `../01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` §7. Verdict-based
  tag changes happen in a separate ratification document.

- **CLOSED-NEGATIVE stays a live option throughout.** The closure
  attempt's author must be willing -- and equipped -- to write the
  CLOSED-NEGATIVE report. Engineering toward FOUND is itself a
  process violation that yields no admissible verdict.

---

## §9 -- Method (LOCKED -- 11 ordered steps)

The closure attempt executes exactly these steps and reports each.
**The order is locked.** Reordering -- in particular, performing step
10 (numerical comparison) before step 8 (falsifier check) -- invalidates
the closure attempt and requires re-execution.

1. **State the proposed ARC tuple `(P, A_obs, O_EM, R, C)`** in full.
   Each element references the §4 catalog primitives it is built from
   (by item number 1-7) and cites the LEDGER row or `SPEC_*` section
   that establishes that primitive as FTD-native.

2. **Derive the observable algebra `A_obs`** from the §4 catalog
   primitives. Each step states: the primitive(s) used, the algebraic
   operation (bilinear, plaquette product, loop trace, boundary
   transfer, reflexive projection), and the FTD-native source. No
   step uses items 8 or 9 as construction input.

3. **Verify gauge / translation / O_h invariance of `A_obs`** (per
   §5 contract item 2). State the symmetry group and the
   gauge-invariance argument for each independent generator.

4. **Construct the transfer / readout operator `T_O`** on `A_obs`.
   Derive its characteristic / fixed-point equation symbolically.
   **DO NOT check whether this equation is the master quadratic
   until step 5.**

5. **Compare the characteristic equation of `T_O` to the master
   quadratic** / FQCR Model V branch quadratic at `t = 1`:
   - If equal (and the derivation in step 4 was forward, not
     reverse-engineered): structural match achieved; proceed to step 6.
   - If equal but the derivation reverse-engineered from a target:
     F-j fires; verdict per §6 (c).
   - If not equal: construction is structurally PARTIAL (UNDERDETERMINED)
     or CLOSED-NEGATIVE per §6.

6. **State the dominant-branch selection rule** (per D4): why `x_+`
   and not `x_-`. The rule is an FTD-internal property derivable from
   steps 2-4 (positivity, stability, dominance, accessibility, or
   fixed-point uniqueness). If no such rule exists -> UNDERDETERMINED
   per §6; if all candidate selection rules are inadmissible (require
   knowing the numerical value first) -> CLOSED-NEGATIVE per §6 (a).

7. **State the operational measurement protocol** (per D3): how the
   configurational preparation `P` generates a measurable response
   whose readout `R(O_EM(P))` is `1/x_+`. Specify (a) the preparation,
   (b) the response, (c) the measurement-apparatus interpretation,
   (d) the reproducibility from the locked specification. If no
   operational protocol can be exhibited -> UNDERDETERMINED per §6;
   if every candidate protocol fails D3 -> CLOSED-NEGATIVE per §6 (a).

8. **Apply the §7 falsifier checklist mechanically.** For each F-a
   through F-j: state whether the rule fires and why / why not, with
   explicit reference to construction content from steps 1-7. Any
   firing -> outcome per §6.

9. **Apply the §8 banned-moves checklist mechanically.** For each
   banned move: state whether it was invoked. Any invocation -> the
   construction is withdrawn; the closure attempt logs the
   attempt-trace, returns no verdict, and the closure cycle resets
   (the author must reformulate from step 1 or terminate the attempt).

10. **Only if steps 1-9 pass:** perform the numerical comparison
    `R(O_EM(P_canonical)) ?= 1/x_+`. Report the relative error. The
    pre-registered precision floor is relative error < 10^(-4) at
    finite `L` sufficient for stability per FTD-0107 G2 protocol, or
    stricter if the construction's analytical form permits an exact
    identity. Comparison values used here are pre-registered as
    benchmark-side (§5), not as construction inputs.

11. **Report the verdict** (per §6) with either: the full construction
    trace and the numerical comparison (FOUND), the specific
    proof-obligation remainder (UNDERDETERMINED with ARC-0..ARC-3
    sub-level), or the specific obstruction (CLOSED-NEGATIVE: which
    step failed, which falsifier fired, which catalog primitive was
    missing).

**Computational substrate.** The closure attempt may be primarily
desk computation: algebraic derivation, group-theory enumeration,
finite-spectrum computation, transfer-matrix eigenvalue analysis.
Supplemented by engine measurements where finite-`L` stability or
transfer-operator spectra need numerical confirmation. Any engine
measurement: instrument SHA recorded, results reproducible against the
hash-locked commit, output lands in a separate
`engine/results/alpha_readout_observable_selection_YYYY-MM-DD/`
directory per the `REF_PREREGISTER_MANIFEST.md` format.

---

## §10 -- What this pre-registration locks vs leaves open

**Locked by the hash** (§11): the question (§2), definitions D1-D6
(§3), the frozen FTD-native observable catalog (§4), the benchmark
contract (§5), the three pre-blessed outcomes (§6), the falsifier
F-a..F-j (§7), the banned moves (§8), the locked 11-step method
ordering (§9).

**Open** -- and only this: the **verdict**. Whether ARC-B1 closes
positive, undetermined, or negative is exactly what the closure attempt
will determine. All three §6 outcomes are pre-blessed; the
prior-favoured outcome is CLOSED-NEGATIVE, but the value of this
pre-registration is in making whichever verdict lands rigorous and
externally defensible.

---

## §11 -- Hash-lock protocol

To lock this pre-registration before the closure attempt runs:

1. Finalise this document. Compute
   `sha256sum docs/theory/10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`.

2. Record the SHA256 in `REF_PREREGISTER_MANIFEST.md` (new section,
   following the format of the FTD-0190 / FTD-0191 / FTD-0192 / FTD-0186
   sections). Add a `../07_assessment/LEDGER.md` row (FTD-0198 or
   next-free identifier; current top per audit 2026-05-23 is FTD-0197)
   tagged `[PRE-REGISTRATION]`, citing this file.

3. `git commit` the pre-registration. Create a lightweight tag:
   ```
   git tag preregister-alpha-readout-observable-selection-v1 \
       -m "Pre-reg for MC-T4.3 ARC-B1 observable-selection closure attempt"
   ```

4. The closure attempt (executing §9) runs only against the tagged
   commit. Its result lands in a separate document --
   `FOUND_ALPHA_READOUT_OBSERVABLE_SELECTION.md`,
   `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION.md`, or
   `AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md` per the
   §6 verdict -- never by editing this file.

5. If a definition (§3) or falsifier (§7) proves defective once the
   closure attempt starts, the correct response is a **v2
   pre-registration**, not an edit to v1. The canonical precedent is
   the FTD-0186 v1->v2 cycle documented in
   `../02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5:
   v1's pre-registered falsifier fired on type-ii closed-negatives, and
   the disciplined response was v2-required, not a v1 edit. This
   precedent governs all subsequent pre-registrations, including this
   one.

6. To verify the tag's commit has not drifted after the lock:
   ```
   git rev-list -n1 preregister-alpha-readout-observable-selection-v1
   git tag -l preregister-alpha-readout-observable-selection-v1
   ```

---

*Pre-registration authored 2026-05-23. **No result.** The closure
attempt (executing §9) is the next step, and runs only after hash-lock.
Per `SPEC_DOCTRINE_LEDGER.md` v1.4 §14 Phase 2 Priority 0 ordering,
this attempt is the most epistemically dangerous in the FTD program;
the §7 falsifiers and §8 banned moves are the discipline that lets the
attempt either close positive rigorously or close negative rigorously.
Engineering toward a verdict invalidates the attempt.*
