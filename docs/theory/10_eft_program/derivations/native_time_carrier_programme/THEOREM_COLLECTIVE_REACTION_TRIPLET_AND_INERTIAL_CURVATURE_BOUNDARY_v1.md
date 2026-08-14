# Theorem — Collective reaction triplet and inertial-curvature boundary v1

**Identifier:** `FTD-0891` / repaired execution `FTD-0892`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — COLLECTIVE REACTION TRIPLET IS AN EXACT SYMPLECTIC SECTOR]` +
`[CONDITIONAL THEOREM — EXACT COMPOSITE DISPERSION AND INERTIAL ADDITIVITY]` +
`[CLOSED NEGATIVE — STATIC STABILITY/HESSIAN/REST OFFSET DO NOT FIX MASS]` +
`[BOUNDARY — TOTAL FIELD-MATTER NOETHER MOMENTUM REMAINS OPEN]` +
`[SELECTION — CONSTITUENT PHASE SPACE AND RELATIVISTIC DISPERSION]` +
`[IMPOSED — CONSTITUENT REST ENERGIES, c, BINDING OFFSET, AND INITIAL DATA]` +
`[REFERENCE IMPLEMENTATION — ISOLATED ftd::eft]` +
`[OPEN — ABSOLUTE MASS SCALE, CONSTITUENT FORMATION, STABLE POLE, PRODUCTION]`

## 1. Verdict

The three canonical pairs required by FTD-0890 are not an additional type
once the selected constituent-complete common action is admitted. For any
number of constituent canonical pairs, an exact orthogonal symplectic
reduction separates the center coordinate and summed momentum,

\[
X=\frac1N\sum_a x_a,\qquad P=\sum_a p_a,                 \tag{1}
\]

from internal coordinates. The pair `(X,P)` is precisely an
orientation-free spatial reaction triplet: three canonical pairs transforming
covariantly under the cubic group. Internal pair impulses cancel from `P`, and
the summed external constituent impulse changes `P` by exactly that sum.

This is a kinematic closure inside the selected common-action phase space. It
does not generate constituents from the ternary substrate, derive the stored
bond graph, or turn the scalar Gauss reaction channel into a vector.

Conditional on the selected relativistic constituent dispersion, the
minimum-energy composite at fixed total momentum again has an exact
relativistic dispersion. Its inertial mass is additive. This does not derive
the absolute mass scale: the constituent rest energies and limiting speed are
inputs. More strongly, no static stable configuration, static Hessian, or
additive rest-energy offset can determine that kinetic curvature by itself.

Finally, the collective `P` is an exact canonical matter variable, but it is
not thereby an exact total field-plus-matter Noether momentum. The current
lattice has exact integer translations and positive Peierls curvature, not a
continuous translation symmetry. The exact total physical momentum law
therefore remains open.

## 2. Exact collective symplectic sector

Let the selected constituent phase space carry

\[
\Theta=\sum_{a=1}^N p_a\cdot dx_a,\qquad
\Omega=-d\Theta.                                         \tag{2}
\]

Choose an orthogonal Helmert matrix `U` whose first row is
`U_{0a}=1/sqrt(N)`. One explicit completion is

\[
U_{ka}=\begin{cases}
[k(k+1)]^{-1/2},&a<k,\\
-k[k(k+1)]^{-1/2},&a=k,\\
0,&a>k,
\end{cases}
\qquad k=1,\ldots,N-1.                                  \tag{3}
\]

Define

\[
q_\mu=\sum_a U_{\mu a}x_a,\qquad
\pi_\mu=\sum_a U_{\mu a}p_a.                           \tag{4}
\]

Since `UU^T=I`, applying the same matrix to positions and momenta is
symplectic and invertible. The canonical one-form becomes

\[
\sum_a p_a\cdot dx_a
=\sum_\mu\pi_\mu\cdot dq_\mu
=P\cdot dX+\sum_{\mu=1}^{N-1}\pi_\mu\cdot dq_\mu,       \tag{5}
\]

because `q_0=sqrt(N)X` and `pi_0=P/sqrt(N)`. Equation (5)
proves the exact split; it is not an approximation or a low-momentum limit.

Signed spatial permutations act on every `x_a,p_a` in the same way and commute
with the constituent-index transform `U`. Hence `(X,P)` transforms as a cubic
vector/covector pair and carries

\[
\Omega_{\rm coll}=\sum_{i=1}^3 dX_i\wedge dP_i.          \tag{6}
\]

This identifies the minimum carrier of FTD-0890 without adding a new selected
vector type. The selection cost was paid earlier when independent constituent
canonical positions, momenta, and relational memory were adopted.

## 3. Exact impulse algebra

For simultaneous constituent impulses `Delta p_a`, equation (1) gives

\[
\Delta P=\sum_a\Delta p_a.                              \tag{7}
\]

Every internal pair interaction contributes equal and opposite impulses, so
its contribution to (7) vanishes. An external or field-mediated set of
constituent impulses contributes its exact sum. This is enough to use `P` as
the reaction triplet inside the selected matter phase space.

Equation (7) is not yet a derivation of the field impulse or of an exact
field-plus-matter Noether charge. It states what the matter collective variable
does once constituent impulses are supplied by the selected transaction.

## 4. Conditional composite dispersion

Adopt, rather than derive, the per-constituent energies

\[
E_a(p_a)=\sqrt{\epsilon_a^2+c^2|p_a|^2},
\qquad \epsilon_a>0,\quad c>0.                          \tag{8}
\]

Each energy is strictly convex. Minimize `sum_a E_a` subject to
`sum_a p_a=P`. The Lagrange equations require a common velocity,

\[
\frac{c^2p_a}{E_a(p_a)}=v,                              \tag{9}
\]

and strict convexity makes the solution unique. With
`Epsilon=sum_a epsilon_a`, the solution is

\[
p_a=\frac{\epsilon_a}{\mathcal E}P.                    \tag{10}
\]

Substitution gives

\[
E_a=\frac{\epsilon_a}{\mathcal E}
\sqrt{\mathcal E^2+c^2|P|^2},                           \tag{11}
\]

and therefore

\[
\boxed{E_{\rm coll}(P)=
\sqrt{\mathcal E^2+c^2|P|^2}}.                          \tag{12}
\]

The zero-momentum curvature is

\[
\left.\frac{\partial^2E_{\rm coll}}
{\partial P_i\partial P_j}\right|_{P=0}
=\frac{c^2}{\mathcal E}\delta_{ij}.                    \tag{13}
\]

Thus, conditionally,

\[
\boxed{M_{\rm coll}=\frac{\mathcal E}{c^2}
=\sum_a\frac{\epsilon_a}{c^2}}.                        \tag{14}
\]

For `N` identical constituents this is `N epsilon/c^2`. Equation (14) is an
exact composition theorem under (8); it is not a prediction of `epsilon_a`,
`c`, or a physical particle mass.

## 5. Binding-energy participation criterion

Suppose a static binding or field contribution `U_0` is added to the rest
energy but is held fixed while only the constituent momenta participate in the
boosted family. Then

\[
E_{\rm rest}=\mathcal E+U_0,
\qquad M_{\rm inertial}=\frac{\mathcal E}{c^2},          \tag{15}
\]

so

\[
\frac{E_{\rm rest}}{c^2}-M_{\rm inertial}
=\frac{U_0}{c^2}.                                       \tag{16}
\]

Unless `U_0=0`, a static offset cannot be counted as inertia merely because it
appears in the rest ledger. To recover mass-energy equivalence for the whole
dressed object, the binding and field dressing must participate dynamically in
the common boosted family. That is a concrete future acceptance condition,
not a bookkeeping convention.

## 6. Static-data mass no-go

Let `V(q)` have a stable minimum `q_*` and positive static Hessian `K`. For any
positive definite kinetic matrix `M`,

\[
H_M(q,p)=\frac12p^TM^{-1}p+V(q)                         \tag{17}
\]

has the same rest configuration and the same static Hessian. Its momentum
curvature is `M^{-1}`, however, and its linear mode matrix is `M^{-1}K`.
Changing `M` changes inertia and frequencies without changing any static datum.

Likewise, infinitely many even strictly convex functions share a prescribed
value `E(0)=E_0` while having different Hessians at zero; for example

\[
E_a(p)=E_0+a|p|^2,\qquad a>0.                           \tag{18}
\]

Therefore the following inference is closed negative in the registered class:

```text
stable rest shape + positive static Hessian + rest-energy value
    => unique inertial mass.
```

The existing selected stable block and its positive Hessian establish local
rest stability. They cannot establish the kinetic normalization that the same
analysis currently inputs through `M_INERTIAL` or
`constituent_mass_scale`.

## 7. Discrete-translation Noether boundary

The microscopic lattice is exactly invariant under integer translations
`Z^3`. It is not exactly invariant under arbitrary continuous translations.
The selected compact composites also have positive Peierls curvature; their
translation-like modes are soft positive modes, not exact zero modes.

Consequently:

1. `P=sum_a p_a` is an exact canonical collective matter coordinate;
2. the selected transaction may exchange vector impulses and close energy;
3. neither statement produces the continuous symmetry required for a standard
   additive `R^3` Noether momentum;
4. Bloch/quasimomentum from `Z^3` is defined modulo reciprocal lattice vectors
   and is not automatically the local additive recoil observable;
5. the existing spline field momentum remains a diagnostic with a measured
   translation defect, not an exact coupled charge.

An exact total physical momentum law needs an additional closure: a
continuously translation-invariant interpolated action, an operationally
identified integer-hop/quasimomentum ledger, or an explicit lattice-stress
reservoir. No option is selected here.

## 8. Certificates and implementation

The first locked FTD-0891 execution is preserved as execution-invalid at
`62/68`. Every substantive mathematical gate passed, but five predicates were
representation-sensitive and the terminal check failed dependently. FTD-0892
froze exactly those verifier normalizations, preserved both parent hashes, and
passed the inherited `68/68` certificate.

| artifact | result |
|---|---|
| parent protocol | SHA256 `D273F1A61E1A55B26781116E3B9D3984DAFF843DB04F18E160C706EBEAC6C595` |
| invalid parent verifier | SHA256 `ED729418595D0B6B0F69F9381CB5DF007DF764E79CDF9145DF15DA9C4B6104FE` |
| repair protocol | SHA256 `3036B665B6C8120D13D33A18A25CF8FDA71BA63ADB2A999C9ABC385DD928366B` |
| repair wrapper | SHA256 `33FA6C6760087AB046AD6B08BB065569158B4B6E9CFF53C7494A836FE04D0A46` |
| inherited exact certificate | `68/68 PASS` |
| isolated header | SHA256 `20D90A9F78CCA724DB8A59834274368C92CC7E11082D603D76BD22D95443FE50` |
| isolated source | SHA256 `3D91C5F451207EBAF1249EEDE37B96AFC20DC78CFDA2615B72ABEB6C54C6E5EE` |
| isolated test | SHA256 `B27D279CCDA7266853D167CEF4C21A96C76CE5876A0856227C76A4E4D47EABF0` |
| focused CTest | `1/1 PASS` |
| isolated actualization chain | `22/22 PASS` |

The implementation lives only in `ftd::eft`. It evaluates the Helmert
reduction, reconstruction, one-form split, impulse sum, conditional composite
dispersion, common-velocity allocation, curvature, and binding-offset mismatch.
Its public result flags explicitly deny an absolute mass derivation, exact
total field-matter Noether momentum, constituent formation, stable-pole
derivation, production coupling, Born-target use, and native `G*`
synchronization.

## 9. What is now closed and what remains open

Closed:

- the required vector triplet is already the exact collective sector of the
  selected constituent phase space;
- no additional selected vector type is required at that level;
- internal impulse cancellation and external impulse summation are exact;
- the selected relativistic constituent dispersion composes exactly;
- inertial mass is conditionally additive;
- static stability, a static Hessian, and a rest-energy offset do not determine
  absolute inertial curvature.

Open:

- substrate formation of constituent canonical phase space and relational
  memory;
- dynamical boost participation of binding and field dressing;
- an exact total field-plus-matter momentum law on the discrete substrate;
- absolute mass-scale selection;
- a stable propagating matter pole;
- production integration, operational Lorentz hiding, Born recovery, and the
  physical `G*` clock gearbox.

The next honest discriminator is therefore not another static normal-mode
calculation. It is a dynamical dressed-boost test: construct a family of moving
solutions in which matter, binding, and field dressing all participate, then
measure whether the same curvature controls total energy, total recoil, and
long-time translation without target-coded mass input.
