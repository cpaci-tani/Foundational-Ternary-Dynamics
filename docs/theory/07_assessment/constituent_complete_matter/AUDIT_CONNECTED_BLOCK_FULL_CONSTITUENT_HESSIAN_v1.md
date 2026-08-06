# Audit — connected-block full constituent Hessian

**Campaigns:** FTD-0634 through FTD-0636  
**Verdict:** `[THREE EXECUTION-INVALID RESULTS; POSITIVE SPECTRA NOT PROMOTED]`

## Audit findings

1. FTD-0634 completes both 4,615-evaluation arms and has positive spectra, but
   both gradients exceed `1e-8`; the invalid verdict is mandatory.
2. FTD-0635 repairs the gradient estimator (`8.81e-9`) without altering the
   Hessian. It still fails the registered parent-translation consistency gate.
   The failure is not waived.
3. Source inspection proves the quadratic coat is `C1` and not `C2` at
   half-integer knots. The frozen state is closer to a knot than the
   FTD-0634/0635 Hessian step, invalidating a single-sector interpretation of
   those matrices.
4. FTD-0636 keeps every stencil within one polynomial sector and closes the
   direct-translation/Rayleigh identity. Its spectra remain positive, but its
   gradient `1.116e-8` exceeds the locked gate. The result remains invalid.
5. Independent NumPy diagonalization reproduces all six recorded 48 by 48
   spectra and every stated minimum/maximum eigenvalue within `1e-7`.
6. FTD-0629's "linear modes" wording is too strong after this diagnosis. The
   registered result supports finite-amplitude reversible modal response, not
   a globally defined infinitesimal Hessian spectrum.

Independent certificates:

- `proof_connected_block_full_constituent_hessian.py`;
- `proof_connected_block_full_constituent_hessian_v2.py`;
- `proof_connected_block_knot_local_hessian.py`.

FTD-0637--0639 subsequently execute the admissible analytic route, refine the
residual without changing the action, and qualify the resulting dynamical rest
state. Additional finite-difference tolerance repair remains inadmissible.
