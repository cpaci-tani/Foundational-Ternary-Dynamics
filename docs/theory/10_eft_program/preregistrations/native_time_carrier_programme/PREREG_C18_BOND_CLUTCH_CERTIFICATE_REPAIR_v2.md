# FTD-0989 — Preregistration: C18 bond-clutch certificate repair v2

**Identifier:** `FTD-0989`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY REPAIR]`  
**Parent protocol SHA-256:** `B85BAAA418F0BFF2AE67678BDB1FBD25532EB1CEC9FF596F2325F8D00AE169DD`  
**Parent certificate SHA-256:** `FA0A0A5885612959D5AC782F8AF396A73A275840F08AC3047F1DF6A69859FAD1`

## 1. Failure record

The immutable first execution of FTD-0988 passed `72/73` checks. Every
mathematical, source-lock, production-census, and epistemic gate passed. The
only failure was the verifier-only string predicate `G6 amplitude audit kept
separate`.

The parent script searched the whitespace-normalized protocol for:

```text
H+2I identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit
```

The frozen protocol contains the same sentence with the required provenance
prefix and Markdown code delimiters:

```text
FTD-0987's `H+2I` identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit
```

No mathematical expression, assumption, source, threshold, classifier, or
firewall failed.

## 2. Authorized repair

The wrapper may make exactly one in-memory substitution in the immutable
parent certificate:

```text
"H+2I identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit"
```

to

```text
"H+2I` identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit"
```

This merely makes the predicate include the closing Markdown delimiter that
is actually present after `FTD-0987's `H+2I`. The wrapper then executes the
repaired in-memory source with the original `__file__` so all source and
protocol hashes remain live.

## 3. Integrity gates

- both parent hashes must match;
- the old predicate must occur exactly once in the parent source;
- the repaired predicate must be absent from the parent source;
- exactly one substitution is made;
- the inherited execution must exit zero;
- all inherited checks must pass and the classifier must remain Outcome B;
- both parent files must remain byte-identical after execution.

No production mutation, numerical search, changed theorem, or epistemic
promotion is authorized.

