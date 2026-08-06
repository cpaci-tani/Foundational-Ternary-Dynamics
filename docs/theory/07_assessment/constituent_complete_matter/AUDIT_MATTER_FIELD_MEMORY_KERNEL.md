# Audit — matter--field memory-kernel theorem

**Ledger ID:** FTD-0667  
**Verdict:** `[THEOREM — LINEARIZED BLOCK ELIMINATION] + [INTERPRETATION]`  
**Production status:** unchanged

For a one-step tangent map partitioned into matter and field blocks, exact
elimination of the field gives

\[
x_{n+1}=A x_n+B D^n y_0+
\sum_{m=0}^{n-1}B D^{n-1-m}C x_m.
\]

The identity is proved by induction. It explains how an invertible,
energy-preserving complete map can show apparent damping and revival in a
matter-only projection. It does not fit a kernel to FTD-0665/0666 or establish
an infinite-volume decay law.

Independent exact-rational certificate:
`scripts/proofs/proof_matter_field_memory_kernel.py`, SHA-256
`CF48EFDA6AAFBE6AA99068CDFAB68B6742309E63ABDF1C65E3591BB6E08EA1D4`.

Scope is a differentiable fixed chart/sector of the selected complete tick.
Nonlinear chart changes, reactions, collisions, stochasticity, and production
adoption are outside the theorem.
