# EXPLR - Dyadic Geometric Byte

**Document type:** Exploratory mathematical note  
**Status:** [THEOREM] for the finite-state encoding and graph structure;
[EXPLORATORY] for geometric and FTD-facing interpretation  
**Primary curve note:** [EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md](EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md)  
**Configuration atlas:** [EXPLR_DYADIC_LACUNARY_PHASE_RIBBONS.md](EXPLR_DYADIC_LACUNARY_PHASE_RIBBONS.md)  
**Verifiers:** `scripts/proofs/proof_dyadic_geometric_byte.py`,
`scripts/proofs/proof_dyadic_edge_census.py`

---

## 0. Result

Eight mode-enable flags form an exact geometric byte:

```text
B_8 = {0,1}^8,
|B_8| = 2^8 = 256.
```

The signs visible in the curve require a larger alphabet. An eight-mode word

```text
w = (w_0,...,w_7),   w_k in {-1,0,+1}
```

is an eight-trit word with

```text
|T_8| = 3^8 = 6561
log_2 |T_8| = 8 log_2(3) = 12.6797... bits.
```

Thus "geometric byte" is literal for support. The signed configuration is a
geometric eight-trit word, not literally one byte.

No FTD physics claim follows from this encoding.

---

## 1. Support projection

Define

```text
pi : T_8 -> B_8,
pi(w)_k = |w_k|.
```

If a support mask `b` has Hamming weight `h`, exactly `h` modes are active.
Each active mode independently chooses chirality `+1` or `-1`, so

```text
|pi^{-1}(b)| = 2^h.                              [THEOREM]
```

Summing the fibers recovers the full signed state space:

```text
sum_{b in B_8} 2^{|b|}
= sum_{h=0}^8 binom(8,h) 2^h
= (1+2)^8
= 3^8.
```

This is the exact relation between the 256-cell support map and the 6,561
signed curves represented by it. A map cell of weight zero carries one signed
word; a weight-eight cell carries 256 chirality assignments.

---

## 2. Exact integer fingerprint

The signed word has the balanced-ternary value

```text
B(w) = sum_{k=0}^7 w_k 3^k.
```

Balanced ternary is unique, hence this is a bijection

```text
T_8 <-> {-3280,-3279,...,3279,3280}.             [THEOREM]
```

For an array-friendly nonnegative index, replace each trit by the ordinary
base-three digit `d_k=w_k+1`:

```text
I(w) = sum_{k=0}^7 (w_k+1)3^k
     = B(w) + 3280,
0 <= I(w) <= 6560.
```

The support mask and ternary index jointly expose two different invariants:

```text
support byte: which clocks exist
ternary word: which clocks exist and which chirality each carries
```

For the atlas reference word `(+,-,+,-,0,0,0,0)`:

```text
support = 00001111_2 = 0x0F,
B(w) = -20,
I(w) = 3260.
```

---

## 3. Configuration graphs

The binary masks are vertices of the 8-cube `Q_8`. Flipping one enable flag
moves along one cube edge. Every mask therefore has eight one-bit neighbors,
and `Q_8` has

```text
8 * 2^7 = 1024
```

undirected edges.

The signed words form the Hamming graph `H(8,3)`. Changing one trit to either
of its other values gives 16 one-coordinate neighbors per word and

```text
(6561 * 16) / 2 = 52,488
```

undirected edges. The atlas's one-mode ramps are continuous geometric paths
associated with these discrete one-coordinate transitions.

This graph view separates three layers:

1. the combinatorial state address;
2. the continuous amplitude path between addresses;
3. the geometric events encountered along that path, such as tangencies,
   self-intersection changes, or loss of regularity.

Only the first layer is completely classified by the byte theorem.

### 3.1 Exact C3-tail edge census

For the default eight-mode C3 geometric tail

```text
a = (1, 1/2, 1/2, 3/8, 3/16, 3/32, 3/64, 3/128),
beta = 2,
```

complete rational enumeration gives:

```text
vertices                         6,561
one-trit edges                  52,488
area-wall edges                 10,132
quotient-wall edges              6,558
support-wall edges              34,992
chirality-wall edges            17,496
selected invariant bins          2,510
```

The area test is exact. Along every edge ordered as `-1 -> 0 -> +1`, the
changed mode's signed-area contribution is monotone, so an area wall is
present exactly when the rational endpoint areas have opposite signs or one
endpoint area is zero.

Deleting selected barrier edges and taking connected components gives the
exact chamber counts

```text
area barriers only                        5
quotient barriers only                    8
support barriers only                   256
support plus chirality barriers        6,561
```

The atlas's 2,510 bins group edges by a declared tuple of exact invariants:
changed mode, transition kind, fixed support size, lowest unchanged mode,
endpoint area signs, area-root count, quotient change, and endpoint pattern
classes. They are an **invariant partition**, not group-theoretic orbits and
not a proof that edges in one bin are geometrically equivalent. The exact
enumeration and chamber counts are independently checked by
`proof_dyadic_edge_census.py` using `Fraction` arithmetic only.

---

## 4. Geometric meaning

For the finite dyadic curve

```text
C_w(t) = sum_k w_k a_k (cos(2^k t), beta(-1)^k sin(2^k t)),
```

the support byte chooses a subset of eight dyadic clocks. The sign word then
chooses the orientation of each active clock's contribution. Because the
visible curve is a projection of their simultaneous phase evolution, changing
one bit or trit can reorganize the projected node network globally even though
the configuration-space move is local.

The lively "tesseract-like" behavior therefore has a precise but restrained
reading: the app displays a two-dimensional geometric readout of paths through
an eight-coordinate configuration graph. It is not evidence that the curve is
a physical tesseract.

---

## 5. FTD boundary

The ternary alphabet resembles FTD's state alphabet, and the support projection
resembles forgetting sign while retaining manifestation. That is a structural
analogy only. The curve uses global Fourier modes, while FTD's ontology and
dynamics are local lattice postulates.

```text
[THEOREM]     256 support masks, 6561 signed words, fiber sizes, encodings.
[EXPLORATORY] configuration-graph interpretation of mutable curve geometry.
[OPEN]        classification of geometric event walls on all 52,488 edges.
[OPEN]        any disciplined map from these global mode words to FTD states.
```

The productive next object is an edge atlas: for each one-trit transition,
record exact or certified event walls and use graph symmetry to reduce the
52,488 raw edges to structural equivalence classes.

The interactive configuration atlas now implements finite band slices
`H(m,3)` for `2 <= m <= 6`. It draws exact one-trit adjacency and colors each
state by analytic invariants or an explicitly sampled minimum-speed statistic.
For a selected edge it also separates analytic area, support, quotient, and
certified `C_3` walls from sampled crossing-count and local-tube brackets. This
is an instrument for the open edge-atlas program, not a completion of the
52,488-edge classification.

*End of document.*
