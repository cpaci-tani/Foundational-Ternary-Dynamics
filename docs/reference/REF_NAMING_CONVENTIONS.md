# FTD Naming Conventions

**Purpose:** Standardized naming rules for all files in the FTD project.
**Effective:** February 2026 (v5.26 reorganization)

---

## File Naming Rules

### Markdown Files (.md)

All markdown files use **UPPER_SNAKE_CASE** with a **semantic prefix** indicating the document's role:

| Prefix | Meaning | Examples |
|--------|---------|---------|
| `META_` | Navigation, indexes, project governance | META_INDEX.md, META_DOCUMENTATION_MAP.md |
| `REF_` | Reference/lookup materials | REF_EPISTEMIC_LABELS.md, REF_CLAIMS_MATRIX.md |
| `SPEC_` | Canonical specifications, master references | SPEC_CLAUDE.md, SPEC_FTD_REFERENCE.md |
| `DERIV_` | Derivations, proofs, mathematical results | DERIV_ALPHA_PRECISION_FORMULA.md |
| `AUDIT_` | Self-assessment, epistemic honesty, evaluations | AUDIT_EPISTEMIC_AUDIT.md, AUDIT_PHYSICS_DEFENSE.md |
| `FOUND_` | Foundational/ontological documents | FOUND_THE_FIRST_DISTINCTION.md |
| `EXPLR_` | Explorations, connections, speculative work | EXPLR_FEIGENBAUM_CONNECTION.md |
| `ARCH_` | Archived/historical (superseded content) | ARCH_GRAND_SYNTHESIS.md |

### Exceptions (Standard Names)

Root-level files that follow universal GitHub conventions keep their standard names:
- `README.md`
- `CHANGELOG.md`
- `CONTRIBUTING.md`
- `LICENSE`

### Python Files (.py)

Python source files follow **PEP 8** conventions:
- `lower_snake_case` for modules and scripts
- Do NOT rename Python files as part of markdown reorganizations (import breakage risk)

### PDF/DOCX Files

Published papers and documents use the same prefix system:
- `SPEC_MASTER_QUADRATIC_PAPER.pdf`
- `DERIV_FUNDAMENTAL_CONSTANTS.pdf`
- `ARCH_REFLEXIVE_DYNAMICS.pdf`

---

## Directory Organization

### Theory Documents (`docs/theory/`)

All 83 core theory documents are prefixed and cataloged in `docs/theory/META_INDEX.md`.

### Archive Policy

A document is **archived** (moved to an `archive/` subdirectory with `ARCH_` prefix) when:
- A newer document supersedes it
- It uses outdated naming (TRD -> FTD)
- It contains speculative content the project has moved beyond

Archived files retain both the `ARCH_` prefix AND placement in `archive/` — the prefix provides at-a-glance identification even when viewed outside directory context.

### Evaluation Documents (`evaluation/`)

Evaluation files use the same prefix system:
- `AUDIT_` for assessment, defense, and certification documents
- `REF_` for reference materials (master ledger, cross-references)
- `ARCH_` for superseded intermediate work
- `TIER*.md` files keep their existing convention (self-descriptive)

### Explorations (`docs/internal/explorations/`)

Organized into topic subdirectories:
- `lemniscate/` — Lemniscate curve analysis
- `consciousness/` — Consciousness/G* explorations
- `mandelbrot/` — Mandelbrot-FTD connections
- `number_theory/` — Number theory investigations

### Media Assets (`media/`)

Images and other non-text assets:
- `media/images/theory/` — Images referenced by theory documents
- `media/images/evaluation/` — Evaluation-related images
- Generated programmatically; filenames match generation scripts

---

## When Adding New Files

1. **Choose the correct prefix** based on the document's purpose
2. **Use UPPER_SNAKE_CASE** for the filename
3. **Update META_INDEX.md** if adding to `docs/theory/`
4. **Update META_DOCUMENTATION_MAP.md** if adding a new directory or category
5. **Add cross-references** using the full prefixed filename

---

## Cross-Reference Conventions

When linking between documents:
- Use relative paths from the linking file's location
- Always use the full prefixed filename: `SPEC_FTD_REFERENCE.md`, not `FTD_REFERENCE.md`
- For archive files referenced from the same archive directory, use just the filename (no `archive/` prefix)
- For archive files referenced from outside, include the full path: `archive/ARCH_FILENAME.md`

---

*This document is the authoritative reference for FTD file naming.*
