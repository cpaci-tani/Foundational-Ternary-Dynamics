# FTD Manuscript Terminology Consistency Report

**Generated:** 2026-01-10
**Scope:** All .qmd files in `manuscript/` and `manuscript/chapters/`
**Total files analyzed:** 79

---

## Executive Summary

This report identifies terminology variations across the FTD manuscript and provides standardization recommendations. The analysis found several areas requiring attention, with the most significant inconsistencies in:

1. **KB vs K_B notation** (major - requires standardization)
2. **Cell vs Voxel usage** (minor - generally consistent but some overlap)
3. **sLoop capitalization** (minor - generally consistent)

---

## 1. Voxel / Lattice Site / Cell

### Findings

| Term | Occurrences | Primary Usage |
|------|-------------|---------------|
| **voxel** | 90+ | Primary term for lattice sites throughout |
| **cell** | ~30 | Used in two contexts: (1) synonym for voxel, (2) biological cells |
| **lattice site** | 0 | Not used |

### Usage by Chapter

- **Consistent "voxel" usage:** Most chapters (2.2-voxel-anatomy.qmd, 1.6-the-causal-loop.qmd, 2.1-the-planck-scale.qmd, etc.)
- **"Cell" used synonymously:**
  - `0.2-mathematics.qmd`: "Moore neighborhood of a cell includes all 26 adjacent cells"
  - `0.1-first-principles.qmd`: "Each cell has state" and "max speed = 1 cell/tick"
  - `0.3-philosophy.qmd`: "1 cell/tick = c"
  - `preface.qmd`: "Three possible states per cell"
  - `1.1-the-void.qmd`: "Every cell exists in exactly one state"

### Recommendation

**STANDARDIZE ON: "voxel"**

- Use "voxel" for all references to lattice sites
- Reserve "cell" exclusively for biological contexts (cell membranes, cell walls, etc.)
- Update ~15 instances where "cell" means "voxel"
- Exception: Keep "cell/tick" in mathematical/formal notation where it's clearly understood

### Files Requiring Updates

1. `preface.qmd` (line 16)
2. `index.qmd` (line 126)
3. `0.1-first-principles.qmd` (lines 41, 55, 57)
4. `0.2-mathematics.qmd` (lines 77, 86, 92, 93, 101)
5. `0.3-philosophy.qmd` (line 152)
6. `1.1-the-void.qmd` (line 25)
7. `1.3-the-two-layers.qmd` (line 21)
8. `14.5-assumption-ledger.qmd` (lines 50, 52)
9. `14.3-glossary.qmd` (line 322 - but this correctly defines voxel)

---

## 2. Flux / Flux Field / J Field

### Findings

| Term | Occurrences | Context |
|------|-------------|---------|
| **flux** | 200+ | General references to the J vector |
| **flux field** | 50+ | When emphasizing it's a field |
| **J field** | 0 | Not used as standalone term |
| **J** | 100+ | In equations and formal notation |

### Current Usage Pattern

- "flux" is the default informal term
- "flux field" used when emphasizing field nature or first introduction
- "J" used in equations: $\mathbf{J}$, $|\mathbf{J}|$, $\nabla \cdot \mathbf{J}$

### Recommendation

**CURRENT USAGE IS CONSISTENT - NO CHANGES NEEDED**

The pattern "flux" (informal) / "flux field" (formal) / "J" (equations) is correctly applied throughout.

---

## 3. Manifestation / Genesis / Particle Creation

### Findings

| Term | Occurrences | Meaning |
|------|-------------|---------|
| **manifestation** | 50+ | The general concept of 0 → ±1 transition |
| **genesis** | 35+ | Specific event of particle creation |
| **pair production** | 10+ | Physics terminology for particle-antiparticle creation |
| **particle creation** | ~3 | Informal description |

### Current Usage Pattern

- **"manifestation"**: Abstract concept, the process itself
- **"genesis"**: The specific event instance
- **"pair production"**: Standard physics terminology

### Recommendation

**CURRENT USAGE IS CONSISTENT - MINOR CLARIFICATION NEEDED**

Add to glossary: "Genesis is an instance of manifestation; manifestation is the process, genesis is the event."

---

## 4. Void / Vacuum / Empty State / State 0 / s = 0

### Findings

| Term | Occurrences | Context |
|------|-------------|---------|
| **void** | 70+ | Primary term for state 0 |
| **vacuum** | 45+ | Physics context (vacuum energy, vacuum fluctuations) |
| **state 0** | 15+ | Formal/technical references |
| **s = 0** | 20+ | Equations |
| **empty state** | 0 | Not used |

### Current Usage Pattern

The terms are used appropriately in different contexts:
- "void" = FTD-specific term for the unmanifested substrate
- "vacuum" = standard physics terminology (vacuum energy, vacuum expectation value)
- "state 0" / "s = 0" = formal mathematical notation

### Recommendation

**CURRENT USAGE IS CORRECT - NO CHANGES NEEDED**

The distinction between "void" (FTD ontology) and "vacuum" (physics concept) is intentional and should be maintained. The glossary at 14.3-glossary.qmd correctly defines:
- "Vacuum: State 0 (void). Not empty—carries flux."

---

## 5. Ternary / Three-State

### Findings

| Term | Occurrences | Files |
|------|-------------|-------|
| **ternary** | 12 | about.qmd, 0.1-first-principles.qmd, 1.2-the-first-division.qmd, 1.10-lemniscate-alpha.qmd, index.qmd, 14.7-sloop-formalization.qmd, 14.8-information-quantification.qmd, 2.3-the-particle-zoo.qmd |
| **three-state** | 0 | Not used |
| **three state** | 0 | Not used |

### Recommendation

**FULLY CONSISTENT - "ternary" is the standard term**

---

## 6. Tick / Time Step / Discrete Time

### Findings

| Term | Occurrences | Context |
|------|-------------|---------|
| **tick** | 40+ | Primary term |
| **time step** | 1 | 0.2-mathematics.qmd: "finite time steps" |
| **time-step** | 0 | Not used |
| **discrete time** | 2 | Formal context |

### Recommendation

**STANDARDIZE ON: "tick"**

The single occurrence of "time steps" in 0.2-mathematics.qmd (line 166) should be changed to "ticks" for consistency.

**File to update:** `0.2-mathematics.qmd` line 166: "finite time steps" → "finite ticks"

---

## 7. sLoop / Self-Loop / Causal Loop

### Findings

| Term | Occurrences | Meaning |
|------|-------------|---------|
| **sLoop** | 60+ | The self-referential consciousness structure |
| **causal loop** | 8 | The update cycle (Chapter 1.6) |
| **self-loop** | 0 | Not used |
| **self-Loop** | 1 | In 14.7-sloop-formalization.qmd definition |

### Current Usage

- **sLoop** (with lowercase s, capital L) is the standard form
- **"causal loop"** refers specifically to the update cycle in Chapter 1.6
- These are DIFFERENT concepts and should remain distinct

### Recommendation

**CURRENT USAGE IS CORRECT**

- "sLoop" = self-referential consciousness structure
- "causal loop" = the tick update cycle
- The one instance of "self-Loop" in the definition is intentional (explaining etymology)

---

## 8. KB / K_B / Manifestation Threshold

### Findings

| Notation | Occurrences | Context |
|----------|-------------|---------|
| **KB** | 50+ | In code, informal text |
| **K_B** | 40+ | In LaTeX equations ($K_B$) |
| **manifestation threshold** | 11 | Descriptive text |

### Inconsistent Examples

**Using KB (no underscore):**
- `1.9-constants.qmd`: "KB = 0.511", "Increase KB", "Decrease KB"
- `1.11-the-action-principle.qmd`: "ρ crosses KB"
- `14.1-constants-reference.qmd`: "Symbol | KB"

**Using K_B (with underscore):**
- `1.1-the-void.qmd`: "$K_B = 0.511$"
- `1.2-the-first-division.qmd`: "$K_B$" in equations
- `0.2-mathematics.qmd`: "$K_B$" in equations
- `14.7-sloop-formalization.qmd`: "$K_B$"

### Recommendation

**STANDARDIZE ON: K_B in LaTeX, KB in code/informal**

This is actually the current practice, but should be made explicit:
- In mathematical expressions: $K_B$ (subscript B)
- In code blocks and parameter listings: `KB` (no subscript)
- In prose: "the manifestation threshold KB" or "K_B"

The current mixed usage is **acceptable** given these contexts. However, some cleanup would help:

**Suggested standardization rule:**
- LaTeX/equations: $K_B$
- Code/config: `KB`
- Prose: "K_B" or "the threshold K_B"

---

## 9. Wave Function / Wavefunction

### Findings

| Term | Occurrences | Files |
|------|-------------|-------|
| **wavefunction** | 9 | preface.qmd, 6.2-metals-and-conductors.qmd, 2.4-quantum-phenomena.qmd, 2.3-the-particle-zoo.qmd, 14.7-sloop-formalization.qmd |
| **wave function** | 0 | Not used |

### Recommendation

**FULLY CONSISTENT - "wavefunction" (one word) is the standard**

---

## 10. Greek Letters and Mathematical Notation

### Findings

| Symbol | Usage | Standard |
|--------|-------|----------|
| **alpha / α / $\alpha$** | 232 occurrences | Consistent: $\alpha$ in LaTeX, "alpha" in prose |
| **ALPHA** | 20 occurrences | Used for the parameter name in code/config |
| **hbar / $\hbar$ / ℏ** | 33 occurrences | Consistent: $\hbar$ in LaTeX |

### Greek Letter Patterns

The manuscript consistently uses:
- LaTeX notation in equations: $\alpha$, $\hbar$, $\pi$, $\phi$
- Spelled out in prose when appropriate: "the fine structure constant alpha"
- CAPS for parameter names: ALPHA, KB, PHI

### Recommendation

**CURRENT USAGE IS CONSISTENT - NO CHANGES NEEDED**

---

## Summary of Required Changes

### High Priority (Terminology Standardization)

| Issue | Files Affected | Action |
|-------|---------------|--------|
| "cell" → "voxel" | 9 files | Replace ~15 instances |
| "time steps" → "ticks" | 1 file | Replace 1 instance |

### Low Priority (Style Polish)

| Issue | Files Affected | Action |
|-------|---------------|--------|
| K_B vs KB | Many | Add style note to CLAUDE.md about when to use each |

### No Changes Needed

- Flux/flux field usage - CONSISTENT
- Manifestation/genesis usage - CONSISTENT
- Void/vacuum distinction - INTENTIONAL
- Ternary terminology - CONSISTENT
- sLoop/causal loop distinction - INTENTIONAL
- Wavefunction spelling - CONSISTENT
- Greek letter notation - CONSISTENT

---

## Detailed Change List

### File: `preface.qmd`
- Line 16: "Three possible states per cell" → "Three possible states per voxel"

### File: `index.qmd`
- Line 126: "Every cell in space exists" → "Every voxel in space exists"

### File: `0.1-first-principles.qmd`
- Line 55: "Each cell has state" → "Each voxel has state"
- Line 57: "1 cell/tick" → "1 voxel/tick"

### File: `0.2-mathematics.qmd`
- Line 77: "Moore neighborhood of a cell" → "Moore neighborhood of a voxel"
- Line 86: "states each cell can take" → "states each voxel can take"
- Line 92: "per cell" → "per voxel"
- Line 93: "26-cell Moore" → "26-voxel Moore"
- Line 101: "1 cell per tick" → "1 voxel per tick"
- Line 166: "finite time steps" → "finite ticks"

### File: `0.3-philosophy.qmd`
- Line 152: "1 cell/tick" → "1 voxel/tick"

### File: `1.1-the-void.qmd`
- Line 25: "Every cell exists" → "Every voxel exists"

### File: `1.3-the-two-layers.qmd`
- Line 21: "1 cell/tick" → "1 voxel/tick"

### File: `14.5-assumption-ledger.qmd`
- Line 50: "Each cell has state" → "Each voxel has state"
- Line 52: "1 cell/tick" → "1 voxel/tick"

---

## Appendix: Terminology Reference Table

For future reference, the canonical terminology is:

| Concept | Standard Term | Alternatives (OK in context) | Avoid |
|---------|---------------|------------------------------|-------|
| Lattice site | **voxel** | - | cell (except biological) |
| Vector field | **flux** / **flux field** | J (in equations) | J field |
| State 0 | **void** | vacuum (physics context), s = 0 | empty state |
| 0 → ±1 transition | **manifestation** / **genesis** | pair production (physics) | particle creation |
| Discrete time unit | **tick** | - | time step |
| Self-referential loop | **sLoop** | - | self-loop |
| Update cycle | **causal loop** | tick sequence | - |
| Threshold parameter | **K_B** (math) / **KB** (code) | manifestation threshold | Kb, kb |
| QM state | **wavefunction** | ψ | wave function |
| Fine structure | **α** (math) / **ALPHA** (code) | alpha (prose) | - |

---

*Report complete. Total inconsistencies requiring correction: 17 instances across 9 files.*
