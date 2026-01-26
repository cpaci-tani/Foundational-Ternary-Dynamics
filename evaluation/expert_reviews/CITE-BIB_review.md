# Expert Review: Bibliography and Citation Standards

**Reviewer:** CITE-BIB (PhD, Bibliography, Citation Standards, and Academic Referencing)
**Document:** Foundational Ternary Dynamics (FTD) Manuscript
**Date:** 2026-01-25
**Review Type:** Formal Academic Review

---

## Executive Summary

This review evaluates the bibliographic and citation practices in the FTD manuscript, focusing on the `references.bib` file and citation usage patterns across representative chapters. The manuscript demonstrates **professional-grade bibliographic infrastructure** with notable strengths in source quality and formatting, but exhibits significant gaps in citation density for foundational claims and shows inconsistent citation practices across chapters.

**Overall Grade: B-**

---

## Detailed Evaluation

### 1. CITATION COMPLETENESS

**Grade: C+**

#### Findings:

**Strengths:**
- Key observational claims in Chapter 15 (Observational Confirmations) are properly cited with specific references (e.g., `@anand2025cloud9`, `@gao2025qsi`, `@desi2025`, `@wang2025bell`)
- Experimental physics claims include appropriate citations to CODATA, Particle Data Group, LIGO/Virgo collaborations
- The gravitational waves chapter (11.2) exemplifies good citation practice with `@einstein1916`, `@abbott2016_gw150914`, `@gwtc3`

**Weaknesses:**
- **Chapter 1.10 (Lemniscate-Alpha):** Central theoretical claims lack citations. The master quadratic derivation and arc length calculations reference no prior work in elliptic integral theory or mathematical constants. Only one citation (`@codata2022`) appears in 900+ lines.
- **Chapter 2.4 (Quantum Phenomena):** Despite discussing foundational quantum mechanics (Born rule, Bell inequalities, entanglement), only internal cross-references are used (e.g., `@sec-two-domains`). No citations to Bell (1964), Born (1926), or Aspect (1982) in the text, despite these being in the bibliography.
- **Chapter 0.3 (Philosophy):** Single citation to Kim (1999) in 850+ lines covering dispositional ontology, modal logic, and consciousness - areas with extensive philosophical literature.
- **Chapter 1.8 (Four Forces):** Zero citations for Yukawa potential, gauge symmetry, or force unification claims.

**Critical Gap:** The manuscript's most novel claims (Lemniscate-Alpha curve, master quadratic, G* derivation) are presented without citation to relevant mathematical literature on elliptic functions, complex multiplication, or number theory.

---

### 2. BIB FORMAT QUALITY

**Grade: A-**

#### Findings:

**Strengths:**
- Clean, well-organized BibTeX format with consistent field structure
- Proper use of `@article`, `@book`, `@incollection`, `@inproceedings` entry types
- Consistent author formatting (Last, First format with proper accent handling)
- DOIs provided for most entries where available
- Helpful organizational comments dividing sections (e.g., `% QUANTUM FOUNDATIONS`, `% GAUGE THEORY`)
- Proper LaTeX escaping for special characters (e.g., `{\"o}`, `{\'e}`)

**Minor Issues:**
- `@article{feynman1985}` incorrectly uses `publisher` field (should be `@book` type)
- Some entries use `note` field inconsistently for supplementary information
- Entry `@article{abbott2016}` and `@article{abbott2016_gw150914}` are duplicates with slightly different content

**Technical Compliance:** The file is valid BibTeX and would compile without errors in standard LaTeX workflows using BibLaTeX or natbib.

---

### 3. SOURCE QUALITY

**Grade: A**

#### Findings:

**Exceptional Strengths:**
- Core physics citations draw from top-tier journals: Physical Review Letters, Nature, Science, Physical Review D, Astronomy & Astrophysics
- Historical foundational works properly included: Einstein (1905, 1915, 1916), Bell (1964), Yang-Mills (1954), Higgs (1964), Weinberg (1967)
- Contemporary cutting-edge research: DESI 2025, GWTC-3 2023, JILA eEDM 2023, LHCb CP violation 2025
- Authoritative data sources: CODATA 2022, PDG 2022/2024, Planck Collaboration 2020
- Appropriate collaboration authorship for large experimental papers (ATLAS, CMS, LIGO, Planck, DESI, LHCb)

**Source Distribution by Type:**
| Category | Count | Quality Assessment |
|----------|-------|-------------------|
| Peer-reviewed articles | ~65 | Excellent - top journals |
| Books/monographs | ~12 | Good - authoritative texts |
| Collaboration papers | ~15 | Excellent - definitive sources |
| Conference proceedings | ~2 | Appropriate for context |
| Historical/primary sources | ~5 | Excellent for foundations |

**No Issues Found:** No predatory journals, non-peer-reviewed preprints without proper attribution, or questionable sources detected.

---

### 4. RECENCY

**Grade: A-**

#### Findings:

**Strengths:**
- Bibliography explicitly updated for "Oxford Submission - January 2026"
- Includes 2025 publications across multiple domains:
  - `@anand2025cloud9` (Cloud-9 discovery)
  - `@yi2026migdal` (Migdal effect - 2026)
  - `@gao2025qsi` (Quantum spin ice)
  - `@desi2025` (Dark energy)
  - `@wang2025bell` (Bell without entanglement)
  - `@lhcb2025cp` (CP violation in baryons)
- PDG updated to 2024 edition
- CODATA 2022 (most recent available)
- GWTC-3 (2023) for gravitational wave catalog

**Minor Gaps:**
- Inflation section could benefit from post-2020 Planck analysis updates
- Some mathematical constants references (Finch 2003) could be supplemented with more recent surveys

**Notable:** The 2025-2026 references demonstrate active engagement with current literature, unusual for a theoretical framework manuscript.

---

### 5. CITATION STYLE CONSISTENCY

**Grade: B**

#### Findings:

**Strengths:**
- Pandoc/Quarto `[@key]` citation syntax used consistently throughout
- Internal cross-references follow consistent `@sec-*` pattern
- Multiple citations formatted correctly (e.g., `[@pohl2010muonicH; @beyer2017electronicH]`)

**Inconsistencies Identified:**
- **Mixed citation density:** Chapter 15 averages 1 citation per 30 lines; Chapter 1.10 averages 1 citation per 900 lines
- **Quote attribution:** Some quotes attributed informally (e.g., "attributed to I.M. Kolthoff" without citation)
- **Figure citations:** Some figures reference papers in captions but not through formal citation
- **Internal vs external:** Heavy reliance on internal cross-references (`@sec-*`) where external citations would be more appropriate for established physics

**Style Framework:** The manuscript uses author-date style appropriate for scientific writing. No numerical or footnote styles mixed in.

---

### 6. SELF-CITATION

**Grade: A**

#### Findings:

The bibliography contains **zero self-citations**, which is appropriate for a novel theoretical framework without prior publication record. This is the correct approach for first publication.

**Note:** The manuscript references internal documents (e.g., `CLAUDE.md`, `THEORETICAL_FOUNDATIONS.md`, companion papers) but these are properly distinguished from external academic citations.

---

### 7. MISSING CITATIONS

**Grade: C**

#### Critical Missing References:

**Mathematical Foundations:**
- No citations for elliptic integral theory beyond Gauss (1866)
- Missing: Ramanujan's work on modular forms (relevant to complex multiplication claims)
- Missing: Silverman & Tate on elliptic curves
- Missing: Cox "Primes of the Form x^2 + ny^2" (complex multiplication)
- Missing: Zagier on L-functions and number theory

**Quantum Foundations:**
- Bell (1964) is in `.bib` but not cited in Chapter 2.4 text
- Born (1926) is in `.bib` but not cited where Born rule is discussed
- Missing: Everett (1957) for many-worlds comparison
- Missing: Ghirardi-Rimini-Weber (1986) for collapse model comparison
- Missing: Zurek decoherence literature

**Philosophy:**
- Missing: Mumford & Anjum "Getting Causes from Powers" (dispositional ontology)
- Missing: Bird "Nature's Metaphysics" (powers and dispositions)
- Missing: Ladyman & Ross "Every Thing Must Go" (ontic structural realism)
- Missing: Wallace on decoherence and emergence
- Whitehead (1929) in `.bib` but not cited in philosophy chapter

**Discrete Physics:**
- 't Hooft (2016) in `.bib` but not cited where cellular automaton interpretation discussed
- Missing: Wolfram (2020) "A Project to Find the Fundamental Theory of Physics"
- Missing: Loop quantum gravity literature (Rovelli, Thiemann)
- Missing: Causal set literature beyond Sorkin (2003)

**Gauge Theory:**
- Yang-Mills (1954) in `.bib` but not cited in gauge symmetry discussion
- Missing: Gell-Mann on color charge
- Missing: Polyakov on confinement

---

## Summary Grades

| Criterion | Grade | Weight | Weighted Score |
|-----------|-------|--------|----------------|
| Citation Completeness | C+ | 25% | 0.575 |
| BibTeX Format | A- | 10% | 0.925 |
| Source Quality | A | 20% | 1.00 |
| Recency | A- | 15% | 0.925 |
| Citation Style Consistency | B | 15% | 0.85 |
| Self-Citation Balance | A | 5% | 1.00 |
| Missing Citations | C | 10% | 0.70 |

**Overall Grade: B- (2.85/4.0)**

---

## Recommendations

### Immediate (Pre-Publication):

1. **Add in-text citations to Chapter 1.10:** The Lemniscate-Alpha derivation requires citations to:
   - Gauss/Legendre elliptic integral theory
   - Complex multiplication literature
   - Heegner number research (e.g., Stark, Ogg)

2. **Add in-text citations to Chapter 2.4:** Bell, Born, and Aspect papers are in the bibliography but not cited where their concepts are discussed.

3. **Add philosophical citations to Chapter 0.3:** Dispositional ontology and modal logic sections need engagement with contemporary literature (Mumford, Bird, Ladyman).

4. **Standardize citation density:** Establish minimum standard of 1 citation per 100 lines for theoretical chapters.

### Short-Term:

5. **Expand mathematical foundations bibliography:** Add references for:
   - Elliptic curve theory
   - Complex multiplication
   - Modular forms
   - Number-theoretic aspects of the derivations

6. **Add comparison literature:** Include citations to competing approaches (loop quantum gravity, causal sets, Wolfram's physics project) with explicit comparisons.

7. **Fix duplicate entry:** Remove `@article{abbott2016}` or `@article{abbott2016_gw150914}` (keep one).

8. **Correct entry type:** Change `@article{feynman1985}` to `@book{feynman1985}`.

### Long-Term:

9. **Develop companion bibliography:** Create a supplementary bibliography document with extended references for each major claim, appropriate for a book-length work.

10. **Citation index:** Consider adding a citation index mapping claims to supporting references.

---

## Conclusion

The FTD manuscript demonstrates professional bibliographic infrastructure with high-quality sources and current references. However, the citation density is highly uneven, with empirical chapters well-cited but theoretical chapters significantly under-cited. The most novel mathematical claims lack connection to the relevant literature, which will raise reviewer concerns about whether the authors are aware of prior work in these areas.

The primary barrier to publication is not source quality but citation completeness - specifically, the gap between having excellent sources in the bibliography and actually citing them where relevant in the text.

---

**Reviewer Certification:**
This review was conducted according to standard academic bibliographic evaluation criteria. The reviewer recommends **major revisions** to citation practices before submission to peer-reviewed venues.

*CITE-BIB*
*PhD, Bibliography and Citation Standards*
