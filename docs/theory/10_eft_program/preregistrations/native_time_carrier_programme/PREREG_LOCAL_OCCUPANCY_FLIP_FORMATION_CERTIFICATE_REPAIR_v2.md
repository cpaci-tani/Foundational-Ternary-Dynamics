# FTD-0992 — Preregistration: local occupancy-flip formation certificate repair v2

**Identifier:** `FTD-0992`  
**Date locked:** 2026-08-12  
**Status before execution:** `[PREREGISTERED — VERIFIER-ONLY REPAIR]`  
**Parent protocol SHA-256:** `34A71B6E77DBB23FA0D256F0032A5A708405F67CDA63D59AC756A15CA49062E7`  
**Parent certificate SHA-256:** `A8F0D61500C5878036E25B4CBEA4148FDD72BC64BDDF94D130EA08BFB38BBA16`

## 1. Failure record

The immutable first execution of FTD-0991 passed `80/83` checks. Every
algebraic, cut-set, work, reversal, reserve, aperture, source-hash,
production-census, and no-search gate passed. Three verifier-only literal
predicates failed:

1. `F1 inherited frequency-normalized action` searched the frozen theorem
   for `H_u=\\omega I_u`; its exact inline disposition is `H_u=omega I`.
   The displayed theorem equation is
   `H_u=1/2(P^2+omega^2 Q^2)=omega I`.
2. `F6 firewall FTD-0990 dual-stiffness law` searched raw protocol text even
   though the phrase is separated elsewhere by additional words and appears
   exactly in the normalized firewall paragraph.
3. `F6 firewall Lorentz hiding` searched raw protocol text even though a line
   break separates `Lorentz` and `hiding`.

No mathematical expression, assumption, source, threshold, classifier, or
epistemic firewall failed.

## 2. Authorized repair

The wrapper may make exactly two in-memory substitutions in the immutable
parent certificate:

```text
"H_u=\\omega I_u" in clutch
```

to

```text
"H_u=omega I" in clutch
```

and

```text
firewall in protocol_text
```

to

```text
firewall in protocol_norm
```

The second repair applies the whitespace-normalized protocol string already
constructed by the parent proof; it changes only literal matching. The
wrapper executes the repaired source in memory with the original `__file__`
so every frozen source and protocol hash remains live.

## 3. Integrity gates

- both parent hashes must match;
- each old predicate must occur exactly once in the parent source;
- each repaired predicate must be absent from the parent source;
- exactly two substitutions are made;
- the inherited execution must exit zero;
- all inherited checks must pass and the classifier must remain Outcome B;
- both parent files must remain byte-identical after execution.

No production mutation, numerical search, changed theorem, changed
classifier, or epistemic promotion is authorized.
