# FOUNDATION — Optional Half-Offset Economy for a Common Spatial Symbol

**Status:** `[THEOREM — EXACT INTEGER-HOP EIGHTEEN AND HALF-ANGLE SEVEN CONSTRUCTIONS]` +
`[MEASURED — NUMERICAL FOUR-SQUARE HALF-ANGLE CANDIDATE]` +
`[OPEN — WHETHER THE ECONOMICAL HALF-OFFSET BASIS IS PHYSICALLY INSTANTIATED]` +
`[BOOKED — FTD-0819]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_carrier_sublattice.py`
**Parents:** `ANALYSIS_INTERSECTOR_CONE_RANK_OBSTRUCTION_v1.md` §4b–4c (FTD-0816,
the sum-of-squares resolution and its minimality correction),
`docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (the
Watson identity and SU(3) from the BCC triple cosine product).
**Production impact:** none. No constant is changed; no tag moves; no
postulate is amended. This document *prices* an amendment; it does not
make one.

> **Binding correction (2026-08-09).** Sections 2--6 preserve the original
> search history but their claims that a half-offset lattice is necessary,
> that body centres contain the unrestricted minimum, and that the carrier
> is forced to be massless are superseded by §7a and this correction. Every
> integer graph hop obeys
> `2w(1-cos(q·d)) = w sin²(q·d) + w(1-cos(q·d))²`; the M18 face and edge
> hops therefore give an exact **18-square integer-frequency** construction.
> Half offsets reduce the known exact count to seven (and support a
> numerical four candidate), but they are an economy, not an existence or
> postulate-amendment requirement. The refined-site/staggered-cochain fork
> is relevant only if that optional economical basis is physically adopted.
> A second original inference is also withdrawn: although the seven
> equal-weight triple-product squares sum to
> `1-S²=(1-S)(1+S)`, the usual BCC Laplacian is proportional to `1-S`.
> The extra factor `1+S` prevents the claim that the two symbols are simply
> points in one constant-weight seven-square cone. What survives is an
> algebraic alignment with the BCC structure factor, not identity of cones.

---

## 1. The question FTD-0816 left, and why it is the load-bearing half

FTD-0816 established that the flux symbol is a sum of squares whose
arguments sit at half-lattice offsets, and §4c established how few squares
are needed. Both are counting results. The ontological question is
different and heavier: **a square's argument fixes the displacements its
hopping operator uses, and displacements are positions.** Asking what a
common cone costs the five postulates is therefore asking *which*
half-offsets appear — because a demand for new positions is a demand on
Postulate 1, and that is a type, not a parameter.

Three shells are available in the nearest-neighbour class on $x = q/2$.
Writing a monomial as $\prod_i g_i(x_i)$ with $g_i \in \{1, \cos, \sin\}$,
the displacement component $m$ is $\{0\}$ when $g_m = 1$ and
$\{\pm\tfrac12\}$ otherwise, since $\cos(q/2)$ and $\sin(q/2)$ are both
combinations of $e^{\pm iq/2}$:

| monomial type | displacements | count | reading |
|---|---|---|---|
| $s_i$ | $(\pm\tfrac12, 0, 0)$ | 3 | face centre |
| $s_ic_j$, $s_is_j$ | $(\pm\tfrac12,\pm\tfrac12,0)$ | 9 | edge midpoint |
| $s_ic_jc_k$, $s_is_jc_k$, $s_1s_2s_3$ | $(\pm\tfrac12,\pm\tfrac12,\pm\tfrac12)$ | **7** | **body centre** |

Read off the DFT rather than asserted: each monomial's frequency support
is computed on a $5^3$ grid, which resolves $k \in [-2,2]$ without
aliasing, and the two non-body shells are carried as controls.

## 2. Body centres suffice, and nothing else is needed

The decisive measurement. Restricting the search to the 7-dimensional
body-centre sector and re-running the rank minimisation:

| $n$ | BCC-restricted | unrestricted (§4c) |
|---|---|---|
| 7 | found (exact, covariant) | found |
| 6 | found | found |
| **5** | **found** | **found** |
| 4 | none, 300 restarts | none, 400 restarts |

**The BCC-restricted minimum equals the unrestricted minimum.** The face
and edge half-offsets buy nothing at any length. Every shortest
decomposition can be taken to live entirely on the body centres.

This is stronger than §4c and it is what makes the ontological reading
tractable: the carrier does **not** want a finer cubic lattice. A finer
cubic lattice would halve the spacing and move `a_phys` — currently
declared $\equiv \ell_P$ — and with it every dimensional prediction
downstream. That price is not incurred.

⚠ The particular unrestricted five reported in FTD-0816 §4c spends 52% of
its coefficient weight off the body centres. That is where its random
start landed, not a requirement; the same length is available inside the
BCC sector.

## 3. The carrier sector **is** the BCC structure factor's complement

The eight half-argument triple products $\prod_i g_i(q_i/2)$ with
$g_i \in \{\cos,\sin\}$ satisfy, identically,

$$\sum_{\text{all }8}\Big(\prod_i g_i\Big)^2 \;=\; \prod_i(\cos^2\!\tfrac{q_i}2 + \sin^2\!\tfrac{q_i}2) \;=\; 1 .$$

One of the eight is the pure cosine $S = \prod_i\cos(q_i/2)$ — and $S$ is
exactly the **BCC nearest-neighbour structure factor**, the eight
neighbours sitting at $(\pm\tfrac12,\pm\tfrac12,\pm\tfrac12)$, with
$-L_{\rm BCC} = 8(1-S)$. The remaining seven are precisely the carrier
sector. Hence

$$\boxed{\;\sum_{\text{carrier }7}\Big(\prod_i g_i\Big)^2 \;=\; 1 - S^2 \;=\; (1-S)(1+S)\;}$$

verified symbolically. Two consequences.

**The exact covariant seven is the diagonal decomposition of this sector** —
one square per basis monomial, weights $(4,4,4,\tfrac{16}3,\tfrac{16}3,
\tfrac{16}3,4)$. That is *why* it is covariant and *why* it is exactly
seven: seven is the dimension of the sector, not a search outcome.

**The half-angle basis is algebraically aligned with the BCC structure
factor, not identical to its Laplacian cone.** Equal weights give
$(1-S)(1+S)$, whereas $-L_{\rm BCC}=8(1-S)$; the extra $1+S$ factor is
load-bearing. The flux symbol is diagonal in the same seven basis functions,
but this does not make the two Laplacians points in one constant-weight
square cone.

This is the third independent arrival at BCC in FTD. The first two are the
Watson identity $W_3 = G^{*2}/2\pi$ and the SU(3) gauge group, both of
which arise from the BCC eigenvalue's triple cosine product
(`DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`) — the same $S$, at the same half
arguments.

## 4. Two ontological readings, priced

**(A) The site set is BCC.** SC sites $\cup$ body centres *is* the BCC
lattice. Postulate 1 currently declares a cubic lattice; this reading
amends it. Price: the site set doubles; nearest-neighbour separation
becomes $\tfrac{\sqrt3}2 a$ rather than $a$, so `a_phys` recalibrates and
every dimensional prediction conditional on it moves; and the flux sector,
whose symbol $-L_{18}$ is an 18-point *SC* stencil, must be re-expressed on
the bipartite structure. This is a genuine amendment to a postulate and
should be priced as one.

**(B) The carrier is a field on the cubes.** The body centres are the
centres of the existing SC lattice's 3-cells. A field there is a
3-cochain, not a new site set. Price: no new positions, one new field at a
new cell degree. FTD's two-layer ontology already carries a site-valued
$s$ and a link-like $J$; a cube-valued carrier is the natural third, and it
is the standard lattice-field-theory move.

**(B) is markedly cheaper and is the reading this document recommends
carrying forward** — with the caveat that it is a recommendation about
where to look, not an adopted commitment. What would decide between them:
whether the carrier must be *dynamical in its own right* (which wants
sites) or is a derived object of the existing fields (which cells suffice
for).

## 5. The obstacle neither reading removes

Postulate 3 declares states ternary, $s \in \{-1,0,+1\}$, on sites. The
carrier needs **five mutually anticommuting structures**, i.e. at least a
four-component spinor. That is not a ternary state under either reading.
So the honest fork is:

- **matter is emergent** — the five anticommuting structures arise from
  patterns of ternary states, in which case they must be *derived* and this
  document has only located where to look; or
- **matter is adopted** — the ontology takes on a spinor-valued field,
  which is a new type with a real price, to be registered as an adoption
  and never presented as a derivation.

Nothing computed here chooses between these, and the first is the only one
consistent with the Number-One Goal's derivation face. The result's
contribution is to make the target concrete: whatever emerges must
reproduce five anticommuting structures on the body-centre displacements.

## 6. The mass ladder

$H = \sum_\mu \Gamma^\mu\phi_\mu + \Gamma^{n+1}M$ squares to
$\sum\phi_\mu^2 + M^2$ only if the mass structure anticommutes with every
kinetic one, so a mass costs one further structure. Dimension $2^k$ carries
$2k+1$:

| carrier | structures | spinor dim | spare |
|---|---|---|---|
| five, massless | 5 | **4** | **0** |
| five + mass | 6 | 8 | 1 |
| seven (covariant), massless | 7 | **8** | **0** |
| seven + mass | 8 | 16 | 1 |

Both minimal carriers **exactly saturate** their Clifford algebra. The
five-square carrier at Dirac dimension has no structure left for a mass
term: **it is necessarily massless there**, and giving it a mass forces
dimension 8. Cubic covariance *and* a mass together force dimension 16 —
which is where FTD-0816's nine landed by accident, for the wrong reason.

Saturation is a constraint with teeth rather than a curiosity: at dimension
4 there is no room for a $\gamma^5$ twist, a Wilson term, or any additional
structure, so any such term is a falsifiable commitment to a larger spinor.

## 7. Open questions this raises

- **Which reading, (A) or (B)?** The decision belongs to the owner and
  should go through the FC-W adoption pipeline if it is taken as an
  extension rather than a reinterpretation.
- **The BCC tension, now sharper.** The Link 8 closure audit records that
  the engine's production stencil is $(\mathrm{SC}+\mathrm{FCC})/2$ and
  **BCC-orthogonal**, while the master quadratic lives on the BCC Watson
  integral (`AUDIT_LINK8_CLOSURE.md`). This result is a *third* structure
  wanting BCC. Three of FTD's structures now point at a lattice the
  production stencil is orthogonal to, and that conflict is not resolved by
  anything here — it is made harder to ignore.
- **Does the five emerge, or must it be adopted?** §5.
- **The covariant minimum inside the BCC sector.** Pure-sector orbits there
  are 3 (`scc`), 3 (`ssc`) and 1 (`sss`), so covariant lengths are
  $3,4,6,7$; the five is again non-covariant. Whether a mixed-sector
  covariant five or six exists is open, for the reason recorded in
  FTD-0816 §4c.
- **Time discretisation** remains unmatched (FTD-0816 §5), and can only add
  conditions.

## 7a. Correction (2026-08-09)

FTD-0816 §4d found an unrestricted numerical four-square candidate after
the earlier fixed-scale search reported five. No exact or interval
existence certificate is supplied, so the rigorous bounds remain
`3 <= n_min <= 7`. This changes §2's headline:

- restricted to body centres the dispersed search reaches **five and
  stalls** (2000 starts), so "body centres suffice" holds **at length
  five**, and the shortest carrier buys one square by leaving them for
  the face/edge shells;
- the ontology is **not yet chosen**: refined physical sites give spacing
  `a/2` and force an `a_phys` recalibration, whereas staggered cochains
  retain Bravais spacing `a` but add face/edge/body carrier types;
- the structure-factor complement identity (§3) is exact and unaffected;
- the mass-ladder replacement is conditional: an exactly certified four
  would leave one structure spare and admit a mass at Dirac dimension;
  the "necessarily massless" line is withdrawn, but the replacement is
  not theorem-grade.

The stall at four is search evidence of exactly the kind that
mis-reported the minimum, and is asserted at that confidence and no
higher; the only proof in this document's rank claims is `n ≥ 3`.

## 8. Reproduction

```
python scripts/experiments/temporal_interior/derive_carrier_sublattice.py
```

Under two minutes. The run asserts the shell classification against a DFT
read-off, the structure-factor complement identity, the covariant seven,
and the mass ladder; it prints the BCC-restricted rank table.
