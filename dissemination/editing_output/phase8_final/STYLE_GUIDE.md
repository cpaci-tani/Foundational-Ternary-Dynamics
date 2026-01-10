# FTD Manuscript Style Guide

**Document:** Ternary Realization Dynamics
**Version:** 1.0
**Date:** 2026-01-10
**Purpose:** Standardize formatting and terminology for publication

---

## 1. Terminology Standards

### 1.1 Core FTD Terminology

| Concept | Standard Term | Avoid | Notes |
|---------|---------------|-------|-------|
| Lattice site | **voxel** | cell (except biological) | Primary discrete spatial unit |
| Vector field | **flux** / **flux field** | J field | Use "J" only in equations |
| State 0 | **void** | empty state | FTD-specific term |
| Physics empty space | **vacuum** | void | Standard physics context |
| 0 → ±1 transition | **manifestation** | particle creation | Abstract concept |
| Specific 0 → ±1 event | **genesis** | birth | Instance of manifestation |
| ±1 → 0 → ±1 pair | **pair production** | - | Physics terminology |
| Discrete time unit | **tick** | time step | FTD standard |
| Update cycle | **causal loop** | - | Chapter 1.6 concept |
| Self-referential structure | **sLoop** | self-loop | Consciousness structure |
| Manifestation threshold | **K_B** | KB (code), Kb | Energy threshold |
| QM state function | **wavefunction** | wave function | One word |

### 1.2 Notation Conventions

| Context | Format | Example |
|---------|--------|---------|
| Mathematical expressions | LaTeX subscript | $K_B$, $\alpha$, $\hbar$ |
| Code/config blocks | Plain caps | `KB`, `ALPHA` |
| Prose reference | Subscript or plain | "the threshold K_B" or "KB" |

### 1.3 Epistemic Tags

Use these tags to indicate claim status:

| Tag | Meaning | Example |
|-----|---------|---------|
| **[AXIOM]** | Foundational postulate | "[AXIOM] Space is a 3D cubic lattice" |
| **[THEOREM]** | Proven from axioms | "[THEOREM] α = 1/137.036 follows from..." |
| **[SELECTION]** | Argued but not proven unique | "[SELECTION] j = 1728 is preferred" |
| **[CONJECTURE]** | Proposed interpretation | "[CONJECTURE] Triads correspond to nucleons" |
| **[IMPOSED]** | Parameter choice | "[IMPOSED] γ = α for dissipation rate" |
| **[EMERGENT]** | Arises from dynamics | "[EMERGENT] U(1) gauge symmetry" |
| **[OPEN]** | Unresolved question | "[OPEN] Full QFT correspondence" |

---

## 2. Formatting Standards

### 2.1 Headers

```markdown
# Chapter Title {#sec-identifier}

## Major Section

### Subsection

#### Sub-subsection (use sparingly)
```

- H1 (`#`): Chapter titles only, with Quarto anchor
- H2 (`##`): Major conceptual divisions
- H3 (`###`): Detailed subtopics
- H4 (`####`): Rare, for deep nesting only

### 2.2 Equations

**Display equations:**
```markdown
The energy is given by:

$$
E = mc^2
$$

This fundamental relation shows...
```
- Always include blank line before and after `$$`
- Use `\begin{aligned}...\end{aligned}` for multi-line

**Inline equations:**
```markdown
The coupling constant $\alpha \approx 1/137$ determines...
```
- Use `$...$` for variables and short expressions

**Equation labels (recommended for key equations):**
```markdown
$$
x^2 - 16c^2x + 16c^3 = 0
$$ {#eq-master-quadratic}
```

### 2.3 Lists

**Unordered lists:**
```markdown
- First item
- Second item
  - Nested item
```
- Use hyphen (`-`) consistently

**Ordered lists:**
```markdown
1. First step
2. Second step
```

**Definition lists:**
```markdown
**Term**
: Definition text here
```

### 2.4 Tables

```markdown
| Column 1 | Column 2 | Column 3 |
|----------|----------|----------|
| Data 1   | Data 2   | Data 3   |
```
- Use pipe delimiters
- Include header separator row
- Align columns for readability in source

### 2.5 Callout Blocks

```markdown
::: {.callout-note}
## Key Insight

Content explaining the important point.
:::
```

Types available:
- `.callout-note` - General information
- `.callout-important` - Critical points
- `.callout-warning` - Caveats, limitations
- `.callout-tip` - Practical guidance

### 2.6 Code Blocks

**Python code:**
````markdown
```python
def example():
    return True
```
````

**ASCII diagrams / pseudo-code:**
````markdown
```
  ○───○
 /|  /|
○───○ |
```
````

**Inline code:**
```markdown
The `is_locked` flag controls...
```

---

## 3. Cross-Reference Standards

### 3.1 Section References

**Preferred (Quarto syntax):**
```markdown
See @sec-lemniscate-alpha for the derivation.
```

**Acceptable (when Quarto ref not available):**
```markdown
See Chapter 1.10 for the derivation.
```

### 3.2 Equation References

```markdown
From @eq-master-quadratic, we derive...
```

### 3.3 Figure References

```markdown
As shown in @fig-three-states, the void can...
```

### 3.4 Citation References

```markdown
This was first shown by Bell [@bell1964].
Multiple sources confirm this [@planck2020; @pdg2024].
```

---

## 4. Units and Dimensions

### 4.1 Natural Units Convention

The manuscript uses natural units where:
- c = 1 (speed of light)
- ℏ = 1 (reduced Planck constant)
- 1 voxel = Planck length
- 1 tick = Planck time

**When explicit units needed:**
- State at beginning: "In SI units..." or "Restoring ℏ and c..."
- Use standard abbreviations: GeV, MeV, keV, m, s, K

### 4.2 Physical Constants Format

| Constant | Symbol | Value |
|----------|--------|-------|
| Fine structure | α | 1/137.036 or 0.00729... |
| Planck constant | ℏ | 1 (natural) or 1.055 × 10⁻³⁴ J·s |
| Speed of light | c | 1 (natural) or 3 × 10⁸ m/s |
| Electron mass | m_e | 0.511 MeV/c² |
| Planck mass | m_P | 1.22 × 10¹⁹ GeV/c² |

---

## 5. Mathematical Notation

### 5.1 Vectors

- Bold roman: **J**, **F**, **v**
- LaTeX: `$\mathbf{J}$` or `$\vec{J}$`
- Magnitude: |**J**| or `$|\mathbf{J}|$`

### 5.2 Operators

| Operator | LaTeX | Meaning |
|----------|-------|---------|
| Gradient | `$\nabla f$` | ∂f/∂x, ∂f/∂y, ∂f/∂z |
| Divergence | `$\nabla \cdot \mathbf{J}$` | ∂Jx/∂x + ∂Jy/∂y + ∂Jz/∂z |
| Curl | `$\nabla \times \mathbf{J}$` | Standard definition |
| Laplacian | `$\nabla^2 f$` | Discrete: Σ neighbors - 6f |

### 5.3 Greek Letters

| Letter | Usage | LaTeX |
|--------|-------|-------|
| α | Fine structure constant | `$\alpha$` |
| γ | Decay rate, Lorentz factor | `$\gamma$` |
| ρ | Density | `$\rho$` |
| ψ | Wave function | `$\psi$` |
| φ | Phase, golden ratio | `$\phi$` |
| θ | Angles (Weinberg, mixing) | `$\theta$` |
| λ | Wolfenstein parameter | `$\lambda$` |

### 5.4 Subscripts and Superscripts

- Particle masses: m_e, m_p, m_W, m_H
- Coupling strengths: α_s, α_G, α_em
- Spatial indices: J_x, J_y, J_z or J_1, J_2, J_3
- Time derivatives: ∂_t, ∂²_t

---

## 6. Figure Standards

### 6.1 Figure Placement

```markdown
![Descriptive caption explaining the figure content](../figures/chXX/fig-name.png){#fig-id width="80%"}
```

### 6.2 Figure Naming

Format: `fig-<topic>-<descriptor>.png`

Examples:
- `fig-void-three-states.png`
- `fig-lemniscate-alpha-curve.png`
- `fig-master-quadratic-roots.png`

### 6.3 Figure Formats

- PNG: Primary format for print/PDF
- SVG: Vector format for HTML output
- Both formats should exist for each figure

---

## 7. Citation Standards

### 7.1 Required Citations

**Experimental values must cite source:**
- Particle masses: PDG (2024)
- α, fundamental constants: CODATA (2022)
- Cosmological parameters: Planck Collaboration

**Theoretical claims must cite original work:**
- Bell's theorem: Bell (1964)
- Hawking radiation: Hawking (1974)
- Sakharov conditions: Sakharov (1967)

### 7.2 Citation Format

```bibtex
@article{author2024title,
  author = {Author, First and Second, Author},
  title = {Paper Title},
  journal = {Journal Name},
  volume = {XX},
  pages = {YYY--ZZZ},
  year = {2024},
  doi = {10.xxxx/xxxxx}
}
```

---

## 8. File Naming Conventions

### 8.1 Chapter Files

Format: `X.Y-chapter-title.qmd`

Examples:
- `1.10-lemniscate-alpha.qmd`
- `2.3-the-particle-zoo.qmd`
- `14.5-assumption-ledger.qmd`

### 8.2 Figure Files

Format: `fig-<descriptor>.png` / `fig-<descriptor>.svg`

Location: `manuscript/figures/chXX/`

---

## 9. Quality Checklist

Before finalizing any chapter:

- [ ] All equations have blank lines before/after
- [ ] Key terms use standard FTD vocabulary
- [ ] Epistemic tags mark claim types
- [ ] Cross-references use `@sec-` syntax
- [ ] Citations added for external claims
- [ ] Figures embedded with captions
- [ ] Units specified in tables
- [ ] Code blocks have language tags

---

*Style Guide Version 1.0*
*Based on FTD Manuscript Audit 2026-01-10*
