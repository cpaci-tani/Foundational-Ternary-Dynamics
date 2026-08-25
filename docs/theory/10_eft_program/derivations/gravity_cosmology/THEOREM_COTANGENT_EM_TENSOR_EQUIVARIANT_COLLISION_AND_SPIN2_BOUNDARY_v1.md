# Cotangent EM/tensor equivariant collision and spin-2 boundary v1

**Date:** 2026-08-24
**Status:** **[THEOREM — EXACT LAYER-COVARIANT JOINT EM/TENSOR COLLISION]** +
**[THEOREM — TWO SYMMETRY-FORCED $E_g$ SURPLUS INVARIANTS]** + **[CLOSED
NEGATIVE, SCOPED — TWO-RECORD MASSLESS SPIN-2 CONE AND GENERIC MAXWELL
ISOTROPY]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificates:**

- [proof_cotangent_em_tensor_equivariant_collision.py](../../../../../scripts/proofs/proof_cotangent_em_tensor_equivariant_collision.py):
  128,499 exact checks; and
- [proof_cotangent_em_tensor_bloch_spin2_boundary.py](../../../../../scripts/proofs/proof_cotangent_em_tensor_bloch_spin2_boundary.py):
  exact 21-mode co-rotating first-moment closure.

No target pole, gravitational coefficient, or measured datum enters the
construction.

---

## 1. Equivariant collision price

The unrestricted joint number+$E/B$+$Q/P$ pair relation has rank 173 and
nullity 19. Under $O_h\times C_4$, its 18,336 pair states form 115 orbits,

\[
 \{96^{39},192^{76}\}.                             \tag{1}
\]

Sixty orbits contain the 10,368 mandatory singleton pair states and must be
fixed. Of the remaining 55 active orbits, 43 admit self-involutions and twelve
require six compatible distinct-orbit exchanges.

The union of **every** admissible equivariant orbit map has transition rank

\[
 \boxed{171},                                      \tag{2}
\]

two below the unrestricted rank 173. A deterministic rank-greedy global
collision attains this ceiling, fixes exactly the mandatory 10,368 states,
and involutively matches all 7,968 active states.

Its nullity is therefore 21. The two surplus additive invariants are exactly
the phase-blind diagonal-traceless $E_g$ components of the FCC dyad:

\[
 D_{xx}-D_{yy},
 \qquad
 2D_{zz}-D_{xx}-D_{yy}.                            \tag{3}
\]

Thus the surplus is a classified cubic shear doublet, not an unidentified
numerical nullspace.

Clock conjugation generates all three cotangent-layer collisions, each of
rank 171/nullity 21, with

\[
 UC_q=C_{q-1}U.                                    \tag{4}
\]

## 2. Co-rotating Bloch result

Remove the native zero-wavevector tensor carrier rotation

\[
 (Q,P)\mapsto(-P,Q).                               \tag{5}
\]

The exact three-layer first-order slow generator acts on 21 variables:

\[
 1+6+12+2.                                         \tag{6}
\]

Every row and column involving the twelve C4 tensor-doublet modes vanishes at
$O(k)$. In particular, the complete TT tensor envelope has

\[
 \boxed{v_{\rm TT}(k=0)=0}.                        \tag{7}
\]

The phase-blind $E_g$ pair is not inert. It mixes with the
scalar--longitudinal sector and changes the characteristic polynomial to

\[
 \boxed{
 \chi(\lambda,k)
 =\frac{\lambda^{15}}{36^3}
 \prod_{a=x,y,z}
 \left(36\lambda^2+|k|^2+k_a^2\right).}           \tag{8}
\]

On a cubic symmetry axis, two transverse pairs still have speed $1/6$ while
the longitudinal/$E_g$ pair has speed $1/\sqrt{18}$. For generic $k$, the
three factors in equation (8) are distinct. The joint collision therefore
introduces cubic birefringence and removes the generic twofold Maxwell
polarization degeneracy that the seven-invariant electromagnetic collision
had preserved.

## 3. Scoped closure

This exact two-record route is closed negative for two reasons:

1. the native C4 tensor doublet has no $k$-linear massless TT cone; and
2. its symmetry-forced phase-blind $E_g$ companions spoil generic Maxwell
   isotropy.

The tensor carrier and shared STF source remain valid kinematic results. What
is closed is their realization through this pairwise, same-site,
field-and-tensor-preserving collision family.

## 4. Required gravity repair

A surviving common action must change at least one structural ingredient:

- use a higher-occupancy collision able to remove the forced $E_g$ pair;
- give tensor transport its own staggered face/cell incidence rather than
  reading it only as a local dyad;
- use a finite-range composite whose center-of-mass step carries a genuine
  helicity-two first moment; or
- generate the tensor pole collectively from a bound material/capacity phase.

Any repair must retain the already-passed seven-invariant vacuum Maxwell
sector. Static attraction, universal matter-clock response, nonlinear
completion, and lensing remain open after that pole is found.

The subsequent
[STF parity-price theorem](THEOREM_COTANGENT_STF_PARITY_PRICE_AND_SPIN2_CURL_TARGET_v1.md)
sharpens the required stagger. The vanished tensor $O(k)$ block is forced
already by inversion because this collision preserved two even tensor
quadratures. The existing handed flag supplies an odd STF partner, and the
unique isotropic even/odd symmetric-curl target has the required TT cone.
This does not repair the present collision: a new finite primal/dual lift or
the exact rank-twenty on-site phase/parity price is required.
