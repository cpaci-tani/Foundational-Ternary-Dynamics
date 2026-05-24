"""
FTD math node map parsers package.

Modules:
    ledger_parser              -- P1: LEDGER.md -> ledger-claim node list
    spine_parser               -- P2: SPEC_ALGEBRAIC_SPINE.md -> theorem node list
    check_pattern_extractor    -- E1: check(label, computed, claim) AST walker
    proof_suite_extractor      -- E2: ProofSuite.assert_close()/assert_true() walker
    heuristic_extractor        -- E3: bare print/assert pattern (best-effort)
    object_aliases             -- canonical-name reconciliation table
    mermaid_renderer           -- Mermaid block generator for the Markdown output

Consumed by scripts/verification/build_math_node_map.py.
"""
