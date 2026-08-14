# Theorem — Native plaquette `C4` circulation and embedded-leakage boundary v1

**Identifier:** `FTD-0918`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT NATIVE C4 CIRCULATION OBSERVABLE]` +
`[THEOREM — CONDITIONAL ISOTROPIC CONSERVATION AND TORQUE BALANCE]` +
`[ENGINE FACT — ELEMENTARY PLAQUETTE IS NOT AN INVARIANT FREE-WAVE SUBSPACE]` +
`[CLOSED NEGATIVE — BARE INTERNAL MAP HAS AN EXACT FINITE INTEGER PERIOD]` +
`[OPEN — BOUNDED SOURCE-BALANCED CIRCULATION CARRIER]`

## 1. Result

The production flux and wave-velocity fields already contain the minimum
orientation-sensitive phase-space datum sought after FTD-0915. On any ordered
four-corner plaquette, define

\[
q={J_0-J_2\over2},\qquad r={J_1-J_3\over2},
\]

\[
p_q={W_0-W_2\over2},\qquad p_r={W_1-W_3\over2}.
\]

Then

\[
\boxed{\mathcal L_P=q\!\cdot p_r-r\!\cdot p_q}
\]

is an exact native `C4` circulation observable. It is invariant under a
quarter-turn, odd under reflection, and odd under canonical time reversal.
It is the phase-space version of the ordered bivector that FTD-0914 found was
lost by the symmetric square.

For an invariant isotropic `C4` doublet, the production-order kick--drift map
conserves `L_P` exactly. With sources, damping, and noise it obeys an exact
torque ledger. But the four vertices of an elementary plaquette are **not** an
invariant subspace of the production 18-point Laplacian: an exterior site
receives a nonzero kick immediately. The internal block also has no exact
finite integer return.

Thus the missing object has been split cleanly:

- the **orientation charge already exists** in native real fields;
- the **bounded clock body that retains it does not yet exist** in production.

No new complex field, engine term, `G*` factor, gamma magnitude, Born weight,
or measurement context enters this result.

## 2. The first `C4` harmonic

For a scalar or vector-valued plaquette word

\[
F=(F_0,F_1,F_2,F_3),
\]

define

\[
\Pi_1 F=(q_F,r_F)
=\left({F_0-F_2\over2},{F_1-F_3\over2}\right).
\]

Let the forward site shift be

\[
S(F_0,F_1,F_2,F_3)=(F_3,F_0,F_1,F_2).
\]

Direct substitution gives

\[
\Pi_1SF=(-r_F,q_F)
=R\Pi_1F,
\qquad
R=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.
\]

Therefore

\[
R^2=-I,\qquad R^4=I,\qquad R^{-1}=R^T=-R.
\]

This is multiplication by `i` written on a real two-coordinate plane. The
complex abbreviation `z=q+i r` introduces no additional field type.

## 3. Native circulation and its parities

Apply the projection separately to the production canonical fields `J` and
`W`:

\[
(q,r)=\Pi_1J,
\qquad
(p_q,p_r)=\Pi_1W.
\]

The determinant pairing on the `C4` coordinate plane, contracted over the
three spatial vector components, is

\[
\mathcal L_P=q\cdot p_r-r\cdot p_q.
\]

Rotating both pairs by `R` yields

\[
(-r)\cdot p_q-q\cdot(-p_r)=\mathcal L_P.
\]

Reflecting the second coordinate sends

\[
(q,r,p_q,p_r)\mapsto(q,-r,p_q,-p_r)
\]

and hence `L_P -> -L_P`. Canonical time reversal leaves `(q,r)` fixed and
sends `(p_q,p_r)->(-p_q,-p_r)`, again giving `L_P -> -L_P`.

This proves that the native phase-complete substrate can distinguish
clockwise from counterclockwise. An instantaneous `J` word still cannot:
orientation lives in the relation between configuration and conjugate
velocity.

## 4. Why isotropy is forced on a symmetric `C4` doublet

Let a real symmetric quadratic generator on the doublet have stiffness

\[
K=\begin{pmatrix}a&b\\b&c\end{pmatrix}.
\]

Requiring `KR=RK` gives

\[
b=0,\qquad a=c.
\]

Therefore every real symmetric operator commuting with this irreducible real
quarter-turn is

\[
K=\kappa I.
\]

The two projected coordinates must have the same stiffness whenever the
doublet is both symmetric and `C4` invariant. This is a theorem about the
restricted generator, not a claim that an elementary plaquette is closed
under the full lattice generator.

## 5. Exact conservation and torque ledger

For step `h`, take the isolated isotropic kick--drift map

\[
p_q^+=p_q-h\kappa q,
\qquad
p_r^+=p_r-h\kappa r,
\]

\[
q^+=q+h p_q^+,
\qquad
r^+=r+h p_r^+.
\]

Then

\[
\begin{aligned}
\mathcal L_P^+
&=(q+h p_q^+)\cdot p_r^+
 -(r+h p_r^+)\cdot p_q^+\\
&=q\cdot p_r^+-r\cdot p_q^+\\
&=q\cdot p_r-r\cdot p_q
=\mathcal L_P.
\end{aligned}
\]

The drift cross terms cancel identically, and the isotropic kick terms cancel
identically.

Now add arbitrary projected impulses `(u_q,u_r)` to the kick. The same
calculation yields

\[
\boxed{\Delta\mathcal L_P=q\cdot u_r-r\cdot u_q.}
\]

If a common scalar damping `rho` and subsequent additive impulse/noise
`(eta_q,eta_r)` act on momentum after the drift, then

\[
\boxed{
\mathcal L_{P,\mathrm{end}}
=\rho\left(\mathcal L_P+q\cdot u_r-r\cdot u_q\right)
+q^+\cdot\eta_r-r^+\cdot\eta_q.}
\]

This is the ledgerable law: every change in local handed circulation is
assigned to an exterior/source torque, a common dissipative factor, or an
additive impulse. Nonuniform genesis drains and other nonlinear production
maps are not silently absorbed into an alleged conservation theorem.

## 6. Exact production internal block

For the unit square's four corners, consecutive corners are face neighbors
with weight `1/3`, and the opposite corner is an edge neighbor with weight
`1/6`. The internal block of the production 18-point Laplacian is

\[
\Delta_{P,\mathrm{int}}=
\begin{pmatrix}
-4&1/3&1/6&1/3\\
1/3&-4&1/3&1/6\\
1/6&1/3&-4&1/3\\
1/3&1/6&1/3&-4
\end{pmatrix}.
\]

On the first harmonic word

\[
(q,r,-q,-r)^T,
\]

the two adjacent contributions cancel while the opposite contribution adds
`-1/6` to the self weight. Hence

\[
\Delta_{P,\mathrm{int}}(q,r,-q,-r)^T
=-{25\over6}(q,r,-q,-r)^T.
\]

With production `C_WAVE^2=1/3` and unit step, the isolated internal stiffness
is therefore

\[
\kappa={25\over18}.
\]

For either scalar doublet coordinate, the kick--drift matrix on `(q,p)` is

\[
M=
\begin{pmatrix}
1-\kappa&1\\-\kappa&1
\end{pmatrix}.
\]

It has

\[
\det M=1,
\qquad
\operatorname{tr}M=2-\kappa={11\over18}.
\]

Its elliptic eigenphase satisfies

\[
\cos\theta={\operatorname{tr}M\over2}={11\over36}.
\]

An order-four kick--drift orbit in this family requires `M^2=-I`, which
requires `kappa=2`; production gives `25/18`, not `2`.

More strongly, suppose `M^N=I` for some positive integer `N`. Its eigenvalues
would be roots of unity, so `theta/pi` would be rational. Because
`cos(theta)=11/36` is rational, the rational-cosine theorem would require

\[
\cos\theta\in\{0,\pm1/2,\pm1\}.
\]

But `11/36` is not in this set. Therefore the bare internal map has no exact
finite integer return. This is an exact exclusion, not a near-period search.

## 7. Embedded leakage witness

The internal eigenvector calculation does not make the four-site support an
invariant subspace. Put the `xy` word

\[
(q,0,-q,0)
\]

on a unit square and set every exterior site to zero. Consider the exterior
site immediately beyond the `+q` corner along a face direction. The `+q`
corner is its face neighbor, while the `-q` opposite corner is outside its
18-neighborhood. Its first Laplacian value is exactly

\[
\Delta_{18}J_{\mathrm{ext}}={q\over3}.
\]

Thus the first free-wave kick excites exterior support. The elementary
plaquette is open to its environment, and its local charge must exchange
torque with that environment. This explains the FTD-0915 production census:
the substrate readily forms the four-site clock face, but the face is neither
an isolated rotor nor a closed exact quarter-turn orbit.

## 8. Circular and inert branches of the isolated reference oscillator

For the selected isolated Hamiltonian

\[
H={1\over2}(\|p_q\|^2+\|p_r\|^2)
+{\kappa\over2}(\|q\|^2+\|r\|^2),
\]

the two circular branches are

\[
p_q=-\sigma\sqrt\kappa\,r,
\qquad
p_r=\sigma\sqrt\kappa\,q,
\qquad \sigma\in\{+1,-1\}.
\]

They have equal kinetic and potential energies,

\[
T=V={\kappa\over2}(\|q\|^2+\|r\|^2),
\]

and carry

\[
\mathcal L_P
=\sigma\sqrt\kappa(\|q\|^2+\|r\|^2).
\]

A standing/radial branch with `(p_q,p_r)=a(q,r)` has `L_P=0`. This makes the
special role of the earlier “inert” patterns precise: they may store energy,
but they do not carry net chiral circulation. Matter-like occupancy and
clockwise/counterclockwise action are distinct ledgers.

The circular branch is a reference solution conditional on isolation. It is
not claimed to arise spontaneously in the current production engine.

## 9. What `i` does and does not supply

The matrix

\[
R=\begin{pmatrix}0&-1\\1&0\end{pmatrix}
\]

is the real action of `i`. It supplies:

- a quarter-turn orientation operator;
- the clockwise/counterclockwise sign;
- the antisymmetric pairing defining `L_P`.

It does not supply:

- the stiffness `kappa`;
- a nonzero initial circulation charge;
- a confining or topological barrier;
- a coupling magnitude called `gamma`;
- a `G*` period; or
- Born frequencies.

Those are distinct dynamical or normalization questions. In particular,
`G*` may later calibrate a maintained clock's period only after a closed
oscillating carrier has been derived. Multiplying a stationary or leaky mode
by a period factor cannot create circulation.

## 10. Epistemic result and next admissible step

The preregistered outcome is:

```text
OUTCOME=A_NATIVE_OBSERVABLE_WITH_EMBEDDED_CONSERVATION_BOUNDARY
C4_CIRCULATION_CHARGE=EXACT_NATIVE_OBSERVABLE
ISOLATED_ISOTROPIC_CONSERVATION=EXACT_CONDITIONAL
EMBEDDED_ELEMENTARY_PLAQUETTE_INVARIANT=FALSE
BARE_INTERNAL_FINITE_INTEGER_RETURN=FALSE
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
GAMMA_DERIVED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

The next admissible task is not to impose rotation by hand. It is to determine
whether the full existing action admits one of:

1. a bounded invariant `C4` doublet larger than one plaquette;
2. a finite region whose boundary torque closes through an explicit source
   and return path;
3. a normal mode or defect whose circulating charge is protected against
   exterior leakage; or
4. a theorem-grade obstruction showing that additional selected dynamics is
   unavoidable.

Only after such a body exists can `G*` be tested as a gearbox ratio rather
than a numerical period normalization.

## 11. Certificate

- Locked protocol:
  `PREREG_NATIVE_PLAQUETTE_C4_CIRCULATION_AND_EMBEDDED_LEAKAGE_BOUNDARY_v1.md`
  (`SHA-256 2B2CA0D8D2696AFC529308D3B184ADADB87C9F88408194A53ADE42E9F4473157`)
- Exact certificate:
  `scripts/proofs/proof_native_plaquette_c4_circulation_embedded_leakage.py`
  (`SHA-256 9202930807042FF7460DE802AC1E793AE70638584C7FAA44E4E9E062EC58022F`)
- Result: `48/48` checks passed.

The certificate source-locks the production field operator, update phases,
field action, FTD-0876 canonical-carrier theorem, FTD-0914 plaquette theorem,
and FTD-0915 production report. It changes no engine source or CMake target.
