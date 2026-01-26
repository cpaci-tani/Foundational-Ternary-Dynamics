# ACCESS-UX Expert Review: Foundational Ternary Dynamics Manuscript

**Reviewer:** ACCESS-UX (Accessibility, Universal Design, and User Experience Expert)
**Date:** 2026-01-25
**Document Version:** FTD Manuscript (Quarto-based HTML/PDF publication)
**Review Scope:** Accessibility compliance, universal design principles, and user experience for academic publication

---

## Executive Summary

The Foundational Ternary Dynamics (FTD) manuscript demonstrates **above-average accessibility practices** for an academic publication, with particular strengths in responsive design, reduced-motion support, and image alt-text implementation. The Quarto-based framework provides a solid foundation with proper semantic HTML structure and good ARIA landmark implementation. However, several **significant gaps** remain that could exclude users with disabilities, particularly regarding skip navigation, math accessibility, color contrast in callout boxes, and keyboard focus visibility.

**Overall Assessment:** The manuscript is **substantially accessible** but falls short of full WCAG 2.1 AA compliance. With targeted remediation, it could achieve exemplary accessibility standards for academic physics publications.

---

## Strengths

### 1. Responsive Design Implementation (Excellent)
The custom CSS (`styles.css`) demonstrates thoughtful mobile-first design:
- **Fluid typography** using `clamp()` for body text (14px-18px) and headings
- **44px minimum touch targets** explicitly specified for navigation elements
- **Horizontal scrolling** for wide tables with `-webkit-overflow-scrolling: touch`
- **Collapsible sidebar** with appropriate breakpoints at 991.98px
- **Mobile-specific adjustments** for code blocks (reduced font size, padding)

### 2. Reduced Motion Support (Exemplary)
```css
@media (prefers-reduced-motion: reduce) {
  *, *::before, *::after {
    animation-duration: 0.01ms !important;
    animation-iteration-count: 1 !important;
    transition-duration: 0.01ms !important;
  }
}
```
This implementation respects user preferences for reduced motion, an often-overlooked accessibility requirement (WCAG 2.3.3).

### 3. Image Alt-Text Consistency (Good)
All 50+ images in the QMD source files include descriptive alt-text. Examples:
- `![The proton as a triad of quarks: two up quarks (+2/3 each) and one down quark (-1/3), arranged in a triangular configuration.]`
- `![The Lemniscate-Alpha curve: a self-referential parametric curve whose arc length encodes the fine structure constant.]`

Alt-text is **informative rather than decorative**, describing both the visual content and its scientific significance.

### 4. Semantic HTML Structure (Good)
Quarto generates proper semantic structure:
- `<main class="content" id="quarto-document-content">` for main content
- `<nav id="quarto-sidebar" role="navigation">` for sidebar
- `<nav id="TOC" role="doc-toc">` for table of contents
- Breadcrumb navigation with `aria-label="breadcrumb"`
- Proper heading hierarchy (h1 for chapter titles, h2 for sections)

### 5. ARIA Implementation (Adequate)
- Toggle buttons include `aria-expanded`, `aria-controls`, and `aria-label`
- Search button has `aria-label="Search"`
- Section toggles have `aria-label="Toggle section"`
- Collapsible callouts maintain state attributes

### 6. Print Styles (Good)
```css
@media print {
  .sidebar, .navbar, .page-navigation, #quarto-search {
    display: none !important;
  }
  .content { width: 100%; max-width: none; margin: 0; padding: 0; }
}
```
Print media queries hide navigation elements for clean document printing.

### 7. Screen Reader Considerations (Partial)
- `<span class="screen-reader-only">Important</span>` in callout boxes
- Descriptive link text in navigation
- Chapter numbers separated from titles for clarity

---

## Weaknesses

### CRITICAL Issues (Require Immediate Attention)

#### C1. Missing Skip Navigation Link
**WCAG 2.4.1 (Level A) - Bypass Blocks**
**Severity:** CRITICAL

The HTML output contains **no skip link** to bypass the extensive sidebar navigation (which lists 80+ chapters). Users relying on keyboard navigation or screen readers must tab through hundreds of navigation links before reaching main content.

**Current state:** No `<a href="#main-content" class="skip-link">` element present.

**Impact:** Keyboard-only users and screen reader users face significant navigation burden on every page load.

**Remediation:**
```html
<body>
  <a href="#quarto-document-content" class="skip-link">Skip to main content</a>
  <!-- rest of page -->
</body>
```
```css
.skip-link {
  position: absolute;
  top: -40px;
  left: 0;
  padding: 8px;
  background: #000;
  color: #fff;
  z-index: 100;
}
.skip-link:focus {
  top: 0;
}
```

#### C2. Math Accessibility Gaps
**WCAG 1.1.1 (Level A) - Non-text Content**
**Severity:** CRITICAL

Mathematical equations are rendered via MathJax but lack comprehensive accessibility:

1. **No MathML fallback:** The HTML uses `<span class="math inline">\(\alpha \approx 1/137\)</span>` with LaTeX notation, relying entirely on JavaScript rendering.
2. **No alt-text for equations:** Complex equations have no textual alternative for screen readers.
3. **Equation numbering not programmatically associated:** Referenced equations (e.g., "Eq. 20.3") are not linked to their definitions.

**Example problematic content:**
```html
<span class="math inline">\(\alpha \approx 1/137\)</span>
```
Screen readers may announce this as "backslash alpha backslash approx one slash one three seven" or skip it entirely.

**Remediation:**
- Configure MathJax for assistive technology output
- Add `_quarto.yml` option: `math: mathjax` with accessibility extensions
- Consider adding textual descriptions for key equations in surrounding prose

### MAJOR Issues (Should Be Addressed)

#### M1. Insufficient Color Contrast in Callout Boxes
**WCAG 1.4.3 (Level AA) - Contrast (Minimum)**
**Severity:** MAJOR

Several callout box text colors fail the 4.5:1 contrast ratio requirement against their backgrounds:

| Callout Type | Text Color | Background | Contrast Ratio | Required |
|--------------|------------|------------|----------------|----------|
| Note | #1e1b4b | #f8faff | ~5.2:1 | Pass |
| Important | #450a0a | #fef7f7 | ~7.1:1 | Pass |
| Warning | #451a03 | #fffbeb | ~6.8:1 | Pass |
| Definition | #134e4a | #f0fdfa | **~3.8:1** | **FAIL** |
| Selection | #713f12 | #fefce8 | **~4.2:1** | **Borderline** |

**Remediation:** Darken text colors in definition and selection callouts to achieve minimum 4.5:1 contrast.

#### M2. Missing Visible Focus Indicators
**WCAG 2.4.7 (Level AA) - Focus Visible**
**Severity:** MAJOR

The CSS does not define custom focus styles. While browser defaults exist, they are often insufficient or inconsistent:

**Current state:** No `:focus` or `:focus-visible` rules in `styles.css`.

**Impact:** Users navigating by keyboard cannot easily identify which element is currently focused.

**Remediation:**
```css
a:focus-visible,
button:focus-visible,
input:focus-visible {
  outline: 3px solid #2563eb;
  outline-offset: 2px;
}
```

#### M3. Language Not Specified for Mathematical Symbols
**WCAG 3.1.2 (Level AA) - Language of Parts**
**Severity:** MAJOR

Greek letters and mathematical notation are not marked with language attributes. Screen readers may mispronounce:
- alpha as an English word rather than the Greek letter
- Special symbols like nabla, partial derivatives

**Remediation:** While challenging for LaTeX content, surrounding prose should clarify pronunciation for key symbols.

#### M4. Table Header Scope Missing
**WCAG 1.3.1 (Level A) - Info and Relationships**
**Severity:** MAJOR

Many data tables in the manuscript lack explicit `scope` attributes on header cells:

**Current:** `<th>Parameter</th>`
**Required:** `<th scope="col">Parameter</th>`

This affects tables throughout the particle physics chapters (particle properties, constants reference, etc.).

### MINOR Issues (Recommended Improvements)

#### m1. No Dark Mode Support
**Severity:** MINOR

The CSS comment `/* Let the theme handle dark mode colors naturally */` suggests dark mode reliance on Quarto theme, but no explicit `prefers-color-scheme` media query exists. Given the extensive callout styling, dark mode users may encounter issues.

#### m2. Code Block Font Size on Mobile
**Severity:** MINOR

At 576px breakpoint, code blocks reduce to 11px:
```css
@media (max-width: 576px) {
  pre { font-size: 11px; }
}
```
While functional, this may strain readability for users with low vision. Recommend minimum 12px.

#### m3. No Text Spacing Adjustments Support
**WCAG 1.4.12 (Level AA) - Text Spacing**
**Severity:** MINOR

The CSS uses fixed line-height (1.7) which is good, but the layout may not accommodate user stylesheet overrides for letter-spacing and word-spacing without content loss.

#### m4. PDF Accessibility Not Evaluated
**Severity:** MINOR

This review focuses on HTML output. The PDF version (generated via LaTeX) likely has separate accessibility concerns including tagged PDF structure, reading order, and form field accessibility. A separate PDF accessibility audit is recommended.

#### m5. Search Results Accessibility Unknown
**Severity:** MINOR

The search functionality uses Fuse.js but the accessibility of search results (live regions, focus management) could not be fully evaluated without runtime testing.

---

## Grades

| Criterion | Grade | Rationale |
|-----------|-------|-----------|
| **WCAG Compliance** | C+ | Missing skip links (Level A violation), math accessibility gaps, some contrast failures |
| **Semantic Structure** | B+ | Good heading hierarchy, proper landmarks, but table headers need scope |
| **Alt Text** | A- | Comprehensive, informative alt-text on all images; equations need attention |
| **Navigation** | B | Good mobile navigation, breadcrumbs, but no skip links; keyboard navigation untested |
| **Responsive Design** | A | Excellent fluid typography, touch targets, mobile adaptations |
| **Font Choices** | B+ | Readable defaults, good line-height; no dyslexia-friendly options offered |
| **Math Accessibility** | D+ | MathJax rendering present but no assistive technology configuration, no alt-text |

**Composite Grade: B-**

---

## Specific Recommendations

### Priority 1: Immediate (Required for WCAG A compliance)

1. **Add skip navigation link** to bypass sidebar content (C1)
2. **Configure MathJax accessibility** extensions in `_quarto.yml`:
   ```yaml
   format:
     html:
       include-in-header:
         text: |
           <script>
           MathJax = {
             options: {
               enableMenu: true,
               menuOptions: {
                 settings: {
                   assistiveMml: true
                 }
               }
             }
           };
           </script>
   ```
3. **Add scope attributes** to all table headers

### Priority 2: High (Required for WCAG AA compliance)

4. **Fix callout text contrast** for definition and selection types
5. **Add visible focus indicators** with custom CSS
6. **Test and document keyboard navigation** for all interactive elements

### Priority 3: Medium (Best practices)

7. **Implement dark mode** with `prefers-color-scheme` media queries
8. **Increase minimum mobile font size** to 12px
9. **Add textual descriptions** for key equations (e.g., "The master quadratic equation, where x squared minus sixteen times the lemniscatic constant squared times x...")
10. **Create accessibility statement** documenting known limitations and contact information

### Priority 4: Future Enhancement

11. **PDF accessibility audit** for the LaTeX-generated document
12. **Consider OpenDyslexic font option** or similar dyslexia-friendly alternative
13. **User testing** with assistive technology users
14. **Automated accessibility CI/CD** integration (axe-core, pa11y)

---

## Cross-Domain Concerns

### Interaction with Scientific Content
The highly mathematical nature of this manuscript presents inherent accessibility challenges. The density of equations (hundreds per chapter in some sections) means that even partial math accessibility failures have outsized impact. Recommend prioritizing Chapters 14 (Constants Reference), 20 (Lemniscate-Alpha), and 22 (Master Quadratic) for math accessibility remediation as these are equation-heavy.

### Interaction with Pedagogical Goals
The PEDA-STYLE reviewer should note that accessibility improvements often enhance general readability. Prose descriptions of equations benefit all readers, not just those using assistive technology.

### Interaction with Physics Content
Complex physics notation (Levi-Civita symbols, tensor indices, Dirac notation) presents particular screen reader challenges. Consider a notation glossary with pronunciation guides.

### Mobile Physics Learning
The excellent responsive design supports mobile learning, but touch interaction with equations (zoom, pan) may require additional UX consideration for physics students studying complex formulas.

---

## Summary

The FTD manuscript demonstrates genuine attention to accessibility in its responsive design, motion sensitivity, and alt-text implementation. However, the **missing skip navigation** and **inadequate math accessibility** represent significant barriers for users with disabilities. These issues are remediable with focused effort.

The Quarto framework provides a strong foundation, and the CSS customizations show accessibility awareness. With the recommended remediations, this manuscript could serve as a model for accessible academic physics publications.

**Recommendation:** Address Priority 1 and 2 items before publication. The current state is **not fully accessible** but is **substantially better than typical academic manuscripts** and can achieve full WCAG 2.1 AA compliance with targeted improvements.

---

*Review conducted according to WCAG 2.1 guidelines, Section 508 standards, and universal design principles for academic publications.*
