# FTD-0621 — Ternary block-bipole Peierls scaling analysis

**Status:** `[THEOREM — EXACT FINITE-VOLUME SPECTRAL IDENTITY]` +
`[MEASURED — INTEGER EXTENSION SUPPRESSES RELATIVE PINNING]` +
`[OPEN — CONNECTED DYNAMICS / FIXED-ENERGY PARTICLE LIMIT]`  
**Verdict:** `INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS`  
**Production status:** unchanged

## 1. Result

An exactly ternary extended source can move the compact quadratic coat's
field-energy spectrum toward the infrared without fractional site polarity or
a fitted smooth envelope.

The registered object consists of one `w x w x w` block of `+1` sites adjacent
to an equal block of `-1` sites. It contains exactly `w^3` sites of each sign,
is neutral, has connected finite support, and uses only primitive ternary
states. All 90 held-out volume/orientation/translation arms pass.

For the representative orientation at `L=257`:

| `w` | field energy `E` | parallel barrier `B_parallel` | `Pi_parallel=B/E` | transverse barrier `B_perp` | `Pi_perp` |
|---:|---:|---:|---:|---:|---:|
| 5 | 4.65440682 | 0.01599732 | 3.43703e-3 | 0.00933440 | 2.00550e-3 |
| 9 | 91.1072668 | 0.06215453 | 6.82213e-4 | 0.03842152 | 4.21717e-4 |
| 15 | 1184.15582 | 0.19083474 | 1.61157e-4 | 0.12132744 | 1.02459e-4 |
| 23 | 10061.7488 | 0.47492235 | 4.72008e-5 | 0.30647481 | 3.04594e-5 |
| 35 | 81879.5025 | 1.14402135 | 1.39720e-5 | 0.74562150 | 9.10633e-6 |

The least improvement from `w=5` to `w=35` over the nine cubic arms is
`220.231`. The largest final pinning index is `1.39720e-5`, below the locked
`5e-5` observer threshold.

## 2. Exact content

The source transform factorizes as

\[
\widetilde s_{w,d}(k)=
\prod_{j=1}^{3}\left(\sum_{r=0}^{w-1}e^{-ik_jr}\right)
\left(1-e^{-iwk_d}\right).
\]

Substitution into the existing quadratic-coat finite-volume energy gives the
Peierls identity

\[
\frac{B_i}{E}=
\left\langle
\left(\frac{1-\cos k_i}{3+\cos k_i}\right)^2
\right\rangle_E.
\]

This equality is theorem-grade for the selected source family and coat. The
numerical record checks the independently evaluated sides to
`4.34e-19`. Closed-form Dirichlet factors agree with compensated finite sums
to `1.76e-14`; cubic covariance closes to `1.63e-14`.

FTD-0579 remains intact: every finite member has a strictly positive absolute
barrier. Extension suppresses the barrier relative to total field energy; it
does not make a finite carrier continuously translation invariant.

## 3. Held-out scaling

On the locked fit window `w={9,15,23,35}`, the representative slopes are

| quantity | parallel | transverse | dimensional target |
|---|---:|---:|---:|
| field energy | 5.00774 | 5.00774 | 5 |
| absolute barrier | 2.14466 | 2.18358 | 2 |
| relative pinning `Pi` | -2.86309 | -2.82416 | -3 |

The worst residuals from the target exponents over the cubic orbit are
`0.00774`, `0.18358`, and `0.17584`, all inside the locked `0.35` bands. The
largest main/replication difference on the registered non-crowded widths is
`0.2341%`.

The scaling has a simple interpretation: the fixed-density bipole's Coulomb
field energy is volumetric/collective (`~w^5`), whereas the lattice-phase
penalty is boundary dominated (`~w^2`). Their ratio therefore falls as
`~w^-3`.

## 4. Ontological consequence

The primitive ternary alphabet does not force matter to be a point carrier or
a fractional cloud. It can encode a finite many-site polarity texture whose
coarse response is increasingly insensitive to subcell lattice phase. Thus
the compact-core Peierls pathology can be a ultraviolet property of an
overlocalized surrogate rather than a universal obstruction to extended
ternary matter.

This is the first native-integer representability witness for that statement.
The favorable FTD-0555 smooth envelope is no longer the only known spectral
example: the present source is exactly `-1/0/+1` at every primitive site.

## 5. What the result does not establish

The absolute barrier grows approximately as `w^2`; it does not fall. The ratio
falls because the object also contains `2w^3` manifested sites and its field
energy grows approximately as `w^5`. Width therefore changes constituent
number, dipole moment, energy, and any prospective inertia. This campaign does
not exhibit a fixed-charge or fixed-mass particle family.

The block bipole is a static source architecture, not a solution of a common
action. Nothing here shows that the production tick can create it, that local
dynamics bind its interface, that it preserves shape, or that an impulse moves
it coherently. It is net neutral and does not derive electromagnetic charge.

Calling the result a particle, soliton, atom, membrane, or stable matter object
would therefore be incorrect. The precise conclusion is only:

> Exact finite ternary configurations exist for which the compact-coat
> Peierls barrier is positive at every finite width but becomes small relative
> to the configuration's field energy with the registered `~w^-3` scaling.

## 6. Numerical-correction provenance

The first two executions are preserved as
`ftd_0621_invalid_execution_1.json` and
`ftd_0621_invalid_execution_2.json`. Their physical scaling gates passed, but
the observer was invalid under the locked algebraic thresholds because naive
summation accumulated `~2e-11` residuals. The final implementation uses
compensated spectral sums and compares a closed-form Dirichlet transform with
an independently compensated finite sum. No formula, width, tolerance,
classification rule, or physical threshold changed.

## 7. Next gate

FTD-0622 must construct one connected local action for an integer extended
architecture and measure two quantities together as width changes:

1. the relative Peierls index; and
2. the matter-plus-field continuous-translation reaction defect per a locked
   physical normalization.

Independent copies of the six-constituent carrier do not qualify. Nor does a
single imposed collective centre coordinate unless it is reconstructible from
the complete state and its local current/action are derived. The first
practical widths should be small enough for exact state-only inversion, with a
predeclared route from those local degrees of freedom to a scalable domain or
interface dynamics.

