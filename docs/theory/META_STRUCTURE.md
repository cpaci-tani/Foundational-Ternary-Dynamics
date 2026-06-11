# FTD Theory Structure Guide

**Purpose:** Keep the theory corpus organized, navigable, and maintainable as the repo grows.  
**Scope:** `docs/theory/` active categories, `docs/theory/archive/`, and the boundary between theory Markdown and publication outputs.  
**Primary catalog:** [META_INDEX.md](META_INDEX.md)  
**Framework overview:** [../SPEC_FTD.md](../SPEC_FTD.md) — readable orientation, **not** a status authority; the precedence of all status-bearing documents is fixed in [§ Canonical Hierarchy](#canonical-hierarchy) below.

---

## What Lives Here

`docs/theory/` is the Markdown-first theory corpus for FTD. It is not the paper/PDF output layer.

- Use `docs/theory/` for active theory documents, derivations, audits, and references.
- Use `docs/papers/` for publication PDFs and paper source trees.
- Use `docs/theory/archive/` for superseded or historical theory Markdown that should remain preserved.

---

## Category Map

| Directory | Purpose | Typical prefixes |
|-----------|---------|------------------|
| `01_reference/` | Canonical reference docs, capstones, master overviews | `SPEC_`, `MATH_`, `BRIDGE_`, `PHYS_`, `MONOGRAPH_` |
| `02_foundations/` | Ontology, emergence, axioms, dimensional arguments | `FOUND_`, selective `DERIV_` |
| `03_derivations/` | Core physics derivations and major sector documents | `DERIV_` |
| `04_coupling/` | Constants, precision formulas, coupling structure | `DERIV_` |
| `05_particles/` | Particle-physics applications and prediction docs | `DERIV_`, `PRED_`, `REF_` |
| `06_reference_frames_and_measurement/` | Measurement, observer, and reference frame context layer | `FOUND_`, `DERIV_` |
| `07_assessment/` | Epistemic audits, claims matrices, document status | `AUDIT_`, `REF_`, `TRACKER_`, `REPORT_` |
| `08_structural/` | Geometry, topology, information, lattice structure | `DERIV_`, `EXPLR_` |
| `09_mathematical/` | Number theory, CM, special-function, and pure-math links | `MATH_`, `PROOF_`, `CONJ_`, `DERIV_`, `EXPLR_` |
| `archive/` | Historical, superseded, or preserved-but-noncanonical docs | `ARCH_` |

---

## Placement Rules

When adding or cleaning a theory document:

1. Put canonical overviews and roadmap-level documents in `01_reference/`.
2. Put ontology and emergence arguments in `02_foundations/`.
3. Put physics-sector derivations in `03_derivations/` unless they are primarily constant/coupling work, which belongs in `04_coupling/`.
4. Put self-audits, claims inventories, and status trackers in `07_assessment/`.
5. Move superseded Markdown into `archive/` instead of leaving inactive duplicates in active folders.

---

## Active Vs. Archive

Keep a theory doc active only if at least one of these is true:

- it is still cited as a current reading-path or dependency doc
- it remains the best maintained version of its topic
- it contains current epistemic labeling and current terminology

Move a theory doc to `archive/` when any of these become true:

- a newer document fully supersedes it
- it exists only for historical traceability
- it still has value, but should no longer compete for canonical status

If a document must remain in place temporarily for continuity, label it clearly as superseded in [META_INDEX.md](META_INDEX.md).

---

## Maintenance Rules

- Treat [META_INDEX.md](META_INDEX.md) as the curated navigation layer.
- Treat [07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) as the canonical bedrock truth tracker, [07_assessment/core_ledgers/LEDGER.md](07_assessment/core_ledgers/LEDGER.md) as the per-claim provenance layer, and [07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md](07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) as the open-work queue.
- Avoid brittle count claims inside category headings; counts drift faster than structure.
- When a new active theory doc is added, update the catalog in the same change.
- When a doc is superseded, update the index entry in the same change.
- Keep epistemic labeling explicit: theorem, selection, conjecture, imposed input, emergent behavior, or open question.

---

## Canonical Hierarchy

The corpus has hundreds of documents, but only a few are *canonical* — authoritative
for the epistemic status of a claim. Every other document is *downstream*: it may
explain, derive, apply, or summarize, but if it disagrees with a canonical document,
the canonical document is right and the downstream document is the drift to fix.

**Canonical, status-bearing documents — by jurisdiction:**

| Tier | Document | Authoritative for | Answers |
|------|----------|-------------------|---------|
| Bedrock | [07_assessment/core_ledgers/LEDGER.md](07_assessment/core_ledgers/LEDGER.md) | The **epistemic tag** of every load-bearing claim (`FTD-NNNN` rows) — `[THEOREM]`, `[DERIVED]`, `[SMC]`, `[PARAMETRIC]`, `[CLOSED NEGATIVE]`, … — with tag history and dependencies. | "What is claim X's status, and how did it get there?" |
| Bedrock | [07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md](07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) | The **truth tier** (T1–T5) of the ~dozen claims that carry the framework (`OT-N.M` rows). Co-canonical with the LEDGER — the LEDGER wins on the *tag*, this tracker wins on the *tier*. | "How solid is X on the 5-tier bedrock scale?" |
| Theorems | [01_reference/SPEC_ALGEBRAIC_SPINE.md](01_reference/SPEC_ALGEBRAIC_SPINE.md) | The **theorem statements** — which numbered results are theorem-grade, which are honestly tiered below it, and the exact statement + proof obligation of each. | "Is X actually a theorem, and what precisely does it say?" |
| Roll-up | [01_reference/SPEC_DOCTRINE_LEDGER.md](01_reference/SPEC_DOCTRINE_LEDGER.md) | A **single-page status map** of the three above. It introduces no claim of its own; every row points at a Bedrock/Theorems source. If it disagrees with one, it is the drift. | "What is the whole status map at a glance?" |

**Supporting canonical registers** — authoritative only for their narrow domain, not for claim status:

- [07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md](07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) — the enumeration of which formulas are `[PARAMETRIC]` / `[DERIVED]` / `[IMPOSED]`.
- [../reference/REF_EXTERNAL_CONSTANTS.md](../reference/REF_EXTERNAL_CONSTANTS.md) — the CODATA/PDG edition for every externally-measured comparison value.

**Open-work queues** — authoritative for what is unfinished:

- [07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md](07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) — every `[OPEN]` item across code and theory.
- [01_reference/SPEC_OPEN_MATH_BY_SECTOR.md](01_reference/SPEC_OPEN_MATH_BY_SECTOR.md) — the same queue, organized by physics sector.

**Everything else is downstream** — including [../SPEC_FTD.md](../SPEC_FTD.md), [META_INDEX.md](META_INDEX.md),
[../../CLAUDE.md](../../CLAUDE.md), every `DERIV_` / `FOUND_` / `EXPLR_` document, and every
paper under `docs/papers/`. Downstream documents **cite** canonical status; they never
**define** it. A `[THEOREM]` tag printed in a derivation doc is a copy of the LEDGER
tag — not an independent grant of theorem status.

**Conflict rule.** When two documents disagree about a claim, the more-canonical one is
correct and the other is a drift site. Reconciliation copies the canonical status into
the drifted document — it never promotes or demotes the claim itself.

---

## Canonical-Change Protocol

Drift accumulates when a claim's status changes in one canonical document but not the
others. The fix is a rule: **a status change is not finished until it is propagated.**
When the epistemic tag, truth tier, or theorem status of any claim changes, the *same
commit* carries every applicable update below.

**Propagation checklist** — skip a line only if it genuinely does not apply:

1. [LEDGER.md](07_assessment/core_ledgers/LEDGER.md) — update the `FTD-NNNN` row's tag; add a dated `tag_history` entry (old tag → new tag, with the reason).
2. [TRACKER_ONTIC_TRUTH.md](07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md) — if the claim is a truth-tier (`OT-N.M`) entry, update its tier and star rating.
3. [SPEC_ALGEBRAIC_SPINE.md](01_reference/SPEC_ALGEBRAIC_SPINE.md) — if the claim is theorem-grade, or crosses into/out of theorem grade, update its numbered entry and the §0 count.
4. [SPEC_DOCTRINE_LEDGER.md](01_reference/SPEC_DOCTRINE_LEDGER.md) — update the roll-up row so the single-page map stays true.
5. [TRACKER_OPEN_ITEMS.md](07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md) / [SPEC_OPEN_MATH_BY_SECTOR.md](01_reference/SPEC_OPEN_MATH_BY_SECTOR.md) — if the change opens or closes work.
6. [CATALOG_PARAMETRIC_INSERTIONS.md](07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) — if the change reclassifies a formula among `[PARAMETRIC]` / `[DERIVED]` / `[IMPOSED]`.
7. [../../CLAUDE.md](../../CLAUDE.md) — if the change moves the *headline* epistemic state, refresh the "Current epistemic state" section.
8. **Downstream docs** — search for the claim's `FTD-NNNN` id and key phrasing; reconcile any derivation, paper, or index that quotes the old status.

**No silent promotion.** Propagation *copies* a status that evidence has already decided;
it never upgrades a tag as a side effect of tidying. A genuine promotion or demotion is a
separate, evidence-bearing change with its own LEDGER `tag_history` entry.

**One theme per commit.** Propagate a single status change — or one coherent batch — per
commit, with a message naming the claim and its old → new transition, so the history
stays auditable.

**Periodic consistency check.** Before any release or external submission — and as a
healthy cadence between them — run the read-only audit agents and clear their findings:

- `epistemic-auditor` — tag coverage, derivation-vs-insertion accuracy, broken cross-references, `META_INDEX`  filesystem sync.
- `constants-sentinel` — numerical-constant drift across `scripts/constants.py` and the C++/JS mirrors.

**Keep the canonical set small.** The checklist above *is* the maintenance cost — every
canonical document is another place a change must reach. Do not add new status-bearing
documents. A new analysis is downstream: it cites the LEDGER, it does not become a
second ledger. Fewer canonical documents ⇒ a smaller propagation surface ⇒ less drift.

---

## Paper Boundary

Theory Markdown and publication PDFs should not be mixed.

- Active theory prose lives in `docs/theory/`.
- Active publication PDFs live in [../papers](../papers).
- Archived publication PDFs live under [../papers/archive](../papers/archive).

If a theory document later becomes a paper, keep the Markdown theory source in `docs/theory/` and publish the PDF through `docs/papers/` rather than duplicating the role inside the theory tree.
