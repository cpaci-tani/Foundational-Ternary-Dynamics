# Moving causal-tube energy identity v1

**Status:** `[THEOREM — EXACT DISCRETE TRANSPORT ALGEBRA] + [DERIVED TEST CRITERION; NO MATTER OR PARTICLE PROMOTION]`  
**Ledger scope:** FTD-0768 infrastructure correction; no new identifier  
**Evidence boundary:** algebraic identity plus the qualified FTD-0768 CPU/CUDA observer  
**Production status:** unchanged

## 1. The defect this identity prevents

A fixed region and a region that follows a candidate object answer different
questions. Summing valid one-step balances over a region whose mask changes
does not produce the energy change of the moving region unless the energy
swept by the changing mask is included.

This is not a continuum assumption. It is finite-sum algebra on the lattice.

## 2. Exact derivation

Let `h_t(a)` be any declared site/face/edge energy contribution at tick `t`,
and let `chi_t(a)` be the zero/one indicator of a finite observer region. Define

\[
E_t[\chi]=\sum_a \chi(a)h_t(a).
\]

Add and subtract the before-state energy evaluated with the new mask:

\[
\begin{aligned}
E_{t+1}[\chi_{t+1}]-E_t[\chi_t]
={}&\underbrace{E_{t+1}[\chi_{t+1}]
                 -E_t[\chi_{t+1}]}_{Q_t[\chi_{t+1}]}\\
 &+\underbrace{E_t[\chi_{t+1}]
                 -E_t[\chi_t]}_{S_t}.
\end{aligned}
\tag{1}
\]

`Q_t` is the physical change inside one fixed mask during the tick. For the
matched FTD observer it decomposes as

\[
Q_t[\chi_{t+1}]=\Phi_t[\chi_{t+1}]+W_t[\chi_{t+1}],
\tag{2}
\]

where `Phi` is oriented source-free field transport into the region and `W` is
the local current-to-field source exchange. The second term in (1),

\[
S_t=\sum_a(\chi_{t+1}(a)-\chi_t(a))h_t(a),
\tag{3}
\]

is the exact discrete Reynolds or **mask-sweep** term. It is caused by changing
which coefficients the observer counts. It is not a force, source, Poynting
flux, wake, radiation field, or new primitive.

Combining (1)--(3) gives the moving-control-volume ledger

\[
E_{t+1}[\chi_{t+1}]-E_t[\chi_t]
-\Phi_t-W_t-S_t=0.
\tag{4}
\]

For the complementary mask `bar chi=1-chi`, exact partition gives

\[
S_t[\chi]+S_t[\bar\chi]=0.
\tag{5}
\]

Equation (5) is an independent orientation check: energy reclassified into the
region is reclassified out of its complement. Summing (4) from `0` to `N-1`
telescopes to the endpoint identity

\[
E_N[\chi_N]-E_0[\chi_0]
=\sum_{t=0}^{N-1}(\Phi_t+W_t+S_t).
\tag{6}
\]

Equations (1), (5), and (6) are the exact algebraic gates. No limiting process,
smooth boundary, or Lorentz symmetry is used.

## 3. Three observer questions that must remain separate

### Fixed laboratory memory

Set `chi_t=chi_0`. Then `S_t=0`. A nonzero post-passage laboratory response
must be accounted for entirely by physical boundary transport and source work.
This is the appropriate first discriminator for a deposited wake or lingering
environmental response.

### Co-moving object retention

Let `chi_t` follow the relational center. Then `S_t` is generally nonzero.
Equation (6), not a sum of fixed-mask step changes, determines whether energy
remains associated with the moving tube. The result is still observer-relative:
the mask is a measurement chart, not a material membrane.

### Outgoing radiation

Use fixed or explicitly expanding shells and measure outward oriented transport
through separated surfaces. An expanding shell also has a sweep term. A pulse
does not become radiation merely because a recentered near region stops
counting it.

## 4. Ontological consequence

A matter boundary cannot yet be identified with an ontic membrane. The exact
ledger separates:

```text
physical transport  = field evolution across one fixed boundary,
source exchange     = matter/current work into the field,
mask sweep          = observer reclassification caused by moving the boundary.
```

Only the first two are dynamical exchanges. The third records how the chosen
object chart moves through an already existing energy distribution.

This sharpens the selected matter story. A relational kernel may move and
continually reconstruct its constraint field while a co-moving observer
reclassifies coefficients at its front and rear. That alone neither proves a
carried aura nor leaves a wake. A wake requires a fixed-laboratory residual
after the kernel has cleared the region, with (2) closing independently.

## 5. Reversal does not classify radiation

Complete state-only reversal is an essential invertibility and implementation
gate, but it is not a wake/radiation discriminator. Reversible radiation also
returns under exact reversal. Reversal establishes that the forward record was
not information-destroying at the tested scope; spatial propagation, surface
transport, persistence, and absorption tests determine the morphology.

## 6. Consequences for mass and inertia

An energy-versus-boost curve cannot be called inertial mass if its co-moving
energy changes are contaminated by unrecorded mask sweep. Before fitting
curvature, the program must show that

1. the same relational family is followed at each boost;
2. endpoint tube energy satisfies (6);
3. fixed-lab environmental energy and outgoing transport are separated;
4. the inferred curvature is stable under admissible tube radii and lattice
   directions; and
5. it agrees with impulse response and an on-shell pole residue.

The identity therefore does not derive mass, but it removes a specific false
route to mass.

## 7. Intuitive questions made exact

1. When the object chart advances one cell, how much apparent carried energy is
   physical transport and how much is mask sweep?
2. Does the front of the tube acquire energy by source work, by incoming field
   transport, or merely by counting coefficients that were already there?
3. Does the rear lose energy through an outward flux, or only because the chart
   stopped counting it?
4. Is there a tube radius for which endpoint energy stabilizes after all three
   terms in (6) are included?
5. Does the fixed laboratory region retain energy after the moving tube has
   cleared it, and can every retained unit be traced to `Phi+W`?
6. Does an expanding outgoing shell report a propagating pulse after its own
   sweep correction?
7. Under collision, can two moving masks be maintained without double-counting
   their overlap energy?
8. At decay, does loss of relational membership precede, coincide with, or
   follow the outward physical energy flux?
9. Does the inferred boost curvature remain invariant when the tube convention
   changes but equation (6) is held exact?
10. Can a distant incoming packet change the fixed-lab ledger before causal
    contact, or would that expose an observer or locality defect?

## 8. FTD-0768 implementation consequence

The first two FTD-0768 aborts repaired complementary boundary quadrature and
checkpoint/certificate provenance. The third pre-result run exposed the
missing term (3): its fixed laboratory ledger was sound, but its recentered
radius-eight cumulative ledger summed different masks without sweep.

The repaired observer evaluates both old and new masks on the same resident
before-state, records the inside and complementary sweep, checks (1) per tick,
checks endpoint chaining, and reconstructs (5)--(6) on every checkpoint
interval. This is observer infrastructure only. No FTD-0768 physics outcome
follows from that qualification alone. The later fourth artifact passes the
transport identities but fails continuous reverse recovery
(`3.8786822642578e-9 > 1e-10`), so its final outcome is
`LONG_TRANSPORT_EXECUTION_INVALID` and still supplies no FTD-0768 physics
classification.

## 9. From an observer tube to a candidate material membrane

For a boundary that moves with normal speed encoded by `chi_t`, the energy
transport relative to that boundary is the combination

\[
\Phi_t^{\rm rel}=\Phi_t+S_t.
\tag{7}
\]

This does not make `S_t` a new physical current. Equation (7) says that
crossing a moving boundary is measured by physical field transport plus the
change in what the moving boundary encloses. A candidate **material membrane**
requires more than choosing such a boundary. Its mask must be selected from the
instantaneous state, not from a stored trajectory or an arbitrary display
radius.

At minimum, a membrane claim must establish:

1. **state selection:** `chi_t` is a deterministic local/covariant functional
   of the relational state;
2. **interface persistence:** small normal perturbations return to the same
   equivalence class rather than selecting a different object by convention;
3. **relative balance:** endpoint energy closes with `Phi_rel+W` under changes
   of admissible chart resolution;
4. **selective permeability:** environmental radiation may cross while the
   relational identity and constitutive constraint remain inside;
5. **stress balance:** an independently derived momentum/stress ledger explains
   the boundary acceleration or rest condition; and
6. **factorization:** two distant membranes can be counted as two objects
   without overlap-dependent double counting.

These gates distinguish three possibilities:

```text
self-bound matter       = persistent interface with no required incoming support,
environmentally confined = persistent interface maintained by incoming flux/stress,
tracked transient        = persistence disappears when the observer convention changes.
```

The flame analogy belongs to the second or mixed case: identity can persist
while constituents and energy cross the interface. The solid analogy belongs
to a low-permeability, long-residence regime. Neither analogy follows from a
pretty streamline image or from a co-moving cube.

The immediate intuitive questions are therefore:

1. Does the zero of the core-membership margin define the same boundary at
   different observer radii?
2. If the incoming field is removed outside causal contact, does the interface
   persist, shrink, or dissolve?
3. Is the apparent surface pressure an actual momentum flux or only an energy
   mask-sweep effect?
4. Which quantities may cross while identity remains—field energy,
   constituents, polarity, or all three?
5. Does a stable rest object require continuous inward environmental support?
6. Does acceleration tilt or compress the selected interface in a reproducible
   direction-dependent way?
7. Is decay initiated by loss of internal binding margin or by a net outward
   relative flux through the interface?
8. Can two interfaces merge and later separate while preserving an exact
   accounting of which relational identities survived?

## 10. A state-space boundary is not a spatial membrane

The frozen FTD-0755 relational-core predicate makes this distinction exact.
For the selected reciprocal pair it uses

\[
g(X)=r_{\rm cut}^2-\lVert x_1-x_0\rVert^2,
\qquad
e(X)=-\big(K(p_1,p_2)+V(x_1-x_0)\big),
\tag{8}
\]

and declares membership only when `g(X)>0` and `e(X)>0`. Therefore

\[
\mathcal M=\{X:g(X)>0\ \text{and}\ e(X)>0\}
\tag{9}
\]

is a region of the selected **state space**. Its boundary contains the graph-
separation surface `g=0` and the zero-binding-energy surface `e=0`. Neither is
a two-dimensional surface embedded in the physical lattice.

The failure of the spatial interpretation follows by counterexample. Hold
`x_1,x_2` fixed and increase the equal-and-opposite constituent momenta. Then
`g` and every position-based envelope remain unchanged while `K` increases,
so `e` can cross zero and the pair can leave `M` without any spatial boundary
moving. Conversely, translate both constituents by the same integer vector.
Both margins remain unchanged while every spatial envelope translates. Thus
the current membership margins cannot define a unique material membrane.

This is not a defect in the predicate. It states what the predicate actually
certifies: a reciprocal pair belongs to a bound relational family at one
instant. The predicate supplies a **basin boundary for identity**, not a local
skin separating matter from nonmatter.

For a many-constituent candidate, a possible next construction is a local
interaction graph whose vertices are constituents and whose edges satisfy
registered relational and energy margins. Loss of coordination at the graph's
edge could then operationally distinguish bulk-like and surface-like
constituents. But converting that graph boundary into a spatial surface still
requires a frozen local support rule or state-selected density level set. An
alpha shape, union of balls, streamline envelope, or arbitrary field-energy
threshold would be a selected observer convention unless the common action
fixes its scale and balance law.

The resulting ontological hierarchy is therefore

```text
elementary identity boundary = boundary of a relational stability basin,
candidate solid bulk         = persistent, multiply connected bond network,
candidate material surface   = coordination/support boundary of that network,
field/environment boundary   = separate causal transport ledger.
```

This makes a concrete version of the flame/solid analogy possible. A flame-like
object may preserve graph-level organization while constituents and energy
cross its spatial support. A solid-like object would be a low-rearrangement,
high-coordination regime in which breaking or exchanging relations has a
measurable energy barrier. Solidity would then be a dynamical residence-time
and response property, not permanent occupation of a voxel and not a primitive
membrane.

The next intuitive questions are:

1. Does an `N`-constituent extension yield a connected interaction graph whose
   membership is stable under the same complete-state evolution?
2. Does the distribution of coordination numbers separate interior, surface,
   and detached constituents without a fitted spatial radius?
3. Is there a nonzero action/energy barrier for changing that graph's
   connectivity, and does it predict resistance to deformation?
4. Can constituents cross the graph-defined surface while the macroscopic
   relational pattern persists, as they do in a flame?
5. Does the state-selected Gauss representative give the same spatial support
   boundary as the constituent graph, or do those boundaries describe
   different things?
6. Under acceleration, does the graph deform before the environmental field
   responds, or are both changes one common transaction?
7. At decay, which occurs first: an energy-margin crossing, a graph
   disconnection, or outward relative energy transport?
8. Can two distant graphs factorize exactly and then merge without assigning
   shared environmental field energy to both objects?

## 11. Conditional many-body extrapolation from the compact pair law

**Status of this section:** `[CONDITIONAL DERIVATION FROM AN UNIMPLEMENTED
PAIRWISE EXTENSION]`. The current common-action branch admits exactly two
opposite-polarity constituents. Nothing below asserts that an `N`-body sum is
the native law.

The selected pair potential, with `q=r^2` and well depth `epsilon`, is

\[
V(q)=-16\epsilon(q-3/2)^2(q-3/4),\qquad q<3/2,
\tag{10}
\]

and zero beyond the cutoff. Direct differentiation gives

\[
V'(q)=-48\epsilon(q-3/2)(q-1).
\tag{11}
\]

Therefore the stable minimum is `q=1`, where `V=-epsilon`; the cutoff is
`q=3/2`, where both `V` and `V'` vanish. A quiet bond costs exactly `epsilon`
to move from the minimum to the separated zero-energy branch. For a small
longitudinal stretch `r=1+u`,

\[
V((1+u)^2)=-\epsilon+48\epsilon u^2+O(u^3)
          =-\epsilon+\tfrac12(96\epsilon)u^2+O(u^3).
\tag{12}
\]

Thus a hypothetical pairwise network would have single-bond harmonic
stiffness `96 epsilon` and a quiet missing-bond cost `epsilon`. If `E` is its
bond set and every bond is at its minimum, then

\[
U=-\epsilon|E|=-\frac{\epsilon}{2}\sum_i z_i,
\tag{13}
\]

where `z_i` is the coordination number. Relative to a chosen bulk
coordination `z_bulk`, the quiet surface excess is conditionally

\[
\Delta U_{\rm miss}=\frac{\epsilon}{2}
  \sum_i (z_{\rm bulk}-z_i).
\tag{14}
\]

Equation (14) is the precise form of “a surface is where the bond network runs
out.” It is not yet an FTD surface tension: strained bonds, field energy,
angular response, graph rearrangement, entropy, and environmental pressure are
absent.

There is also an immediate topology constraint. If every edge retains the
current rule that it joins opposite polarities, assigning polarity gives a
two-colouring of the interaction graph. The graph must therefore be
**bipartite** and cannot contain an odd cycle. Nearest-neighbour simple-cubic
and body-centred-cubic graphs are bipartite, with quiet coordination 6 and 8.
The nearest-neighbour face-centred-cubic graph contains triangles and is not
bipartite, with coordination 12. Consequently a literal FCC nearest-neighbour
material network cannot be built solely by copying the current reciprocal
pair interaction. It requires at least one of:

1. a second same-polarity or non-polar bond channel;
2. a genuinely many-body interaction not reducible to reciprocal edges;
3. a bipartite parent whose FCC appearance is only a projection or sublattice
   readout; or
4. rejection of FCC as the constituent interaction graph.

This is a useful discriminator for the proposed SC/FCC physical domain and BCC
temporal analogy. The current reciprocal law is naturally compatible with SC
or BCC connectivity, but does not derive either geometry. FCC is positively
obstructed at the nearest-neighbour graph level unless the ontology contains
more relational structure than the present pair.

The resulting next tests are structural before they are numerical:

1. Does a common-action `N`-body generalization follow uniquely from summing
   pair terms, or do simultaneous face currents generate irreducible
   many-body terms?
2. Does exact inversion survive a graph-edge creation or deletion at the
   compact cutoff?
3. Do SC and BCC clusters retain negative total energy without an imposed
   external container?
4. Does their measured deformation energy approach the bond stiffness in
   (12), including the field contribution rather than only the selected well?
5. Is an FCC-like spatial density obtainable as a projection of a bipartite
   dynamical graph without double-counting constituents?
6. Does an environmental pressure stabilize a network that is not self-bound,
   producing the flame/cloud regime rather than the solid regime?

## 12. Exact isolated-pair stability and the decay-energy gate

Within the valid selected pair sector, the two FTD-0755 membership margins are
not logically independent. The constituent kinetic energy is rest-subtracted,

\[
K=\sum_{a=1}^2\left(
\sqrt{E_{\rm rest}^2+C_{\rm SPEED}^2\lVert p_a\rVert^2}
-E_{\rm rest}\right)\ge 0.
\tag{15}
\]

Equation (10) is nonnegative for `q<=3/4`, negative only for
`3/4<q<3/2`, and zero for `q>=3/2`. Hence

\[
K+V(q)<0 \quad\Longrightarrow\quad 3/4<q<3/2.
\tag{16}
\]

The positive energy margin `-(K+V)>0` therefore already implies the positive
graph margin `3/2-q>0`. The graph margin remains a useful distance-to-cutoff
diagnostic, but it adds no independent membership restriction for an exact
valid-sector state with negative pair energy.

For the isolated pair transaction, exact conservation of `K+V` then proves a
strong finite-model result: a state beginning with negative pair energy cannot
reach the graph cutoff, because at and beyond the cutoff `K+V=K>=0`. It also
cannot enter the inner `q<=3/4` region, where `K+V>=0`. Its separation remains
inside the compact negative-potential annulus for every valid exactly
energy-conserving continuation.

In the coupled matter--field transaction, pair energy need not remain fixed;
only the complete declared matter--binding--field energy is conserved. Any
dissociation must therefore receive at least the missing pair energy through
accounted current/field work. At tick resolution, loss of the negative-energy
margin must precede or coincide with graph disconnection. This supplies a
precise operational decay sequence:

```text
bound pair                 : K+V < 0,
field/environmental loading: accounted energy enters the pair sector,
dissociation threshold     : K+V >= 0,
geometric separation       : r^2 >= 3/2 may then occur.
```

This is not yet radioactive or irreversible decay. Exact complete-state
inversion can reassemble the pair, and no probability law, lifetime, emitted
spectrum, or autonomous reaction channel has been derived. What is established
is narrower: the selected isolated pair cannot simply “wear out”; its breakup
must be an energy-balanced interaction with the rest of the complete state.

The immediate questions become:

1. Which field observable supplies the positive pair-energy change before a
   registered disconnection event?
2. Does the field lose exactly the same energy after moving-mask and boundary
   transport terms are separated?
3. Is there a minimum incoming packet energy below which dissociation is
   impossible at every phase and direction?
4. Does excess energy leave as constituent kinetic energy, detached field
   transport, or both?
5. Can a many-body network lose one edge while retaining negative total energy
   through redistribution among its remaining bonds?
6. What additional coarse-graining or open-environment limit would turn the
   reversible threshold crossing into an approximately exponential lifetime?
