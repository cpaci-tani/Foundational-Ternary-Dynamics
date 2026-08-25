# Cotangent stabilizer-packet reversible Gauss source v1

**Date:** 2026-08-24
**Status:** **[THEOREM — MINIMAL CONTEXT-FREE $D_4$ SOURCE PACKET]** +
**[THEOREM — REVERSIBLE LOCAL GAUSS-PRESERVING INCIDENCE TRANSACTION]** +
**[THEOREM — EIGHT-RECORD CAPACITY/TOKEN-ENERGY PRICE]** + **[SELECTION —
TERNARY SIGN AS ELECTRIC CHARGE]** + **[OPEN — HAMILTONIAN SWITCHING WORK AND
FULL-TICK SOURCE COMPOSITION]**
**Production status:** unchanged
**Ledger status:** no row minted

**Exact certificate:**
[proof_cotangent_stabilizer_packet_gauss_source.py](../../../../../scripts/proofs/proof_cotangent_stabilizer_packet_gauss_source.py)
performs 2,316 exact checks under all 48 signed cubic transformations and all
twelve internal clock positions.

---

## 1. Context price of a directed electric source

A directed polar edge $d$ does not select one perpendicular axial normal. Its
stabilizer in $O_h$ is $D_4$ of order eight, acting transitively on

\[
 \{(n,h):n\perp d,\ h=\pm1\}.                     \tag{1}
\]

Therefore the smallest context-free source packet at a common C4 phase is the
complete orbit

\[
 \mathcal P(d,p)
 =\{(d,n,h,p):n\perp d,\ h=\pm1\},                \tag{2}
\]

containing eight distinct finite records. A smaller packet requires an
external plane/face context and is not invariant under the directed-edge
stabilizer.

At cotangent layer zero,

\[
 \sum_{z\in\mathcal P}E(z)=8d,
 \qquad
 \sum_{z\in\mathcal P}B(z)=0.                     \tag{3}
\]

After every internal flag/phase update accompanied by the global cotangent
layer shift, equation (3) remains exact through all twelve clock positions.
The packet is covariant under all of $O_h$ and under C4 phase advance.

## 2. Canonical source normalization

At any layer the seven conserved rows have Gram matrix

\[
 \operatorname{Gram}(n,E,B)
 =\operatorname{diag}(192,64I_3,64I_3).            \tag{4}
\]

The packet has conserved totals $(8,8d,0)$. Conditioning on or compensating
its carrier-number component, its field norm is

\[
 (8d)^T(64I_3)^{-1}(8d)=1.                        \tag{5}
\]

Thus the symmetry-complete eight-record orbit is exactly one canonically
normalized electric edge quantum. Equation (5) is a native carrier
normalization, not yet the physical fine-structure coupling.

The batch transaction retains exactly eight finite records in both reserve
and active ownership. With the existing positive one-unit microscopic token
ledger,

\[
 N_{\rm token}=E_{\rm token}=8                    \tag{6}
\]

on both sides. This proves the discrete capacity price and conservation of the
token-energy ledger. It does not determine the Hamiltonian energy difference
between active field ownership and the reserve configuration.

## 3. Manifestation and Gauss incidence

Let $e$ be the oriented edge from tail $x$ to head $x+d$, with boundary

\[
\partial e=\delta_{x+d}-\delta_x.                \tag{7}
\]

The reversible source transaction moves the complete packet between reserve
and active ownership while toggling the endpoints

\[
 (0,0;\mathrm{reserve})
\longleftrightarrow
 (-\epsilon,+\epsilon;\mathrm{active}).           \tag{8}
\]

If ternary sign is selected as electric charge and the normalized packet is
the edge field quantum, then

\[
 \Delta E=e,
 \qquad
 \Delta\rho=\partial e,
 \qquad
 \boxed{\Delta(\operatorname{div}E-\rho)=0}.       \tag{9}
\]

The same incidence algebra proves that a current transaction

\[
 \Delta\rho=-\partial j,
 \qquad
 \Delta E=-j                                      \tag{10}
\]

preserves Gauss identically. These are local chain-complex identities, not an
imposed continuum divergence equation.

## 4. Remaining coupling boundary

Equation (7) proves a reversible ownership/source stencil, but it does not yet
derive:

- why ternary manifestation sign is electric rather than another signed
  charge;
- the Hamiltonian switching work between the eight-record reserve and active
  configurations;
- the response of the finite collision--streaming action to repeated or
  separated sources;
- static lattice Coulomb energy and its long-distance residue;
- equality of that residue with the master-quadratic fine-structure root; or
- stable matter/detector structures that supply a smaller contextual face
  packet.

The next native coupling observable is the long-distance source-response
residue after composing equations (8)--(9) with the layer-covariant collision. It
must be measured from the action itself before any comparison with $1/x_+$.

The subsequent
[native-alpha action-scale obstruction](THEOREM_COTANGENT_NATIVE_ALPHA_ACTION_SCALE_OBSTRUCTION_v1.md)
proves that the unit packet norm and vacuum speed still leave one positive
action-normalization orbit. In canonical packet coordinates the missing
dimensionless scalar is the blocked curvature $\chi_{\rm EM}=\Gamma/I_*$,
and the current scalar collision block also lacks the charged massless pole
needed for the source-response measurement. Token count and packet norm cannot
substitute for either result.

The later
[framed-plaquette radiation-release theorem](THEOREM_COTANGENT_FRAMED_PLAQUETTE_NUMBER_NEUTRAL_RADIATION_RELEASE_v1.md)
proves that this Gauss packet must remain bound dressing. A free transverse
seed instead requires a number-neutral particle--hole pair of complete packet
orbits on each edge and a closed framed plaquette. This preserves the present
source theorem while pricing the distinct radiative carrier explicitly.
