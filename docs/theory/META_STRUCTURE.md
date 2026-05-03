# FTD Theory Structure Guide

**Purpose:** Keep the theory corpus organized, navigable, and maintainable as the repo grows.  
**Scope:** `docs/theory/` active categories, `docs/theory/archive/`, and the boundary between theory Markdown and publication outputs.  
**Primary catalog:** [META_INDEX.md](META_INDEX.md)  
**Primary spec:** [../SPEC_FTD.md](../SPEC_FTD.md)

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
| `06_consciousness/` | Measurement, observer, and consciousness layer | `FOUND_`, `DERIV_` |
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
- Treat [07_assessment/TRACKER_ONTIC_TRUTH.md](07_assessment/TRACKER_ONTIC_TRUTH.md) as the canonical bedrock truth tracker, [07_assessment/LEDGER.md](07_assessment/LEDGER.md) as the per-claim provenance layer, and [07_assessment/TRACKER_OPEN_ITEMS.md](07_assessment/TRACKER_OPEN_ITEMS.md) as the open-work queue.
- Avoid brittle count claims inside category headings; counts drift faster than structure.
- When a new active theory doc is added, update the catalog in the same change.
- When a doc is superseded, update the index entry in the same change.
- Keep epistemic labeling explicit: theorem, selection, conjecture, imposed input, emergent behavior, or open question.

---

## Paper Boundary

Theory Markdown and publication PDFs should not be mixed.

- Active theory prose lives in `docs/theory/`.
- Active publication PDFs live in [../papers](../papers).
- Archived publication PDFs live under [../papers/archive](../papers/archive).

If a theory document later becomes a paper, keep the Markdown theory source in `docs/theory/` and publish the PDF through `docs/papers/` rather than duplicating the role inside the theory tree.
