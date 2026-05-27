# Execution Plan — Engine Architecture Mapping

This plan describes the step-by-step process to analyze the FTD C++ engine codebase and produce a comprehensive architecture map, dependency analysis, and structural documentation.

## Verification Steps
1. **Milestone M1: Exploration & Inventory**
   - Goal: Map every file in the `engine/` directory and detail its role.
   - Verification: Produce an inventory catalog in the M1 explorer handoff.
2. **Milestone M2: Dependency & Flow Analysis**
   - Goal: Map compile-time header inclusions, runtime tick cycle execution, and CUDA host-device boundaries.
   - Verification: Generate structured flow charts and graph descriptions in the M2 worker handoff.
3. **Milestone M3: Structural Documentation & Gap Analysis**
   - Goal: Catalog toggles, mathematical formulas, and engineering stubs/gaps. Write the final markdown architecture report.
   - Verification: Verify the report exists at the designated path, covers all requirements R1, R2, and R3, and conforms to standard formats.

## Subagent Dispatch Strategy
- We will spawn a read-only exploration agent (`teamwork_preview_explorer`) for M1 to read and catalog the files.
- We will spawn a worker agent (`teamwork_preview_worker`) for M2 to extract detailed dependencies, tick flows, and data transitions.
- We will spawn a worker agent (`teamwork_preview_worker`) for M3 to draft the comprehensive report.
- We will spawn a reviewer agent (`teamwork_preview_reviewer`) to check the final report against requirements.
