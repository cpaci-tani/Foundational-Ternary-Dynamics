# Hodge-flag fixed-frame full-tick zero-mode boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT TWELVE-FRAME ZERO-MODE INTERSECTION]** +
**[CLOSED NEGATIVE, SCOPED — FIXED-FRAME FIELD COLLISION PLUS SHARED-EDGE
TICK]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_hodge_flag_selected_collision_full_tick_boundary.py](../../../../../scripts/proofs/proof_hodge_flag_selected_collision_full_tick_boundary.py)
composes the previously selected collision with the exact internal C3 flag
turn and C4 phase advance. It includes the 880,560-check parent construction
and adds an exact 192-channel product-reference calculation.

---

## Result

The selected fixed-frame collision conserves

\[
 1,\quad E_x,E_y,E_z,\quad B_x,B_y,B_z             \tag{1}
\]

and has correction rank 185. Its correction is an exact
negative-semidefinite sum

\[
 N=-2\sum_{\{x,Cx\}}(a_{Cx}-a_x)(a_{Cx}-a_x)^T.   \tag{2}
\]

Let $U$ be the internal flag-plus-phase permutation, $U^{12}=1$. The fixed
frame collision does not commute with $U$: 17,544 of the 18,336 pair states
fail pairwise commutation.

The twelve clock images of the seven rows in equation (1) span 37 independent
rows. The exact persistent zero-mode constraint is

\[
 \bigcap_{t=0}^{11}\ker(NU^t),                    \tag{3}
\]

and has dimension one. Equivalently, the co-rotating first-order collision
sum

\[
 \bar N=\sum_{t=0}^{11}U^{-t}NU^t                \tag{4}
\]

has rank 191 and nullity one. Negative semidefiniteness prevents damping from
different clock frames from cancelling. Only uniform record number survives;
all six fixed-frame field modes are gapped already at zero wavevector.

## Boundary

This selected collision plus the shared-edge C3×C4 tick cannot produce a
Maxwell cone at any long wavelength. The obstruction occurs before a
finite-$k$ fit: the required field zero modes do not exist.

The result does not close all Hodge lifts. It proves that a viable lift must
either use a field readout whose seven-dimensional invariant space is closed
under the internal tick, or change the edge--face transport itself.

