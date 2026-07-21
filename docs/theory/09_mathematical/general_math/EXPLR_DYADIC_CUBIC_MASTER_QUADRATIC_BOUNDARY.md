# EXPLR - Dyadic Cubic / Master-Quadratic Boundary

**Document type:** Exploratory mathematical boundary note
**Status:** [THEOREM] for the stated algebraic separation results; [OPEN] only
for a future independently forced construction that introduces a new bridge
**Octave-cubic source:** [EXPLR_DYADIC_OCTAVE_BIFURCATION_ATLAS.md](EXPLR_DYADIC_OCTAVE_BIFURCATION_ATLAS.md)
**Master-quadratic reference:** [SPEC_ALGEBRAIC_SPINE.md](../../01_reference/SPEC_ALGEBRAIC_SPINE.md)
**Verifier:** `scripts/proofs/proof_dyadic_cubic_master_quadratic_boundary.py`

---

## 0. Question

The octave-8 atlas produces the regularity polynomial

```text
G(q) = 128q^3 + 16q^2 - 18q - 3.
```

Its small coefficients and branch-organizing role can look reminiscent of the
FTD master quadratic

```text
M_g(x) = x^2 - 16g^2 x + 16g^3,
```

where the canonical application takes `g=G*`.

This note asks a narrow question: is there an exact, structurally forced bridge
between these two polynomials?

Result:

```text
No direct polynomial identification or family-invariant bridge is present.
Both are closure polynomials, but they close different systems.
```

This is a boundary result. It neither changes the octave atlas nor weakens the
master quadratic's independently documented algebraic status.

---

## 1. Different sources

The dyadic cubic is a resultant factor. It appears only after eliminating the
hidden cosine `u=cos(t)` from the two speed-zero conditions of the explicitly
chosen Fourier slice

```text
a = (1, 1/2, 1/2, q),
beta = 2.
```

It is therefore a condition on a freely tunable mode amplitude `q`.

The master quadratic is a period relation in the FTD algebraic-spine program.
Its coefficients are `16G*^2` and `16G*^3`; their stated period lineage and
the separate `[SELECTION]` boundary around operator assembly are documented in
the algebraic spine. The variable `x` is not a Fourier-mode amplitude.

No map `q=f(G*)` is supplied by either construction. Introducing one here
would be a parametric insertion, not a derivation.

---

## 2. Exact cubic data

The octave cubic is irreducible over the rationals, with discriminant

```text
disc(G) = 2^10 * 3 * 367.
```

The discriminant is not a rational square. Thus its irreducible cubic has the
generic `S_3` splitting-field type; its quadratic resolvent subfield is
controlled by `sqrt(3*367)`.

After the canonical translation `q=z-1/24` and division by `128`, it becomes

```text
z^3 - (7/48)z - 241/13824 = 0.
```

The power-of-two leading coefficient and the visible `16` are therefore not
normal-form invariants. They originate in the Chebyshev/dyadic elimination.

For its three roots `q_1,q_2,q_3`, Vieta gives

```text
q_1 + q_2 + q_3             = -1/8,
q_1q_2 + q_1q_3 + q_2q_3    = -9/64,
q_1q_2q_3                   = 3/128,

1/q_1 + 1/q_2 + 1/q_3      = -6.
```

The last identity is a curve-slice invariant only. It is not the
master-quadratic tower's harmonic invariant.

---

## 3. Exact master-quadratic contrast

Over the formal coefficient field `Q(g)`, the master polynomial has degree two
and discriminant

```text
disc_x(M_g) = 64g^3(4g-1).
```

With `x=g y`, its normalized form is

```text
y^2 - 16g y + 16g = 0.
```

Its two normalized roots obey the master-tower invariant

```text
1/y_+ + 1/y_- = 1.
```

The `-6` cubic reciprocal sum and the master value `1` are different Vieta
facts of polynomials with different degrees. Neither determines the other.

Most importantly, a cubic root divisor has three projective points and a
quadratic root divisor has two. No invertible affine or projective change of
variable can identify the full root structures without deleting or adding a
root. The degree mismatch is an exact direct-identity obstruction.

---

## 4. Natural counter-slice

The strongest separation test preserves the dyadic frequencies and
alternating-chiral convention, but removes only the `4t` mode:

```text
a = (1, 1/2, 0, q),
beta = 2.
```

Eliminating the speed-zero conditions for this neighboring slice gives

```text
Res = nonzero_constant * q^8 (4q+1)(64q^2 - 8q - 1)^3.
```

The new quadratic threshold factor is coprime to the original octave cubic:

```text
gcd(128q^3+16q^2-18q-3, 64q^2-8q-1) = 1.
```

So the cubic is not a mode-family invariant, not a universal dyadic
regularity polynomial, and not a stable analogue of the master quadratic.
It is forced by the specific lower-mode choice used by `C_3`.

This is an exact controlled counterexample, not a numerical perturbation scan.

---

## 5. What remains meaningful

There is one disciplined commonality:

```text
hidden constraints -> elimination -> low-degree closure polynomial -> branches.
```

For the Fourier family, the hidden variables are phase/cosine coordinates and
the roots organize geometric regularity regimes. For the master program, the
polynomial organizes a period-relation root structure. That is a useful
mathematical pattern, but a pattern is not a derivation.

The correct status is therefore:

```text
[STRUCTURAL ANALOGY], not an FTD bridge.
```

---

## 6. Falsifier for a future bridge claim

A future positive bridge would have to supply all of the following before any
promotion:

1. an independently derived map from the Fourier amplitude `q` to an FTD
   algebraic-spine variable;
2. a common invariant that survives a documented class of dyadic mode slices,
   not just the `C_3` coefficient choice;
3. a degree-preserving algebraic construction, such as a canonical cubic
   resolvent or discriminant relation, rather than a fitted substitution;
4. a proof that the construction imports no undeclared coefficient choice.

Until then, the direct-bridge hypothesis is unsupported.

---

## 7. Verification

Run:

```text
python scripts/proofs/proof_dyadic_cubic_master_quadratic_boundary.py
```

The verifier uses exact polynomial algebra only. It does not conduct a
numerical near-miss search.

*End of document.*
