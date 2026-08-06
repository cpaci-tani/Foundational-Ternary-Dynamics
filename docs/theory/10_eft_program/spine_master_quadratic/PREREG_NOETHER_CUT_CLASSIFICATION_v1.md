# PRE-REGISTRATION — The Noether Cut: Invariant / Context Classification of the FTD Claim Corpus

**Version:** v1 (design + rule lock)
**Date:** 2026-06-13
**Status:** `[PRE-REGISTERED]` — classification rule fixed *before* measurement.
**Provenance:** the type-rule in §1 was fixed in the session transcript (2026-06-13) prior to any classification run; this document formalizes it. Recommended external provenance: git tag `preregister-noether-cut-classification-v1`.
**Epistemic note:** this is a **classification test**, not a derivation. It can *confirm or kill an organizing hypothesis about the corpus*; it promotes **no** FTD tag and derives **nothing**. Per Constraint 11, a clean result labels structure — it does not fill any gap the structure leaves open.

---

## 0. Hypothesis under test

**The discrete contextual Noether correspondence (`[SELECTION PRINCIPLE]`-grade hypothesis).** FTD's discrete, finite-symmetry structure *forces* a sector of invariants and *declines to force* a complementary sector of context-dependent dynamical quantities. If true, the project's own forced/not-forced verdicts — accumulated independently over a year and recorded as LEDGER/CATALOG tags — should separate **cleanly along a type cut defined without reference to those verdicts.**

This is the generalization of Noether's theorem from *continuous geometric* symmetry (orientation/position/boost ↔ conserved current) to *discrete contextual* symmetry (the lattice's finite group ↔ forced invariant), with the complementary prediction — like Noether's — that everything *not* corresponding to the symmetry is dynamical / context-dependent and therefore *not* forced.

---

## 1. The locked type-rule

Assign each claim's **target** to one bucket by the **type of quantity**, *before* looking at its tag/verdict.

**Column I — INVARIANT** (predicted: **forced**). Target is determined by the discrete/finite symmetry with zero structural freedom: an integer count, a finite group or its abelianization, a spatial dimension, an exact algebraic or number-theoretic identity/constant, a representation-theoretic multiplicity, a topological quantum number, a class-field/CM constant, a structural-uniqueness fact, or a structural no-go. **Marker:** does *not* run with renormalization scale; a fixed mathematical/structural object.

**Column C — CONTEXT-DEPENDENT** (predicted: **not forced**). Target carries genuine dynamical/calibration freedom: the numerical *value* of a coupling, a mass in physical units, a ratio of dynamical eigenvalues, a decay rate/width/lifetime, a cross-section, a transition temperature, a scaling-law calibration constant, a profile exponent, an event/survival count, or the *identification* of an algebraic object with a measured physical value. **Marker:** runs with renormalization scale, or is fixed only by calibration/fit/dynamics.

**Bucket META — NON-CLAIM / PROCESS** (excluded from the cut). Target is a document, paper, audit, referee round, build, infrastructure scaffold, or cross-document synthesis — *not* a physics/math proposition with a forced-or-not target. Reported transparently; **not** counted as obey or scatter.

**Bucket AMBIGUOUS** — a genuine physics/math claim whose target type cannot be decided by the rule. Used sparingly; counts against the cut (see falsifier F3).

**Tie-break (uniform, type-based):** if a claim bundles a forced algebraic part and an unforced physical-value part (e.g. "the polynomial root *equals* 1/α"), classify by the part the **claim asserts as its content**. If the assertion is the physical-value identification → Column C. The slice must be by *type* (algebra vs value) and applied to *every* such row identically.

**Forcing reading of the tag (`actual_forcing`):**
- **forced:** THEOREM, DERIVED, SELECTION, NUMERICAL FACT (structural), NULL-PREDICTION.
- **not_forced:** PARAMETRIC, STRUCTURALLY MOTIVATED PARAMETRIC, IMPOSED, STRONGLY MOTIVATED CONJECTURE, CONJECTURE, HYPOTHESIS, CLOSED NEGATIVE, BOUNDARY, OPEN, OBSERVATION, MEASURED (dynamical).
- **partial:** mixed tags ("THEOREM (math) / X (physics)"), rows whose tag explicitly splits.

**`obeys` resolution:** `I & forced → obey`; `C & not_forced → obey`; `I & not_forced → SCATTER (F1)`; `C & forced → SCATTER (F2)`; `partial → soft`; `AMBIGUOUS → ambiguous`; `META → excluded`.

---

## 2. Prediction

Over Column I ∪ Column C (META excluded): **I → forced, C → not_forced**, with a low scatter rate.

---

## 3. Falsifiers (pre-committed)

- **F1.** Any Column-I target with a *not-forced* verdict (a symmetry that failed to force an invariant) — hard scatter.
- **F2.** Any Column-C target *genuinely DERIVED/THEOREM* from the ontology (a dynamical value the symmetry forced) — hard scatter.
- **F3.** More than **15%** of cut-eligible (I∪C∪AMBIGUOUS) rows require the AMBIGUOUS bucket — the cut is not binary.

---

## 4. Outcome scheme (decided in advance)

- **CLEAN-CUT:** scatter < 5%, zero unanticipated F1/F2 hard falsifiers, ambiguous < 10%.
- **SUBSTANTIAL-WITH-SEAMS:** scatter + soft < 15%, any hard falsifiers confined to the **pre-named seams** (gauge/mixing ratios; dimensionless mass ratios), ambiguous < 15%.
- **SCATTERED:** otherwise — the cut is narrative, not structure; the hypothesis is demoted to `[CONJECTURE]` or killed.

**Pre-named seams** (where the rule is known to lean on judgment, not mechanism): the gauge/mixing ratios (sin²θ_W, α_s, θ₁₃ — defended as Column C because they *run with scale*) and the dimensionless lepton-mass ratios (Column C as calibration-conditional ratios of dynamical eigenvalues). A hard falsifier *inside* these seams is anticipated and bounded; a hard falsifier *outside* them kills CLEAN-CUT.

---

## 5. Protocol

1. **Deterministic extraction** (done): 253 canonical LEDGER rows → `ledger_rows.txt`; 52 CATALOG claim rows (tag-legend excluded) → `catalog_rows.txt`. No LLM summarization in extraction.
2. **Blind per-row classification** against §1, by independent agents, target-type assigned before tag is weighed.
3. **Adversarial verification** of every contestable/contested row + a per-batch calibration sample: an independent skeptic tries to *refute* each column assignment, defaulting to scatter/ambiguous when an assignment is convenient rather than type-forced.
4. **Tally + verdict** against §3–§4. Every scatter/soft/ambiguous row listed by ID with its reason.

---

## 6. Banned moves

- No re-defining the rule after seeing results.
- No per-claim slicing that is not type-based and uniformly applied.
- No dumping inconvenient *claims* into META; META is for genuine process/infrastructure rows only, and each is adversarially checked.
- No promotion of any FTD tag as a result of this test.

---

## 7. Integrity caveat

The test was designed and is being run by the same agent that finds the hypothesis attractive (failure modes F3/F9). Mitigations: (a) extraction is deterministic, not summarized; (b) classification and verification are by independent agents under a rule fixed in advance; (c) the column marker "does it run with scale?" is an objective, externally-checkable property, not an FTD-internal notion; (d) this document is hash-lockable before the measurement commit. A *clean* result is the expected one and therefore the one to distrust most — external (non-AI) audit of the assignments is the recommended next gate before any publication use.

---

## 8. v2 amendment (2026-06-14) — forcing-state correction

**The v1 run did not complete as a valid test.** A server-side rate-limit storm (transient throttle, not a usage limit) wiped the entire adversarial-verification phase (~150 verifiers failed) and two classification batches (~40 of 305 rows never classified). The reported v1 figure (scatter 18.1%, verdict SCATTERED) is therefore computed on an incomplete set with **no** skeptic pass and must not be cited as a result.

The partial run nonetheless exposed a **specification ambiguity in the v1 forcing-map.** The tag **CLOSED NEGATIVE** is overloaded in the FTD corpus:
- for a **structural no-go** (e.g. FTD-0071/0072/0074 no-Clifford-on-block, FTD-0050 master-quadratic ≠ RG char-poly, FTD-0164/0184) it denotes a **proven** result — the symmetry *forces* the absence ⇒ **forced**;
- for a **failed value-derivation** it denotes a coupling that was **not** forced.

v1 mapped both to `not_forced`, manufacturing ~17 false-scatter rows from proven no-gos and unproven conjectures. Likewise **CONJECTURE/HYPOTHESIS/OPEN/OBSERVATION** denote *undetermined* forcing-status (pending), not a refutation of the cut.

**v2 corrects this with a five-state `forcing_state` field, decided per row:**
- **forced** — proven positive (THEOREM/DERIVED/SELECTION/NUMERICAL FACT).
- **forbidden** — proven structural no-go / NULL-PREDICTION (symmetry forces an absence/impossibility); counts as *forced* for the cut.
- **not_forced** — PARAMETRIC/IMPOSED/STRONGLY-MOTIVATED-CONJECTURE-with-obstruction, or CLOSED-NEGATIVE *failed derivation of a value/coupling*.
- **open** — CONJECTURE/HYPOTHESIS/OPEN/OBSERVATION; forcing undetermined; **excluded** from the cut (pending), reported separately.
- **partial** — mixed.

`obeys` (computed deterministically in code, not by the agent): `I&(forced|forbidden)→obey`; `C&not_forced→obey`; `I&not_forced→scatter`; `C&(forced|forbidden)→scatter`; `open→pending(excluded)`; `partial→soft`; `META→excluded`; `AMBIGUOUS→ambiguous`.

**Integrity note.** This change was motivated by the v1 artifact and is disclosed as a post-hoc correction. It is defensible only because (a) v1 did not complete validly, and (b) the correction is *principled* — a proven no-go IS a forced structural result; an unproven conjecture has undetermined forcing-status — rather than tuned to rescue any named row. v2 also narrows verification to the genuinely contested outcomes (scatter/soft/ambiguous + a small calibration sample) to avoid the rate-limit storm. The outcome scheme (§4) and falsifiers (§3) are unchanged. **If v2 still scatters outside the pre-named seams, the hypothesis is demoted or killed — no further rule patching.**
