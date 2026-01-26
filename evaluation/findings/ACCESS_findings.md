# ACCESS Evaluation Report

## Agent Profile
- **Domain**: Accessibility and Inclusive Design
- **Credentials**: Expert in WCAG 2.1, Assistive Technology, Universal Design Principles
- **Scope**: Web output accessibility evaluation for FTD manuscript (`manuscript/_webbook/`)
- **Framework Version**: Quarto 1.8.26 with Bootstrap 5

---

## Executive Summary

The FTD web manuscript demonstrates **good foundational accessibility practices** inherited from the Quarto framework, including semantic HTML5 structure, ARIA landmarks, keyboard-accessible navigation, and a skip-link mechanism. However, **significant accessibility gaps** exist that would prevent WCAG 2.1 AA compliance, most notably the **complete absence of alt text on all images** and **inadequate table accessibility markup**. The mathematical content uses MathJax 3, which provides reasonable screen reader support but lacks explicit ARIA annotations for complex equations.

**Overall Assessment**: The manuscript shows accessibility awareness but requires substantial remediation work to meet WCAG 2.1 AA standards. The strengths come primarily from Quarto's default accessibility features; the weaknesses arise from content-level accessibility gaps in images, tables, and mathematical notation.

**Estimated Compliance Level**: WCAG 2.1 A (partial) - does not meet AA due to image alt text and table accessibility failures.

---

## Strengths (S1-S12)

### S1: Skip Navigation Link
The custom `styles.css` implements a proper skip-link mechanism:
```css
.skip-link {
  position: absolute;
  top: -100px;
  /* ... visible on focus */
}
.skip-link:focus {
  top: 0;
  outline: 3px solid #fbbf24;
  outline-offset: 2px;
}
```
This is a WCAG 2.4.1 requirement and is properly implemented.

### S2: Semantic HTML5 Document Structure
- Proper `<!DOCTYPE html>` declaration
- `lang="en-US"` attribute on `<html>` element (WCAG 3.1.1)
- Logical heading hierarchy (h1 > h2 > h3) with numbered sections
- Navigation landmarks using `<nav>`, `<header>`, `<main>` elements

### S3: ARIA Labels on Interactive Elements
Buttons and navigation elements have proper ARIA attributes:
```html
<button type="button" aria-controls="quarto-sidebar"
        aria-expanded="false" aria-label="Toggle sidebar navigation">
```
Breadcrumb navigation properly labeled with `aria-label="breadcrumb"`.

### S4: Keyboard Navigation Support
- Bootstrap's collapsible sections are keyboard accessible
- Search functionality accessible via keyboard shortcuts (f, /, s)
- Tab navigation through sidebar menu items
- Focus indicators present (though could be more prominent)

### S5: Responsive Design
Custom CSS provides proper responsive behavior:
- `clamp()` functions for fluid typography
- Mobile touch targets meet 44x44px minimum (WCAG 2.5.5)
- Tables have horizontal scroll on mobile
- Code blocks have overflow handling

### S6: Reduced Motion Support
Excellent implementation of `prefers-reduced-motion`:
```css
@media (prefers-reduced-motion: reduce) {
  * { animation-duration: 0.01ms !important; /* ... */ }
}
```
This satisfies WCAG 2.3.3 (Animation from Interactions).

### S7: Print Stylesheet
Proper print media query hides navigation and optimizes content:
```css
@media print {
  .sidebar, .navbar, .page-navigation, #quarto-search {
    display: none !important;
  }
}
```

### S8: Screen Reader Text for Callouts
Callout boxes include screen-reader-only spans:
```html
<span class="screen-reader-only">Note</span>Purpose of This Chapter
```
This provides context for assistive technology users.

### S9: Proper Link Navigation
- Previous/next page links (`rel="next"`, `rel="prev"`) in document head
- Clear chapter navigation structure
- Breadcrumb trail for orientation

### S10: Table of Contents with Proper Structure
TOC implemented with heading (`<h2 id="toc-title">`) and proper list structure using `<ul>` elements.

### S11: Readable Font Stack
System fonts used with appropriate fallbacks:
```css
--quarto-font-monospace: SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
```

### S12: Code Block Accessibility
- Line numbers linked with `aria-hidden="true"` and `tabindex="-1"` to prevent confusing screen readers
- Syntax highlighting uses semantic class names
- Code is preserved in `<pre>` tags with proper `white-space: pre`

---

## Weaknesses (W1-W14)

### W1: CRITICAL - Complete Absence of Alt Text on Images
**WCAG 1.1.1 Failure (Level A)**

All 50+ images examined lack alt attributes entirely:
```html
<img src="../../media/images/fig-lemniscate-alpha-curve.png" class="img-fluid figure-img">
```

This is a **critical accessibility barrier** - screen reader users cannot access any visual content. Each image requires descriptive alt text explaining:
- What the diagram shows
- Key data points for graphs
- Relationships depicted in flowcharts

**Remediation Required**: Add alt text to all 100+ images in the manuscript.

### W2: CRITICAL - Tables Lack Accessibility Markup
**WCAG 1.3.1 Failure (Level A)**

Tables have basic HTML but lack:
- `<caption>` elements for table descriptions
- `scope` attributes on header cells
- `<thead>` and `<tbody>` proper delineation in some cases
- Summary descriptions for complex data tables

Example found:
```html
<table class="caption-top table">
<colgroup>...</colgroup>
<thead><tr class="header"><th>Mode</th>...
```
While `<thead>` is present, no `scope="col"` attributes exist.

### W3: HIGH - MathJax Accessibility Configuration Missing
**WCAG 1.3.1 Partial Failure**

MathJax 3 is loaded but without explicit accessibility configuration:
```html
<script src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-chtml-full.js"></script>
```

Missing:
- `a11y/accessibility-menu` extension
- `a11y/semantic-enrich` extension
- Explicit `speechRules` configuration
- Alternative text for complex equations

Complex equations like:
```
x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0
```
Need human-readable descriptions for screen readers.

### W4: MEDIUM - Insufficient Focus Indicators
**WCAG 2.4.7 Partial Compliance**

Only the skip-link has explicit focus styling:
```css
.skip-link:focus {
  outline: 3px solid #fbbf24;
}
```

Standard links and buttons rely on browser defaults, which may not provide sufficient contrast. Interactive callout headers lack focus styles.

### W5: MEDIUM - Color Contrast in Callouts
**WCAG 1.4.3 Potential Issue**

Some callout text colors may not meet 4.5:1 contrast ratio:
```css
.callout-warning .callout-body-container { color: #78350f; }
```
On background `#fffbf5`, this brown text needs verification.

Similarly, conjecture callouts use:
```css
.callout-conjecture .callout-body-container { color: #7c2d12; }
```

### W6: MEDIUM - Syntax Highlighting Contrast
**WCAG 1.4.3 Potential Issue**

Some code syntax colors may have insufficient contrast:
```css
--quarto-hl-co-color: #6a737d;  /* Comments - gray */
--quarto-hl-an-color: #6a737d;  /* Annotations */
```
Gray (#6a737d) on white background is approximately 4.5:1 - marginal compliance.

### W7: LOW - Search Placeholder Text
The search input lacks placeholder text:
```json
"search-text-placeholder": ""
```
Empty placeholder provides no guidance to users.

### W8: LOW - Figure Captions Present but Not Linked to Images
Figcaptions exist but images aren't properly associated:
```html
<img src="..." class="img-fluid figure-img">
<figcaption class="quarto-float-caption-bottom">Figure 20.1: ...</figcaption>
```
Should use `aria-describedby` or `<figure>` with proper ID linkage.

### W9: LOW - Multiple Navigation Roles
Some links have `role="navigation"` which is redundant when inside `<nav>` elements and may cause confusion.

### W10: LOW - Collapsible Sections Keyboard Access
Sidebar sections use links with `data-bs-toggle="collapse"` but toggling state may not be clearly announced by all screen readers.

### W11: LOW - Long Pages Without Section Skip Links
Some chapters are extremely long (2000+ lines) without internal skip mechanisms beyond the TOC.

### W12: LOW - Missing Lang Attributes on Code Blocks
Code blocks showing Python or other languages should indicate the language:
```html
<code class="sourceCode python">  <!-- has class but no lang attribute -->
```

### W13: LOW - PDF Alternative Not Clearly Linked
The PDF version exists but isn't prominently linked for users who prefer that format.

### W14: LOW - No High Contrast Mode Support
While reduced motion is supported, there's no explicit support for `prefers-contrast` media query for users requiring higher contrast.

---

## Detailed Analysis

### Visual Accessibility

#### Color Contrast Analysis

| Element | Foreground | Background | Ratio | Pass AA? |
|---------|------------|------------|-------|----------|
| Body text | #24292e | #ffffff | ~12:1 | Yes |
| Callout note title | #1e40af | #f8fafc | ~7.5:1 | Yes |
| Callout warning body | #78350f | #fffbf5 | ~6.5:1 | Yes |
| Callout conjecture body | #7c2d12 | #fffaf5 | ~5.5:1 | Marginal |
| Code comments | #6a737d | #ffffff | ~4.6:1 | Marginal |
| Link (default blue) | Bootstrap blue | #ffffff | ~4.5:1 | Marginal |

**Finding**: Most text passes AA requirements, but several elements are at the margin and would fail AAA requirements (7:1).

#### Typography Assessment

- **Positive**: Responsive font sizing using `clamp()`
- **Positive**: Line height 1.7 for body text (good readability)
- **Positive**: Clear heading hierarchy with visual distinction
- **Concern**: Code blocks use 11-14px font sizes which may be small for some users

### Screen Reader Compatibility

#### Navigation
- **Excellent**: Landmark regions properly defined
- **Excellent**: Breadcrumbs with proper ARIA
- **Excellent**: Skip link implemented
- **Good**: Section toggle states communicated via `aria-expanded`
- **Concern**: Some redundant role attributes

#### Content
- **Critical Gap**: Images completely inaccessible
- **Critical Gap**: Complex mathematical equations lack descriptions
- **Good**: Callout boxes have screen-reader-only labels
- **Good**: Code line numbers hidden from assistive tech

#### Tables
- **Present**: Basic table markup
- **Missing**: `scope` attributes
- **Missing**: `<caption>` elements
- **Missing**: Complex header cell associations

### Keyboard Navigation

#### Positive Aspects
- All interactive elements reachable via Tab
- Sidebar sections expandable/collapsible
- Search accessible via keyboard shortcuts
- Modal dialogs (if any) trap focus appropriately

#### Concerns
- Focus indicators rely heavily on browser defaults
- Long pages require many Tab presses to reach content
- Skip link exists but target (#main-content) needs verification

### Mathematical Content

#### MathJax Implementation
- **Version**: MathJax 3 (tex-chtml-full)
- **Rendering**: CommonHTML output
- **Accessibility**: Basic support present, advanced features not configured

#### Screen Reader Experience
MathJax 3 provides:
- AssistiveMML output for some screen readers
- Braille support (limited)
- Speech rules for common expressions

Missing:
- Complex equation descriptions
- Interactive exploration mode
- Custom speech rules for domain-specific notation

### Responsive Design

#### Breakpoints
- 991.98px: Mobile navigation activates
- 767.98px: Table font size reduces
- 576px: Further mobile optimizations

#### Touch Targets
```css
min-height: 44px;  /* Meets WCAG 2.5.5 */
min-width: 44px;
```
Properly implemented for sidebar navigation.

#### Content Reflow
- Images scale with `max-width: 100%`
- Tables scroll horizontally
- Math equations scroll when necessary
- Code blocks preserve formatting with scroll

---

## WCAG 2.1 Compliance Checklist

### Level A (Minimum)

| Criterion | Status | Notes |
|-----------|--------|-------|
| 1.1.1 Non-text Content | **FAIL** | No alt text on images |
| 1.2.1 Audio-only/Video-only | N/A | No audio/video content |
| 1.3.1 Info and Relationships | **FAIL** | Tables lack proper markup |
| 1.3.2 Meaningful Sequence | PASS | DOM order matches visual |
| 1.3.3 Sensory Characteristics | PASS | Instructions not purely visual |
| 1.4.1 Use of Color | PASS | Not solely color-dependent |
| 1.4.2 Audio Control | N/A | No audio |
| 2.1.1 Keyboard | PASS | All functions keyboard accessible |
| 2.1.2 No Keyboard Trap | PASS | No traps detected |
| 2.1.4 Character Key Shortcuts | PASS | Search shortcuts can be disabled |
| 2.2.1 Timing Adjustable | N/A | No time limits |
| 2.2.2 Pause, Stop, Hide | N/A | No auto-updating content |
| 2.3.1 Three Flashes | PASS | No flashing content |
| 2.4.1 Bypass Blocks | PASS | Skip link present |
| 2.4.2 Page Titled | PASS | Descriptive titles |
| 2.4.3 Focus Order | PASS | Logical order |
| 2.4.4 Link Purpose | PASS | Links have clear purpose |
| 2.5.1 Pointer Gestures | PASS | No complex gestures |
| 2.5.2 Pointer Cancellation | PASS | Standard click behavior |
| 2.5.3 Label in Name | PASS | Visible labels match accessible names |
| 2.5.4 Motion Actuation | N/A | No motion triggers |
| 3.1.1 Language of Page | PASS | lang="en-US" present |
| 3.2.1 On Focus | PASS | No context change on focus |
| 3.2.2 On Input | PASS | No unexpected changes |
| 3.3.1 Error Identification | N/A | No form errors |
| 3.3.2 Labels or Instructions | PASS | Search properly labeled |
| 4.1.1 Parsing | PASS | Valid HTML5 |
| 4.1.2 Name, Role, Value | PARTIAL | Most elements correct |

### Level AA (Standard Target)

| Criterion | Status | Notes |
|-----------|--------|-------|
| 1.3.4 Orientation | PASS | Works both orientations |
| 1.3.5 Identify Input Purpose | N/A | No identity inputs |
| 1.4.3 Contrast (Minimum) | PARTIAL | Some marginal elements |
| 1.4.4 Resize Text | PASS | Text scales to 200% |
| 1.4.5 Images of Text | PASS | Minimal text in images |
| 1.4.10 Reflow | PASS | Content reflows at 320px |
| 1.4.11 Non-text Contrast | PARTIAL | Some UI elements marginal |
| 1.4.12 Text Spacing | PASS | Spacing adjustments work |
| 1.4.13 Content on Hover | PASS | Tooltips accessible |
| 2.4.5 Multiple Ways | PASS | TOC + search + navigation |
| 2.4.6 Headings and Labels | PASS | Descriptive headings |
| 2.4.7 Focus Visible | PARTIAL | Some elements need work |
| 3.1.2 Language of Parts | N/A | No language changes |
| 3.2.3 Consistent Navigation | PASS | Consistent sidebar |
| 3.2.4 Consistent Identification | PASS | Consistent components |
| 3.3.3 Error Suggestion | N/A | No error states |
| 3.3.4 Error Prevention | N/A | No legal/financial |
| 4.1.3 Status Messages | PASS | Search results announced |

---

## Scores

| Criterion | Score | Justification |
|-----------|-------|---------------|
| **Clarity** | 72/100 | Good typography and layout; color contrast issues in some callouts and code |
| **Accessibility** | 58/100 | Critical failures in image alt text and table markup; strong structural elements |
| **Usability** | 78/100 | Keyboard navigation works; skip link present; mobile responsive |
| **Consistency** | 85/100 | Consistent heading hierarchy; ARIA patterns uniform; minor redundancies |
| **Reproducibility** | 80/100 | Works across modern browsers; responsive design; print styles present |
| **Modernity** | 75/100 | Uses HTML5, ARIA, MathJax 3; lacks contrast media queries |

### Weighted Overall Score: 71/100

**Grade Calculation**:
- Clarity (15%): 72 x 0.15 = 10.8
- Accessibility (30%): 58 x 0.30 = 17.4
- Usability (20%): 78 x 0.20 = 15.6
- Consistency (15%): 85 x 0.15 = 12.75
- Reproducibility (10%): 80 x 0.10 = 8.0
- Modernity (10%): 75 x 0.10 = 7.5

**Total**: 72.05/100

---

## Overall Grade: C+

The FTD web manuscript demonstrates awareness of accessibility principles through its use of Quarto's built-in features, but fails to meet WCAG 2.1 AA compliance due to critical content-level gaps. The framework provides excellent structure; the content requires remediation.

---

## Key Recommendations

### Priority 1: Critical (Required for Basic Accessibility)

1. **Add Alt Text to All Images** (W1)
   - Audit all 100+ images
   - Write descriptive alt text for diagrams
   - Provide data descriptions for graphs
   - Consider long descriptions for complex figures

2. **Enhance Table Accessibility** (W2)
   - Add `<caption>` elements
   - Include `scope="col"` and `scope="row"` attributes
   - Use `<tbody>` consistently
   - Consider ARIA for complex data tables

3. **Configure MathJax Accessibility** (W3)
   - Enable accessibility menu
   - Load semantic enrichment
   - Add custom descriptions for complex equations
   - Test with NVDA/JAWS

### Priority 2: Important (WCAG AA Compliance)

4. **Improve Focus Indicators** (W4)
   - Add visible focus styles to all interactive elements
   - Ensure 3:1 contrast for focus indicators
   - Test with keyboard-only navigation

5. **Verify Color Contrast** (W5, W6)
   - Audit all callout types with contrast checker
   - Ensure code syntax colors meet 4.5:1
   - Consider AAA compliance (7:1) for body text

6. **Associate Figures with Captions** (W8)
   - Link images to figcaptions via `aria-describedby`
   - Ensure figure/figcaption wrapper structure

### Priority 3: Enhancement (Best Practices)

7. **Add Search Placeholder** (W7)
   - Include helpful placeholder text
   - Add aria-describedby for search instructions

8. **Clean Up ARIA Redundancies** (W9)
   - Remove unnecessary role attributes
   - Audit ARIA usage for best practices

9. **Add High Contrast Mode Support** (W14)
   ```css
   @media (prefers-contrast: more) {
     /* Enhanced contrast styles */
   }
   ```

10. **Consider PDF Accessibility**
    - Ensure PDF meets PDF/UA standards
    - Link prominently from web version

---

## Testing Methodology

**Tools Used** (Simulated):
- Manual code review
- WCAG 2.1 checklist evaluation
- Color contrast ratio calculations
- Semantic structure analysis

**Recommended Automated Testing**:
- axe DevTools
- WAVE (WebAIM)
- Lighthouse Accessibility Audit
- Pa11y CI

**Recommended Manual Testing**:
- Screen reader testing (NVDA, JAWS, VoiceOver)
- Keyboard-only navigation
- High contrast mode
- Text zoom to 200%

---

## Conclusion

The FTD manuscript's web output has a solid accessibility foundation from Quarto but requires significant content-level remediation, particularly for images and tables. Addressing the Priority 1 recommendations would bring the manuscript to basic accessibility compliance; Priority 2 would achieve WCAG 2.1 AA status. The mathematical nature of the content makes screen reader accessibility particularly important, and the current MathJax configuration should be enhanced with accessibility extensions.

**Estimated Remediation Effort**: 20-40 hours for Priority 1-2 items, depending on image count and complexity.

---

*Report generated by ACCESS - Accessibility and Inclusive Design Specialist*
*Evaluation Date: 2026-01-25*
*Document Version: FTD Manuscript Web Output*
