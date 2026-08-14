# FTD-0948 — Preregistration: minimum nonlinear relative-field recursive-charge repair-integrity v3

**Identifier:** `FTD-0948`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED WRAPPER-INTEGRITY REPAIR]`  
**FTD-0947 repair protocol:** `PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_CERTIFICATE_REPAIR_v2.md`, SHA-256 `62BEA97459DD1EE7F6455F0C081AC3035126DDD494B1E8F7C35D394218ADDDC2`  
**FTD-0947 wrapper:** `scripts/proofs/proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame_v2.py`, SHA-256 `63DBE4FD4D701A8A2C3150B393848E3EC3D698415C0D83A7A89784C01DB468D8`, first immutable execution failed before certificate execution  
**Preserved parent record:** FTD-0946 `70/79`, Outcome D — no theorem

## 1. Failure

FTD-0947 correctly inserted

```python
protocol_flat = " ".join(protocol_text.split())
```

immediately after the original `protocol_text = ...` line. Its post-repair
integrity loop then rejected repair 1 because it required the old line to be
absent, even though that line is intentionally a substring of the declared
two-line replacement. No repaired certificate gate executed.

## 2. Sole permitted repair

Execute the frozen FTD-0947 wrapper from an in-memory copy after replacing
exactly this meta-integrity condition:

```python
if old in source or source.count(new) != 1:
```

with:

```python
if source.count(new) != 1 or (old not in new and old in source):
```

This retains the absence test for repairs whose replacement does not contain
the old fragment, while recognizing the intentional containment in repair 1.

No parent protocol, certificate, repair list, equation, source hash, outcome,
epistemic tag, or physics gate changes.

## 3. Integrity and outcome

The v3 wrapper must fail closed unless the FTD-0947 protocol and wrapper hashes
match, the old meta-condition occurs exactly once, the new condition occurs
zero times before substitution and once after substitution, and the inherited
FTD-0947 wrapper exits zero.

If those gates and the inherited repaired `79/79` certificate pass, FTD-0948
registers the repaired Outcome B. Otherwise no theorem is issued. Parent
FTD-0946 and FTD-0947 failures remain immutable provenance.
