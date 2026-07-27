# THEOREM — Local polarity regularity trilemma

**Identifier:** `FTD-0540`  
**Status:** `[THEOREM — REPRESENTATION NO-GO]` +
`[CONSTRUCTIVE — TWO EXPLICIT ESCAPE WITNESSES]`  
**Inputs:** FTD-0478 site/remainder coupling representation and ordinary
integer-translation covariance.  No field equation, action, energy, force, or
continuum assumption enters.

## 1. Nearest-cell exactness forces the hat

Work first in one dimension. For `x in [0,1]`, let `w_n(x)` be the coupling
weight carried by integer site `n`. Nearest-cell support says only `w_0` and
`w_1` can be nonzero. Partition of unity and exact first-moment reproduction
give

```text
w_0(x)+w_1(x)=1,
            w_1(x)=x.
```

The coefficient matrix has determinant one, so the unique solution is

```text
w_0(x)=1-x,       w_1(x)=x.                         (1)
```

No positivity, polynomial ansatz, or fitting is needed for this conclusion.
Integer-translation covariance stitches (1) into

```text
Lambda(u)=max(1-|u|,0),
w_n(x)=Lambda(x-n).                                 (2)
```

At the cardinal center,

```text
Lambda'(0-)=+1,       Lambda'(0+)=-1,               (3)
```

so the derivative jump has magnitude two. The FTD-0539 reflection-plane cusp
is therefore not removable by changing coefficients while retaining the
FTD-0478 nearest-cell, partition, and first-moment requirements.

## 2. Smooth + nonnegative + cardinal + first moment is impossible

The obstruction is not confined to nearest-cell support.

Assume a locally finite family `{w_n}` has all four properties:

1. every `w_n` is `C1` near `x=0`;
2. every `w_n(x)>=0`;
3. cardinality: `w_n(m)=delta_nm` at integer positions;
4. first-moment reproduction: `sum_n n w_n(x)=x`.

For every `n!=0`, cardinality and nonnegativity make `x=0` a local minimum of
`w_n`, so differentiability gives

```text
w'_n(0)=0,       n!=0.                              (4)
```

Local finiteness allows the first-moment identity to be differentiated:

```text
1 = d/dx [sum_n n w_n(x)] at x=0
  = sum_n n w'_n(0)
  = 0,                                                (5)
```

a contradiction. The `n=0` term cannot help because its moment coefficient is
zero. Partition of unity is compatible with the argument but is not even
needed for the contradiction.

Thus a smooth coupling coat cannot retain both nonnegative weights and exact
site-cardinality while reproducing physical position. This is a theorem about
the representation, not a claim that manifestation itself is spatially
extended.

## 3. Three-dimensional lift

On the unit cube, the multiaffine polynomial space has basis

```text
1, x, y, z, xy, xz, yz, xyz
```

and dimension eight. Evaluation on the eight Boolean vertices is invertible,
so there is a unique multiaffine cardinal basis. For vertex
`v=(v_x,v_y,v_z)` it is

```text
phi_v(r)=product_i [v_i r_i+(1-v_i)(1-r_i)].         (6)
```

Equation (6) is exactly the FTD-0478 signed trilinear shape. Restricting it to
any coordinate line recovers (2)--(3). Tensoring does not regularize the cusp;
it places a copy on every integer reflection plane. This uniqueness statement
is deliberately limited to the declared multiaffine class. Partition and
three first moments alone would not uniquely determine eight arbitrary cube
weights.

## 4. Exact prices of the two smooth exits

### 4.1 Non-cardinal positive coat

The centered quadratic B-spline

```text
B_2(u) = 3/4-u^2,                    |u|<=1/2,
       = (1/2)(3/2-|u|)^2,           1/2<|u|<3/2,
       = 0,                          otherwise
```

is compact, nonnegative, and `C1`. Its integer translates obey

```text
sum_n B_2(x-n)=1,
sum_n n B_2(x-n)=x.                                  (7)
```

But at `x=0` the weights are

```text
(w_-1,w_0,w_+1)=(1/8,3/4,1/8),                       (8)
```

so a site-centered manifested polarity has a smooth three-site coupling coat.
It is not cardinal.

### 4.2 Cardinal signed coat

The compact Catmull-Rom kernel (`a=-1/2`) is `C1`, cardinal, and its integer
translates satisfy both identities in (7). On the outer interval,

```text
K(u)=-|u|^3/2+5|u|^2/2-4|u|+2,       1<|u|<2.
```

Its stationary point at `|u|=4/3` has

```text
K(4/3)=-2/27.                                         (9)
```

Smooth cardinal interpolation is therefore possible only by allowing a
positive manifested polarity to induce some negative coupling weights (or by
relaxing another premise not represented by this witness).

## 5. Consequence for reciprocal mobile matter

FTD-0539's cusp is structural under the FTD-0478 compact trilinear contract.
A new endpoint action using the same shape cannot honestly advertise the cusp
as a tunable numerical artifact. A restart must declare at least one new
price:

- a nonsmooth subgradient/contact selector;
- a smooth non-cardinal coat such as `B_2`;
- a smooth cardinal kernel with signed lobes;
- or a new primitive carrier/shape variable.

None of these choices is forced by P1--P5. The theorem does not repair the
independent FTD-0538/0539 energy failure, derive a force, or license a mobile
engine branch. It converts an apparently local numerical problem into an
explicit ontology/representation decision.

