# C18/FCC Gaussian-current Bloch diffusion boundary v1

**Date:** 2026-08-23  
**Status:** **[THEOREM — EXACT ZERO FIRST-ORDER BLOCH GENERATOR]** +
**[THEOREM — EXACT SECOND-ORDER CHIRAL-DIFFUSION MATRIX]** +
**[CLOSED NEGATIVE, SCOPED — GAUSSIAN-CURRENT COLLISION PLUS
PHASE-INDEPENDENT STREAMING AS A MAXWELL LIGHT-CONE CARRIER]** +
**[OPEN — ORIENTED HODGE/COTANGENT REPAIR, GAUSS LAW, SOURCE WORK, ALPHA]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[proof_c18_fcc_gaussian_current_bloch_boundary.py](../../../../../scripts/proofs/proof_c18_fcc_gaussian_current_bloch_boundary.py)
performs 24 exact matrix checks. It constructs the full 48-dimensional
collision tangent matrix, its exact left and right unit spaces, the reduced
Bloch perturbation on all three cubic axes, a bordered exact inverse for the
second-order coefficient, and the global C4 commutator. No numerical
eigensolver, fit, physical constant, or target dispersion is used.

---

## 1. Question tested

The
[Gaussian-current collision theorem](THEOREM_C18_FCC_GAUSSIAN_CURRENT_COLLISION_AND_MAXWELL_MODE_PRICE_v1.md)
proved that one exact local reversible collision protects precisely

\[
 n,qquad \mathcal C=U+iV,                           \tag{1}
\]

where $n$ is record number and $U,V$ are real FCC vector triplets. This pays
the zero-wavevector conservation and mode-count price for a candidate
electromagnetic carrier.

Conservation is not propagation. This theorem tests the next required object:
the exact small-wavevector spectrum after composing that collision with
ordinary phase-independent one-hop FCC streaming.

---

## 2. Exact Bloch operator

Let

\[
 J_0=I+\delta N,
 \qquad \delta={1\over5^{11}},                      \tag{2}
\]

be the exact product-reference collision Jacobian. For Fourier wavevector
$k$, one-hop streaming is

\[
 D(k)_{(d,p),(d,p)}=e^{-ik\cdot d}.                 \tag{3}
\]

The one-tick Bloch operator is

\[
 M(k)=D(k)J_0.                                      \tag{4}
\]

Let $L$ contain the seven invariant rows from equation (1), let the columns
of $R$ span $\ker N$, and set

\[
 G=LR.                                              \tag{5}
\]

The exact ranks are

\[
 \operatorname{rank}N=41,
 \qquad \dim\ker N=7,
 \qquad \det G\ne0.                                \tag{6}
\]

The parent characteristic polynomial gives algebraic multiplicity seven for
the zero root of $N$. Equation (6) gives geometric multiplicity seven.
Therefore the unit eigenspace of $J_0$ is semisimple, isolated from the 41
strictly damped modes, and ordinary analytic degenerate perturbation theory
applies near $k=0$.

---

## 3. Exact first-order result

For spatial component $a$, define the diagonal streaming generator

\[
 (K_a)_{(d,p),(d,p)}=d_a.                           \tag{7}
\]

The first-order reduced Bloch matrix is, up to the Fourier factor $-i$,

\[
 A_a=LK_aR\,G^{-1}.                                 \tag{8}
\]

The certificate proves exactly

\[
 \boxed{LK_xR=LK_yR=LK_zR=0.}                      \tag{9}
\]

Consequently every protected eigenbranch obeys

\[
 \mu_j(k)=1+O(|k|^2),                               \tag{10}
\]

not

\[
 \mu_\pm(k)=1\pm i c|k|+O(|k|^2).                 \tag{11}
\]

Equation (9) is already sufficient to close the tested Maxwell-cone route.
The collision protects the complex current, but phase-independent streaming
does not couple its slow variables at first spatial order.

---

## 4. Exact second-order dynamics

The second-order calculation identifies what replaces the missing cone. For
each axis, let $X_a$ be the unique gauge-fixed solution of

\[
 NX_a=K_aR,
 \qquad LX_a=0.                                    \tag{12}
\]

Existence follows from equation (9). In conserved-variable coordinates, the
second-order coefficient is

\[
 B_a=L\left(
 {1\over\delta}K_aX_a+{1\over2}K_a^2R
 \right)G^{-1}.                                    \tag{13}
\]

For $k\parallel e_z$, order the variables as

\[
 (n,U_x,U_y,U_z,V_x,V_y,V_z).                      \tag{14}
\]

Then $B_z$ separates exactly into one scalar, two identical transverse
blocks, and one longitudinal block. Each transverse polarization has

\[
 \boxed{
 B_T={1\over104}
 \begin{pmatrix}
 -244140599& 48828125\\
 -48828125&-244140599
 \end{pmatrix}.}                                   \tag{15}
\]

Its two eigenvalues are

\[
 \boxed{
 \beta_T=-{244140599\over104}
 \pm i\,{48828125\over104}.}                       \tag{16}
\]

The same block occurs for both transverse spatial directions. Thus

\[
 \mu_T(k)=1+\beta_T|k|^2+O(|k|^3).                 \tag{17}
\]

The negative real part is diffusion/damping; the imaginary part is a chiral
quadratic phase. Neither term is a linear light-cone frequency.

For completeness, the record scalar coefficient is

\[
 \beta_n=-{651041\over2},                           \tag{18}
\]

and the longitudinal $(U_z,V_z)$ block is

\[
 B_L={1\over8177}
 \begin{pmatrix}
 -17114253724& 2099609375\\
 -2099609375&-17114253724
 \end{pmatrix}.                                    \tag{19}
\]

The $x$- and $y$-axis matrices have the same exact characteristic polynomial
by direct calculation, as required by cubic covariance.

The large rational magnitudes in equations (15)--(19) reflect the sparse
collision weight $\delta=5^{-11}$. They are not physical transport
coefficients and may not be interpreted as a coupling.

---

## 5. Why the native clock does not repair it

Let $P_4$ advance every C4 phase by one. The certificate proves

\[
 P_4^4=I,
 \qquad [P_4,N]=0,
 \qquad [P_4,K_a]=0.                               \tag{20}
\]

On the protected current, $P_4$ sends $(U,V)\mapsto(-V,U)$ and shifts the
carrier quasiphase by a quarter turn. Because it commutes with both collision
and streaming, passing to the co-rotating frame removes this common phase and
leaves equations (9) and (17) unchanged.

Therefore:

\[
 \boxed{
 \text{a global C4 clock supplies phase orientation, not a missing spatial
 light cone.}}                                      \tag{21}
\]

This is consistent with FTD's distinction between the unconditional global
substrate tick and local material-clock recurrence. Neither clock alone is a
propagation law.

---

## 6. Scoped closure

For the exact registered composition

\[
 \text{phase-independent FCC streaming}
 \circ
 \text{Gaussian-current collision},                \tag{22}
\]

the electromagnetic interpretation is closed negative:

\[
 \boxed{
 \omega(k)-\omega(0)=O(|k|^2),
 \quad v_g(0)=0,
 \quad \text{not }\omega=c|k|.}                    \tag{23}
\]

This does not retract the parent conservation theorem. Equation (1) remains
an exact protected current, and the manifestation source remains exactly
aligned with it. The result says that **conservation plus ordinary streaming
is insufficient**.

The closure is also scoped. It does not exclude:

- a phase-dependent or oriented Hodge streaming rule with nonzero equation
  (8);
- a polar/axial bond--plaquette pair producing a first-order discrete curl;
- a cotangent carrier with a nonsemisimple zero-mode shear whose $k^2$
  restoring term splits as $\omega\sim|k|$; or
- a larger reversible transaction in which manifestation/capacity exchange
  changes the hydrodynamic projector.

---

## 7. Dynamical price of a Maxwell repair

The exact failure separates two mathematically distinct repair classes.

### Route A: first-order Hodge/curl pair

Construct two protected carrier types for which

\[
 LK_aR\ne0                                         \tag{24}
\]

on the physical transverse subspace. A natural discrete candidate is a polar
bond current paired with an axial oriented plaquette circulation. The local
incidence/Hodge map could then generate the two Maxwell curls directly.

An unordered equal-phase pair cannot assign the sign of $d\times e$ without
an orientation record. The already existing actualization payload carries
$\epsilon\in\{-1,+1\}$, making that orientation a candidate shared owner, not
a license to impose a magnetic field by hand.

### Route B: cotangent/Jordan pair

Construct a canonical coordinate/momentum pair with a nonsemisimple unit
block at $k=0$. A $k^2$ restoring operator can then split the block with
linear $|k|$ frequency, as in a wave equation written in first-order time.
A finite-order local permutation is diagonalizable over $\mathbb C$, so this
route requires a genuine carry, distributed winding, or blocked canonical
limit rather than another finite C4 cycle alone.

Both routes must be derived from the same finite ownership/work ledger. Route
A is the lower finite-state price presently visible, but this is a
**[SELECTION CANDIDATE]**, not a theorem of physical identity.

---

## 8. Consequences for the unified objective

| Requested sector | Consequence of this theorem |
|---|---|
| Manifestation | source-to-current normalization survives exactly |
| Matter and clocks | recurrent proto-clock survives; common C4 advance does not create propagation |
| Electromagnetism | protected current survives, but the tested kinetic completion is closed negative |
| Gravity/lensing | unchanged; tensor moments are not collision invariants |
| Contextual Born tape | unchanged; prepared finite pushforward remains exact but separate |
| Native alpha | still inadmissible because $G_{\rm vac}$ and $c_{\rm eff}$ do not exist for this route |

The next construction must therefore add the missing **spatial orientation or
cotangent structure**, not tune the damping spectrum of equation (18).

---

## 9. Next locked gate

Before another source-response or alpha campaign, construct an exact local
oriented bond--plaquette transaction using only existing C4 payload,
orientation, capacity, and ternary ownership. The certificate must show:

1. a well-defined axial loop record under unordered pair presentation;
2. cubic covariance, including reflections;
3. an exact inverse and capacity/work ledger;
4. a nonzero first-order reduced Hodge/curl matrix;
5. exactly two transverse propagating modes with no extra gapless species;
6. compatibility with the actualization source and recurrent material clock;
   and
7. no physical target or master root in the construction.

Only a pass restores eligibility for a native $G_{\rm vac}$,
$c_{\rm eff}$, $g_{\rm eff}$, and eventual blind alpha measurement.

**Executed successor:** the
[oriented bond--plaquette Hodge-Maxwell target](THEOREM_ORIENTED_BOND_PLAQUETTE_HODGE_MAXWELL_TARGET_AND_FINITE_LIFT_BOUNDARY_v1.md)
passes the representation and linear-generator parts of this gate. One
orientation bit resolves the axial sign, and the exact centered-incidence
edge--face generator has two divergence constraints and two $\omega=|q|$
polarizations. It remains a kinematic target: no finite payload-complete local
permutation, work ledger, or source closure realizes the generator yet.
