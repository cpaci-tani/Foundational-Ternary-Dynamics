# Minimal many-body matter network v1

**Status:** `[EXACT ALGEBRA FOR A CANDIDATE EXTENSION] + [BLOCKING
COUNTEREXAMPLE] + [SELECTED NO-NEW-PRIMITIVE PATH; NOT IMPLEMENTED]`  
**Evidence boundary:** frozen reciprocal constituent/common-action sector
through the execution-invalid FTD-0768 result  
**Production status:** unchanged  
**Identifier:** none; this document may not consume or rewrite FTD-0768

## 1. Question

The certified object candidate is presently one opposite-polarity reciprocal
pair. Matter in the ordinary sense requires more:

1. a finite number of constituents must compose without double-counting field
   energy;
2. the total energy must be extensive rather than falling as `-N^2`;
3. a local bulk and surface must be identifiable from the state;
4. deformation and breakup must be common-action transactions;
5. distant objects must factorize; and
6. constituent turnover must be distinguishable from loss of object identity.

This document derives the minimal algebraic extension and then shows why its
naive form fails.

## 2. Minimal pairwise candidate

Let constituent `a` carry position `x_a`, momentum `p_a`, and polarity
`sigma_a in {-1,+1}`. Define

\[
A_{ab}=\frac{1-\sigma_a\sigma_b}{2},
\qquad
q_{ab}=\lVert x_a-x_b\rVert^2.
\tag{1}
\]

`A_ab` is one only for opposite polarities. The literal pairwise continuation
of the selected compact law is

\[
U_N(X)=\sum_{a<b}A_{ab}V(q_{ab}),
\tag{2}
\]

where

\[
V(q)=
\begin{cases}
-16\epsilon(q-3/2)^2(q-3/4),&q<3/2,\\
0,&q\ge3/2.
\end{cases}
\tag{3}
\]

Equation (2) is the unique continuation only after **pairwise additivity** is
selected. Pairwise additivity is not forced by the two-body result. Compact
three-body functions can vanish identically for `N=2` while preserving all
two-body data. The status of (2) is therefore minimal selected candidate, not
theorem of the ontology.

## 3. One exact `N`-body common-action impulse exists algebraically

For one step, write

\[
d_{ab}^{0}=x_a^0-x_b^0,
\qquad d_{ab}^{1}=x_a^1-x_b^1
\tag{4}
\]

and define the divided difference

\[
D_{ab}V=
\begin{cases}
\dfrac{V(q_{ab}^1)-V(q_{ab}^0)}{q_{ab}^1-q_{ab}^0},
  &q_{ab}^1\ne q_{ab}^0,\\[6pt]
V'\!\left((q_{ab}^0+q_{ab}^1)/2\right),&
  q_{ab}^1=q_{ab}^0.
\end{cases}
\tag{5}
\]

The binding impulse on constituent `a` is

\[
I_a^{\rm bind}=-\Delta t\sum_{b\ne a}
A_{ab}(d_{ab}^{0}+d_{ab}^{1})D_{ab}V.
\tag{6}
\]

Every pair contributes equal and opposite impulses, so

\[
\sum_a I_a^{\rm bind}=0.
\tag{7}
\]

If the same midpoint kinematics used by the current common action gives
`x_a^1-x_a^0=Delta t vbar_a`, then

\[
\begin{aligned}
\sum_a \bar v_a\mathbin\cdot I_a^{\rm bind}
&=-\sum_{a<b}A_{ab}D_{ab}V
 (d_{ab}^{1}-d_{ab}^{0})\mathbin\cdot
 (d_{ab}^{1}+d_{ab}^{0})\\
&=-\sum_{a<b}A_{ab}
  \big(V(q_{ab}^{1})-V(q_{ab}^{0})\big)\\
&=-\Delta U_N.
\end{aligned}
\tag{8}
\]

Thus pairwise binding, matter momentum, and work can coexist exactly. Summing
the already derived straight-segment face currents over constituents also
preserves exact discrete continuity. An implicit root over all endpoint
momenta could therefore couple (6) to the existing face/edge field transaction
without adding a dynamical variable.

This proves algebraic compatibility only. It does not prove existence,
uniqueness, causal admissibility, regularity, or efficient solution of the
`3N`-dimensional endpoint root.

## 4. Symmetries and topology of the minimal candidate

Equations (1)--(6) are invariant under:

- permutation of same-polarity constituent labels;
- simultaneous polarity conjugation;
- integer translation;
- proper cubic rotations; and
- separation of clusters farther apart than the compact cutoff.

Because every active edge joins opposite polarities, the derived interaction
graph is bipartite. It has no odd cycle. Nearest-neighbour SC and BCC graphs
are abstractly bipartite; nearest-neighbour FCC contains triangles and is not.
This does not derive SC or BCC. It does prove that a literal reciprocal-only
FCC nearest-neighbour bond graph is unavailable.

With constituents pinned to integer cubic sites, the cutoff `q<3/2` selects
only the six face neighbours at squared distance one. In that stricter sector,
the quiet network is naturally SC and has coordination six. BCC requires a
subcell embedding or a different relational layer; FCC requires more than the
present opposite-polarity edge.

## 5. Blocking counterexample: the naive all-pairs law collapses

Take `m` positive constituents at `x_+` and `m` negative constituents at
`x_-`, with

\[
\lVert x_+-x_-\rVert=1.
\tag{9}
\]

There are `m^2` opposite-polarity pairs, all at the potential minimum. Hence

\[
U_N=-\epsilon m^2=-\frac{\epsilon N^2}{4},
\qquad N=2m.
\tag{10}
\]

The present constituent variables have no hard same-polarity exclusion. Even
if exact coincidence were prohibited, arbitrarily tight same-polarity packets
approach (10). The binding energy per constituent therefore has no finite
large-`N` limit.

The linear Gauss field does not automatically repair this. If the unit dipole
field is `F_1`, the stacked density is `m` times the unit density, its selected
linear field is `m F_1`, and its quadratic field energy is

\[
E_F(m)=m^2 E_F(1).
\tag{11}
\]

The complete quadratic coefficient is consequently

\[
E_{\rm bind+field}(m)=m^2\big(E_F(1)-\epsilon\big).
\tag{12}
\]

This gives a trilemma: a positive coefficient rejects the stacked channel, a
negative coefficient collapses superextensively, and exact equality leaves a
flat multiplicity direction. None by itself produces an extensive bulk.

The certified FTD-0739 bound control shows that the negative branch is not
hypothetical at its frozen preparation. At tick zero it records

```text
pair energy  = -0.0095599710745429192
field energy = +0.00056112859596711728
sum          = -0.008998842478575802
```

The selected well depth is `epsilon=0.01`; the field coefficient is much
smaller. Stacking the same source/field shape would therefore retain a negative
`m^2` coefficient. The registered two-body action is sound, but an unrestricted
all-pairs promotion is not a material thermodynamic limit.

## 6. What can block collapse without inventing a particle

At least one saturation mechanism is required:

1. **ternary capacity:** derive a local unsigned occupancy from the existing
   constituent shapes and forbid capacity above one;
2. **bounded valence:** allow only finitely many active bonds per constituent;
3. **irreducible many-body repulsion:** add a state-local term that grows with
   crowding;
4. **nonlinear field response:** replace the present linear/quadratic scaling
   by a derived saturating field mechanism; or
5. **new constituent type:** add explicit exclusion/internal phase-space
   structure and price it as a new primitive.

Options 2--5 are not presently derived. The least ontologically expensive
candidate is option 1 because the primitive ternary state already has bounded
site magnitude.

## 7. Ternary-capacity candidate

Let `W_a(v)>=0` be the existing normalized compact shape associated with
constituent `a`, so `sum_v W_a(v)=1`. Define the unsigned occupancy sidecar

\[
n(v;X)=\sum_a W_a(v;X).
\tag{13}
\]

The capacity region is

\[
0\le n(v;X)\le1\qquad\text{for every site }v.
\tag{14}
\]

This does not identify a constituent with one voxel. A constituent remains a
relational worldline whose coupling support may occupy several sites. Equation
(14) says only that multiple coupling shapes may not demand more unsigned
manifestation capacity than a ternary site possesses.

The constraint is state-only, local, translation-covariant, cubic-covariant,
and polarity-blind. It also blocks opposite-polarity cancellation from hiding
over-occupation because it uses unsigned rather than net density.

A no-new-state implementation could introduce per-step solver multipliers
`lambda_v` satisfying

\[
\lambda_v\ge0,
\qquad 1-n(v)\ge0,
\qquad \lambda_v(1-n(v))=0.
\tag{15}
\]

The multipliers would be algebraic transaction outputs, not stored ontic
variables. Their impulses must come from the same discrete interaction
functional and do zero net work except for exactly accounted elastic/contact
energy.

### Capacity proves a linear lower bound

The capacity proposal already closes the thermodynamic-scaling defect at the
kinematic level. For the tensor quadratic B-spline coat, choose any nearest
lattice site `y_a` to constituent `a`. Each coordinate separation is at most
`1/2`, and the one-dimensional kernel obeys `b_2(u)>=1/2` there. Hence

\[
W_a(y_a)\ge(1/2)^3=1/8.
\tag{16}
\]

Equation (14) permits at most eight constituents to choose the same nearest
site. If constituent `b` is inside the interaction radius of `a`, then for
each coordinate

\[
|y_{b,i}-x_{a,i}|<1/2+\sqrt{3/2}<1.725.
\tag{17}
\]

An interval of this radius contains at most four integers per coordinate, so
there are at most `4^3=64` possible nearest-site classes for interacting
constituents. Each class contains at most eight constituents. Therefore every
vertex has degree at most 511, excluding itself.

Because the compact potential satisfies `V(q)>=-epsilon`, the capacity-limited
binding energy obeys the extensive lower bound

\[
U_N\ge-\epsilon|E|
    \ge-\frac{511}{2}\epsilon N.
\tag{18}
\]

Kinetic and field energies are nonnegative, so the same lower bound applies to
the declared rest-subtracted complete energy. The constant 511 is deliberately
coarse; site-aligned SC has degree six. Its importance is logical: the
existing shape plus unsigned capacity is sufficient to replace the `-N^2`
instability by a linear stability bound without a stored bond label or new
dynamical coordinate.

This route is not yet constructive. The hard questions are uniqueness at
simultaneous contacts, exact inversion when constraints activate or release,
and compatibility with the earlier chart/hop no-go results. A chart-dependent
“one anchor each” rule is insufficient.

## 8. Conditional bulk and surface algebra after saturation

If a saturation rule bounds coordination and a quiet network places every
active bond at `r=1`, then

\[
U=-\epsilon|E|=-\frac{\epsilon}{2}\sum_i z_i.
\tag{19}
\]

For a bulk reference coordination `z_bulk`, the missing-bond excess is

\[
\Delta U_{\rm surface}=
\frac{\epsilon}{2}\sum_i(z_{\rm bulk}-z_i).
\tag{20}
\]

The compact potential also gives longitudinal bond stiffness
`k_bond=96 epsilon`. These are conditional microscopic elastic data. A
physical surface tension or elastic modulus still requires field energy,
geometry relaxation, angular modes, environmental pressure, and a volume
limit.

For site-aligned SC with `z_bulk=6`, the quiet binding energy is `-3 epsilon`
per bulk constituent. A flat cleavage breaks bonds and produces coordination
deficits on two surfaces. This is the first mathematically controlled sense in
which a material surface could emerge from the relational graph rather than
being a primitive membrane.

## 9. Three distinct kinds of matter identity

The many-body extension forces a distinction that the pair alone cannot show:

```text
molecular identity = approximately the same constituent set and bond graph,
body identity      = one connected material graph with limited turnover,
process identity   = persistent local organization despite constituent flow.
```

A solid candidate requires long edge autocorrelation and nonzero relaxed shear
response. A liquid candidate retains local coordination but loses long-time
edge identity and static shear response. A flame/cloud candidate may retain a
localized reaction/constraint pattern while constituents and energy cross its
surface. A gas/plasma candidate has only short-lived components or positive
dissociation margins.

Useful state/history observables are therefore:

\[
f_b=\frac{2|E|}{N z_{\rm ref}},
\qquad
C_E(\tau)=\frac{|E_t\cap E_{t+\tau}|}{|E_t\cup E_{t+\tau}|},
\tag{21}
\]

together with relative constituent diffusion, capacity-contact fraction,
energy-barrier crossings, and the relaxed second derivative of energy under a
frozen shear deformation. No single one of these is “matter”; their conjunction
separates persistence, rigidity, flow, and breakup.

## 10. Interaction with the FTD-0768 field/object gate

FTD-0768 must finish before a many-body campaign is frozen. If its cleared lab
response decays, the many-body object should not annex the residual field; its
identity is the saturated relational graph plus state-selected constraint
data. If a cleared response persists, that response remains environmental
memory until a separate composability test decides whether it follows one
graph, remains in the laboratory, or propagates away.

The moving radius-eight mask is an observer chart in either outcome. It cannot
serve as the capacity surface or material membrane.

## 11. Required gates for the first `N`-constituent campaign

Before execution, a successor must freeze:

1. whether pairwise additivity is selected or a many-body term is admitted;
2. the exact unsigned-capacity functional and its derivation from the existing
   shape;
3. the complementarity/contact solve and state-only inverse map;
4. admissible constituent counts and neutral polarity assignments;
5. SC, BCC, translated, rotated, and deliberately collapsing controls;
6. exact energy, current, Gauss, momentum/stress, and locality ledgers;
7. graph connectivity, edge turnover, and factorization estimators;
8. failure outcomes for non-extensivity, root nonuniqueness, and contact
   irreversibility; and
9. an explicit prohibition on calling an imposed initial graph an emergent
   material phase.

No campaign should begin by injecting an FCC cluster. The reciprocal-only
graph obstruction must first be resolved or accepted.

## 12. Recursive intuitive questions

1. Is “one unit per ternary site” a true exclusion principle for distributed
   constituent shapes, or only a property of primitive `s` snapshots?
2. Can two same-polarity constituents exchange places without a fact of the
   matter about which label continued?
3. Does unsigned capacity generate an elastic collision, a bounce, an
   exchange, or a failed algebraic root?
4. Does the field supply enough repulsion to select a finite equilibrium
   density once exact stacking is forbidden?
5. Is a solid simply the regime in which relational edges live much longer
   than the observation time?
6. Is a liquid the same local graph with rapid edge exchange rather than a
   different ontology?
7. Can flame-like identity be defined by a persistent causal tube even when no
   constituent remains for the full lifetime?
8. Does decay begin when one bond energy reaches zero, when the graph loses
   connectivity, or when environmental energy transport becomes outward?
9. Does a macroscopic surface coincide with low coordination, active capacity,
   a field-stress jump, or only their intersection?
10. Can two distant graphs carry independent Gauss constraints without sharing
    an indivisible residual field?
11. Does BCC belong to an internal/temporal relation graph while SC describes
    site-capacity packing, as the proposed two-domain picture suggests?
12. Can an FCC spatial density emerge as a projection of two bipartite layers,
    even though FCC cannot be the reciprocal bond graph itself?
13. What exact experiment distinguishes self-bound matter from matter held
    together by continuous environmental pressure?
14. If the complete evolution is reversible, what coarse observable makes
    material fracture or decay effectively irreversible?

## 13. Current verdict

The two-body common action contains enough algebra to write an exact
pairwise `N`-constituent transaction without a new dynamical primitive. It does
**not** yet contain enough structure to make that transaction a material law.
Unrestricted reciprocal additivity is superextensive and collapses. A
state-derived local capacity constraint provably restores a linear lower bound
without adding a stored variable, but its reversible common-action contact
solve remains open.

Accordingly:

```text
bound reciprocal pair      = constructive selected matter kernel,
unrestricted pair network  = closed as an extensive bulk candidate,
capacity-limited network   = extensive kinematics proved; dynamics open,
SC/BCC material phase      = not derived,
FCC reciprocal bond phase  = obstructed,
macroscopic matter         = open.
```

## 14. Exact SC/FCC polarity decomposition

The proposed SC/FCC physical-domain intuition has one exact realization that
does not require FCC to be the bond graph. Let

\[
\Lambda_+=\{x\in\mathbb Z^3:x_1+x_2+x_3\ \text{even}\},
\qquad
\Lambda_-=\mathbb Z^3\setminus\Lambda_+.
\tag{22}
\]

Each parity class is an FCC lattice: its shortest nonzero difference vectors
are the 12 permutations of `(+-1,+-1,0)`, with squared length two. The full SC
lattice is the disjoint union of these two translated FCC sublattices.

Assign `sigma_x=(-1)^(x_1+x_2+x_3)`. Every one of the six SC face neighbours
has opposite polarity and squared separation one, exactly the compact bond
minimum. Every nearest neighbour within either FCC sublattice has the same
polarity, squared separation two, and lies beyond the `q<3/2` reciprocal
cutoff. Therefore

```text
SC occupancy lattice       = both polarity sectors together,
FCC plus sublattice        = one spatial polarity sector,
FCC minus sublattice       = the conjugate spatial polarity sector,
reciprocal interaction graph = SC face edges between the FCC sectors.
```

This resolves the graph-theoretic FCC obstruction rather than evading it. FCC
is available as a spatial sublattice, but not as the present reciprocal bond
graph.

### Exact capacity saturation

Place one constituent at every integer site. Translation partition of unity
for the quadratic B-spline gives

\[
n(v)=\sum_{x\in\mathbb Z^3}
\prod_{i=1}^3 b_2(v_i-x_i)=1.
\tag{23}
\]

Thus the SC filling exactly saturates the unsigned capacity candidate; it does
not exceed it. Every reciprocal bond is at its energy minimum, so binding
forces vanish and the quiet binding energy is `-3 epsilon` per constituent.
This proves a stationary geometry for the binding/capacity sector, not a
stationary solution of the complete field-coupled action.

The signed coat density does not cancel site by site. At an integer site,
`b_2(0)=3/4` and `b_2(1)=1/8`, so separability gives

\[
\rho(v)=\prod_{i=1}^3\left[
\left(\frac34-2\frac18\right)(-1)^{v_i}\right]
=\frac18(-1)^{v_1+v_2+v_3}.
\tag{24}
\]

The checkerboard therefore excites the highest-wave-number Gauss sector. Its
redressed face-field energy and force must be included before the geometry can
be called a material ground state.

### Capacity pressure

If the constraint (14) is imposed by multipliers `lambda_v`, those multipliers
are naturally interpreted as a discrete pressure conjugate to local capacity.
Because partition of unity gives

\[
\sum_v n(v)=N,
\qquad
\sum_v\nabla_X n(v)=0,
\tag{25}
\]

adding an admissible constant to all multipliers changes no constituent force. This is the
usual pressure-zero gauge, derived here from fixed constituent number rather
than introduced as a fluid postulate. Spatial differences in `lambda_v`, not
its absolute value, can supply capacity stress.

Attraction and capacity then have distinct roles:

```text
compact reciprocal well = cohesion and bond elasticity,
unsigned site capacity  = saturation, contact, and pressure,
face/edge field          = polarity response and transported energy.
```

Capacity alone is fluid-like: it supplies compression resistance but not a
long-time shear modulus. Persistent bond topology is required for solid-like
response.

### Acoustic and polarity modes

Displace both FCC sublattices by the same vector. Every pair separation is
unchanged, so uniform common translation is a zero binding mode. Instead
displace the plus sublattice by `+u/2` and the minus sublattice by `-u/2`.
Using the single-bond expansion in the companion derivation and summing the six
face bonds gives

\[
\Delta U_{\rm bind}=48\epsilon N\lVert u\rVert^2
+O(\lVert u\rVert^3).
\tag{26}
\]

In the nonrelativistic limit the constituent inertial coefficient is
`m_eff=E_REST/C_SPEED^2`. The binding-only uniform relative mode would have

\[
\omega_{\rm rel}^2=\frac{384\epsilon}{m_{\rm eff}}.
\tag{27}
\]

Equation (27) is not a physical pole prediction. The relative displacement
changes (24), couples to the face field, and may mix with capacity and lattice
modes. Its importance is structural: the same two-sublattice material has a
common-motion channel and a polarity-separation channel. The latter is a
concrete candidate for dielectric/electromagnetic response.

### Surface polarity and static electrification

For a finite filled region define

\[
Q_{\rm pol}=N_+-N_-.
\tag{28}
\]

Balanced bulk cells cancel. Regular boundary termination can leave a
sublattice imbalance concentrated at the surface scale. Moving one manifested
constituent between two bodies changes their `Q_pol` values oppositely while
preserving the total in the reaction-free sector. This supplies an exact
candidate mechanism for static electrification:

```text
microscopic sign      = ternary/constituent polarity,
neutral material bulk = paired FCC sublattices,
static surface charge = sublattice population imbalance,
electric response     = face-field solution sourced by the imbalance.
```

It does not yet prove conserved physical electric charge, a Wilson current,
local `U(1)`, or the observed electron charge. Production reactions can also
change signed-state count, as already established by FTD-0421.

### Relation to the proposed BCC temporal domain

This section establishes only the SC/FCC spatial decomposition. A BCC graph is
also bipartite, but the Moore corner distance and the current compact-pair
cutoff do not make it the same graph. BCC can remain a candidate temporal or
update-context layer only if it is derived from the tick operator or an
explicit center--corner relation. It may not be inferred from the spatial
decomposition alone.

There is a second separation that is now exact. The selected FTD-0411 BCC
clock kernel is

\[
T_B(\theta)=\frac23(1-\cos^3\theta),\qquad T_B(0)=0.
\]

For a pure temporal quadratic term independent of material strain,

\[
S_B[Q]=\sum_\theta T_B(\theta)\lVert\widehat Q(\theta)\rVert^2,
\]

a static deformed history `Q_gamma(t)=Q_gamma` has support only at
`theta=0`. Consequently `S_B[Q_gamma]=0` for every `gamma`, and

\[
\mu_{B,\mathrm{time}}
=\frac1{\mathcal V}\left.
  \frac{\partial^2 S_B[Q_\gamma]}{\partial\gamma^2}
 \right|_{\gamma=0}
=0.
\]

This is a conditional exact statement about the selected pure clock kernel:
it may supply recurrence, dispersion, or inertia, but it cannot by itself
repair a static shear modulus. The conditional body-diagonal bond calculation
in equation (32) is a different object--a spatial potential graph--and cannot
be imported from `T_B`.

A BCC-like history relation could affect static rigidity only if its temporal
transport depends on deformation, for example through a link/connection
holonomy `U_gamma` in a term such as
`||Q(t+1)-U_gamma Q(t)||^2`. A constant history can then carry energy when
`U_gamma Q != Q`, but that energy is an explicit strain--connection coupling,
not a consequence of the clock symbol alone. It may still be derived from the
existing face/edge field variables rather than added as a new primitive; until
that derivation closes common action, recoil, inversion, and translation
covariance, it is new candidate dynamics and not material rigidity.

The next intuitive questions are:

1. Does the fully redressed checkerboard state remain stationary once its
   nonzero high-frequency Gauss field is included?
2. Does the field raise or lower the effective relative-sublattice frequency?
3. Can a slowly varying relative displacement produce a propagating
   polarization mode with a stable residue?
4. Does a common long-wavelength displacement recover the same cone as the
   face field?
5. Are polar surfaces stable, or does constituent transfer reconstruct a
   neutral termination?
6. Can two bodies exchange one polarity unit through an exact contact/current
   transaction while total `Q_pol` stays fixed?
7. Does bond topology give nonzero shear after the capacity constraint and
   field are relaxed, or is the SC central-force network marginal?
8. Can the existing face/edge transport define a deformation-dependent
   holonomy without adding a new primitive?
9. Does that holonomy gap the exact row-slide modes, or merely pin matter to
   the preferred lattice?
10. Can the same coupling derive both inertial response and static stress from
    one action rather than assigning them independently?
11. Does the period-two BCC-clock prototype change only finite-frequency
    response, as the zero-frequency theorem predicts?
12. Can a translated and rotated finite block recover the same relaxed shear
    response after the field and pressure multipliers are eliminated?

## 15. Mechanical exposure: cohesive does not yet mean solid

The site-aligned SC reciprocal network has nonzero compression stiffness but
is marginal under shear. Under a uniform dilation `x -> (1+eta)x`, every one
of the `3N` undirected face bonds stretches by `eta+O(eta^2)`. Equation (12) of
the moving-tube companion therefore gives

\[
\Delta U_{\rm dilation}=144\epsilon N\eta^2+O(\eta^3).
\tag{29}
\]

Under simple shear `x -> x+gamma y e_x`, the `x` and `z` bonds keep unit
length, while a `y` bond has

\[
r=\sqrt{1+\gamma^2}=1+\frac12\gamma^2+O(\gamma^4).
\tag{30}
\]

There are `N` undirected `y` bonds, so

\[
\Delta U_{\rm shear}=12\epsilon N\gamma^4+O(\gamma^6).
\tag{31}
\]

The binding-only harmonic shear modulus is exactly zero. The reciprocal SC
network is cohesive and compression-resistant, but it is not a harmonic solid.

An active hard-capacity constraint could resist shear because the deformed
coats are sampled on the fixed cubic substrate. Such resistance must not be
called material elasticity until it is separated from Peierls locking. A
legitimate shear modulus must survive integer translations, admissible
fractional charts, volume growth, and relaxation of the field and capacity
multipliers.

A conditional **spatial-graph** BCC comparison explains why an internal
body-diagonal relation is mechanically interesting. If eight unit
body-diagonal central bonds per constituent were
available, a simple shear changes each bond length at first order by
`+-gamma/3`. With four undirected bonds per constituent,

\[
\Delta U_{\rm BCC,shear}=\frac{64}{3}\epsilon N\gamma^2
+O(\gamma^3).
\tag{32}
\]

That spatial graph has genuine quadratic shear stiffness. The current
Moore-scale corner distance and compact cutoff do not supply it. Nor does the
pure FTD-0411 BCC temporal kernel: its static mode vanishes exactly. Equation
(32) is therefore a target only for a derived strain-dependent
connection/internal relation, not permission to add BCC bonds or to identify a
clock kernel with an elastic potential.

FCC central bonds would also remove the SC shear marginality, but the current
opposite-polarity rule forbids FCC as the reciprocal nearest-neighbour graph.
The open rigidity fork is therefore sharp:

```text
SC bond network alone        = zero harmonic shear,
capacity/substrate response  = possible pinning, not yet material elasticity,
BCC spatial/holonomy relation = conditional rigidity candidate,
pure BCC temporal clock      = inertia/dispersion candidate, zero static shear,
new angular/diagonal channel = priced extension.
```

## 16. Surface orientation and polarity

The parity construction predicts that not every crystal face has the same
polarity. On a `(111)` lattice plane,

\[
x_1+x_2+x_3=c,
\tag{33}
\]

every site has the same polarity `(-1)^c`. Consecutive `(111)` layers alternate
sign. A `(100)` plane instead fixes only `x_1`; its `x_2+x_3` checkerboard
contains both polarities and is neutral in a balanced patch. The same is true
for a balanced `(110)` patch.

Thus the candidate material has polar and nonpolar terminations before any
phenomenological charge formula is added. A polar cut also breaks more of the
SC face-bond network and creates a larger field burden, so the redressed action
may favor reconstruction into a neutral surface. This supplies testable
questions for static electricity:

1. Does a finite `(111)` termination retain a nonzero `Q_pol` after full
   common-action relaxation?
2. Does a `(100)` control remain neutral under the same preparation?
3. Does contact between unlike terminations transfer constituent polarity and
   leave equal/opposite body imbalances?
4. Does the face field carry the resulting surface stress and energy without a
   legacy Coulomb force?
5. Does surface reconstruction eliminate the polarity before separation,
   closing the static-electrification mechanism?

Any orientation-locked surface charge is also a preferred-frame exposure. It
must either wash out in macroscopic reconstructed matter or remain below
empirical anisotropy bounds.

## 17. Conditional exact periodic bulk solution

The nonzero checkerboard source in (24) can be redressed exactly on an even
periodic lattice. Let

\[
\chi(v)=(-1)^{v_1+v_2+v_3},
\qquad \rho(v)=\frac18\chi(v),
\tag{34}
\]

and assign every positively oriented face component

\[
E_i(v)=\frac1{48}\chi(v),
\qquad B_i(v)=0.
\tag{35}
\]

Because `chi(v-e_i)=-chi(v)`, the face divergence is

\[
\sum_{i=1}^3\big(E_i(v)-E_i(v-e_i)\big)
=6\frac{\chi(v)}{48}=\rho(v).
\tag{36}
\]

Equation (35) is a discrete gradient and is curl-free. The raw quadratic field
energy per site is

\[
e_F=\frac12\sum_{i=1}^3 E_i(v)^2=\frac1{1536}.
\tag{37}
\]

With the established mapped field-work coefficient `beta`, the quiet
rest-subtracted internal energy per constituent is conditionally

\[
e_{\rm int}=-3\epsilon+\frac{\beta}{1536}.
\tag{38}
\]

At each integer constituent position, the two adjacent faces on every axis
have equal magnitude and opposite sign. The symmetric quadratic-coat gather
therefore gives zero electric impulse. Binding impulses vanish because every
SC bond is at `q=1`; magnetic impulses vanish because `B=0`; and zero capacity
multiplier is admissible. Thus the saturated checkerboard plus (35) is a
conditional exact static solution of the proposed pairwise binding--capacity--
field algebra.

This is the first complete many-constituent equilibrium supplied by the
candidate algebra, but it is not yet a material object:

- it fills the periodic volume rather than occupying a finite uncontained
  region;
- pairwise `N`-body dynamics and capacity contact are not implemented;
- the finite surface may reconstruct or evaporate;
- common fractional translation may expose lattice pinning; and
- no phonon/polarization spectrum or positive relaxed Hessian has been
  certified.

The next finite-matter question is therefore exact: can a bounded piece of
this bulk retain a state-selected surface under its missing-bond attraction,
capacity pressure, and redressed face-field stress without an imposed
container?

## 18. Conditional finite uncontained block

The binding--capacity sector already supplies a finite self-bound candidate.
Take a checkerboard-filled rectangular block of dimensions
`L_x x L_y x L_z`. Its constituent count is

\[
N=L_xL_yL_z,
\tag{39}
\]

and its exact number of reciprocal face bonds is

\[
|E|=(L_x-1)L_yL_z+L_x(L_y-1)L_z+L_xL_y(L_z-1).
\tag{40}
\]

Therefore

\[
U_{\rm bind}=-3\epsilon N
+\epsilon\big(L_yL_z+L_xL_z+L_xL_y\big).
\tag{41}
\]

The second term is half the geometric face area

\[
A=2\big(L_yL_z+L_xL_z+L_xL_y\big),
\tag{42}
\]

so the binding-only face tension is exactly

\[
\gamma_{\rm bind}=\frac{\epsilon}{2}
\tag{43}
\]

per unit site-face area. This is not an imposed membrane. It is the energy of
the reciprocal bonds missing at the termination of a capacity-limited graph.

For fixed `N`, separated constituents have zero binding energy, whereas every
connected block with at least one bond has negative binding energy. Increasing
adjacency lowers the energy and capacity forbids unlimited local stacking.
Thus attraction plus capacity favors compact finite support without an
external container. This is the first conditional realization of the user's
“finite but uncontained” matter picture.

Local removal costs are also discrete. Ignoring field relaxation, removing a
constituent of coordination `z` costs `z epsilon`. A large SC block therefore
has approximate removal thresholds

```text
corner constituent : 3 epsilon,
edge constituent   : 4 epsilon,
face constituent   : 5 epsilon,
bulk constituent   : 6 epsilon.
```

Evaporation should begin at low-coordination sites; cleavage energy counts the
bonds cut. A `(100)` cleavage through `A_cut` site faces breaks `A_cut` bonds,
costing `epsilon A_cut` and creating two surfaces, consistent with (43).

The net polarity of an axis-aligned block is not generically surface-
extensive. With an origin on the plus sublattice,

\[
Q_{\rm pol}=
\prod_{i\in\{x,y,z\}}
\sum_{n=0}^{L_i-1}(-1)^n,
\tag{44}
\]

so `Q_pol=0` if any side length is even and `Q_pol=+1` if all three are odd
(with the conjugate sign after a one-site translation). Large ordinary cubes
are therefore neutral or carry only one unmatched polarity unit; extensive
surface polarity requires a polar termination such as the `(111)` construction
of §16, not merely any boundary.

The block is not yet a certified matter object. Equation (41) omits its finite
redressed field and capacity-contact dynamics. Its binding-only shear modulus
is zero, and collective fractional translation may be pinned. The licensed
interpretation is narrower:

On a periodic lattice, an isolated Gauss redressing additionally requires
`Q_pol=0`; an odd-by-odd-by-odd block needs an explicitly declared compensating
environment or nonperiodic boundary and cannot be silently treated as an
isolated periodic object.

```text
finite support       = explicit,
no external container = explicit in the static binding/capacity energy,
surface energy       = exact missing-bond term,
self-bound full action = open,
mobile droplet/body  = open,
solid                = closed for binding-only SC harmonic response.
```

Section 19 answers the first energetic question constructively for all-even
cubes: minimum Gauss redressing preserves negative formation energy. It also
rules out a super-extensive field burden, but its conservative routed-field
bound does not yet isolate the relaxed surface coefficient from the bulk. The
remaining decisive finite-object questions are:

1. Does the exact minimum redressed energy separate into the periodic bulk
   coefficient plus area, edge, and corner terms, or retain another extensive
   boundary-reconstruction contribution?
2. Does a released block remain compact under the common action, or evaporate
   first from corners as the coordination hierarchy predicts?
3. Can the block translate without a substrate-sized Peierls barrier?
4. Does an incoming field packet remove a corner constituent only after
   depositing the registered binding-plus-field threshold?
5. Can two blocks merge, reduce total surface area, and release exactly the
   missing surface energy as outgoing field transport?
6. Can a larger block split only when the incoming work exceeds the exact
   cleavage energy plus field correction?
7. Does constituent turnover preserve body-level identity even though the
   molecular constituent set changes?

## 19. Exact finite Gauss redressing and an extensivity bound

The finite field burden can be bounded without running a many-body campaign.
Let the sampled one-dimensional quadratic coat be

\[
b(0)=\frac34,\qquad b(\pm1)=\frac18,\qquad b(r)=0
\ \text{otherwise},
\tag{45}
\]

and define the deposited line density of an alternating block by

\[
f_L(v)=\sum_{n=0}^{L-1}(-1)^n b(v-n).
\tag{46}
\]

For even `L`, direct summation gives

\[
\sum_v f_L(v)=0,
\qquad
\sum_v f_L(v)^2=\frac{4L+5}{16}.
\tag{47}
\]

The three-dimensional block density factorizes exactly:

\[
\rho(v_x,v_y,v_z)=f_{L_x}(v_x)f_{L_y}(v_y)f_{L_z}(v_z).
\tag{48}
\]

This is stronger than total neutrality. If all three side lengths are even,
every coordinate-line factor has zero sum, so the monopole, dipole, and every
moment of total degree below three vanish. The first permitted long-distance
moment is octupole-like. Thus an all-even axis-aligned block is the clean
nonpolar finite control; a block with only one even side is neutral but need
not be dipole-free.

There is also an explicit compact field satisfying Gauss. Define

\[
g_L(v)=\sum_{u=-1}^{v}f_L(u),
\qquad
g_L(v)-g_L(v-1)=f_L(v).
\tag{49}
\]

For even `L`, `g_L` returns exactly to zero outside the block coat and

\[
\sum_v g_L(v)^2=\frac{10L-1}{32}.
\tag{50}
\]

Routing the field in the `x` direction,

\[
E_x(v)=g_{L_x}(v_x)f_{L_y}(v_y)f_{L_z}(v_z),
\qquad E_y=E_z=0,
\tag{51}
\]

gives `div E=rho` exactly and has raw energy

\[
T_x=rac{(10L_x-1)(4L_y+5)(4L_z+5)}{16384}.
\tag{52}
\]

Equation (51) is an admissible Gauss field, not the claimed static field: it
generally carries curl at the transverse boundary. The curl-free minimum-
energy electrostatic redressing `E_G` therefore obeys the rigorous upper bound

\[
0\le E_G\le \min_{i:L_i\ {m even}}T_i.
\tag{53}
\]

This bound is independent of the empty ambient volume once the compact coat
does not wrap. It proves that finite redressing is at most extensive with
surface, edge, and corner corrections; it cannot restore the `-N^2` collapse
closed in §6.

The minimum itself also has an exact periodic representation. On an ambient
periodic lattice of volume `V_Omega`, let

\[
k_i=\frac{2\pi m_i}{M_i},\quad
\lambda(k)=4\sum_i\sin^2\!\left(\frac{k_i}{2}\right),\quad
\widehat b(k)=\frac34+\frac14\cos k,
\tag{54}
\]

and

\[
S_L(k)=\sum_{n=0}^{L-1}(-1)^n e^{-ikn}
=\frac{1-(-e^{-ik})^L}{1+e^{-ik}},
\tag{55}
\]

with the removable value at `k=pi` taken by continuity. Then

\[
\widehat\rho(k)=\prod_i \widehat b(k_i)S_{L_i}(k_i),
\qquad
E_G=\frac1{2V_\Omega}\sum_{k\ne0}
\frac{|\widehat\rho(k)|^2}{\lambda(k)}.
\tag{56}
\]

No fitted scale appears in (56). The complete selected rest-subtracted static
functional, before any additional capacity energy, is conditionally

\[
H_{\rm block}=-3\epsilon N
+\epsilon(L_yL_z+L_xL_z+L_xL_y)+\beta E_G.
\tag{57}
\]

For an even cube this gives the certified sufficient bound

\[
\frac{H_{\rm block}}{N}\le
-3\epsilon+\frac{3\epsilon}{L}
+\beta\frac{(10L-1)(4L+5)^2}{16384L^3}.
\tag{58}
\]

At the frozen selected values `epsilon=0.01` and
`beta=G_C^2/C_WAVE^2=0.021892057692994273`, the right-hand side is already
negative at `L=2` (`-0.01446368868063198` per constituent) and decreases for
all `L>=2`; its infinite-size limit is
`-0.02978621037409185`. Therefore every even cube in this family has negative
rest-subtracted formation energy under the minimum Gauss redressing. This is
an energetic binding certificate, not a dynamical matter certificate:

- the minimum field can exert nonzero boundary impulses;
- capacity-contact forces and their reversible transaction remain undefined;
- a negative energy does not prove a positive relaxed Hessian;
- the binding-only shear modulus remains zero; and
- release, evaporation, translation, merging, and cleavage remain unrun.

The next engine test is consequently narrower than “does the block bind?” It
must determine whether common-action relaxation reaches a finite stationary
shape rather than reconstructing, shearing, pinning, or evaporating.

## 20. Capacity gives a diffuse surface and incompressibility, not solidity

The unsigned capacity sidecar now supplies an exact finite surface geometry.
For a one-dimensional block define

\[
h_L(v)=\sum_{n=0}^{L-1}b(v-n).
\tag{59}
\]

For `L>=2`, direct evaluation gives

\[
h_L(-1)=h_L(L)=\frac18,
\quad h_L(0)=h_L(L-1)=\frac78,
\quad h_L(v)=1\quad(1\le v\le L-2).
\tag{60}
\]

The three-dimensional occupancy factorizes as

\[
n(v)=h_{L_x}(v_x)h_{L_y}(v_y)h_{L_z}(v_z).
\tag{61}
\]

Consequently the exactly saturated capacity bulk contains

\[
N_{\rm sat}=(L_x-2)(L_y-2)(L_z-2)
\tag{62}
\]

sites when every side is at least two. The remaining occupied sites form a
one-coat-thick slack layer. An `L=2` cube is energetically bound by §19 but has
no saturated interior site; it is not a bulk-matter control. The smallest
all-even, neutral, dipole-free cube with a nonempty saturated interior is
`L=4`, with 64 constituents and eight saturated interior sites.

An exact diffuse-interface measure is

\[
\mathcal A_{\rm cap}=\sum_v n(v)(1-n(v)).
\tag{63}
\]

It vanishes in exactly full and exactly empty regions and is positive only in
the coupling-scale transition. Since `sum_v n(v)=N` and

\[
\sum_v h_L(v)^2=L-\frac7{16},
\tag{64}
\]

the block interface is

\[
\mathcal A_{\rm cap}
=\frac7{16}(L_yL_z+L_xL_z+L_xL_y)
-\frac{49}{256}(L_x+L_y+L_z)
+\frac{343}{4096}.
\tag{65}
\]

Thus the capacity interface has an exact area-leading term, followed by edge
and corner corrections. It is a derived finite-thickness surface observer,
not an ontic membrane and not an energy. The material-surface candidate is the
coincidence of three independently defined structures:

```text
bond-coordination deficit = cohesive surface energy,
0 < n(v) < 1              = diffuse capacity interface,
field-stress transition   = dynamical environmental loading.
```

### Exact row-slide theorem

Capacity does not supply a bulk shear modulus. On the infinite saturated
lattice, independently translate every `x`-directed row:

\[
x_{mnl}=(m+a_{nl},n,l),
\tag{66}
\]

where every `a_nl` is arbitrary. At any lattice site,

\[
\begin{aligned}
n(v)&=\sum_{n,l}b(v_y-n)b(v_z-l)
      \sum_m b(v_x-m-a_{nl})\\
&=\sum_{n,l}b(v_y-n)b(v_z-l)=1.
\end{aligned}
\tag{67}
\]

The first equality uses the tensor coat; the second uses partition of unity
separately on every row. Independent row sliding therefore preserves capacity
**exactly**, not merely to first order. In a finite block the same local
identity holds wherever the shifted row still supplies the complete coat
partition; for sufficiently small shear, capacity changes are confined to the
coat-scale surface.

This closes one tempting escape: capacity resistance cannot repair the zero
harmonic shear modulus found in §15. The axial binding network plus hard
capacity is fluid/slip-like in the bulk. A harmonic solid requires a nonzero
relaxed field contribution, a derived diagonal/angular relation, or another
priced material structure. Substrate pinning is not such a modulus.

### One-contact energy root and the recoil defect

Let

\[
g_v(X)=1-n(v;X)\ge0,
\qquad a_v=\nabla_X g_v.
\tag{68}
\]

At one isolated active contact, hold position fixed and apply an impulse along
the contact normal,

\[
P^+=P^-+\lambda a_v,
\qquad
K(P)=\sum_a\sqrt{E_{\rm REST}^2+C_{\rm SPEED}^2|p_a|^2}.
\tag{69}
\]

If the contact is incoming, `a_v dot V(P^-)<0`. The function
`phi(lambda)=K(P^-+lambda a_v)` is strictly convex, begins with negative
derivative, and diverges for large positive `lambda`. Therefore there is one
and only one nonzero `lambda_* > 0` satisfying

\[
K(P^-+\lambda_*a_v)=K(P^-).
\tag{70}
\]

This supplies an exact relativistic elastic-reflection candidate for one
isolated contact. It conserves matter energy and has a time-reversed conjugate.
It does **not** yet close the common action, because its total matter impulse is

\[
\Delta P_{\rm matter}
=\lambda_*\sum_a\nabla_{x_a}g_v
=\lambda_*\left.\nabla_\delta g_v(X+\delta)\right|_{\delta=0}.
\tag{71}
\]

The right-hand side is the occupancy's common-translation gradient relative
to the preferred lattice. It is not generically zero. Eight coincident coats
at `(1/2,1/2,1/2)` exactly saturate each of the eight surrounding sites; at
one such site the common `x` derivative of `n` is exactly `-2`. A local contact
multiplier there therefore exerts nonzero total matter impulse. Equal
multipliers over the complete eight-site pattern cancel by the pressure-zero
gauge, but an arbitrary active pattern need not.

This identifies the missing ledger exactly. A capacity contact must do one of
the following in the same atomic transaction:

1. select multipliers whose summed common-translation gradient vanishes;
2. deposit equal and opposite momentum into the face/edge field; or
3. declare and measure substrate recoil as an additional preferred-frame
   channel.

The bare reflection (69) changes momentum at fixed position, deposits no face
current, and leaves the field unchanged. It is therefore energy-exact but not
yet a closed isolated common-action event.

With several simultaneous active sites, the inequality `n(v)<=1` supplies
contact normals but no unique impact law. Energy conservation is one scalar
condition on several multipliers; nonpenetration and positivity determine an
admissible cone, not necessarily one point. A time-symmetric discrete
variational principle, a registered specular selector, or an explicit
reaction/internal-phase branch is still required. The pressure gauge and the
row-slide null modes must be quotiented without losing state-only inversion.

### Recursive intuitive questions sharpened by the algebra

1. Is the multiplier field literally material pressure, or only a constraint
   reaction with no independent ontology?
2. Can the face field absorb contact recoil without introducing a post-hoc
   impulse or violating magnetic zero-work?
3. If row sliding is exact, is the capacity-saturated phase naturally a liquid
   rather than a solid?
4. Does a BCC/internal diagonal relation provide shear while SC capacity
   continues to define physical volume?
5. Can the minimized checkerboard field itself generate a positive harmonic
   shear modulus?
6. Is the material surface best identified by the overlap of missing bonds,
   capacity slack, and a field-stress jump rather than any one of them?
7. When two surfaces meet, does the reversible event look like elastic
   reflection, constituent exchange, bond reconnection, or annihilation?
8. Can an unlabeled constituent exchange make the contact map invertible even
   when labeled worldlines appear to cross?
9. Can the active contact set be reconstructed from the endpoint alone, or is
   an internal collision phase required?
10. Does the pressure multiplier converge to a continuum stress while its
    residual lattice recoil scales to zero?
11. Is the acoustic speed set by bond compression, capacity pressure, field
    stress, or a coupled mode of all three?
12. Are void sites merely empty space, or are they the free-volume variable
    that distinguishes solid, liquid, gas, and flame-like regimes?

The exact next finite-object control is consequently `L=4`, not `L=2`. It is
the smallest all-even block containing both a saturated bulk and a slack
surface. No campaign is preregistered here: FTD-0768 must first resolve, and
the contact-recoil transaction must be frozen before execution.

## 21. Minimum-field recoil no-go for elastic contact

The first finite-block preparation uses the unique minimum-energy Gauss field
and zero magnetic field. For fixed deposited density, write any other
Gauss-admissible electric field as

\[
E=E_G+E_T,
\qquad \nabla\!\cdot E_T=0,
\qquad \langle E_G,E_T\rangle=0.
\tag{72}
\]

The last identity is the discrete gradient--divergence Hodge orthogonality.
With the selected positive field coefficient,

\[
H_F(E,B)=H_F(E_G,0)
+\frac{\beta}{2}\left(\lVert E_T\rVert^2+\lVert B\rVert^2\right).
\tag{73}
\]

Therefore

\[
H_F(E,B)=H_F(E_G,0)
\quad\Longrightarrow\quad
E=E_G,\ B=0
\tag{74}
\]

after the registered harmonic/global-flux sector is fixed to zero. This is
not a small-amplitude statement; it is positivity of the exact quadratic
field energy.

Now apply the elastic single-contact candidate of §20 at fixed position.
Binding energy and deposited density do not change, and equation (70) keeps
matter energy unchanged. Exact total-energy conservation then requires zero
field-energy change. Equation (74) forces the field state to remain unchanged,
so any field-momentum functional of that state remains unchanged as well. The
field cannot absorb the nonzero matter impulse in (71).

Thus a minimum-dressed isolated contact has an exact trilemma:

```text
net contact impulse = 0   -> elastic internal contact remains possible,
net contact impulse != 0  -> explicit substrate recoil is required,
field absorbs recoil      -> matter must fund a nonminimum field excitation.
```

The third branch is a reversible **radiative/inelastic** contact when the
complete outgoing field is retained; its time reverse requires the conjugate
incoming field. It is not the elastic reflection of §20. A pre-existing
transverse environmental field could also redistribute momentum at fixed
total field energy, but that would make the collision environment-dependent
and is absent from the minimum-dressed control.

The immediate static problem can now be stated without ambiguity. For the
`L=4` block, let `F_field` be the exact gradient of the minimum Gauss energy,
let `F_bind=0` at the unit bonds, and let `A` be the Jacobian of the eight
saturated capacity constraints. A stationary free body exists at the
site-aligned geometry only if there is a nonnegative pressure vector `lambda`
such that

\[
F_{\rm field}+A^T\lambda=0,
\qquad
\sum_a(A^T\lambda)_a=0.
\tag{75}
\]

The second condition forbids unledgered lattice recoil. It is redundant only
if the first equation and the complete field-stress ledger already enforce
total balance. Failure of this feasibility problem would close the exact
site-aligned block as a free equilibrium; it would not authorize a fitted
pressure, external container, or post-hoc counterforce. Success would provide
the pressure field about which the constrained relaxed Hessian can be tested.

For `L=4`, equation (75) is not a large free fit. The eight active capacity
sites are `{1,2}^3` and form one orbit of the cube group. Any admissible
pressure solution can be averaged over that group; nonnegativity is preserved,
and the averaged solution has one common multiplier `lambda`. Define

\[
B(x)=\sum_{u\in\{1,2\}}b(u-x),
\qquad
D(x)=\sum_{u\in\{1,2\}}b'(u-x).
\tag{76}
\]

At constituent coordinates `x=0,1,2,3`, these are exactly

\[
B=\left(\frac18,\frac78,\frac78,\frac18\right),
\qquad
D=\left(-\frac12,-\frac12,\frac12,\frac12\right).
\tag{77}
\]

The cubic-invariant capacity force on constituent `(x,y,z)` is therefore

\[
C_{xyz}=\big(D(x)B(y)B(z),\ B(x)D(y)B(z),\
                 B(x)B(y)D(z)\big),
\qquad F_{\rm cap}=\lambda C.
\tag{78}
\]

It is outward, has exact zero total impulse, and has positive centered virial

\[
\sum_{xyz}C_{xyz}=0,
\qquad
\sum_{xyz}(x-x_c)\!\cdot C_{xyz}=24,
\quad x_c=(3/2,3/2,3/2).
\tag{79}
\]

Thus the static capacity sector already contains a legitimate net-zero
internal pressure pattern. The field calculation has only one falsifier: for
every constituent component,

\[
R_{a i}=-\frac{F_{{\rm field},a i}}{C_{a i}}
\tag{80}
\]

must equal the same nonnegative number `lambda`. Every denominator is nonzero.
If the ratios disagree, no nonuniform pressure escape exists: group-averaging
would convert any solution into the failed invariant one. If they agree, the
site-aligned block is stationary without substrate recoil and the common value
is derived pressure, not a fitted parameter.

The minimum-field force entering (80) has a fixed adjoint expression. With

\[
\rho(v)=\sum_a\sigma_a W_a(v),
\qquad (-\Delta)\phi=\rho,
\tag{81}
\]

the minimized field energy is `H_G=(beta/2)<rho,phi>` and

\[
F_{{\rm field},a}
=-\beta\sigma_a\sum_v\phi(v)\nabla_{x_a}W_a(v).
\tag{82}
\]

This is the analytic derivative of the minimum, not a finite-difference force
and not a separately imposed Coulomb branch. Summing over constituents gives

\[
\sum_aF_{{\rm field},a}
=-\left.\nabla_\delta H_G(X+\delta)\right|_{\delta=0}.
\tag{83}
\]

For the centered `L=4` checkerboard, inversion sends `delta` to `-delta` and
conjugates every polarity. The quadratic field energy is conjugation-even, so
`H_G(delta)=H_G(-delta)` and the total field force in (83) vanishes exactly.
The test is therefore about internal pressure balance, not net acceleration.

Cubic symmetry reduces the 192 scalar force components to six representatives.
Let `k` be the number of inner coordinates in a constituent and let `t` be one
when the measured force component lies along an inner coordinate. The six
orbits are

\[
(k,t)\in\{(0,0),(1,0),(1,1),(2,0),(2,1),(3,1)\}.
\tag{84}
\]

The corresponding nonzero capacity magnitudes belong to

\[
|C_{a i}|\in\left\{\frac1{128},\frac7{128},\frac{49}{128}\right\}.
\tag{85}
\]

Only six adjoint field forces are therefore required. Their six ratios must
agree with zero residual before any relaxation, pressure, or material claim is
licensed.

FTD-0768 still gates execution, so equation (75) is a derived successor
problem rather than a preregistered or evaluated result.

## 22. Independent exact certificate

[`proof_minimal_many_body_matter_network.py`](../../../scripts/proofs/proof_minimal_many_body_matter_network.py)
uses integer and rational arithmetic only. It performs no fit, scan, or
floating-point tolerance comparison. Its 105 checks cover:

- compact-potential values and endpoint regularity;
- exact multi-bond impulse sum and binding-work identity;
- the stacked `-m^2` counterexample;
- quadratic-coat partition, capacity, and degree bounds;
- SC/FCC parity-neighbour counts;
- checkerboard coat density, capacity saturation, Gauss field, curl, centered
  cancellation, and `1/1536` field energy;
- exact well, dilation, SC shear, BCC shear, and polarity-mode coefficients;
- rectangular-block bonds, surface excess, and polarity for four independent
  integer geometries;
- the exact finite-line density norms, compact routed Gauss construction, and
  finite-block field-energy bound in §19;
- the monotonic selected even-cube formation bound and its infinite-volume
  limit;
- the finite capacity profile, saturated-interior count, and exact diffuse
  area/edge/corner interface expansion;
- exact independent row-slide capacity invariance; and
- the half-cell eight-coat saturation and nonzero single-site recoil
  gradient; and
- the unique cubic `L=4` pressure weights, zero net impulse, virial, component
  magnitudes, and six cubic force orbits; and
- the `L=4` active-capacity Jacobian/Gram spectrum, binding rank, exact
  row-slide tangent space, and common-pressure restricted Hessian; and
- conservation and exact injectivity of the signed-source and unsigned-
  occupancy first-variation maps on the 27-dimensional pressure kernel;
- general even-`L` end-cap support of complete-row source/occupancy
  variations; and
- the exact periodic translation-phase energy, half-cell bulk cancellation,
  and finite half-phase area-scaling Gauss construction.

The thin pytest wrapper
[`test_minimal_many_body_matter_network.py`](../../../scripts/tests/test_minimal_many_body_matter_network.py)
passes. This certificate validates the algebra in this document; it does not
execute the unimplemented many-body/contact dynamics.

## 23. Conditional matter and phase dictionary

The derivation now supports a sharper ontological statement. A finite material
body is not one occupied voxel and not its entire field. It is a bounded
history of a saturated-or-partly-saturated constituent network for which:

1. the complete energy has an extensive lower bound;
2. formation energy is negative relative to separated constituents;
3. a finite capacity/bond surface is state-identifiable;
4. relational organization persists under the declared turnover rule; and
5. motion, contact, field transport, recoil, and inversion close in one common
   action.

The candidate algebra closes items 1--3. The selected pair histories support a
finite-time precursor to item 4. Capacity contact and many-body common-action
relaxation leave item 5 open. “Matter derived” would require the full
conjunction, not merely negative energy or a visually compact field.

Within that one ontology, familiar phases become different dynamical regimes
rather than different substances:

| Regime candidate | Capacity/graph behavior | Required discriminator |
|---|---|---|
| solid | saturated bulk, persistent graph, positive relaxed shear | field/diagonal/angular rigidity and long edge autocorrelation |
| liquid | saturated bulk and cohesive surface, rapid edge exchange, zero static shear | finite viscosity, diffusion, and surface persistence |
| gas | mostly slack capacity, short-lived components, positive separation margins | equation of state and collision closure |
| plasma | weak material graph, mobile polarity imbalance and field stress | screened collective modes without importing charge ontology |
| flame/cloud | localized process and interface persist while constituents turn over | reaction throughput, causal localization, and environmental energy balance |

This table is a test program, not a claim that any phase has been produced.
It nevertheless explains why the exact row-slide result is useful rather than
merely negative: capacity plus cohesion already has the kinematic signature of
a liquid-like bulk. A solid requires an additional restoring channel; a flame
requires controlled turnover; a gas requires contact; and a plasma requires a
qualified collective polarity/field mode.

The same variables suggest operational meanings for several macroscopic
quantities:

```text
density         = coarse occupancy n,
pressure        = capacity multiplier field lambda,
surface tension = missing-bond energy plus relaxed field correction,
elasticity      = constrained relaxed energy Hessian,
viscosity       = decay of row-slide/relative momentum into internal modes,
temperature     = coarse distribution of internal excitation energy,
charge candidate = persistent polarity imbalance plus its Gauss dressing,
mass/inertia    = collective boost-energy curvature of the complete body.
```

Every line remains conditional until its corresponding history observable and
common-action ledger close. In particular, `lambda` is not promoted to a new
field, temperature is not a primitive random variable, and polarity imbalance
is not yet conserved physical charge.

The next intuitive questions are therefore:

1. Does the `L=4` pressure feasibility problem have a solution without lattice
   recoil?
2. If it does, is the constrained Hessian liquid-like or solid-like?
3. Does the field supply the missing shear channel, or only reconstruct the
   surface?
4. Can two finite interfaces meet and exchange constituents while the complete
   state remains invertible?
5. Does edge turnover conserve body identity while destroying molecular
   identity?
6. Can a detached constituent carry its own minimum dressing, or must it pull
   energy from the parent surface?
7. Is evaporation a deterministic threshold event whose statistical rate
   arises only after coarse-graining internal phases?
8. Can a moving body transport its pressure and surface profile without
   entraining the entire environmental residual field?
9. Does a polarity-rich surface relax by field emission, constituent transfer,
   or internal reconstruction?
10. Which internal mode provides the first clock that can measure the body's
    own proper-time-like evolution?

## 24. Existing-variable rigidity boundary: there is no free holonomy

The BCC-clock correction in §14 leaves an apparent escape: perhaps the
existing face/edge field supplies a deformation holonomy and therefore the
missing shear without a new primitive. The committed types make this question
decidable before another campaign.

Let the static candidate state be

\[
X=(x_a,\sigma_a;E_f,B_e),
\]

with pressure multipliers enforcing capacity but not counted as independent
ontic fields. Once the constituent positions are prescribed, define the
existing-action relaxed energy

\[
\mathcal E_L(\gamma,\delta)
=\min_{E,B,\lambda,\,\text{admissible internal relaxation}}
\left[U_{\rm bind}+H_F\right].
\]

Here `gamma` is an imposed macroscopic shear and `delta` is the common
fractional translation phase relative to the substrate. The minimization must
enforce Gauss, capacity, zero unledgered recoil, and the registered boundary
conditions. The complete no-new-type static modulus is

\[
\mu_L(\delta)=\frac1{\mathcal V_L}
\left.\frac{\partial^2\mathcal E_L(\gamma,\delta)}
{\partial\gamma^2}\right|_{\gamma=0}.
\]

This quantity already contains every static restoring effect of the existing
face/edge fields. Calling the same effect a holonomy does not add another
channel.

### Gauge-neutral bond lemma

Suppose instead that a real link connection `A_ab` is introduced with the
usual endpoint transformation

\[
A_{ab}\longmapsto A_{ab}+\chi_b-\chi_a.
\]

The present constituent data `(x,p,sigma)` do not transform under this
redundancy. A local bond energy `V_ab(A_ab;x,p,sigma)` that is invariant for
arbitrary independent `chi_a,chi_b` must obey

\[
V_{ab}(A_{ab}+c)=V_{ab}(A_{ab})
\quad\text{for every real }c,
\]

and is therefore independent of `A_ab`. A gauge connection cannot exert a
local bond force on gauge-neutral matter.

Reconstructing `A` from the existing field strength does not evade the lemma.
The historical `A=P_TJ` representative is spatially nonlocal (FTD-0416), and
equal curvature can occupy inequivalent flat/harmonic sectors on a periodic or
multiply connected domain. Choosing one inverse, gauge, and harmonic sector is
observer reconstruction, not a local state-only interaction. If a future
transaction distinguishes those equal-field sectors, the connection trigger
in the matter-ontology branch matrix fires and `A` must be carried as state;
until then, reconstructed open-path transport is not an ontic observable.

There are only two standard escapes:

1. use a closed-loop functional such as a Wilson/plaquette holonomy; or
2. give matter a transforming local phase `theta_a`, so that
   `theta_b-theta_a-q A_ab` is invariant.

The first route is already a functional of field curvature on contractible
loops. If the loop is fixed to the substrate it is field energy, not material
elasticity. If the loop is tied to a deforming constituent graph, that
graph--connection coupling is new dynamics and requires its own normalization
and common-action derivation. The second route requires a phase-bearing matter
coordinate at every relevant endpoint. A single global body phase cancels
from every bond difference and cannot supply internal shear.

FTD-0658 proves that the current registered rest candidate has no intrinsic
nonzero phase; prepared action--angle excitations carry phase only at nonzero
action and do not extend to the quiet fixed point. FTD-0494/0495 are even more
direct: the existing face-field work defines a non-flat lattice one-form, but
its open-path value requires a real history fiber `D`. That fiber closes scalar
bookkeeping only; variational enforcement introduces a free multiplier and
restores the rejected branch force. Its holonomy therefore cannot be silently
reused as elastic energy.

Consequently the no-new-primitive branch has exactly one static rigidity
test: the constrained relaxed Hessian of the action already on the books. A
new holonomy term that changes that Hessian is either a new selected coupling
or a new phase/connection type.

### Zero-frequency separation theorem

After linearizing a relaxed body, write the shear response schematically as

\[
\Gamma^{(2)}_L(\omega)
=K_L^{(0)}-\omega^2M_L+i\omega\Xi_L+O(\omega^3).
\]

The three coefficients answer different physical questions:

```text
K_L^(0)  = static elastic restoring coefficient,
M_L      = inertial/temporal coefficient,
Xi_L     = dissipative or coarse-grained viscous coefficient.
```

The pure BCC clock can modify `M_L`; it cannot create `K_L^(0)`. A decaying
row-slide can reveal `Xi_L`; it does not by itself prove a solid. Static matter
rigidity is therefore a zero-frequency statement after field, pressure,
translation phase, and internal row modes have been relaxed.

### Material elasticity versus Peierls pinning

The minimum discriminator must retain the substrate phase rather than testing
one privileged chart. Define

\[
P_L=\max_\delta\mathcal E_L(0,\delta)
    -\min_\delta\mathcal E_L(0,\delta),
\qquad
\Delta\mu_L=\max_\delta\mu_L(\delta)-\min_\delta\mu_L(\delta),
\]

and the translation-relaxed modulus from

\[
\mathcal E_L^{\rm free}(\gamma)=\min_\delta
\mathcal E_L(\gamma,\delta).
\]

The classifications are:

```text
positive fixed-chart curvature only
    = substrate pinning candidate;

positive curvature after common translation and independent row relaxation
    = material static-rigidity candidate;

zero static curvature but nonzero omega^2 response
    = temporal inertia, not solidity;

zero static curvature with finite decay of row momentum
    = viscosity/liquid-like response, not solidity.
```

A material claim additionally requires stable volume scaling, cubic controls,
and an explicit bound on `P_L/V_L` and `Delta mu_L`. Exact microscopic
fractional-translation symmetry is not assumed. The test asks whether the
macroscopic coefficient is robust to the substrate phase rather than being
manufactured by one commensurate placement.

This sharpens the research order. After FTD-0768 resolves, the `L=4` pressure
ratios are evaluated first. If they disagree, the current site-aligned block
is not a static body and no Hessian is licensed. If they agree, the constrained
Hessian and the translation/row-relaxed controls decide whether the existing
field creates a solid, a pinned configuration, or a liquid-like cohesive
medium. A phase/connection extension is priced only after that result.

The next intuitive questions are:

1. Do the six `L=4` field-to-capacity ratios agree before any fitting?
2. If pressure exists, does eliminating it leave a positive or null shear
   eigenvalue?
3. Does minimizing over common fractional translation destroy the apparent
   stiffness?
4. Do independent row shifts remain exact zero modes after the field is
   relaxed?
5. Does the field couple rows together through curvature, or only pin each row
   separately to the substrate?
6. Is the first positive restoring coefficient present at `omega=0`, or does
   it scale as `omega^2` and therefore describe inertia?
7. Can a prepared action--angle excitation transmit stress without being
   permanently occupied in the rest state?
8. Can a local phase be reconstructed from one complete state on every
   constituent, or only from trajectory history?
9. Does the FTD-0494 one-form act on any native representation, or is its
   holonomy currently observable only as path-dependent work?
10. If a material loop is required, what state-only rule identifies the same
    loop after bond exchange or fracture?
11. Would a connection coupling introduce one universal coefficient, or a
    separate stiffness for every bond/orientation sector?
12. Can two separated bodies choose independent internal phases while the
    environmental field remains composable?

## 25. Exact `L=4` constraint geometry: pressure cannot make the solid

The first finite block has `N=64`, hence 192 constituent displacement
coordinates. Its eight saturated capacity sites are

\[
\mathcal A=\{1,2\}^3.
\]

Let `A` be the `8 x 192` Jacobian of their occupancies with respect to the
constituent coordinates. Exact rational elimination gives

\[
\operatorname{rank}A=8,
\qquad \dim\ker A=184.
\]

This is not a numerical conditioning claim. The Gram matrix `A A^T` depends
only on the Hamming distance `d` between two active sites:

\[
G_d=\left\{
\frac{1083}{2048},\frac{57}{512},\frac9{512},0
\right\}_{d=0,1,2,3}.
\]

Its exact Walsh eigenvalues, listed by character weight and multiplicity, are

\[
\left(\frac{1875}{2048},1\right),\quad
\left(\frac{1275}{2048},3\right),\quad
\left(\frac{819}{2048},3\right),\quad
\left(\frac{507}{2048},1\right).
\]

All are positive, proving full row rank independently of a chosen elimination
order.

Now restrict attention to axial row slides. For each axis, independently move
one of the 16 rows parallel to that axis. These give 48 exact displacement
directions. The certificate proves

\[
A Z_{\rm row}=0.
\]

Thus every row slide is first-order admissible at all eight active capacity
sites. The harmonic central-force binding matrix has rank 144 and nullity 48:
its null space is exactly this axial row-slide space. Binding and the linear
capacity constraints therefore leave all 48 directions unresolved.

Pressure enters the constrained Hessian through the curvature of the active
constraints. For the cubic equilibrium reduction, every active multiplier is
the same `lambda`. Let

\[
R=Z_{\rm row}^{T}
\nabla^2\!\left(\sum_{v\in\mathcal A}n(v)\right)
Z_{\rm row}.
\]

Exact rational elimination gives

\[
R=R^T,\qquad
\operatorname{rank}R=21,\qquad
\operatorname{tr}R=0,
\qquad \max_{ij}|R_{ij}|=\frac7{32}.
\]

Because `R` is symmetric, nonzero, and has zero trace, it has both positive
and negative eigenvalues. Multiplication by any nonzero common pressure merely
swaps their signs if the constraint convention is reversed; it cannot make
the row-slide sector positive semidefinite. Its nullity is exactly 27. The
three common translations lie in that kernel, leaving 24 further exact null
coordinates at the pressure-plus-binding level. Those 24 must still be
classified into acceptable collective motions and genuine internal floppy
modes; they may not all be called shear.

The kernel is not merely counted; it has a closed form. Let

\[
D=\frac12(-1,-1,1,1),
\qquad D^\perp=\{u\in\mathbb R^4:D\cdot u=0\}.
\]

For motion along a fixed axis, the row-slide amplitudes form a `4 x 4` array
over the two transverse coordinates. The pressure kernel is exactly

\[
\ker R
=\bigoplus_{i=x,y,z}\left(D^\perp\otimes D^\perp\right)_i.
\]

Each tensor square has dimension `3 x 3=9`, giving 27. The constant transverse
array supplies one common translation in each axial sector. The remaining
eight patterns per axis, 24 total, are explicit nontranslation internal slips:
pressure is blind whenever the slide has zero `D`-weighted inner/outer moment
along both transverse coordinates. A solidifying field need not guess what to
gap; this is the exact target subspace.

Pressure blindness does not imply field blindness. Let `S_sigma` map row-slide
amplitudes to the first variation of the signed deposited source on the full
compact support `{-1,0,...,4}^3`, and let `S_0` be the corresponding unsigned
occupancy map. Exact rational elimination gives

\[
\operatorname{rank}\!\left(S_\sigma|_{\ker R}\right)=27,
\qquad
\operatorname{rank}\!\left(S_0|_{\ker R}\right)=27.
\]

Each of the three axial nine-dimensional sectors separately has rank 9 under
both maps. Both first variations conserve their total weight exactly, so every
signed variation lies in the zero-mean source subspace. Consequently, for
every nonzero pressure-null displacement `u`,

\[
\rho'[u]\ne0,
\qquad
\langle\rho'[u],G\rho'[u]\rangle>0.
\]

This proves that the manifestly positive part of the Gauss curvature detects
every pressure-null motion. It does **not** prove that the complete field
curvature stabilizes any of them: the background-source term below can cancel
or overwhelm it. The unsigned result also localizes the distinction. All 27
motions are tangent to the saturated interior constraints, yet every nonzero
combination changes the full finite occupancy profile at first order. They are
therefore boundary/surface-visible slips, not invisible gauge coordinates.
At `L=4` the surface is a large fraction of the body, so this result proves no
bulk shear modulus. Larger-volume scaling must distinguish a surface-restored
droplet from a volume-restored solid.

This is a stronger result than “capacity has no shear.” It says:

```text
binding alone  = 48 harmonic row-slide zero modes,
linear capacity = all 48 remain admissible,
common pressure = 21 mixed curvatures but necessarily indefinite,
                  plus 27 exact null directions,
field response  = must stabilize every downhill internal mode and determine
                  which remaining null modes are collective versus floppy.
```

The relaxed Gauss field is not automatically stabilizing. With
`G=(-Delta)^+` and

\[
H_G(\gamma)=\frac\beta2\langle\rho(\gamma),G\rho(\gamma)\rangle,
\]

its exact second variation is

\[
H_G''(0)=\beta\left[
\langle\rho',G\rho'\rangle
+\langle\rho'',G\rho\rangle
\right].
\]

Only the first term is manifestly nonnegative. The second, background-source
curvature term can have either sign. The complete pressure-qualified Hessian,
not field energy positivity alone, decides stability.

For the integer-aligned control, failure of the six pressure ratios would make
even this Hessian unlicensed. Passing them would establish a stationary finite
control only. Section 26 proves that the corresponding volume-filling bulk has
an exact negative common-translation curvature, so this is no longer the
mainline scalable matter candidate. Its originally planned calculation remains
well defined as a saddle diagnostic:

1. construct the full binding plus minimum-Gauss plus common-pressure Hessian;
2. project it into `ker A`;
3. classify common translations and candidate rigid rotations separately;
4. minimize the shear response over internal row modes and common fractional
   translation;
5. require no negative internal eigenvalue before calling the block stable;
6. require a positive volume-scaled relaxed shear coefficient before calling
   it solid-like.

The result could still be a stable liquid-like finite body: stability needs no
negative internal mode, whereas solidity additionally needs a positive static
shear modulus. Those are now separate gates.

## 26. Translation-phase theorem: neutral bulk, field-bearing surface

The `L=4` injectivity result initially looked like a possible bulk coupling of
all pressure-null rows. The general even-`L` calculation proves a narrower and
more useful statement: at the integer phase, the first-order coupling is
carried entirely by row endpoints.

For an even alternating line of length `L`, define

\[
f_L(v;\delta)=\sum_{j=0}^{L-1}(-1)^j b_2(v-j-\delta),
\qquad
n_L(v;\delta)=\sum_{j=0}^{L-1}b_2(v-j-\delta).
\]

Direct differentiation at `delta=0` gives

\[
\partial_\delta n_L(v;0)=
\begin{cases}
-\tfrac12,&v=-1,0,\\
+\tfrac12,&v=L-1,L,\\
0,&\text{otherwise},
\end{cases}
\tag{86}
\]

and

\[
\partial_\delta f_L(v;0)=
\begin{cases}
-\tfrac12,&v=-1,L,\\
+\tfrac12,&v=0,L-1,\\
0,&\text{otherwise}.
\end{cases}
\tag{87}
\]

Both variations sum to zero. For an `x`-directed row-slide pattern `a_yz`,
separability therefore gives

\[
\rho'_x(v)=
\partial_\delta f_L(v_x;0)
\sum_{y,z}(-1)^{y+z}a_{yz}
b_2(v_y-y)b_2(v_z-z).
\tag{88}
\]

The unsigned occupancy variation has the same factorization with equation
(86) and no transverse parity factor. Hence every complete axial row slide is
first-order invisible in the interior and visible only in a two-site-thick
end-cap layer. The earlier rank-27 theorem is genuine injectivity, but it is
**surface injectivity**, not evidence of a volume shear modulus.

The bulk translation phase can be solved exactly. On the infinite alternating
line, for `|delta|<=1/2`,

\[
\sum_{j\in\mathbb Z}(-1)^j b_2(v-j-\delta)
=(-1)^v A(\delta),
\qquad
A(\delta)=\frac12-2\delta^2.
\tag{89}
\]

For a three-dimensional common shift `delta=(delta_x,delta_y,delta_z)`,

\[
\rho_\delta(v)=(-1)^{v_x+v_y+v_z}
A(\delta_x)A(\delta_y)A(\delta_z).
\tag{90}
\]

The checkerboard is the Laplacian eigenmode with eigenvalue 12, so its relaxed
field energy per site is

\[
e_G(\delta)=\frac{\beta}{24}
\left[A(\delta_x)A(\delta_y)A(\delta_z)\right]^2.
\tag{91}
\]

Holding `delta_y=delta_z=0` gives

\[
e_G(\delta_x,0,0)=\beta\left(
\frac1{1536}-\frac{\delta_x^2}{192}
+\frac{\delta_x^4}{96}
\right),
\qquad
\frac{\partial^2e_G}{\partial\delta_x^2}(0)=-\frac\beta{96}.
\tag{92}
\]

Binding depends only on relative positions and is unchanged by common
translation. Infinite-fill capacity remains exactly one by partition of
unity. No common pressure can act along this path. Therefore the integer-
aligned periodic medium is a stationary **saddle**, not a stable material
bulk. The negative mode is exact and extensive; an `L=4` force balance at that
phase cannot establish a scalable equilibrium.

At a half-cell phase, `A(1/2)=0`. If any one coordinate is half phased, the
infinite bulk signed source and relaxed bulk field energy vanish. For a finite
even line,

\[
f_L(v;1/2)=\frac12\delta_{v,0}-\frac12\delta_{v,L}.
\tag{93}
\]

Thus a finite block half phased along `x` carries signed source only on its two
`x` boundary planes. Routing Gauss flux along a transverse even direction
constructs the exact bound

\[
H_G^{\min}\le
\frac{\beta}{2048}
\min\left\{
(10L_y-1)(4L_z+5),
(10L_z-1)(4L_y+5)
\right\}.
\tag{94}
\]

This is area scaling, independent of `L_x`. The half-phase candidate therefore
has the desired energetic hierarchy:

```text
binding          = negative volume term + missing-bond surface term,
capacity         = saturated bulk + slack one-coat interface,
signed Gauss data = canceled bulk + field-bearing boundary,
field-energy bound = at most area scaling.
```

This is the first exact algebra in this branch that resembles ordinary neutral
matter: opposite microscopic polarities cancel in the bulk while boundaries,
terminations, and defects retain field response. It proves neither a stable
finite shape nor physical electric charge. The finite half-phase boundary
forces, pressure set, constrained Hessian, and release dynamics must be
derived afresh.

It also changes the mobility question. The zero-bulk-field set is the union of
translation-phase planes on which at least one coordinate is half phased.
These planes intersect, producing connected zero-energy corridors in three
translation-phase dimensions. A body might therefore move by advancing a
phase sheet or constituent-turnover front rather than displacing every
constituent rigidly through the volume-energy maximum. That is a derived
geometric possibility, not an implemented motion law.

### 26.1 The neutral manifold is also a representation seam

The half-phase result does not erase the prior subcell obstruction. It lands
exactly on it.

FTD-0500 proves that no single-valued integer anchor section can be both
integer-translation covariant and inversion covariant at `x=1/2`: the two
requirements imply `2a(1/2)=1`. Its face shape and Whitney current are
independent of which equivalent chart is chosen, so equations (89)--(94) are
well defined on the coupling quotient. Primitive manifested-anchor sourcing,
collision ordering, and state-only reversal are not chart independent.

FTD-0624 encountered the same distinction dynamically. Under independent
nearest-site projection, exact-half endpoints placed opposite-polarity
constituents on shared ternary anchors and failed the reversible state gate.
FTD-0626 then reused the selected multiplicity-two shared-anchor fibre: the
same class executed and inverted without a physical collision, proving that
the defect was representation loss rather than annihilation. Its fixed
internal-rest gate nevertheless closed negative over the registered horizon;
the body breathed reversibly instead of remaining static.

The resulting status is therefore

```text
shape/current quotient at half phase = exact and chart independent,
bulk source cancellation             = exact,
unique one-value-per-anchor state     = obstructed at the tie,
shared-anchor constituent fibre       = constructive research representation,
fixed half-phase material rest        = not established,
production ternary ontology           = unchanged.
```

The connected zero-field corridors are consequently corridors in the smooth
coupling quotient, not yet admissible paths in the frozen unique-anchor state
space. A physical phase front must choose and close exactly one of three
priced mechanisms:

1. carry the already selected shared-anchor constituent fibre through the
   transaction;
2. stay off the exact tie and pay the explicitly accounted field-energy
   barrier; or
3. perform atomic manifestation turnover so the old anchor is released as the
   new one is created without an accepted duplicate state.

These mechanisms are not equivalent. Each must separately close continuity,
energy, recoil, cubic covariance, and state-only inversion. The half-phase
theorem identifies the correct energetic geometry; it does not supply the
ontic transaction that traverses it.

### Intuitive recursion forced by the theorem

1. Is macroscopic neutrality the half-phase cancellation of microscopic
   polarity rather than the absence of polarity?
2. Is observable charge a boundary termination or phase-slip defect where
   that cancellation fails?
3. Can a body translate by propagating a half-phase sheet through itself, so
   only an area-sized transition layer is ever off the neutral manifold?
4. Is inertia the energy and field momentum carried by that moving transition
   layer rather than a cost of rigidly moving the whole bulk at once?
5. Does fracture cost energy because it creates two new field-bearing
   cancellation boundaries in addition to missing bonds?
6. Are solids distinguished by pinned or interlocked phase sheets, liquids by
   mobile sheets, and flame/cloud matter by continual creation and removal of
   the sheets at an environmental boundary?
7. Can a closed phase-slip surface end, or does its inability to end provide
   an emergent conservation law for defect charge?
8. Does the BCC temporal clock coordinate the order in which orthogonal
   half-phase corridors are traversed without itself supplying static shear?
9. Does cubic symmetry require equal populations of the three half-phase
   orientations in an isotropic macroscopic body?
10. Can the same local common-action transaction that moves one constituent
    advance such a phase sheet while closing current, recoil, energy, and
    state-only inversion?

The next static mainline is therefore not the old integer-aligned six-ratio
test. FTD-0768 has resolved only administratively as execution-invalid; it
selects no successor. The active constraints and field forces must be
rederived at the half-phase finite candidate, with the integer-phase result
retained as a certified saddle control. The eventual dynamical mainline is a
phase-front/turnover test against rigid translation, not an assumed rigid
crystal hop.
