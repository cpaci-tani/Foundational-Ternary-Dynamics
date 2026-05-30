# PRE-REGISTRATION -- Structural / Dynamical Discriminator (Boundary Theorem, Stage 1), v1

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-05-20
**Hash-lock target tag:** `preregister-structural-dynamical-discriminator-v1`
**LEDGER row reservation:** FTD-0186 (the boundary-theorem program)
**Supersedes:** none -- first pre-registration of the boundary-theorem program (plan: `.claude/plans/take-the-role-of-fancy-kahn.md`).
**Companion docs:** `FOUND_STRUCTURAL_DECOUPLING.md` (FTD-0129), `SPEC_DISCRETE_NATIVE_DERIVATION.md` (FTD-0136), `SPEC_ALPHA_READOUT_CONTRACT.md` (FTD-0152), `FOUND_META_PATTERNS.md` (MP-0a/0b, §8.4), `SPEC_DIMENSIONAL_MAP.md`, `LEDGER.md`, `CATALOG_PARAMETRIC_INSERTIONS.md`; methodological template `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md`.

> **Pre-registration discipline.** The discriminator definition (§2) and the partition falsifier (§4) are committed before the classification (§3) is run. After commit: SHA256 -> `REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §2 or §4 invalidates v1; a v2 is required before the classification is run or re-run.

---

## §1 -- Why this pre-registration

CLAUDE.md's #1 goal: "Derive everything we can from a discrete ontology -- **and rigorously establish what we cannot.**" The second clause has no rigorous deliverable. This pre-registration opens the program that produces one: the **boundary theorem** -- a structural characterization of the class of quantities a deterministic discrete substrate of FTD's type cannot fix.

Two existing documents circled this finding; **neither is a theorem:**

- **FTD-0129** (`FOUND_STRUCTURAL_DECOUPLING.md`) consolidates four independent engine tests into the empirical finding that the master-quadratic value does not flow into engine dynamical observables under any classical-gauge-field channel tested. It is explicitly a `[SYNTHESIS]` and explicitly states (§5, §8) it is "empirical, not a no-go theorem."
- **FTD-0136** (`SPEC_DISCRETE_NATIVE_DERIVATION.md`) reframes the same closed-negatives: the action-shaped substrate-derivation question "was malformed." It is explicitly a `[METHODOLOGICAL REFRAME]` -- "not a derivation, not a claim."

Neither carries the one object the boundary theorem needs: a **formal line** between what discreteness fixes (D=3, |Aut(E)|, the Moore integers -- "structural") and what it does not (alpha, g_c, mass ratios -- "non-universal dynamical"). `FOUND_META_PATTERNS.md` §8.4 gestures at it ("FTD's primary selections are boundaries; derived quantities may be interior values") but does not formalize it.

**Stage 1 produces that line** -- the structural / dynamical **discriminator**. It is the keystone of the boundary theorem: Stage 2 (the Structural Decoupling Theorem) cannot be stated without it.

**Why pre-register.** The discriminator's correctness test (§4) is that it must *cleanly partition the existing epistemic record*. If the definition is written after seeing which quantities must land where, the partition is gerrymandered and proves nothing. Committing the definition + the partition rule before the classification makes the partition a genuine test.

---

## §2 -- The pre-registered discriminator

Let Q be any load-bearing quantity-**claim** that the framework derives, claims, or matches. Q is assigned **exactly one** of three classes.

### STRUCTURAL

Q is STRUCTURAL iff its value is **forced by the substrate's finite discrete data** -- automorphism-group orders, lattice neighbour/shell counts, polyhedral-decomposition multiplicities, lattice dimension, or exact spectra/determinants of finite substrate operators -- such that **both**:

- **(S1) Discrete-combinatorial origin.** Q lies in the countable set generated, by field operations and finite algebraic extension, from that finite combinatorial data.
- **(S2) Free-choice invariance.** Q's value is invariant under every free choice in the framework: the calibration declarations (`a_phys = l_P`, `K_B`, `t_phys`), any global rescaling, and the choice of initial / boundary conditions.

Paradigm cases: D = 3 (from |Aut(E)|^2 = 2^D (D-1)!); |Aut(E)| = 4 and the coefficient 16 = |Aut(E)|^2; the Moore integers {N_base=4, N_eff=13, b_3=7}; the colour count N_c = 3 read as a topological/representation count.

### NON-UNIVERSAL DYNAMICAL

Q is NON-UNIVERSAL DYNAMICAL iff it is a **dimensionless continuous parameter** -- a coupling strength, a dynamical-scale ratio, or a mass ratio -- whose value **fails (S1)**: it is not forced by the finite combinatorics. Determining its value requires an external input -- a measured anchor, an action-level parameter, or an initial condition.

Paradigm cases: alpha (the fine-structure coupling), g_c, sin^2(theta_W), alpha_s, the lepton/hadron mass ratios.

### CALIBRATION-CONDITIONAL

Q is CALIBRATION-CONDITIONAL iff it is a **dimensional** quantity whose numerical value **fails (S2)**: it depends on the calibration declarations. This is the dimensional category of `SPEC_DIMENSIONAL_MAP.md` and the absolute-scale column of FTD-0136 §6.4.

Paradigm cases: m_e in MeV, G_N in SI units, any quantity carrying physical units.

**Scope.** The boundary theorem concerns the **STRUCTURAL vs NON-UNIVERSAL DYNAMICAL** split (both dimensionless). CALIBRATION-CONDITIONAL is recorded for completeness -- it is already mapped by `SPEC_DIMENSIONAL_MAP.md` -- and is not the theorem's subject.

**Tie-break rule (locked).** The discriminator classifies *claims*, not *symbols*. If a symbol carries both a structural and a dynamical reading -- e.g. N_c, where *the integer 3 as a colour count* is structural but *the match x_- = 3.024 <-> N_c* is a `[STRONGLY MOTIVATED CONJECTURE]` identification -- the two readings are entered as **separate quantity-claims**: the count is STRUCTURAL, the root-identification is NON-UNIVERSAL DYNAMICAL (it identifies a continuous algebraic root with a physical sector).

---

## §3 -- The pre-registered classification procedure

Upon hash-lock, classify **every load-bearing quantity-claim** in `docs/theory/07_assessment/LEDGER.md` (all FTD-NNNN rows) and `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md`. For each, record: the quantity-claim, its current LEDGER tag, its §2 class, and a one-line justification. The classification is mechanical given §2 alone -- no class may be chosen to fit §4. Output: `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md`, verified by `scripts/proofs/proof_structural_dynamical_partition.py`.

---

## §4 -- The pre-registered falsifier / acceptance criteria

The discriminator earns its keystone role only if it **cleanly partitions the existing epistemic record**.

### Outcome A -- discriminator confirmed (clean partition)

Both hold:
- **A1.** Every `[CLOSED NEGATIVE]` LEDGER entry recording a failed *derivation attempt* targeted a quantity-claim classified NON-UNIVERSAL DYNAMICAL (or CALIBRATION-CONDITIONAL).
- **A2.** Every algebraic-spine `[THEOREM]` / `[DERIVED]` claim is classified STRUCTURAL.

Then: the discriminator is confirmed as the keystone; Stage 2 proceeds; LEDGER FTD-0186 records Stage 1 closed positive.

### Outcome B -- discriminator falsified (misfit)

Any `[CLOSED NEGATIVE]` derivation attempt targeted a STRUCTURAL quantity, **or** any spine `[THEOREM]`/`[DERIVED]` classifies NON-UNIVERSAL DYNAMICAL. Then: §2 is wrong. Diagnose the misfit, issue a v2 with a corrected definition, re-run. This is a genuine, informative result.

### Outcome C -- partial / genuinely ambiguous

A small number of quantity-claims cannot be classified by §2 without judgement (genuine boundary cases beyond the tie-break rule). Then: the discriminator stands for the cleanly-classified majority; the ambiguous cases are documented as its known limits, and Stage 2's theorem is scoped to exclude them.

Outcome A is the prior-favoured result -- §2 was constructed to match the structural intuition behind the existing tags. Outcome B would be a genuine surprise. All three are publishable.

---

## §5 -- Pre-registered consequences

- **Outcome A** -> the discriminator is the locked keystone of the Structural Decoupling Theorem (Stage 2); LEDGER FTD-0186 opens [Stage 1 closed positive].
- **Outcome B** -> v2 reissue; FTD-0186 records the misfit diagnosis.
- **Outcome C** -> Stage 2 proceeds with the scoped discriminator; ambiguous cases logged.

In every case the algebraic spine is untouched -- this program classifies existing claims; it promotes and demotes nothing.

---

## §6 -- Relation to FTD-0129, FTD-0136, ARC, FOUND_META_PATTERNS

- **FTD-0129** (empirical four-channel synthesis): Stage 2 will **rigorize** it -- upgrade its `[SYNTHESIS]` to a `[THEOREM]` with a stated axiom set. FTD-0129 supplies the empirical base; Stage 1 supplies the discriminator that lets the finding be stated as a theorem.
- **FTD-0136** (the discrete-native-derivation reframe): FTD-0136 already reframed the action-channel closed-negatives as "the question was malformed." The boundary theorem is the **rigorous backing for that reframe** -- Stage 2's partial theorem proves *why* the action/classical-channel class cannot carry a non-universal dynamical value, which is the precise content of "malformed." Stage 2 and FTD-0136 are **complementary, not in conflict.** **Honest tension, flagged:** FTD-0136 §5 / §6.4 holds open the hope that FTD-native channels (Class C cluster-interaction) can still yield dimensionless couplings as measurement-level predictions. The boundary theorem's Stage 3 (the *full* no-go) would be in tension with that hope; Stage 3 must explicitly address FTD-0136's discrete-native optimism, and is for that reason -- among others -- marked an open research stretch, not promised. **Stages 1+2 carry no such tension.**
- **ARC / FTD-0152**: the boundary theorem is ARC's dual -- ARC states what an admissible alpha-readout must look like; the boundary theorem characterizes why the failed channel-class fails. A full Stage-3 no-go would establish ARC-3 is unreachable for the dynamical sector.
- **FOUND_META_PATTERNS §8.4**: its observation "FTD's primary selections are boundaries; derived quantities may be interior values" is the informal seed of the §2 discriminator, recast here on the structural/dynamical axis.

---

## §7 -- Risk register

| Risk | Severity | Mitigation |
|------|----------|------------|
| The discriminator does not cleanly partition the record | Medium | This is Outcome B -- a pre-registered, informative result, not a failure; forces a v2. |
| Dual-reading symbols (N_c and similar) resist classification | Medium | The §2 tie-break rule (classify claims, not symbols); residuals -> Outcome C. |
| §2 was written by the same agent planning the theorem (GTCA F9: a definition gerrymandered to fit) | High | Pre-registration: §2 is hash-locked before §3 is run; Outcome B is a real declared outcome; the partition is checked mechanically by a proof script. |
| Stage 3 tension with FTD-0136's discrete-native optimism | Acknowledged | Out of scope for Stage 1; flagged in §6; Stage 3 is explicitly an unpromised research stretch. |

---

## §8 -- Hash-lock

After owner review and commit:

```
git tag preregister-structural-dynamical-discriminator-v1 <commit-sha>
sha256sum docs/theory/10_eft_program/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md
```

Record the SHA256 in `REF_PREREGISTER_MANIFEST.md`; git tag is local-only per project policy. §2 and §4 are the pre-registered content; the classification (§3) is run only after this commit.

---

## §9 -- What this pre-registration does NOT cover

- **Stage 2** (the Structural Decoupling Theorem) and **Stage 3** (the full no-go) -- separate work, gated on Outcome A.
- **The engine phase-diagram track** -- secondary, separately pre-registered if pursued.
- **Any tag change** to any existing LEDGER claim -- the classification reads existing tags; it does not alter them.

---

## §10 -- Status

**DRAFT v1 -- authored 2026-05-20. Not yet hash-locked.** Pending owner review of §2 (the discriminator) and §4 (the falsifier), then: commit -> `git tag` -> SHA256 to `REF_PREREGISTER_MANIFEST.md` -> the §3 classification is run. Per the `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` precedent, drafting lands the bounded thinking; the classification + Stage 2 follow the lock.
