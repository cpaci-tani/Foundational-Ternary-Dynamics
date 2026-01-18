You are preparing a ~1000-page theoretical physics manuscript for publication. The work is structured as 7 books, multiple parts, and ~72 chapters, presenting a unified universal framework addressing quantum mechanics, general relativity, and wave function collapse.

### PRIMARY DIRECTIVE
Use maximum parallelization. Spawn subagents for each book or logical unit to work simultaneously. Coordinate results at the end.

### MANUSCRIPT STRUCTURE
- 7 Books (major divisions)
- 15 Parts (thematic groupings)
- 72 Chapters (individual units)

### PHASE 1: STRUCTURAL AUDIT (Parallel by Book)
Spawn 7 subagents, one per book. Each agent should:
1. Map the complete structure: parts, chapters, sections, subsections
2. Flag any numbering inconsistencies or orphaned references
3. Identify missing transitions between chapters
4. Check that each chapter has: clear introduction, developed argument, conclusion
5. Note any chapters that seem incomplete or redundant
6. Output a structural report for their book

### PHASE 2: MATHEMATICAL CONSISTENCY (Parallel by Book)
For each book, verify:
1. All equations are numbered consistently (Eq. X.Y format or chosen convention)
2. Variables are defined before first use in each chapter
3. Notation is consistent throughout (e.g., ℏ vs h-bar, α vs alpha)
4. Greek letters, subscripts, superscripts used consistently
5. Units are consistent and SI-compatible where appropriate
6. Cross-references to equations in other chapters are correct
7. Derivation steps are complete (no missing intermediate steps)
8. Flag any equations that appear to have typos or dimensional inconsistencies

### PHASE 3: PROSE EDITING (Parallel by Part - 15 agents)
Spawn 15 subagents, one per part. Each should:
1. Fix grammar, punctuation, spelling
2. Improve sentence clarity without changing technical meaning
3. Eliminate redundancy and wordiness
4. Ensure consistent voice and tone (academic but accessible)
5. Fix dangling modifiers and unclear antecedents
6. Standardize terminology (create a running glossary of key terms)
7. Flag jargon that needs definition
8. Ensure paragraph flow and logical progression
9. Check that claims are properly qualified (avoid overclaiming)

### PHASE 4: CITATION & REFERENCE AUDIT (Single agent, full manuscript)
1. Verify all in-text citations have corresponding bibliography entries
2. Check bibliography entries for completeness (authors, title, journal, year, DOI)
3. Standardize citation format (choose: APA, Chicago, or physics standard)
4. Flag any claims that appear to need citations but lack them
5. Identify self-citations and ensure they're appropriate
6. Check for broken or outdated URLs

### PHASE 5: FIGURE & TABLE AUDIT (Parallel by Book)
1. Verify all figures/tables are referenced in text
2. Check captions are complete and descriptive
3. Ensure consistent figure numbering (Fig. X.Y)
4. Flag low-resolution or unclear figures
5. Verify axis labels, legends, units on all plots
6. Check that tables have headers and are readable

### PHASE 6: FRONT & BACK MATTER
1. Generate/verify table of contents matches actual structure
2. Check index entries (if present) or flag key terms for indexing
3. Verify acknowledgments section
4. Check appendices are properly referenced
5. Verify glossary completeness against terms used

### PHASE 7: CONSISTENCY CROSS-CHECK (Parallel by category)
Spawn agents to check manuscript-wide consistency for:
- Agent 1: Terminology (same concepts use same terms throughout)
- Agent 2: Abbreviations (defined at first use, used consistently)
- Agent 3: Formatting (headers, spacing, fonts described)
- Agent 4: Cross-references ("as shown in Chapter X" all valid)
- Agent 5: Key claims (central thesis stated consistently)

### PHASE 8: FINAL INTEGRATION
Merge all agent reports into:
1. **Master Edit Log**: All changes made, organized by chapter
2. **Outstanding Issues**: Problems requiring author decision
3. **Style Guide**: Document of all conventions used
4. **Executive Summary**: Overall manuscript quality assessment

### OUTPUT FORMAT
For each phase, create files in an `/editing_output/` directory:
- `/editing_output/phase1_structure/book_N_structure.md`
- `/editing_output/phase2_math/book_N_math_audit.md`
- `/editing_output/phase3_prose/part_N_edits.md`
- `/editing_output/phase4_citations/citation_audit.md`
- `/editing_output/phase5_figures/book_N_figures.md`
- `/editing_output/phase6_frontback/matter_audit.md`
- `/editing_output/phase7_consistency/category_N.md`
- `/editing_output/final_report.md`

### DECISION LOG
When encountering ambiguous choices, document them rather than guessing:
- "Chapter 12 uses both 'wave function' and 'wavefunction' - which preferred?"
- "Equation 4.7 appears to have a sign error - verify with author"
- "Section 8.3 references 'the previous theorem' but multiple candidates exist"

### PRIORITIES
1. Mathematical correctness > prose elegance
2. Consistency > local optimization
3. Preserve author voice while improving clarity
4. Flag rather than silently change anything substantive

Begin by reading the manuscript structure, then spawn parallel agents for Phase 1. Report back with the structural audit before proceeding.