# FTD-0947 — Preregistration: minimum nonlinear relative-field recursive-charge certificate repair v2

**Identifier:** `FTD-0947`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED VERIFIER-ONLY REPAIR]`  
**Parent protocol:** `PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_v1.md`, SHA-256 `F8DFB7BC2461D2566FA746111A656FAF606FD930F7E06E7D0FA0BE1D0BA666E1`  
**Parent certificate:** `scripts/proofs/proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame.py`, SHA-256 `76A5ADA0CE3C0F52E3FE789870C8CA8940B5AB4B7138EF343CF3355C4CF15680`  
**First immutable execution:** `70/79`, **Outcome D — no theorem**

## 1. Scope

The parent run passed every source hash and every substantive algebraic,
variational, charge, energy-current, split-tick, gamma-separation, source,
frame, parity, and scope claim. Nine checks failed because of verifier
normalization defects. This repair may change only those comparisons in an
in-memory copy of the parent source.

No parent file is edited. No source, equation, assumption, outcome condition,
epistemic tag, model term, or promotion boundary changes.

## 2. Locked repairs

The wrapper must make exactly these verifier-only changes:

1. normalize protocol whitespace before testing the wrapped phrase
   `not, by themselves, an uncontained existence proof` in G1;
2. compare `J_e Pi_e` and `Pi_e J_e` directly with `J_e`; the parent
   accidentally compared them with `|e|^2 J_e`, although the locked protocol
   assumes a unit axis and states the correct identity;
3. simplify the forced quartic before comparing it with
   `c2*y*(y-A**2)`;
4. correct the exact product of the two registered quartic sign samples from
   `-c2**2*A**8` to `-c2**2*A**8/2`; only negativity is used by the protocol;
5. compare the radial derivative after expansion rather than requiring one
   particular factor ordering;
6. simplify the fixed-charge kinetic expression before comparing it with
   `Q**2/(2*N)`;
7. normalize protocol whitespace in the repeated G6 localization firewall;
8. simplify the damped charge identity before comparison; and
9. let the inherited Outcome-B classifier recompute after repairs 1--8.

## 3. Integrity gates

The wrapper must:

- fail closed unless both parent hashes and this repair-protocol hash match;
- require exactly one occurrence of every old verifier fragment;
- perform exactly one substitution per fragment;
- execute the repaired source only in memory;
- report the inherited certificate count separately from repair integrity;
- preserve the parent `70/79` record as invalid provenance; and
- reject any production, engine, CMake, constant, source-equation, outcome,
  numerical-search, fitting, Born, Bell, `G*`, or gamma change.

## 4. Outcome

- inherited `79/79` plus all repair-integrity gates: register FTD-0947 as the
  repaired certificate for the FTD-0946 protocol;
- otherwise: Outcome D, no theorem.

The repaired result, if valid, retains the parent protocol's frozen Outcome B:
conditional same-field recursive charge, finite-tick exact-energy debt, and a
universal source-frame/handedness obstruction.
