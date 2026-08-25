# C18 two-record phase-complete scattering and axial-routing boundary v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT TWO-RECORD MOMENTUM-SECTOR CENSUS]** +
**[REFERENCE CONSTRUCTION — MINIMUM PHASE-COMPLETE COLLISION]** +
**[THEOREM, CONDITIONAL ON THE DISPLAYED RULE — REVERSIBILITY, CUBIC AND C4 COVARIANCE, CONSERVATION]** +
**[THEOREM — UNEQUAL-SCALAR-PHASE AXIAL ROUTING OBSTRUCTION]** +
**[OPEN — BLOCKED KERNEL, AUTONOMOUS FORMATION, MATTER, PHYSICAL SECTOR IDENTIFICATION]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_two_record_momentum_sector_census.py](../../../../../scripts/proofs/proof_c18_two_record_momentum_sector_census.py)
enumerates every unordered pair of distinct C18 directions, all signed cubic
transformations, all two-record C4 payload states, and every global C4 shift.
It performs 134,709 exact checks without a fit or physical constant.

---

## 1. Why two records are the first interacting sector

The
[single-record collision no-go](THEOREM_C18_EQUIVARIANT_SINGLE_RECORD_COLLISION_NO_GO_v1.md)
proves that a fixed cubic-equivariant one-record direction rule is only
identity or antipodal reversal. Interaction must therefore begin with a joint
local configuration.

Take two occupied, distinct directed C18 channels at one site. Their
unlabelled spatial state is

\[
 X=\{d_1,d_2\},\qquad d_1\ne d_2,                  \tag{1}
\]

and define the exact sector label

\[
 \sigma(X)=
 \bigl(\text{SC/FCC shell content},\,P=d_1+d_2\bigr). \tag{2}
\]

There are

\[
 {18\choose2}=153                                  \tag{3}
\]

direction pairs and 94 distinct labels (2). A reversible collision preserving
shell content and total directed momentum may permute microstates only within
one such sector.

---

## 2. Exact sector census

The complete census is:

| Shell content | Sector size | Number of sectors | Number of states |
|---|---:|---:|---:|
| SC--SC | 1 | 12 | 12 |
| SC--SC | 3 | 1 | 3 |
| SC--FCC | 1 | 24 | 24 |
| SC--FCC | 3 | 8 | 24 |
| SC--FCC | 4 | 6 | 24 |
| FCC--FCC | 1 | 24 | 24 |
| FCC--FCC | 2 | 18 | 36 |
| FCC--FCC | 6 | 1 | 6 |

The first canonical scattering opportunity is the set of eighteen
FCC--FCC doubleton sectors. Swapping the two microstates in every doubleton
and fixing every other pair defines a target-blind involution. Because cubic
transformations map doubletons to doubletons and preserve their unique
alternative member, this unlabelled swap commutes with all 48 elements of
$O_h$.

The doubletons split further by exact momentum norm:

\[
 12\text{ sectors with }|P|^2=2,\qquad
 6\text{ sectors with }|P|^2=4.                    \tag{4}
\]

The distinction in equation (4) controls whether phase payloads can be routed
without additional state.

---

## 3. Grazing doubletons have a canonical payload route

Attach a C4 phase $k_j\in\mathbb Z/4\mathbb Z$ to each occupied direction.
In every $|P|^2=2$ doubleton, each incoming direction has a **unique**
outgoing direction maximizing the integer dot product. Route its phase record
to that direction.

For example,

\[
 \{(-1,-1,0),(0,1,-1)\}
 \longleftrightarrow
 \{(-1,1,0),(0,-1,-1)\}.                            \tag{5}
\]

For the first incoming direction in equation (5), the two outgoing dot
products are 0 and 1; for the second they are 1 and 0. Thus the payload routing
is unique. Dot products are cubic invariants, so this definition uses no
coordinate ordering or external frame.

The same maximum-dot rule applied after scattering returns each payload to
its original channel. The collision is therefore an involution.

---

## 4. Axial doubletons expose an exact routing obstruction

A representative $|P|^2=4$ sector is

\[
 \{e_x+e_y,e_x-e_y\}
 \longleftrightarrow
 \{e_x+e_z,e_x-e_z\}.                               \tag{6}
\]

The reflection $z\mapsto-z$ fixes both incoming directions in equation (6)
pointwise but exchanges the two outgoing directions. If two unequal phase
payloads transform as spatial scalars, the complete labelled input is fixed by
that reflection. Cubic equivariance would then require a labelled output to be
simultaneously fixed and exchanged, which is impossible.

Therefore:

\[
 \boxed{\text{unequal scalar phases cannot be routed equivariantly through an
 axial doubleton without additional state.}}        \tag{7}
\]

The certificate verifies this stabilizer obstruction in all six axial
doubletons. Equal phases are indistinguishable and may scatter without choosing
a route. Unequal phases require at least one of:

- a dynamically carried spatial pseudoscalar or ordered local frame;
- a non-scalar transformation law, such as a separately justified parity
  action on phase; or
- a joint outgoing bond record that postpones individual channel ownership.

No such repair is silently added here.

---

## 5. Minimum phase-complete reference collision

On a site containing exactly two occupied channels and no others, define
$\mathcal C_2$ as follows:

1. if the pair is an FCC doubleton with $|P|^2=2$, swap to the alternative
   pair and route each complete C4 payload by the unique maximum-dot rule;
2. if it is an FCC doubleton with $|P|^2=4$ and the two phases are equal,
   swap to the alternative pair with that common phase;
3. otherwise act as identity.

On the $153\times16=2448$ two-record phase states, $\mathcal C_2$ acts
nontrivially on exactly 432 states. Exact enumeration proves:

\[
 \mathcal C_2^2=1,                                  \tag{8}
\]

\[
 \mathcal C_2(gX)=g\mathcal C_2(X)
 \quad(g\in O_h),                                  \tag{9}
\]

and

\[
 \mathcal C_2(R_mX)=R_m\mathcal C_2(X)
 \quad(m\in\mathbb Z/4\mathbb Z),                 \tag{10}
\]

where $R_m$ is a global C4 phase shift. It also preserves exactly:

- record number and FCC shell content;
- total directed momentum $P$;
- the complete multiset of C4 phase payloads;
- any positive token energy depending only on occupancy and shell; and
- a local inverse, because equation (8) holds.

The rule is finite, Moore-local, target-blind, and needs no external frame.
It is a **reference construction**, not a theorem that FTD must select this
collision.

---

## 6. What has and has not been gained

This is the first explicit interaction candidate on the strict-discrete C18
carrier. Unlike the bare streaming arm, it redistributes two records among
different outgoing rays while retaining exact momentum and phase ownership.
Streaming composed with $\mathcal C_2$ is a reversible interacting lattice-gas
step.

But $\mathcal C_2$ only scatters existing records. It does not yet:

- create or annihilate manifestation records;
- exchange phase, charge, or capacity between the two records;
- generate an electromagnetic source response;
- produce a localized recurrent body or its material clock;
- derive a tensor pole, lensing, a Born pushforward, or a native coupling.

Indeed, preservation of each phase payload means this collision alone has
more conserved information than a generic interacting field theory. The next
test is whether composing it with the already proved reversible
cancellation/actualization gates yields one payload-complete local permutation
and whether the blocked two-point kernel has the required vector and capacity
transport modes.

---

## 7. Next locked gate

Before interpreting any mode physically:

1. define the complete local state ownership shared by $\mathcal C_2$, dark
   cancellation, reserve debit, and ternary manifestation;
2. prove that their composition is one reversible local permutation rather
   than an order-dependent list of unrelated rules;
3. derive the exact conserved left modes and linearized blocked collision
   operator around a target-blind reference ensemble; and
4. inspect its poles, causal cone, and cubic anisotropy before naming
   electromagnetism or gravity.

The axial obstruction must remain visible. If a later action needs those
unequal-phase channels, it must pay explicitly for the missing handedness or
joint ownership variable.

The registered product-reference calculation has now been completed in the
[exact linearized-kernel and tensor-boundary theorem](THEOREM_C18_TWO_RECORD_LINEARIZED_KERNEL_AND_TENSOR_BOUNDARY_v1.md).
The FCC correction has rank 41 and a seven-dimensional nullspace consisting
only of four phase counts and three momentum components. Its capacity shears
relax with distinct cubic rates, so this selected collision is closed negative
as a gapless tensor carrier in that linearization. It remains available only
as a scattering primitive for a larger action.
