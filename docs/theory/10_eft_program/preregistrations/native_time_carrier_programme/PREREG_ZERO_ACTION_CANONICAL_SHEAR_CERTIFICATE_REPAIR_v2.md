# FTD-0994 — Preregistration: zero-action canonical shear certificate repair v2

**Identifier:** `FTD-0994`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY REPAIR]`  
**Parent protocol SHA-256:** `9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753`  
**Parent certificate SHA-256:** `4F158B7A8847852D1DEF98E29E30999634FF769B27C56C04E1E39C2048029831`

## 1. Failure record

The immutable first execution of FTD-0993 passed `95/96` checks. Every
symplectic, inverse, energy, phase, locality, causal-radius, no-cloning, join,
production, and epistemic gate passed. The sole failure was the source-hash
literal for
`THEOREM_C18_BOND_CLUTCH_CURRENT_AND_WORK_ACTION_NORMALIZATION_v1.md`.

The locked protocol contains the correct hash:

```text
2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8
```

The parent verifier mistyped the middle bytes as:

```text
2A93D9CFF23DFFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8
```

No source file, mathematical expression, assumption, classifier, threshold,
or firewall failed.

## 2. Authorized repair

The wrapper may make exactly one in-memory substitution in the immutable
parent certificate, replacing the mistyped hash literal with the locked
protocol hash. It then executes the repaired source in memory with the
original `__file__`, preserving every live protocol and source check.

## 3. Integrity gates

- both parent hashes must match;
- the mistyped literal must occur exactly once in the parent source;
- the corrected literal must be absent from the parent source;
- exactly one substitution is made;
- the inherited execution exits zero with `96/96` and Outcome B;
- both parent files remain byte-identical.

No production mutation, numerical search, changed theorem, changed
classifier, or epistemic promotion is authorized.
