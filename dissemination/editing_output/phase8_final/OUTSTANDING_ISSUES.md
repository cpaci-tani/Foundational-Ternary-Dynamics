# FTD Manuscript - Outstanding Issues

**Document:** Ternary Realization Dynamics
**Date:** 2026-01-10
**Status:** Pre-Publication Review

---

## Issue Tracking Summary

| Severity | Count | Status |
|----------|-------|--------|
| 🔴 CRITICAL | 5 | Blocking publication |
| 🟠 HIGH | 5 | Should fix before publication |
| 🟡 MEDIUM | 4 | Recommended improvements |
| 🟢 LOW | 4 | Optional polish |
| **Total** | **18** | |

---

## 🔴 CRITICAL Issues (P0)

### C-001: Missing Citations Throughout Manuscript
**Location:** All chapters
**Description:** Only 5 formal citations exist in 11.2-gravitational-waves.qmd. 58 of 63 bibliography entries are unused. ~50+ claims require citations.
**Impact:** Academic standards not met; could be flagged in peer review
**Resolution:**
1. Add `[@codata2022]` to all fine structure constant references
2. Add `[@pdg2024]` to all particle mass tables
3. Add `[@bell1964]` to Bell theorem discussions
4. Add historical citations (Sakharov, Hawking, Bekenstein, etc.)
**Estimated Time:** 4-6 hours
**Assigned:** TBD

---

### C-002: Missing Bibliography Entry for Cloud-9
**Location:** 15.1-observational-confirmations.qmd
**Description:** Leisman et al. 2025 referenced but not in references.bib
**Impact:** Broken reference, incomplete scholarship
**Resolution:** Add BibTeX entry:
```bibtex
@article{leisman2025cloud9,
  author = {Leisman, Lukas and others},
  title = {Cloud-9: Ultra-Diffuse Galaxy with Spherical Dark Matter Halo},
  journal = {TBD},
  year = {2025},
  note = {Cited as observational confirmation of FTD predictions}
}
```
**Estimated Time:** 15 minutes
**Assigned:** TBD

---

### C-003: VEV Derivation Formula Incorrect
**Location:** 2.5-the-higgs-mechanism.qmd, Line 68
**Description:** Formula `v = K_B / alpha * sqrt(2) = 246 GeV` is mathematically incorrect:
- K_B = 0.511 MeV, α = 1/137
- Calculation: 0.511 MeV × 137 × √2 = 99 MeV ≠ 246 GeV
**Impact:** Mathematical credibility undermined
**Resolution:** Either:
1. Correct the formula to match 246 GeV, OR
2. Clarify that K_B here represents a different scale, OR
3. Use the correct derivation: v = m_P √(2π) α⁸
**Estimated Time:** 30 minutes
**Assigned:** TBD

---

### C-004: CKM Phase Discrepancy
**Location:** 2.6-flavor-physics.qmd, Lines 174-175
**Description:** Derived δ_CKM = 148° but experimental value is ~68°
- Formula: δ = 2π × 7/17 = 148°
- Experimental: ~68-70°
**Impact:** Claimed derivation appears falsified by data
**Resolution:** Either:
1. Explain phase convention difference (180° - 148° ≈ 32°, still wrong)
2. Correct the formula
3. Add disclaimer about phase convention ambiguity
**Estimated Time:** 1 hour
**Assigned:** TBD

---

### C-005: K_B vs Planck Energy Confusion
**Location:** 2.1-the-planck-scale.qmd, Line 20
**Description:** Text states "E_P (Planck energy): Defined as K_B" but:
- K_B = 0.511 MeV (electron mass scale)
- E_P = 1.22 × 10¹⁹ GeV (Planck energy)
- These differ by 24 orders of magnitude
**Impact:** Fundamental conceptual error
**Resolution:** Clarify:
- K_B is the manifestation threshold (~m_e)
- E_P is the Planck energy (lattice spacing scale)
- They are related via α hierarchy, not equal
**Estimated Time:** 30 minutes
**Assigned:** TBD

---

## 🟠 HIGH Priority Issues (P1)

### H-001: Figures Not Embedded in Chapters
**Location:** All 71 chapter .qmd files
**Description:** 300+ generated figures exist but zero `![](...)` embeds in source
**Impact:** Figures won't appear in rendered output
**Resolution:**
1. Run `replace_ascii_diagrams.py`
2. Manually add figure references where needed
3. Use syntax: `![Caption](../figures/chXX/fig-name.png){#fig-id}`
**Estimated Time:** 2-3 hours
**Assigned:** TBD

---

### H-002: Evidence Figures Not Generated
**Location:** manuscript/figures/
**Description:** 22 planned evidence figures (E01-E22) for validation graphics not found
**Impact:** Key scientific evidence visualizations missing
**Resolution:** Run `generate_evidence_figures.py`
**Estimated Time:** 1 hour
**Assigned:** TBD

---

### H-003: Gas Constant R Not Defined
**Location:** 5.1-states-of-matter.qmd, Line 74
**Description:** PV = nRT equation uses R without definition
**Impact:** Reader confusion, incomplete technical writing
**Resolution:** Add: "where R = 8.314 J/(mol·K) is the ideal gas constant"
**Estimated Time:** 10 minutes
**Assigned:** TBD

---

### H-004: Mixed Natural Units Convention
**Location:** 2.4-quantum-phenomena.qmd
**Description:** Some equations use explicit ℏ, others assume ℏ = 1
**Impact:** Inconsistent dimensional analysis
**Resolution:** Choose one convention and apply throughout:
- Option A: Natural units (ℏ = c = 1) stated at chapter start
- Option B: Explicit ℏ, c in all equations
**Estimated Time:** 1 hour
**Assigned:** TBD

---

### H-005: Mass Tables Lack Units
**Location:** 2.3-the-particle-zoo.qmd
**Description:** Mass columns show values (0.6, 0.511, 2.1) without unit specification
**Impact:** Ambiguous whether values are MeV, GeV, or FTD units
**Resolution:** Add header row specifying "Mass (MeV)" or similar
**Estimated Time:** 30 minutes
**Assigned:** TBD

---

## 🟡 MEDIUM Priority Issues (P2)

### M-001: "Cell" vs "Voxel" Terminology
**Location:** 9 files, 17 instances
**Description:** "Cell" used where "voxel" is the standard FTD term
**Files:**
- preface.qmd (1)
- index.qmd (1)
- 0.1-first-principles.qmd (2)
- 0.2-mathematics.qmd (6)
- 0.3-philosophy.qmd (1)
- 1.1-the-void.qmd (1)
- 1.3-the-two-layers.qmd (1)
- 14.5-assumption-ledger.qmd (2)
**Resolution:** Find and replace "cell" with "voxel" in context
**Estimated Time:** 30 minutes
**Assigned:** TBD

---

### M-002: Display Equation Spacing
**Location:** Multiple chapters
**Description:** Some display equations lack blank lines before/after
**Impact:** Minor formatting inconsistency
**Resolution:** Add blank line before and after all `$$...$$` blocks
**Estimated Time:** 1 hour
**Assigned:** TBD

---

### M-003: Chapter Numbering Inconsistency
**Location:** index.qmd, Line 31
**Description:** "Chapter 67" referenced but file system uses "14.5"
**Impact:** Confusion about chapter numbering scheme
**Resolution:** Either:
1. Change to "Chapter 14.5"
2. Add note explaining sequential vs hierarchical numbering
**Estimated Time:** 15 minutes
**Assigned:** TBD

---

### M-004: Key Equations Lack Numbers
**Location:** Throughout Books I-XIV
**Description:** Important equations not numbered for cross-reference
**Impact:** Difficult to reference specific equations
**Resolution:** Add `{#eq-name}` labels to key equations
**Estimated Time:** 2 hours
**Assigned:** TBD

---

## 🟢 LOW Priority Issues (P3)

### L-001: Unused Bibliography Entries
**Location:** references.bib
**Description:** 58 of 63 entries never cited
**Impact:** Bibliography appears over-prepared
**Resolution:** Either add citations or create "Further Reading" section
**Estimated Time:** 2 hours
**Assigned:** TBD

---

### L-002: Double Blank Lines
**Location:** Various chapters
**Description:** Occasional double blank lines in source
**Impact:** Cosmetic inconsistency
**Resolution:** Remove extra blank lines
**Estimated Time:** 30 minutes
**Assigned:** TBD

---

### L-003: K_B vs KB Notation
**Location:** Throughout
**Description:** Mixed use of $K_B$ (LaTeX) and KB (code/prose)
**Impact:** Minor inconsistency
**Resolution:** Document convention:
- $K_B$ in equations
- `KB` in code
- K_B in prose
**Estimated Time:** 1 hour (documentation only)
**Assigned:** TBD

---

### L-004: Duplicate Bibliography Entry
**Location:** references.bib
**Description:** `abbott2016` and `abbott2016_gw150914` are essentially the same
**Impact:** Redundancy
**Resolution:** Remove `abbott2016`, keep more specific key
**Estimated Time:** 5 minutes
**Assigned:** TBD

---

## Issue Resolution Workflow

### Pre-Publication Checklist

**Phase 1: Critical Issues (Must complete)**
- [ ] C-001: Add all missing citations
- [ ] C-002: Add Leisman et al. 2025 to bibliography
- [ ] C-003: Correct VEV derivation
- [ ] C-004: Resolve CKM phase discrepancy
- [ ] C-005: Clarify K_B vs E_P

**Phase 2: High Priority (Should complete)**
- [ ] H-001: Embed figures in chapters
- [ ] H-002: Generate evidence figures
- [ ] H-003: Define gas constant R
- [ ] H-004: Standardize units convention
- [ ] H-005: Add units to mass tables

**Phase 3: Medium Priority (Recommended)**
- [ ] M-001: Standardize voxel terminology
- [ ] M-002: Fix equation spacing
- [ ] M-003: Clarify chapter numbering
- [ ] M-004: Add equation labels

**Phase 4: Low Priority (If time permits)**
- [ ] L-001: Clean up bibliography
- [ ] L-002: Remove extra blank lines
- [ ] L-003: Document notation conventions
- [ ] L-004: Remove duplicate bib entry

---

## Estimated Total Remediation Time

| Priority | Issues | Time |
|----------|--------|------|
| Critical (P0) | 5 | 6-8 hours |
| High (P1) | 5 | 4-5 hours |
| Medium (P2) | 4 | 4 hours |
| Low (P3) | 4 | 4 hours |
| **Total** | **18** | **18-21 hours** |

---

*Generated: 2026-01-10*
*Status: Active tracking*
