# Formatting Consistency Report

**Document:** FTD Manuscript
**Analysis Date:** 2026-01-10
**Files Analyzed:** 75+ .qmd files in `manuscript/` and `manuscript/chapters/`

---

## Executive Summary

The manuscript demonstrates **strong overall consistency** in formatting conventions. Most formatting elements follow clear patterns with only minor variations. This report documents the standards in use, identifies inconsistencies, and provides recommendations for final publication.

**Overall Assessment:** GOOD - Ready for publication with minor standardization needed

---

## 1. Header Levels

### Current Usage Pattern

| Level | Format | Usage |
|-------|--------|-------|
| H1 | `# Title {#sec-id}` | Chapter titles only |
| H2 | `## Section` | Major sections within chapters |
| H3 | `### Subsection` | Subsections, detailed topics |
| H4 | `#### Sub-subsection` | Rare, used for deep nesting |

### Observations

**CONSISTENT:**
- All chapter files begin with `# Chapter Title {#sec-identifier}`
- Cross-reference anchors consistently use `{#sec-...}` format
- H2 used for major conceptual divisions
- H3 used for detailed subtopics

**VARIATIONS FOUND:**
- Some callout boxes use H2 (`## Key Insight`) inside callout blocks
- Glossary uses H2 for letter groupings (`## A`, `## B`, etc.)

### Recommendation

**STANDARD:** Current usage is consistent. No changes needed.

---

## 2. Equation Formatting

### Current Usage Pattern

| Type | Format | Example |
|------|--------|---------|
| Display equation | `$$...$$` | Multi-line, centered equations |
| Inline equation | `$...$` | Single variables or short expressions |
| Aligned equations | `$$\begin{aligned}...\end{aligned}$$` | Multi-line derivations |

### Observations

**CONSISTENT:**
- Display equations use `$$...$$` on separate lines
- Inline math uses `$...$` for variables like `$\alpha$`, `$K_B$`
- Aligned environments used for multi-line derivations
- LaTeX math notation used throughout (no MathML or other formats)

**VARIATIONS FOUND:**
- Some files have blank lines before/after `$$`, others do not
- Equation labeling varies (some use `{#eq-label}`, most do not)

### Recommendation

**STANDARD:** Add blank lines before and after display equations for readability:
```markdown
The formula is:

$$
F = ma
$$

This shows...
```

**ACTION NEEDED:** Consider adding equation labels for key equations that are referenced elsewhere.

---

## 3. List Formatting

### Current Usage Pattern

| Type | Format | Example |
|------|--------|---------|
| Unordered | `-` (hyphen) | Most common |
| Ordered | `1.` | Numbered sequences |
| Definition | `**Term**:` or `**Term**\n: Definition` | Glossary entries |

### Observations

**CONSISTENT:**
- Unordered lists use `-` (hyphen) exclusively
- Ordered lists use `1.`, `2.`, etc.
- Nested lists properly indented with 2-4 spaces

**VARIATIONS FOUND:**
- Some definition lists use `:` syntax (Pandoc style), others use `**Term**: definition`
- Glossary uses proper definition list syntax: `**Term**\n: Definition`

### Recommendation

**STANDARD:**
- Use `-` for unordered lists (already consistent)
- Use Pandoc definition list syntax for glossary:
  ```markdown
  **Term**
  : Definition text
  ```

---

## 4. Bold/Italic Usage

### Current Usage Pattern

| Format | Usage | Examples |
|--------|-------|----------|
| `**bold**` | Key terms, emphasis, important concepts | `**DERIVED**`, `**matter**` |
| `*italic*` | Epigraphs, foreign terms, technical definitions | `*"Quote text"*` |
| `***bold italic***` | Not used | N/A |

### Observations

**CONSISTENT:**
- Epigraphs use `*"quoted text"*` format
- Key terms introduced in bold: `**triad**`, `**flux**`
- Epistemic tags in bold: `**[THEOREM]**`, `**[CONJECTURE]**`
- Status markers in bold: `**DERIVED**`, `**VERIFIED**`

**VARIATIONS FOUND:**
- Some files use `**Bold:**` before explanations, others don't
- Emphasis within sentences varies between bold and italic

### Recommendation

**STANDARD:**
- Bold for: terms being defined, epistemic tags, status markers, emphasis
- Italic for: epigraphs, quotes, foreign terms, book/paper titles
- Keep current patterns - they are internally consistent

---

## 5. Code Block Formatting

### Current Usage Pattern

| Type | Format | Usage |
|------|--------|-------|
| Inline code | `` `text` `` | Variable names, small expressions |
| Fenced code | ```` ```language ```` | Multi-line code, algorithms |
| Plain text blocks | ```` ``` ```` | ASCII diagrams |

### Observations

**CONSISTENT:**
- Python code blocks use ```` ```python ````
- ASCII diagrams use plain ```` ``` ```` without language
- Inline code for: `KB`, `is_locked`, `position_remainder`

**VARIATIONS FOUND:**
- Some pseudo-code lacks language specification
- Occasional mixing of code and prose within blocks

### Recommendation

**STANDARD:**
- Use ```` ```python ```` for Python code
- Use ```` ``` ```` (no language) for ASCII diagrams and pseudo-code
- Keep inline code for technical identifiers and variable names

---

## 6. Callout Block Formatting

### Current Usage Pattern

| Type | Syntax | Usage |
|------|--------|-------|
| Note | `::: {.callout-note}` | Key insights, clarifications |
| Important | `::: {.callout-important}` | Critical points, central claims |
| Warning | `::: {.callout-warning}` | Caveats, epistemic disclaimers |
| Tip | `::: {.callout-tip}` | Practical guidance, derivation hints |

### Observations

**HIGHLY CONSISTENT:**
- All callouts use Quarto's `::: {.callout-type}` syntax
- Each callout has a `## Title` inside
- Content follows the title with proper spacing
- Closing `:::` on separate line

**Standard Format:**
```markdown
::: {.callout-note}
## Title Here

Content text goes here.
:::
```

**VARIATIONS FOUND:**
- Some callouts have blank lines after title, some don't
- Occasional nested callouts (should be avoided)

### Recommendation

**STANDARD:** Current format is excellent. Maintain:
```markdown
::: {.callout-type}
## Descriptive Title

Content paragraph(s).
:::
```

---

## 7. Table Formatting

### Current Usage Pattern

All tables use Markdown pipe table syntax:

```markdown
| Column 1 | Column 2 | Column 3 |
|----------|----------|----------|
| Data 1   | Data 2   | Data 3   |
```

### Observations

**CONSISTENT:**
- Header row followed by separator row
- Columns aligned with pipes
- Consistent use of left-alignment (default)

**VARIATIONS FOUND:**
- Some tables have extra spacing for visual alignment in source
- Column width varies (cosmetic, not functional)
- Some tables lack caption/title

### Recommendation

**STANDARD:** Current table format is correct. For publication:
- Consider adding table captions using Quarto's `{#tbl-id}` syntax where needed
- Ensure all tables render properly in target output format (PDF, HTML)

---

## 8. Figure Caption Formatting

### Observations

The manuscript uses minimal figures (primarily text and equations). When images are referenced:
- Not applicable for most chapters (text-based content)
- ASCII diagrams used instead of images

### Recommendation

If figures are added:
```markdown
![Caption text](path/to/image.png){#fig-id}
```

---

## 9. Cross-Reference Formats

### Current Usage Pattern

| Type | Format | Example |
|------|--------|---------|
| Section | `@sec-identifier` | `@sec-lemniscate-alpha` |
| Equation | `@eq-identifier` | (rarely used) |
| Table | `@tbl-identifier` | (rarely used) |
| Figure | `@fig-identifier` | (rarely used) |

### Observations

**CONSISTENT:**
- Section references use `@sec-...` format
- Chapter identifiers defined with `{#sec-...}` in H1
- Cross-chapter references work correctly

**VARIATIONS FOUND:**
- Some internal references use plain text instead of `@sec-` links
- Example: "see Chapter 1.11" vs "see @sec-action-principle"

### Recommendation

**ACTION NEEDED:** Standardize all cross-references to use `@sec-` format:
- Replace "Chapter X.Y" with `@sec-identifier` where possible
- This enables automatic link generation in output

---

## 10. Spacing and Line Breaks

### Current Usage Pattern

| Element | Spacing |
|---------|---------|
| After H1 | 1 blank line |
| After H2, H3 | 1 blank line |
| Between paragraphs | 1 blank line |
| Before/after equations | 1 blank line (mostly) |
| Before/after code blocks | 1 blank line |
| Before/after callouts | 1 blank line |
| Before/after tables | 1 blank line |

### Observations

**MOSTLY CONSISTENT:**
- Paragraphs separated by single blank lines
- Headers followed by blank line, then content
- Code blocks and callouts have proper spacing

**VARIATIONS FOUND:**
- Some display equations lack blank line before/after
- Occasional double blank lines (cosmetic inconsistency)
- Some list items have extra spacing

### Recommendation

**STANDARD:**
- Single blank line between all block elements
- Ensure display equations have blank lines before and after
- Remove any double blank lines

---

## Summary of Inconsistencies and Actions

### Critical Issues (Must Fix)

None identified - the manuscript is publication-ready.

### Minor Issues (Recommend Fixing)

| Issue | Priority | Action |
|-------|----------|--------|
| Cross-reference format | Medium | Standardize to `@sec-` syntax |
| Equation spacing | Low | Add blank lines around `$$...$$` |
| Double blank lines | Low | Remove for cleaner source |

### Style Variations (Acceptable)

| Variation | Status |
|-----------|--------|
| Definition list syntax | Both styles work, keep as-is |
| Table column spacing | Cosmetic, no impact |
| Callout spacing variations | Minor, acceptable |

---

## Recommended Standards for Future Editing

### Quick Reference

```markdown
# Chapter Title {#sec-identifier}

## Major Section

### Subsection

::: {.callout-note}
## Callout Title

Content here.
:::

Paragraph text with **bold** for emphasis and *italic* for quotes.

$$
\text{Display equation}
$$

Inline math like $\alpha$ and $K_B$ for variables.

- Unordered list item
- Another item

1. Ordered list
2. Second item

| Column | Column |
|--------|--------|
| Data   | Data   |

See @sec-other-chapter for more details.

```python
def code_example():
    pass
```
```

---

## Conclusion

The FTD manuscript demonstrates **excellent formatting consistency** across 75+ files. The authors have maintained clear standards for:

1. Header hierarchy
2. Equation formatting (LaTeX)
3. List styles (hyphen for unordered)
4. Callout blocks (Quarto syntax)
5. Table formatting (pipe tables)
6. Bold/italic emphasis patterns

The minor variations identified do not affect readability or functionality. The manuscript is **ready for publication** with optional cleanup of cross-reference formats and equation spacing.

**Final Status: APPROVED for formatting consistency**
