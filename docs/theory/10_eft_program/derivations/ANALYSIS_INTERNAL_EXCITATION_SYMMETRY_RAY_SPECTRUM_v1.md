# FTD-0698 — Internal-excitation symmetry-ray spectrum v1

**Status:** `[EXECUTION INVALID — CLASSIFIER INPUT/DOMAIN]`  
**Production status:** unchanged

The complete `L=113`, 96-forward/96-reverse history and 32,592 spectral rows
were emitted, but the run is not promotable.

Two registered execution defects occurred:

1. the classifier received the continuous Hessian frequency
   `1.2140869502262857` instead of the discrete tick phase
   `1.0911648733663635` named in the protocol;
2. the all-mode sign-power gate reached `4.678842826534076e-4`, above `1e-4`,
   on a mode with integrated field power only `~2.31e-29`.

The common-action, radial, energy, and inverse gates passed, but they do not
repair the spectral execution contract. Raw peak locations are retained only
as debugging provenance and do not count as resonance evidence. FTD-0699 is a
fresh corrected-amplitude run.
