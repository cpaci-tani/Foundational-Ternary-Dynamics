# Expert Review: Data Visualization and Scientific Figures

**Reviewer**: VIS-DATA (Tenured PhD Expert in Data Visualization, Scientific Figures, and Visual Communication)

**Manuscript**: Foundational Ternary Dynamics (FTD)

**Date**: 2026-01-25

**Review Type**: Formal Academic Evaluation of Visual Elements and Data Presentation

---

## Executive Summary

The FTD manuscript presents an ambitious attempt to visualize complex theoretical physics concepts through a combination of programmatically-generated figures, schematic diagrams, and mathematical typesetting. The visual infrastructure is substantial, with over 100 Python figure-generation scripts and approximately 50 referenced figures across the manuscript chapters. The quality is uneven: some figures effectively communicate core concepts while others fall short of publication standards. The LaTeX equation formatting is generally strong, though mobile responsiveness issues exist. The manuscript would benefit significantly from a dedicated figure revision pass before publication.

**Overall Visual Communication Grade: B-**

---

## 1. Figure Quality Assessment

### 1.1 Resolution and Technical Specifications

**Grade: B**

**Strengths**:
- Figures are generated programmatically via Python/matplotlib at 150 DPI (as seen in `fig_01_lemniscate_alpha.py`)
- PNG format is appropriate for web rendering
- Vector-style generation ensures clean lines and scalable elements
- Consistent output dimensions across the figure generation suite

**Weaknesses**:
- 150 DPI is adequate for screen but insufficient for print publication (300 DPI recommended)
- Some generated figures lack anti-aliasing on curves (visible jaggies on lemniscate)
- No SVG export option for true vector scalability
- Missing retina/high-DPI variants for modern displays

**Specific Observations**:
- The Lemniscate-Alpha curve figure (`fig-lemniscate-alpha-curve.png`) shows a clean lemniscate with colored parameter progression points, but the axis labels and scaling information are minimal
- The cosmic web survey slice (`fig-cosmic-web-survey-slice.png`) has adequate resolution but sparse labeling
- The Mandelbrot zoom (`fig-mandelbrot-zoom.png`) demonstrates proper colorbar implementation

**Recommendation**: Increase base DPI to 300 for all figures. Implement SVG export for critical mathematical diagrams. Add high-DPI variants.

---

### 1.2 Professional Appearance

**Grade: B+**

**Strengths**:
- Consistent color palette defined in a central `utils/style.py` module
- Clean white backgrounds throughout
- Professional font choices (likely matplotlib defaults)
- Thoughtful use of alpha transparency for overlapping elements

**Weaknesses**:
- "(schematic)" label appears in multiple figure titles, reducing professional appearance
- Some figures lack polish (e.g., sparse axis ticks, missing units)
- Inconsistent annotation styles across figures
- Missing figure borders or frames in some cases

**Specific Examples**:
- The Standard Model Overview (`fig-standard-model-overview.png`) is clean and well-organized with appropriate color-coding for quarks (red/pink) and leptons (blue)
- The Constants Dependency diagram (`fig-constants-dependency.png`) shows clear hierarchical flow with appropriate node coloring
- The Proton Configuration (`fig-proton-config.png`) is exceptionally clear with proper charge annotations

**Recommendation**: Remove "(schematic)" from titles or move to captions. Standardize annotation fonts and sizes. Add subtle borders where appropriate.

---

## 2. Data Visualization Effectiveness

### 2.1 Quantitative Graphs and Charts

**Grade: A-**

**Strengths**:
- The Mass Ratios log-log plot (`fig-evidence-13-mass-pred-vs-measured.png`) is exemplary:
  - Appropriate use of log-log scale for spanning multiple orders of magnitude
  - Clear y=x reference line (dashed green)
  - Annotation highlighting worst-case error (Z boson, 0.49%)
  - Arrow pointing to specific outlier
  - Professional labeling
- The Error Scoreboard (`fig-evidence-16-headline-scoreboard.png`) effectively communicates prediction accuracy:
  - Appropriate log scale for error magnitudes
  - Clear labeling of all predictions
  - Note explaining unit mixing (ppm vs %)
  - Green color choice conveys "success" semantically

**Weaknesses**:
- Error bars absent from quantitative comparisons
- No confidence intervals shown
- Some axis labels truncated at small sizes
- Missing grid lines on some plots that would benefit from them

**Specific Quantitative Figures Assessed**:
1. Mass predictions vs measured: **A** (excellent log-log visualization)
2. Error scoreboard: **A-** (good bar chart, could use error bars)
3. Derivation verification code blocks: **B+** (inline Python output is clear)

**Recommendation**: Add error bars/confidence intervals to all quantitative comparisons. Consider adding subtle grid lines to log-scale plots.

---

### 2.2 Conceptual Diagrams

**Grade: B**

**Strengths**:
- The Proton/Neutron configurations effectively show quark arrangements with clear charge labels
- The Constants Dependency flow diagram clearly shows derivation hierarchy
- Color-coded Standard Model overview is intuitive

**Weaknesses**:
- Many referenced figures appear to be missing or not yet generated (based on file references)
- Conceptual diagrams often lack sufficient detail
- Some diagrams are too sparse (e.g., cosmic web slice)
- Missing intermediate complexity diagrams for key concepts

**Critical Missing Figures** (referenced but not found as images):
- `fig-action-terms.png` (action principle visualization)
- `fig-two-layers-split.png` (flux/state layer diagram)
- `fig-genesis-pair-production.png` (pair production)
- `fig-force-unification.png` (force unification diagram)
- Many others referenced in chapters but only Python generators exist

**Recommendation**: Generate all referenced figures. Add more intermediate-complexity diagrams. Include process flow diagrams for key derivations.

---

## 3. Equation Formatting

### 3.1 LaTeX Quality

**Grade: A-**

**Strengths**:
- Consistent use of display math mode for important equations
- Proper equation numbering with `{#eq-label}` cross-referencing
- Good use of aligned environments for multi-line equations
- Appropriate sizing (not too large or small)
- Greek letters and special symbols rendered correctly

**Examples of Well-Formatted Equations**:
```latex
$$x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$$ {#eq-master-quadratic}

$$\alpha_G = \left(\frac{m_p}{m_P}\right)^2 = 2\pi \cdot \left(\frac{N_{\text{base}}^2}{N_c}\right)^2 \cdot \left(n_{\text{eff}} + \frac{N_c}{b_3}\right)^2 \cdot \alpha^{20}$$
```

**Weaknesses**:
- Some inline math causes horizontal scrolling on mobile (per styles.css overflow-x handling)
- Occasional inconsistent spacing around operators
- Some complex fractions would benefit from `\displaystyle`
- Missing equation numbering on some important results

**Mobile Responsiveness Issue**:
The `styles.css` shows mobile scaling to 90% for `mjx-container` at widths < 576px, which helps but may not be sufficient for very long equations.

**Recommendation**: Break very long equations into multiple lines. Add equation numbers consistently. Use `\displaystyle` for important inline fractions.

---

### 3.2 Mathematical Notation Consistency

**Grade: B+**

**Strengths**:
- Consistent use of bold for vectors ($\mathbf{J}$)
- Proper subscript/superscript formatting
- Good use of `\text{}` for word subscripts
- Framework integers consistently formatted

**Weaknesses**:
- Occasional mixing of notation styles (e.g., $N_c$ vs $N_{\text{c}}$)
- Some symbols defined in text but not in a comprehensive notation table
- Gradient and divergence operators sometimes vary ($\nabla$ vs $\grad$)

**Recommendation**: Create a comprehensive notation glossary figure. Ensure all symbols are defined at first use. Standardize operator notation.

---

## 4. Color Usage

### 4.1 Color Scheme Assessment

**Grade: B+**

**Strengths**:
- Defined color palette in `utils/style.py` with semantic naming (COLORS, MODE_COLORS)
- Matter/antimatter distinction using red/blue contrast (appropriate semantic choice)
- Callout boxes in `styles.css` use sophisticated, accessible color schemes:
  - Note (indigo/blue): `#4f46e5`
  - Important (red): `#dc2626`
  - Warning (amber): `#d97706`
  - Tip (green): `#059669`
- Purple used consistently for lemniscate curve across figures

**Weaknesses**:
- No explicit accessibility testing for color blindness
- Some figures rely heavily on color without pattern/shape alternatives
- Mode colors (1, 2, 4, 8, 16 frequencies) may be difficult to distinguish for colorblind readers
- Missing color legend in some multi-colored figures

**Color Blindness Considerations**:
- Red/blue quark distinction may be problematic for protanopia/deuteranopia
- The viridis-like colormap in Mandelbrot zoom is colorblind-friendly
- Cosmic web slice uses single purple hue (accessible)

**Recommendation**: Add pattern fills or shape markers as color alternatives. Test figures with colorblind simulation tools. Add explicit color legends.

---

### 4.2 Semantic Color Use

**Grade: A-**

**Strengths**:
- Red = matter/positive states (intuitive)
- Blue = antimatter/negative states (appropriate contrast)
- Green = success/verified (error scoreboard)
- Amber/yellow = warning/caution (consistent with conventions)
- Purple = primary theme color (distinctive)

**Weaknesses**:
- "Void" (state 0) has no consistent color representation
- Some diagrams use arbitrary colors without semantic meaning

**Recommendation**: Establish consistent void color (gray or transparent). Document color semantics in style guide.

---

## 5. Diagram Clarity

### 5.1 Conceptual Diagram Assessment

**Grade: B-**

**Strengths**:
- Quark configuration diagrams (proton, neutron) are exemplary
- Standard Model overview effectively organizes complex information
- Constants dependency flow is clear and hierarchical

**Weaknesses**:
- Many key conceptual diagrams are referenced but not generated
- Existing schematics often too sparse
- Missing scale bars, legends, and comprehensive annotations
- No 3D visualizations for lattice structure (crucial for understanding)

**Critical Missing Diagrams**:
1. **Voxel/Moore neighborhood visualization** - fundamental to understanding the model
2. **Flux field propagation animation/sequence** - dynamic concept needs visual support
3. **Triad binding geometry** - core particle concept
4. **Update cycle flowchart** - 12-step process needs visual guide
5. **sLoop mechanism** - complex self-referential concept

**Recommendation**: Prioritize generation of missing conceptual diagrams. Add 3D lattice visualizations. Create animated GIFs for dynamic processes (some exist in special_projects/).

---

### 5.2 Information Density

**Grade: B**

**Strengths**:
- Most figures avoid overcrowding
- Good use of white space
- Clear visual hierarchy in multi-element figures

**Weaknesses**:
- Some figures are too sparse (cosmic web slice, lemniscate curve)
- Others could convey more information without clutter
- Annotation density is inconsistent

**Recommendation**: Aim for moderate information density. Add annotations to sparse figures. Use insets for detail where appropriate.

---

## 6. Figure Captions

### 6.1 Caption Quality

**Grade: B+**

**Strengths**:
- Captions are present for all referenced figures
- Generally descriptive and informative
- Include key interpretive information
- Consistent formatting with Quarto figure syntax

**Example of Good Caption**:
> "The master quadratic and its roots: the electromagnetic coupling (1/alpha approximately 137) and color charges (N_c approximately 3) emerge from the same geometric structure."

**Weaknesses**:
- Some captions too long (approaching paragraph length)
- Missing figure source citations where applicable
- No distinction between figure description and interpretive content
- Inconsistent use of mathematical notation in captions

**Specific Issues**:
- Caption for `fig-cosmic-ray-spectrum.png` runs to multiple sentences but lacks specific value callouts
- Caption for `fig-lemniscate-curve` could include the equation parameters

**Recommendation**: Aim for 1-2 sentence captions with key values. Separate descriptive content from interpretive commentary. Add source citations.

---

### 6.2 Cross-Referencing

**Grade: A-**

**Strengths**:
- Consistent use of Quarto's `@fig-label` referencing system
- Figures properly numbered and referenced in text
- Cross-chapter references work correctly

**Weaknesses**:
- Some figures referenced before their appearance
- Occasional orphaned references to non-existent figures
- Missing list of figures in front matter

**Recommendation**: Add list of figures. Verify all cross-references resolve. Ensure figures appear near first reference.

---

## 7. Visual Consistency

### 7.1 Style Uniformity

**Grade: B**

**Strengths**:
- Central style module (`utils/style.py`) enforces consistency
- `apply_trd_style()` function standardizes axes appearance
- Consistent background colors and fonts within programmatic figures
- Callout box styling is highly polished and consistent

**Weaknesses**:
- Schematic figures generated differently from data plots
- Some figures appear hand-placed (sparse annotation styles)
- Figure border/frame treatment inconsistent
- Title placement varies (above figure vs. in figure)

**Specific Inconsistencies**:
- Lemniscate figure uses light gray axes; cosmic web has no axes
- Standard Model diagram has no frame; proton config has implicit frame
- Text annotation sizes vary across figures

**Recommendation**: Create comprehensive style guide document. Apply consistent framing. Standardize title placement.

---

### 7.2 Inter-Chapter Consistency

**Grade: B**

**Strengths**:
- Similar figure types look similar across chapters
- Color scheme maintained throughout
- Equation formatting consistent

**Weaknesses**:
- Early chapters have more polished figures than later chapters
- Foundational chapters (0.x, 1.x) have better visual support than application chapters
- Figure density varies significantly (some chapters figure-rich, others text-only)

**Figure Distribution Analysis**:
- Chapter 1.10 (Lemniscate-Alpha): 3 figures (appropriate)
- Chapter 2.6 (Flavor Physics): 0 figures (needs visualization)
- Chapter 1.12 (Gravity): 0 figures (needs visualization)
- Chapter 14.1 (Constants Reference): 2 figures (appropriate)

**Recommendation**: Balance figure distribution across chapters. Add visualizations to flavor physics and gravity chapters. Ensure consistent polish throughout.

---

## 8. Technical Infrastructure Assessment

### 8.1 Figure Generation Pipeline

**Grade: A-**

**Strengths**:
- Extensive Python figure generation infrastructure (70+ scripts)
- Centralized physics constants module
- Reproducible figure generation
- Clear naming conventions (`fig_XX_descriptive_name.py`)

**Weaknesses**:
- Not all scripts have corresponding generated PNG files
- No automated figure regeneration pipeline visible
- Missing CI/CD integration for figure updates
- Some scripts import from non-existent modules

**File Structure**:
```
media/images/
├── __init__.py
├── fig_01_lemniscate_alpha.py
├── fig_02_master_quadratic.py
├── ... (70+ Python scripts)
├── fig-lemniscate-alpha-curve.png
├── fig-standard-model-overview.png
├── ... (80+ PNG files)
```

**Recommendation**: Create automated build pipeline for figures. Add figure generation to CI. Verify all scripts generate output.

---

### 8.2 Responsive Design

**Grade: B+**

**Strengths**:
- Comprehensive mobile-responsive CSS (`styles.css`, 857 lines)
- Proper image scaling (`max-width: 100%; height: auto;`)
- Math equation overflow handling
- Touch-friendly navigation

**Weaknesses**:
- Long equations may still overflow on very small screens
- No picture element/srcset for responsive images
- Missing lazy loading for images
- No dark mode image variants

**Recommendation**: Implement srcset for figures. Add lazy loading. Create dark mode variants or ensure figures work on dark backgrounds.

---

## 9. Specific Figure Reviews

### 9.1 Outstanding Figures

1. **Mass Ratios Plot** (`fig-evidence-13-mass-pred-vs-measured.png`)
   - Grade: **A**
   - Exemplary use of log-log visualization
   - Clear error annotation
   - Professional quality

2. **Proton Configuration** (`fig-proton-config.png`)
   - Grade: **A**
   - Clean geometric representation
   - Clear charge labels
   - Appropriate color coding

3. **Constants Dependency** (`fig-constants-dependency.png`)
   - Grade: **A-**
   - Clear hierarchical flow
   - Good use of color-coded categories
   - Appropriate complexity level

### 9.2 Figures Needing Improvement

1. **Lemniscate-Alpha Curve** (`fig-lemniscate-alpha-curve.png`)
   - Grade: **B-**
   - Issue: Missing axis labels, equation annotations, legend
   - Recommendation: Add parametric equations, arc length, G* derivation

2. **Cosmic Web Survey** (`fig-cosmic-web-survey-slice.png`)
   - Grade: **C+**
   - Issue: Too sparse, missing scale, minimal labeling
   - Recommendation: Add scale bar, more labels, higher density

3. **Mandelbrot Zoom** (`fig-mandelbrot-zoom.png`)
   - Grade: **B**
   - Issue: Good colormap but small size, minimal context
   - Recommendation: Larger figure, connection to FTD concepts

---

## 10. Recommendations Summary

### Critical (Must Fix Before Publication)

1. **Generate all referenced figures** - Many figures are referenced in text but PNG files are missing
2. **Increase DPI to 300** for print-ready output
3. **Add error bars** to quantitative comparison figures
4. **Create missing conceptual diagrams** for core concepts (voxel structure, flux propagation, update cycle)

### Important (Significant Improvement)

5. **Remove "(schematic)" from titles** - move to captions if needed
6. **Add color-blind accessible alternatives** (patterns, shapes)
7. **Balance figure distribution** across chapters
8. **Standardize figure borders and framing**
9. **Create comprehensive notation figure/table**

### Desirable (Polish)

10. **Implement SVG export** for key mathematical diagrams
11. **Add animated GIFs** for dynamic processes (some infrastructure exists)
12. **Create dark mode figure variants**
13. **Add lazy loading and responsive srcset**
14. **Document figure style guide**

---

## 11. Grading Summary

| Category | Grade | Weight | Weighted |
|----------|-------|--------|----------|
| Figure Quality | B | 15% | 0.45 |
| Data Visualization | A- | 20% | 0.74 |
| Equation Formatting | A- | 15% | 0.56 |
| Color Usage | B+ | 10% | 0.33 |
| Diagram Clarity | B- | 15% | 0.41 |
| Figure Captions | B+ | 10% | 0.33 |
| Visual Consistency | B | 15% | 0.45 |
| **Overall** | **B-** | 100% | **3.27/4.0** |

---

## 12. Conclusion

The FTD manuscript has established a solid foundation for visual communication with its programmatic figure generation infrastructure, consistent color palette, and professional callout styling. The strongest elements are the quantitative data visualizations (mass predictions, error scoreboard) and the simpler conceptual diagrams (quark configurations, dependency flows). The weakest elements are the incomplete figure generation, inconsistent diagram complexity, and missing visualizations for key concepts.

The primary obstacle to publication-ready visual quality is **completeness**: many referenced figures exist only as Python scripts without generated output files. A dedicated sprint to generate all figures, increase resolution, and add missing conceptual diagrams would significantly elevate the visual presentation.

The technical infrastructure is commendable and positions the manuscript well for iterative improvement. The CSS styling is sophisticated and mobile-responsive. With focused attention on the critical recommendations above, the visual communication could reach publication quality.

**Final Assessment**: The manuscript demonstrates competent visual communication with notable strengths in data visualization and technical infrastructure, but requires additional work on figure completeness, consistency, and conceptual diagram clarity before meeting publication standards for a physics manuscript.

---

*Review prepared by VIS-DATA*
*Expertise: Data Visualization, Scientific Figures, Visual Communication*
*Review framework: Academic publication standards for theoretical physics manuscripts*
