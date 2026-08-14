# Pre-registration — Production phase-connection representability certificate repair v2

**Identifier:** `FTD-0965`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE REPAIR EXECUTION]`  
**Expected classifier:** `Outcome B`

## 1. Parent disposition

The immutable FTD-0964 parent execution returned `70/72`, Outcome D. Every
source hash, canonical-capacity calculation, symplectic packing check,
signed-cubic obstruction, production-law audit, and energy boundary passed.
Only two protocol-source scope markers failed:

1. the proof requested lowercase `no new public...`, while the protocol
   sentence begins with uppercase `No new public...`; and
2. the proof requested the contiguous phrase `no site-local...`, while the
   blockquote wraps after `no` and the next line begins `site-local...`.

The parent protocol and parent proof are preserved unchanged.

## 2. Frozen parents

| Source | Frozen SHA-256 |
|---|---|
| `PREREG_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_CLASSIFIER_v1.md` | `B44C925D56BC66B3C9FCA2781AC29C86D0E8EADCF60DCA90FAA0BAD67B6A3E21` |
| `proof_production_phase_connection_representability_classifier.py` | `2199DE8A4FDB5239B27D1973880B27D7C886DBF34D7BA22FB40A948786FB1C09` |

All nine production-source hashes inherited by FTD-0964 remain frozen.

## 3. Permitted repair

The wrapper may perform exactly two in-memory substitutions in the parent
proof source:

1. change the requested marker from
   `no new public continuous storage type is forced by local scalar capacity`
   to
   `No new public continuous storage type is forced by local scalar capacity`;
2. change the requested marker from
   `no site-local cubic-covariant linear scalar chart exists`
   to
   `site-local cubic-covariant linear scalar chart exists`.

The second repaired marker still verifies the substantive obstruction while
avoiding dependence on Markdown blockquote line wrapping. No equation,
classifier, hash, production source, physical claim, or scope conclusion may
change.

## 4. Repair integrity gates

- both parent hashes and this repair-protocol hash must match;
- each old full source-line anchor must occur exactly once;
- each new full source-line anchor must be absent before repair;
- exactly two substitutions must occur in memory;
- the repaired inherited run must pass `72/72` and retain Outcome B;
- the parent protocol and parent proof hashes must be unchanged after the run.

If any integrity gate fails, the repair outcome is D. A passing wrapper
licenses only the FTD-0964 Outcome-B statement; it does not license production
implementation or native emergence.
