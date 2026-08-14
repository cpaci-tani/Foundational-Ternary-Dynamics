# FTD-0930 — Eight-color source-centered positive-port relaxation and massless-halo boundary v1

**Identifier:** `FTD-0930`  
**Date:** 2026-08-11  
**Status:** `[THEOREM — EXACT EIGHT-COLOR C18 SOURCE-CENTERED PORT DILATION]` +
`[THEOREM — POSITIVE STROBOSCOPIC FIELD-PORT ENERGY]` +
`[THEOREM — FINITE-GROUNDED DYNAMIC AND STATIC CONVERGENCE]` +
`[SCOPED MINIMUM — ONE COMPLETE PORT PAIR IN THE REGISTERED LOCAL CANONICAL CLASS]` +
`[BOUNDARY — UNCONTAINED MASSLESS HALO / PORT RECYCLING / AUTONOMOUS SCHEDULE / SOURCE RECOIL]`  
**Protocol:**
[`PREREG_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md),
SHA-256 `D4BD884513A39EA42F1DB216D2E359A83126BB49195457663A1AE0D2B336B54A`  
**Certificate:**
[`proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py`](../../../../../scripts/proofs/proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py),
SHA-256 `A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E`,
`183/183` exact checks  
**Registered outcome:** `B — POSITIVE LOCAL PORT RELAXATION / MASSLESS-HALO BOUNDARY`

---

## 1. Result

There is a simple positive local mechanism behind the FTD-0929 companion
preparation. It is not the expanding overwrite-history lift. It is an exact
source-centered quarter-turn between one active field residual and one fresh
complete canonical port.

For either the gapped dynamic companion operator

\[
 M_d=2I-K                                                   \tag{1}
\]

or the massless static operator

\[
 M_s=K,                                                     \tag{2}
\]

take a fixed source `b`, field `q`, and local residual

\[
 g=Mq+b.                                                    \tag{3}
\]

The parity triple of a cubic site supplies eight colors. No two C18-coupled
sites share a color, so every active color has diagonal self-block `dI`, with

\[
 d_d={2\over3},\qquad d_s={4\over3}.                       \tag{4}
\]

At every active cell normalize the residual as `u=g/sqrt(d)` and introduce a
fresh port amplitude `a`. The exact gate is

\[
 (u,a)\longmapsto(a,-u).                                   \tag{5}
\]

Equivalently,

\[
 q'=q+E_A{a-u\over\sqrt d}.                                \tag{6}
\]

On the fresh section `a=0`, equation (6) is ordinary local coordinate
relaxation:

\[
 q'=q-E_A{E_A^T(Mq+b)\over d}.                             \tag{7}
\]

It reads only the present local source and field. It does not read the
completed companion, halo, target arm, measurement setting, or desired
frequency.

The field potential lost in equation (7) appears exactly in the outgoing
port. With

\[
 \Phi(q)={1\over2}q^TMq+b^Tq,
 \qquad q_*=-M^{-1}b,
 \qquad \mathcal E(q)={1\over2}(q-q_*)^TM(q-q_*),          \tag{8}
\]

the gate obeys

\[
 \Phi(q')-\Phi(q)={1\over2}(\|a\|^2-\|u\|^2),             \tag{9}
\]

\[
 \boxed{
 \mathcal E(q')+{1\over2}\|a'\|^2
 =\mathcal E(q)+{1\over2}\|a\|^2.}                       \tag{10}
\]

The nonlocal-looking symbol `q_*` is used only to exhibit the positive lower
bound. The update itself computes from equation (3); it never reads `q_*`.

This closes a positive local formation **layer**. It does not yet close the
open environment that must supply fresh ports indefinitely.

---

## 2. Why eight colors are exact

The scalar C18 stiffness is

\[
 (Kq)_x={4\over3}q_x
 -{1\over9}\sum_{y\sim_f x}q_y
 -{1\over18}\sum_{y\sim_e x}q_y.                         \tag{11}
\]

Color each site by

\[
 \chi(x)=(x_1\bmod2,x_2\bmod2,x_3\bmod2).                \tag{12}
\]

A face step changes one parity bit and an edge step changes two. Therefore
every C18 edge joins different colors. For a fixed color `A`,

\[
 E_A^TME_A=dI.                                             \tag{13}
\]

All active residual gates are consequently independent and commute within
one color. The certificate verifies equation (13) for all eight colors for
both operators on the exact zero-extended `3x3x3` witness.

The eight-color schedule is a graph-coloring theorem, not a derived clock.
The order

`000,001,010,011,100,101,110,111`

is selected for the reference construction. An autonomous mechanism that
generates and maintains that order remains open.

---

## 3. Exact canonical and positive Hamiltonian layer

Group one active coordinate against all inactive coordinates and write

\[
 M=\begin{pmatrix}d&c^T\\c&R\end{pmatrix}.
\]

On source-centered deviations and the port coordinate, equation (5) has
matrix

\[
 S=\begin{pmatrix}
 0&-c^T/d&1/\sqrt d\\
 0&I&0\\
 -\sqrt d&-c^T/\sqrt d&0
 \end{pmatrix}.                                           \tag{14}
\]

For `G=diag(M,1)`, exact algebra gives

\[
 S^4=I,
 \qquad
 S^TGS=G.                                                  \tag{15}
\]

The cotangent lift `diag(S,S^{-T})` is symplectic and preserves the positive
phase metric `diag(G,G^{-1})` whenever `M` is positive definite.

More importantly for locality, the gate has the already established
FTD-0886 local positive Hamiltonian interpolation. For active cells define

\[
 u_i={(Mq+b)_i\over\sqrt d},
 \qquad
 \pi_{u_i}={p_i\over\sqrt d}.                              \tag{16}
\]

Equation (13) gives

\[
 \{u_i,\pi_{u_j}\}=\delta_{ij}.                            \tag{17}
\]

With one complete port pair `(a_i,pi_{a_i})`, set

\[
 N={1\over2}\sum_i
 (u_i^2+a_i^2+\pi_{u_i}^2+\pi_{a_i}^2),
 \qquad
 L=\sum_i(a_i\pi_{u_i}-u_i\pi_{a_i}).                     \tag{18}
\]

Then

\[
 \{N,L\}=0,
 \qquad |L|\le N.                                         \tag{19}
\]

The selected clocked Hamiltonian

\[
 H=\omega I+\omega N
 +\sigma{\omega\over4}(1-\cos\theta)L                    \tag{20}
\]

has carrier bound

\[
 H-\omega I\ge{\omega\over2}N\ge0.                       \tag{21}
\]

One reference-clock cycle produces equation (5) exactly on the
zero-conjugate section. Thus the layer is local, symplectic, positive, fourth
order, and exactly reversible.

This is a stroboscopic Hamiltonian layer. No single autonomous local
Hamiltonian has yet been shown to generate the entire eight-color schedule,
the source matter, its recoil, the port rail, and the stopping rule.

---

## 4. Scoped minimum

Fresh relaxation without a port maps every active input to the same local
minimizer. It is noninjective, while every symplectic map is bijective. At
least one outgoing real coordinate is therefore required to retain the
removed residual.

In the registered class, local phase space is a direct sum of nondegenerate
onsite canonical fibers. A single real coordinate cannot carry a
nondegenerate skew form, so the outgoing coordinate requires its conjugate.
One complete canonical pair is necessary in this class. Equation (5) proves
that one pair is sufficient.

Therefore:

\[
 \boxed{
 \text{one complete fresh port pair per active cell is minimum
 within the registered onsite canonical class}.}          \tag{22}
\]

No universal dimension theorem outside that declared class is claimed.

---

## 5. Finite-grounded convergence

On centered error `e=q-q_*`, a fresh layer of color `A` is

\[
 P_A=I-E_A d^{-1}E_A^TM.                                  \tag{23}
\]

Equations (13) and (23) imply

\[
 P_A^2=P_A,
 \qquad
 P_A^TM=MP_A,                                              \tag{24}
\]

and the exact energy drop

\[
 \mathcal E(e)-\mathcal E(P_Ae)
 ={1\over2d}\|E_A^TMe\|^2.                               \tag{25}
\]

Thus each color is an `M`-orthogonal projection. A full sweep is

\[
 P=P_{111}\cdots P_{001}P_{000}.                          \tag{26}
\]

Every factor is nonexpansive in the `M` norm. If a complete sweep preserved
the norm of a nonzero vector, equation (25) would force the residual to
vanish on all eight colors. Then `Me=0`, contradicting positive definiteness.
Compactness of the finite-dimensional unit `M` sphere therefore gives, for
every specified finite grounded region,

\[
 \|Pe\|_M\le\rho_{\Lambda,M}\|e\|_M,
 \qquad 0\le\rho_{\Lambda,M}<1.                           \tag{27}
\]

For the dynamic operator, positivity follows from

\[
 {2\over9}I\le M_d\le2I.                                  \tag{28}
\]

For the static operator on a finite region with zero exterior extension,

\[
 q^TKq=
 \sum_{\{x,y\}_f}{1\over9}(q_x-q_y)^2
 +\sum_{\{x,y\}_e}{1\over18}(q_x-q_y)^2.                 \tag{29}
\]

Equality would make the zero-extended finite field constant on the connected
C18 graph, so the field must be zero. Hence the grounded static compression
is also positive definite.

The certificate verifies every projection identity, all-color residual rank,
absence of a nonzero full-sweep fixed vector, and strict rational-witness
energy decrease for both operators. It uses exact rational and modular-minor
certificates, not floating eigenvalues or fitted rates.

Consequently the same local port gate computes:

- the finite grounded approximation to the FTD-0929 dynamic companion; and
- the finite grounded solution of the static massless field equation.

---

## 6. Causality and the massless boundary

Each color layer reads one C18 neighborhood. Starting from compact source
data and a zero field, `t` color layers have dependency radius at most `t`.
The construction is therefore causal and target-blind. A finite-depth output
remains finitely supported.

The static conclusion must not be overextended. Along the exact Fourier line
`(e^{i theta},1,1)`,

\[
 \kappa(\theta)={2\over3}(1-\cos\theta)\longrightarrow0.  \tag{30}
\]

Thus the finite-region factor in equation (27) cannot be replaced by one
strict geometric factor that is uniform over uncontained scales. The
massless long-wavelength modes approach zero stiffness.

What is established is:

- exact causal local relaxation;
- strict convergence for every specified finite grounded solve; and
- no volume-independent geometric-rate promotion for the uncontained static
  problem.

What remains open is:

- local convergence to a boundary-independent uncontained Green profile;
- the correct function/energy class for that profile;
- persistence and recovery under moving sources;
- physical halo formation without a grounded computational boundary; and
- any gravitational or dark-matter identification.

No `L to infinity` statement is used.

---

## 7. The reservoir cost

After one fresh gate,

\[
 a'=-u.                                                    \tag{31}
\]

The used port is generically nonzero. It carries the old residual's sign,
amplitude, phase-complete state, and positive energy. Reusing it as a blank
port would silently erase history.

Every site is active once per complete eight-color sweep. Therefore one
sweep consumes one fresh complete pair per site, and `N` sweeps consume `N`
pairs per site unless a separate reset, return, compression, or open-rail law
is supplied.

FTD-0875 gives a positive local rail that can transport a prepared record.
FTD-0884 proves that a finite cyclic bank cannot guarantee indefinite generic
freshness. A bilateral or outward open rail with a prepared blank future is
therefore a valid reference environment, but it is not yet substrate-native
formation hardware.

The unresolved physical questions are concrete:

1. where blank complete ports come from;
2. how used ports route through three dimensions without congestion or
   backpressure;
3. whether outgoing records disperse, thermalize, return, or recur;
4. how a moving source pays work and receives recoil;
5. what autonomously advances the eight-color schedule and stops it; and
6. whether the production left/right fields realize the field/port split or
   merely have enough storage capacity.

This is the exact boundary between a positive local recursive system and a
complete self-maintaining one.

---

## 8. Interpretation

FTD-0929's local cotangent history lift showed how reversal could retain an
overwritten field, but its expanding mode made it unsuitable as positive
formation dynamics. FTD-0930 replaces that overwrite with a rotation:

\[
 \text{field residual}
 \quad\longleftrightarrow\quad
 \text{environment port}.                                \tag{32}
\]

That is the simplest rigorous version of a self-dual energy mechanism found
so far. The two sides are not identical records. They are reciprocal roles:
one is the presently relaxed field coordinate; the other is the outgoing
signed residual and its conjugate response. Their common quadratic form is
positive, and the quarter-turn exchanges the roles without deletion.

If the outgoing port becomes inaccessible, the reduced field description is
lossy: locally irrelevant detail has been unactualized from the reduced
record while remaining in the larger environment. Fundamental erasure has
not been proved or assumed.

The construction uses an imposed harmonic reference cycle. `G*` does not
appear in the equations or certificate. A future `G*` gearbox would have to
explain why the quartic calendar supplies the gate cadence without changing
the positive field-port algebra or encoding the desired field profile.

---

## 9. Verification and boundary statement

The byte-frozen protocol has SHA-256
`D4BD884513A39EA42F1DB216D2E359A83126BB49195457663A1AE0D2B336B54A`.
The exact certificate has SHA-256
`A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E`
and passes `183/183` checks.

The registered verdict is Outcome B:

```text
OUTCOME=B_POSITIVE_LOCAL_PORT_RELAXATION_MASSLESS_HALO_BOUNDARY
C18_COLOR_COUNT=8
LOCAL_GATE=SOURCE_CENTERED_CANONICAL_QUARTER_TURN
LOCAL_HAMILTONIAN=POSITIVE_CLOCKED_REFERENCE_LAYER
FIELD_PLUS_PORT_ENERGY=EXACTLY_CONSERVED
FRESH_PORT_MINIMUM=ONE_COMPLETE_PAIR_WITHIN_REGISTERED_LOCAL_CLASS
DYNAMIC_FINITE_GROUNDED_CONVERGENCE=YES
STATIC_FINITE_GROUNDED_CONVERGENCE=YES
UNCONTAINED_STATIC_UNIFORM_GEOMETRIC_RATE=NO
UNCONTAINED_STATIC_HALO_FORMATION=OPEN
INDEFINITE_PORT_RECYCLING=OPEN
AUTONOMOUS_EIGHT_COLOR_CLOCK=OPEN
SOURCE_FORMATION_RECOIL=OPEN
EXISTING_DUAL_FIELD_IDENTIFICATION=OPEN
PRODUCTION_CHANGED=FALSE
GSTAR_USED=FALSE
BORN_BELL_CONTEXT_USED=FALSE
```

No engine source, CMake target, `Voxel` field, toggle, production law,
ontology type, paper, physical constant, or phenomenological formula changed.
No numerical search, fit, near-miss, formula-substitution discovery, or
uncontained-limit promotion was performed.
