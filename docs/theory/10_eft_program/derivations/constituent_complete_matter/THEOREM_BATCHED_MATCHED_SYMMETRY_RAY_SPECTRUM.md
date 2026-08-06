# FTD-0697 — Batched matched symmetry-ray spectrum

**Status:** `[THEOREM — EXACT NUMERICAL OBSERVER EQUIVALENCE]`  
**Production status:** unchanged

The modular-bin batch evaluator reproduces the qualified FTD-0696 direct
carrier-aware Fourier observer on all locked `L=31` rays, harmonics, fields,
translations, zero controls, and invalid inputs.

Worst observed differences are:

- complex coefficient: `7.714531306902981e-19` absolute;
- total/transverse/longitudinal power: `1.999019813669818e-14` relative.

Both lie below the registered `1e-14` and `1e-12` gates. Output order and
fail-closed duplicate/nonprimitive/zero controls pass.

This is an implementation equivalence, not new field or matter physics. It
makes all-harmonic large-volume symmetry-ray observation executable without
changing Fourier sign, carrier phase, normalization, or lattice projection.
